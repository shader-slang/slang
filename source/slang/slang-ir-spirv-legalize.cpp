// slang-ir-spirv-legalize.cpp
#include "slang-ir-spirv-legalize.h"

#include "slang-ir-glsl-legalize.h"

#include "slang-ir-layout-on-types.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-emit-base.h"
#include "slang-glsl-extension-tracker.h"
#include "slang-type-layout.h"

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

    void processGlobalParam(IRGlobalParam* inst)
    {
        // If the global param is not a pointer type, make it so and insert explicit load insts.
        auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
        if (!ptrType)
        {
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
            auto innerType = inst->getFullType();
            if(const auto constantBufferType = as<IRConstantBufferType>(innerType))
            {
                innerType = constantBufferType->getElementType();
            }

            // Make a pointer type of storageClass.
            IRBuilder builder(m_sharedContext->m_irModule);
            builder.setInsertBefore(inst);
            ptrType = builder.getPtrType(kIROp_PtrType, innerType, storageClass);
            inst->setFullType(ptrType);
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

    // Replace getElement(x, i) with, y = store(x); p = getElementPtr(y, i); load(p)
    // SPIR-V has no support for dynamic indexing into values like we do.
    // It may be advantageous however to do this further up the pipeline
    void processGetElement(IRGetElement* inst)
    {
        const auto x = inst->getBase();
        List<IRInst*> indices;
        IRGetElement* c = inst;
        do
        {
            indices.add(c->getIndex());
        } while(c = as<IRGetElement>(c->getBase()), c);
        IRBuilder builder(m_sharedContext->m_irModule);
        builder.setInsertBefore(inst);
        IRInst* y = builder.emitVar(x->getDataType(), SpvStorageClassFunction);
        builder.emitStore(y, x);
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

    void processStructuredBufferType(IRHLSLStructuredBufferTypeBase* inst)
    {
        // const auto elementTypeLayout = inst->getElementType()->findDecoration<IRTypeLayout>();
        // SLANG_ASSERT(elementTypeLayout);
        const auto typeLayoutDecoration = inst->findDecoration<IRLayoutDecoration>();
        SLANG_ASSERT(typeLayoutDecoration);
        const auto typeLayout = as<IRStructuredBufferTypeLayout>(typeLayoutDecoration->getLayout());
        SLANG_ASSERT(typeLayout);
        const auto elemTypeLayout = typeLayout->getElementTypeLayout();
        SLANG_ASSERT(elemTypeLayout);

        IRBuilder builder(m_sharedContext->m_irModule);

        builder.setInsertBefore(inst);
        const auto arrayType = builder.getUnsizedArrayType(inst->getElementType());
        IRArrayTypeLayout::Builder arrayTypeLayoutBuilder(&builder, elemTypeLayout);
        const auto arrayTypeLayout = arrayTypeLayoutBuilder.build();
        IRVarLayout::Builder varLayoutBuilder(&builder, arrayTypeLayout);
        varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::Uniform);
        const auto arrayFieldVarLayout = varLayoutBuilder.build();

        const auto structType = builder.createStructType();
        IRStructTypeLayout::Builder structTypeLayoutBuilder(&builder);
        const auto arrayKey = builder.createStructKey();
        builder.createStructField(structType, arrayKey, arrayType);
        structTypeLayoutBuilder.addField(arrayKey, arrayFieldVarLayout);
        const auto structTypeLayout = structTypeLayoutBuilder.build();

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
        default:
            nameSb << "StructuredBuffer";
            break;
        }
        builder.addNameHintDecoration(structType, nameSb.getUnownedSlice());
        builder.addDecoration(structType, kIROp_SPIRVBufferBlockDecoration);
        SLANG_ASSERT(!structType->findDecoration<IRLayoutDecoration>());
        builder.addLayoutDecoration(structType, structTypeLayout);
        inst->replaceUsesWith(ptrType);
        inst->removeAndDeallocate();
        addUsersToWorkList(ptrType);
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
            case kIROp_HLSLStructuredBufferType:
            case kIROp_HLSLRWStructuredBufferType:
                processStructuredBufferType(as<IRHLSLStructuredBufferTypeBase>(inst));
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
    placeTypeLayoutsOnTypes(module, codeGenContext);
    GLSLExtensionTracker extensionTracker;
    legalizeEntryPointsForGLSL(module->getSession(), module, entryPoints, codeGenContext, &extensionTracker);
    legalizeSPIRV(context, module);
}

} // namespace Slang
