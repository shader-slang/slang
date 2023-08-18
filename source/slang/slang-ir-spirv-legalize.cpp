// slang-ir-spirv-legalize.cpp
#include "slang-ir-spirv-legalize.h"

#include "slang-ir-glsl-legalize.h"

#include "slang-ir-clone.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-emit-base.h"
#include "slang-glsl-extension-tracker.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"

namespace Slang
{

//
// Legalization of IR for direct SPIRV emit.
//

struct SPIRVLegalizationContext : public SourceEmitterBase
{
    SPIRVEmitSharedContext* m_sharedContext;

    IRModule* m_module;
    // We will use a single work list of instructions that need
    // to be considered for specialization or simplification,
    // whether generic, existential, etc.
    //
    OrderedHashSet<IRInst*> workList;

    void addToWorkList(IRInst* inst)
    {
        if (workList.add(inst))
        {
            addUsersToWorkList(inst);
        }
    }

    void addUsersToWorkList(IRInst* inst)
    {
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();

            addToWorkList(user);
        }
    }

    SPIRVLegalizationContext(SPIRVEmitSharedContext* sharedContext, IRModule* module)
        : m_sharedContext(sharedContext), m_module(module)
    {
    }

    // Wraps the element type of a constant buffer or parameter block in a struct if it is not already a struct,
    // returns the newly created struct type.
    IRType* wrapConstantBufferElement(IRInst* cbParamInst)
    {
        auto innerType = as<IRParameterGroupType>(cbParamInst->getDataType())->getElementType();
        IRBuilder builder(cbParamInst);
        builder.setInsertBefore(cbParamInst);
        auto structType = builder.createStructType();
        StringBuilder sb;
        sb << "cbuffer_";
        getTypeNameHint(sb, innerType);
        sb << "_t";
        builder.addNameHintDecoration(structType, sb.produceString().getUnownedSlice());
        auto key = builder.createStructKey();
        builder.createStructField(structType, key, innerType);
        builder.setInsertBefore(cbParamInst);
        auto newCbType = builder.getType(cbParamInst->getDataType()->getOp(), structType);
        cbParamInst->setFullType(newCbType);
        auto rules = getTypeLayoutRuleForBuffer(m_sharedContext->m_targetRequest, cbParamInst->getDataType());
        IRSizeAndAlignment sizeAlignment;
        getSizeAndAlignment(rules, structType, &sizeAlignment);
        traverseUses(cbParamInst, [&](IRUse* use)
        {
            builder.setInsertBefore(use->getUser());
            auto addr = builder.emitFieldAddress(builder.getPtrType(kIROp_PtrType, innerType, SpvStorageClassUniform), cbParamInst, key);
            use->set(addr);
        });
        return structType;
    }

    void processGlobalParam(IRGlobalParam* inst)
    {
        // If the global param is not a pointer type, make it so and insert explicit load insts.
        auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
        if (!ptrType)
        {
            if (as<IRResourceTypeBase>(inst))
                return;

            SpvStorageClass storageClass = SpvStorageClassPrivate;
            // Figure out storage class based on var layout.
            if (auto layout = getVarLayout(inst))
            {
                if (auto systemValueAttr = layout->findAttr<IRSystemValueSemanticAttr>())
                {
                    String semanticName = systemValueAttr->getName();
                    semanticName = semanticName.toLower();
                    if (semanticName == "sv_dispatchthreadid")
                    {
                        storageClass = SpvStorageClassInput;
                    }
                    else if (semanticName == "sv_groupindex")
                    {
                        storageClass = SpvStorageClassInput;
                    }
                }
                else if(const auto parameterGroupTypeLayout =
                        as<IRParameterGroupTypeLayout>(layout->getTypeLayout()))
                {
                    storageClass = SpvStorageClassUniform;
                }
            }

            // Strip any HLSL wrappers
            IRBuilder builder(m_sharedContext->m_irModule);
            bool needLoad = true;
            auto innerType = inst->getFullType();
            if (as<IRConstantBufferType>(innerType) || as<IRParameterBlockType>(innerType))
            {
                innerType = as<IRUniformParameterGroupType>(innerType)->getElementType();
                storageClass = SpvStorageClassUniform;
                // Constant buffer is already treated like a pointer type, and
                // we are not adding another layer of indirection when replacing it
                // with a pointer type. Therefore we don't need to insert a load at
                // use sites.
                needLoad = false;
                // If inner element type is not a struct type, we need to wrap it with
                // a struct.
                if (!as<IRStructType>(innerType))
                {
                    innerType = wrapConstantBufferElement(inst);
                }
                builder.addDecoration(innerType, kIROp_SPIRVBlockDecoration);
            }

            // Make a pointer type of storageClass.
            builder.setInsertBefore(inst);
            ptrType = builder.getPtrType(kIROp_PtrType, innerType, storageClass);
            inst->setFullType(ptrType);
            if (needLoad)
            {
                // Insert an explicit load at each use site.
                List<IRUse*> uses;
                for (auto use = inst->firstUse; use; use = use->nextUse)
                {
                    uses.add(use);
                }
                for (auto use : uses)
                {
                    builder.setInsertBefore(use->getUser());
                    auto loadedValue = builder.emitLoad(inst);
                    use->set(loadedValue);
                }
            }
        }
        processGlobalVar(inst);
    }

    void processGlobalVar(IRInst* inst)
    {
        auto oldPtrType = as<IRPtrTypeBase>(inst->getDataType());
        if (!oldPtrType)
            return;

        // If the pointer type is already qualified with address spaces (such as
        // lowered pointer type from a `HLSLStructuredBufferType`), make no
        // further modifications.
        if (oldPtrType->hasAddressSpace())
        {
            addUsersToWorkList(inst);
            return;
        }

        SpvStorageClass storageClass = SpvStorageClassPrivate;
        if (as<IRGroupSharedRate>(inst->getRate()))
        {
            storageClass = SpvStorageClassWorkgroup;
        }
        else if (const auto varLayout = getVarLayout(inst))
        {
            for (auto rr : varLayout->getOffsetAttrs())
            {
                switch (rr->getResourceKind())
                {
                case LayoutResourceKind::Uniform:
                case LayoutResourceKind::ShaderResource:
                case LayoutResourceKind::DescriptorTableSlot:
                    storageClass = SpvStorageClassUniform;
                    break;
                case LayoutResourceKind::VaryingInput:
                    storageClass = SpvStorageClassInput;
                    break;
                case LayoutResourceKind::VaryingOutput:
                    storageClass = SpvStorageClassOutput;
                    break;
                case LayoutResourceKind::UnorderedAccess:
                    storageClass = SpvStorageClassStorageBuffer;
                    break;
                case LayoutResourceKind::PushConstantBuffer:
                    storageClass = SpvStorageClassPushConstant;
                    break;
                default:
                    break;
                }
            }
        }

        IRBuilder builder(m_sharedContext->m_irModule);
        builder.setInsertBefore(inst);
        auto newPtrType =
            builder.getPtrType(oldPtrType->getOp(), oldPtrType->getValueType(), storageClass);
        inst->setFullType(newPtrType);
        addUsersToWorkList(inst);
        return;
    }

    void processCall(IRCall* inst)
    {
        auto funcValue = inst->getOperand(0);
        if (auto targetIntrinsic = Slang::findBestTargetIntrinsicDecoration(
                funcValue, m_sharedContext->m_targetRequest->getTargetCaps()))
        {
            SpvSnippet* snippet = m_sharedContext->getParsedSpvSnippet(targetIntrinsic);
            if (!snippet)
                return;
            if (snippet->resultStorageClass != SpvStorageClassMax)
            {
                auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
                if (!ptrType)
                    return;
                IRBuilder builder(m_sharedContext->m_irModule);
                builder.setInsertBefore(inst);
                auto qualPtrType = builder.getPtrType(
                    ptrType->getOp(), ptrType->getValueType(), snippet->resultStorageClass);
                List<IRInst*> args;
                for (UInt i = 0; i < inst->getArgCount(); i++)
                    args.add(inst->getArg(i));
                auto newCall = builder.emitCallInst(qualPtrType, funcValue, args);
                inst->replaceUsesWith(newCall);
                inst->removeAndDeallocate();
                addUsersToWorkList(newCall);
            }
        }
    }

    Dictionary<IRInst*, IRInst*> m_mapArrayValueToVar;

    // Replace getElement(x, i) with, y = store(x); p = getElementPtr(y, i); load(p),
    // when i is not a constant. SPIR-V has no support for dynamic indexing into values like we do.
    // It may be advantageous however to do this further up the pipeline
    void processGetElement(IRGetElement* inst)
    {
        IRInst* x = nullptr;
        List<IRInst*> indices;
        IRGetElement* c = inst;
        do
        {
            if (as<IRIntLit>(c->getIndex()))
                break;
            x = c->getBase();
            indices.add(c->getIndex());
        } while(c = as<IRGetElement>(c->getBase()), c);

        if (!x)
            return;

        IRBuilder builder(m_sharedContext->m_irModule);
        IRInst* y = nullptr;
        if (!m_mapArrayValueToVar.tryGetValue(x, y))
        {
            setInsertAfterOrdinaryInst(&builder, x);
            y = builder.emitVar(x->getDataType(), SpvStorageClassFunction);
            builder.emitStore(y, x);
            m_mapArrayValueToVar.set(x, y);
        }
        builder.setInsertBefore(inst);
        for(Index i = indices.getCount() - 1; i >= 0; --i)
            y = builder.emitElementAddress(y, indices[i]);
        const auto newInst = builder.emitLoad(y);
        inst->replaceUsesWith(newInst);
        inst->removeAndDeallocate();
        addUsersToWorkList(newInst);
    }

    void processGetElementPtrImpl(IRInst* gepInst, IRInst* base, IRInst* index)
    {
        if (auto ptrType = as<IRPtrTypeBase>(base->getDataType()))
        {
            if (!ptrType->hasAddressSpace())
                return;
            auto oldResultType = as<IRPtrTypeBase>(gepInst->getDataType());
            if (oldResultType->getAddressSpace() != ptrType->getAddressSpace())
            {
                IRBuilder builder(m_sharedContext->m_irModule);
                builder.setInsertBefore(gepInst);
                auto newPtrType = builder.getPtrType(
                    oldResultType->getOp(),
                    oldResultType->getValueType(),
                    ptrType->getAddressSpace());
                IRInst* args[2] = { base, index };
                auto newInst =
                    builder.emitIntrinsicInst(newPtrType, gepInst->getOp(), 2, args);
                gepInst->replaceUsesWith(newInst);
                gepInst->removeAndDeallocate();
                addUsersToWorkList(newInst);
            }
        }
    }

    void processGetElementPtr(IRGetElementPtr* gepInst)
    {
        processGetElementPtrImpl(gepInst, gepInst->getBase(), gepInst->getIndex());
    }

    void processRWStructuredBufferGetElementPtr(IRRWStructuredBufferGetElementPtr* gepInst)
    {
        processGetElementPtrImpl(gepInst, gepInst->getBase(), gepInst->getIndex());
    }

    void processStructuredBufferLoad(IRInst* loadInst)
    {
        auto sb = loadInst->getOperand(0);
        auto index = loadInst->getOperand(1);
        IRBuilder builder(sb);
        builder.setInsertBefore(loadInst);
        IRInst* args[] = { sb, index };
        auto addrInst = builder.emitIntrinsicInst(
            builder.getPtrType(kIROp_PtrType, loadInst->getFullType(), SpvStorageClassStorageBuffer),
            kIROp_RWStructuredBufferGetElementPtr,
            2,
            args);
        auto value = builder.emitLoad(addrInst);
        loadInst->replaceUsesWith(value);
        loadInst->removeAndDeallocate();
        addUsersToWorkList(value);
    }

    void processRWStructuredBufferStore(IRInst* storeInst)
    {
        auto sb = storeInst->getOperand(0);
        auto index = storeInst->getOperand(1);
        auto value = storeInst->getOperand(2);
        IRBuilder builder(sb);
        builder.setInsertBefore(storeInst);
        IRInst* args[] = { sb, index };
        auto addrInst = builder.emitIntrinsicInst(
            builder.getPtrType(kIROp_PtrType, value->getFullType(), SpvStorageClassStorageBuffer),
            kIROp_RWStructuredBufferGetElementPtr,
            2,
            args);
        auto newStore = builder.emitStore(addrInst, value);
        storeInst->replaceUsesWith(newStore);
        storeInst->removeAndDeallocate();
        addUsersToWorkList(newStore);
    }

    void processFieldAddress(IRFieldAddress* inst)
    {
        if (auto ptrType = as<IRPtrTypeBase>(inst->getBase()->getDataType()))
        {
            if (!ptrType->hasAddressSpace())
                return;
            auto oldResultType = as<IRPtrTypeBase>(inst->getDataType());
            if (oldResultType->getAddressSpace() != ptrType->getAddressSpace())
            {
                IRBuilder builder(m_sharedContext->m_irModule);
                builder.setInsertBefore(inst);
                auto newPtrType = builder.getPtrType(
                    oldResultType->getOp(),
                    oldResultType->getValueType(),
                    ptrType->getAddressSpace());
                auto newInst =
                    builder.emitFieldAddress(newPtrType, inst->getBase(), inst->getField());
                inst->replaceUsesWith(newInst);
                inst->removeAndDeallocate();
                addUsersToWorkList(newInst);
            }
        }
    }

    void processStructuredBufferType(IRHLSLStructuredBufferTypeBase * inst)
    {
        auto layoutRules = getTypeLayoutRuleForBuffer(m_sharedContext->m_targetRequest, inst);

        IRBuilder builder(m_sharedContext->m_irModule);

        builder.setInsertBefore(inst);
        auto elementType = inst->getElementType();
        IRSizeAndAlignment elementSize;
        getSizeAndAlignment(layoutRules, elementType, &elementSize);
        elementSize = layoutRules->alignCompositeElement(elementSize);

        const auto arrayType = builder.getUnsizedArrayType(inst->getElementType(), builder.getIntValue(builder.getIntType(), elementSize.getStride()));
        const auto structType = builder.createStructType();
        const auto arrayKey = builder.createStructKey();
        builder.createStructField(structType, arrayKey, arrayType);
        IRSizeAndAlignment structSize;
        getSizeAndAlignment(layoutRules, structType, &structSize);

        const auto ptrType = builder.getPtrType(kIROp_PtrType, structType, SpvStorageClassStorageBuffer);

        StringBuilder nameSb;
        switch (inst->getOp())
        {
        case kIROp_HLSLRWStructuredBufferType:
            nameSb << "RWStructuredBuffer";
            break;
        case kIROp_HLSLAppendStructuredBufferType:
            nameSb << "AppendStructuredBuffer";
            break;
        case kIROp_HLSLConsumeStructuredBufferType:
            nameSb << "ConsumeStructuredBuffer";
            break;
        case kIROp_HLSLRasterizerOrderedStructuredBufferType:
            nameSb << "RasterizerOrderedStructuredBuffer";
            break;
        default:
            nameSb << "StructuredBuffer";
            break;
        }
        builder.addNameHintDecoration(structType, nameSb.getUnownedSlice());
        builder.addDecoration(structType, kIROp_SPIRVBlockDecoration);
        inst->replaceUsesWith(ptrType);
        inst->removeAndDeallocate();
        addUsersToWorkList(ptrType);
    }

    void processLoop(IRLoop* loop)
    {

        // 2.11.1. Rules for Structured Control-flow Declarations
        // Structured control flow declarations must satisfy the following
        // rules:
        //   - the merge block declared by a header block must not be a merge
        //     block declared by any other header block
        //   - each header block must strictly structurally dominate its merge
        //     block
        //   - all back edges must branch to a loop header, with each loop
        //     header having exactly one back edge branching to it
        //   - for a given loop header, its merge block, OpLoopMerge Continue
        //     Target, and corresponding back-edge block:
        //       - the Continue Target and merge block must be different blocks
        //       - the loop header must structurally dominate the Continue
        //         Target
        //       - the Continue Target must structurally dominate the back-edge
        //         block
        //       - the back-edge block must structurally post dominate the
        //         Continue Target

        // If the continue block has only a single predecessor, pretend like it
        // is just ordinary control flow
        //
        // TODO: could this fail in cases like this, where it had a single
        // predecessor, but it's still nested inside a region?
        // do{
        //   if(x)
        //     continue;
        //   unreachable
        // } while(foo)
        const auto t = loop->getTargetBlock();
        auto c = loop->getContinueBlock();
        if(c->getPredecessors().getCount() <= 1)
        {
            c = t;
            loop->continueBlock.set(c);
        }

        // Our IR allows multiple back-edges to a loop header if this is also
        // the loop continue block. SPIR-V does not so replace them with a
        // single intermediate block
        if(c == t)
        {
            // Subtract one predecessor for the loop entry
            const auto numBackEdges = c->getPredecessors().getCount() - 1;

            // If we have multiple back-edges, make a new block at the end of
            // the loop to be the new continue block which jumps straight to
            // the loop header.
            //
            // If we have a single back-edge, we still may need to perform this
            // transformation to make sure that the back-edge block
            // structurally post-dominates the continue target. For example
            // consider the loop:
            //
            // int i = 0;
            // while(true)
            //     if(foo()) break;
            //
            // If we translate this to
            // loop target=t break=b, continue=t
            // t: if foo goto x else goto y
            // x: goto b -- break
            // y: goto t
            // b: ...
            //
            // The back edge block, y, does not post-dominate the continue target, t.
            //
            // So we transform this to:
            //
            // loop target=t break=b, continue=c
            // t: if foo goto x else goto y
            // x: goto b -- break
            // y: goto c
            // c: goto t
            // b: ...
            //
            // Now the back edge block and the continue target are one and the
            // same, so the condition trivially holds.
            //
            // TODO: We don't need to always perform this, we could replace the
            // below condition with `numBackEdges > 1 ||
            //     !postDominates(backJumpingBlock, c)`
            if(numBackEdges > 0)
            {
                IRBuilder builder(m_sharedContext->m_irModule);
                builder.setInsertInto(loop->getParent());
                IRCloneEnv cloneEnv;
                cloneEnv.squashChildrenMapping = true;

                // Insert a new continue block at the end of the loop
                const auto newContinueBlock = builder.emitBlock();
                newContinueBlock->insertBefore(loop->getBreakBlock());

                // This block simply branches to the loop header, forwarding
                // any params
                List<IRInst*> ps;
                for(const auto p : c->getParams())
                {
                    const auto q = cast<IRParam>(cloneInst(&cloneEnv, &builder, p));
                    newContinueBlock->addParam(q);
                    ps.add(q);
                }
                // Replace all jumps to our loop header/old continue block
                c->replaceUsesWith(newContinueBlock);

                // Restore the target block
                loop->block.set(t);

                // Branch to the target in our new continue block
                builder.emitBranch(t, ps.getCount(), ps.getBuffer());
            }
        }
    }

    void processModule()
    {
        addToWorkList(m_module->getModuleInst());
        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();
            workList.removeLast();
            switch (inst->getOp())
            {
            case kIROp_GlobalParam:
                processGlobalParam(as<IRGlobalParam>(inst));
                break;
            case kIROp_GlobalVar:
                processGlobalVar(as<IRGlobalVar>(inst));
                break;
            case kIROp_Call:
                processCall(as<IRCall>(inst));
                break;
            case kIROp_GetElement:
                processGetElement(as<IRGetElement>(inst));
                break;
            case kIROp_GetElementPtr:
                processGetElementPtr(as<IRGetElementPtr>(inst));
                break;
            case kIROp_FieldAddress:
                processFieldAddress(as<IRFieldAddress>(inst));
                break;
            case kIROp_RWStructuredBufferGetElementPtr:
                processRWStructuredBufferGetElementPtr(as<IRRWStructuredBufferGetElementPtr>(inst));
                break;
            case kIROp_RWStructuredBufferLoad:
            case kIROp_StructuredBufferLoad:
            case kIROp_RWStructuredBufferLoadStatus:
            case kIROp_StructuredBufferLoadStatus:
                processStructuredBufferLoad(inst);
                break;
            case kIROp_RWStructuredBufferStore:
                processRWStructuredBufferStore(inst);
                break;
            case kIROp_HLSLStructuredBufferType:
            case kIROp_HLSLRWStructuredBufferType:
                processStructuredBufferType(as<IRHLSLStructuredBufferTypeBase>(inst));
                break;
            case kIROp_loop:
                processLoop(as<IRLoop>(inst));
                break;
            default:
                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    addToWorkList(child);
                }
                break;
            }
        }
    }
};

SpvSnippet* SPIRVEmitSharedContext::getParsedSpvSnippet(IRTargetIntrinsicDecoration* intrinsic)
{
    RefPtr<SpvSnippet> snippet;
    if (m_parsedSpvSnippets.tryGetValue(intrinsic, snippet))
    {
        return snippet.Ptr();
    }
    snippet = SpvSnippet::parse(intrinsic->getDefinition());
    if(!snippet)
    {
        m_sink->diagnose(intrinsic, Diagnostics::snippetParsingFailed, intrinsic->getDefinition());
        return nullptr;
    }
    m_parsedSpvSnippets[intrinsic] = snippet;
    return snippet;
}

void legalizeSPIRV(SPIRVEmitSharedContext* sharedContext, IRModule* module)
{
    SPIRVLegalizationContext context(sharedContext, module);
    context.processModule();
}

void legalizeIRForSPIRV(
    SPIRVEmitSharedContext* context,
    IRModule* module,
    const List<IRFunc*>& entryPoints,
    CodeGenContext* codeGenContext)
{
    GLSLExtensionTracker extensionTracker;
    legalizeEntryPointsForGLSL(module->getSession(), module, entryPoints, codeGenContext, &extensionTracker);
    legalizeSPIRV(context, module);
}

} // namespace Slang
