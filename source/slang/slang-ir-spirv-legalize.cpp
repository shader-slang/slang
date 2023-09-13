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
#include "slang-ir-dominators.h"

namespace Slang
{

//
// Legalization of IR for direct SPIRV emit.
//

struct SPIRVLegalizationContext : public SourceEmitterBase
{
    SPIRVEmitSharedContext* m_sharedContext;

    IRModule* m_module;
    
    struct LoweredStructuredBufferTypeInfo
    {
        IRType* structType;
        IRStructKey* arrayKey;
        IRArrayTypeBase* runtimeArrayType;
    };
    Dictionary<IRType*, LoweredStructuredBufferTypeInfo> m_loweredStructuredBufferTypes;

    LoweredStructuredBufferTypeInfo lowerStructuredBufferType(IRHLSLStructuredBufferTypeBase* inst)
    {
        LoweredStructuredBufferTypeInfo result;
        if (m_loweredStructuredBufferTypes.tryGetValue(inst, result))
            return result;

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

        result.structType = structType;
        result.arrayKey = arrayKey;
        result.runtimeArrayType = arrayType;
        m_loweredStructuredBufferTypes[inst] = result;
        return result;
    }

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

    static void insertLoadAtLatestLocation(IRInst* addrInst, IRUse* inUse)
    {
        struct WorkItem { IRInst* addr; IRUse* use; };
        List<WorkItem> workList;
        List<IRInst*> instsToRemove;
        workList.add(WorkItem{ addrInst, inUse });
        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto use = workList[i].use;
            auto addr = workList[i].addr;
            auto user = use->getUser();
            IRBuilder builder(user);
            builder.setInsertBefore(user);
            if(as<IRGetElement>(user) || as<IRFieldExtract>(user))
            {
                auto basePtrType = as<IRPtrTypeBase>(addr->getDataType());
                IRType* ptrType = nullptr;
                if (basePtrType->hasAddressSpace())
                    ptrType = builder.getPtrType(kIROp_PtrType, user->getDataType(), basePtrType->getAddressSpace());
                else
                    ptrType = builder.getPtrType(kIROp_PtrType, user->getDataType());
                IRInst* subAddr = nullptr;
                if (user->getOp() == kIROp_GetElement)
                    subAddr = builder.emitElementAddress(ptrType, addr, as<IRGetElement>(user)->getIndex());
                else
                    subAddr = builder.emitFieldAddress(ptrType, addr, as<IRFieldExtract>(user)->getField());

                for (auto u = user->firstUse; u; u = u->nextUse)
                {
                    workList.add(WorkItem{ subAddr, u });
                }
                instsToRemove.add(user);
            }
            else if(const auto spirvAsmOperand = as<IRSPIRVAsmOperandInst>(user))
            {
                // If this is being used in an asm block, insert the load to
                // just prior to the block.
                const auto asmBlock = spirvAsmOperand->getAsmBlock();
                builder.setInsertBefore(asmBlock);
                auto loadedValue = builder.emitLoad(addrInst);
                builder.setInsertBefore(spirvAsmOperand);
                auto loadedValueOperand = builder.emitSPIRVAsmOperandInst(loadedValue);
                spirvAsmOperand->replaceUsesWith(loadedValueOperand);
                spirvAsmOperand->removeAndDeallocate();
            }
            else
            {
                auto val = builder.emitLoad(addr);
                builder.replaceOperand(use, val);
            }
        }

        for (auto i : instsToRemove)
            if (!i->hasUses())
                i->removeAndDeallocate();
    }

    // Returns true if the given type that should be decorated as in `UniformConstant` address space.
    // These are typically opaque resource handles that can't be marked as `Uniform`.
    bool isSpirvUniformConstantType(IRType* type)
    {
        if (as<IRTextureTypeBase>(type))
            return true;
        if (as<IRSamplerStateTypeBase>(type))
            return true;
        switch (type->getOp())
        {
        case kIROp_RaytracingAccelerationStructureType:
        case kIROp_RayQueryType:
            return true;
        default:
            return false;
        }
    }

    void processGlobalParam(IRGlobalParam* inst)
    {
        // If the global param is not a pointer type, make it so and insert explicit load insts.
        auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
        if (!ptrType)
        {
            auto innerType = inst->getFullType();

            auto arrayType = as<IRArrayType>(inst->getDataType());
            IRInst* arraySize = nullptr;
            if (arrayType)
            {
                arraySize = arrayType->getElementCount();
                innerType = arrayType->getElementType();
            }

            SpvStorageClass storageClass = SpvStorageClassPrivate;
            // Figure out storage class based on var layout.
            if (auto layout = getVarLayout(inst))
            {
                auto cls = getGlobalParamStorageClass(layout);
                if (cls != SpvStorageClassMax)
                    storageClass = cls;
                else if (auto systemValueAttr = layout->findAttr<IRSystemValueSemanticAttr>())
                {
                    String semanticName = systemValueAttr->getName();
                    semanticName = semanticName.toLower();
                    if (semanticName == "sv_pointsize")
                        storageClass = SpvStorageClassInput;
                }
            }

            // Opaque resource handles can't be in Uniform for Vulkan, if they are
            // placed here then put them in UniformConstant instead
            if (storageClass == SpvStorageClassUniform
                && isSpirvUniformConstantType(inst->getDataType()))
            {
                storageClass = SpvStorageClassUniformConstant;
            }

            // Strip any HLSL wrappers
            IRBuilder builder(m_sharedContext->m_irModule);
            bool needLoad = true;
            auto cbufferType = as<IRConstantBufferType>(innerType);
            auto paramBlockType = as<IRParameterBlockType>(innerType);
            if (cbufferType || paramBlockType)
            {
                innerType = as<IRUniformParameterGroupType>(innerType)->getElementType();
                if (storageClass == SpvStorageClassPrivate)
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
                
                auto varLayoutInst = inst->findDecoration<IRLayoutDecoration>();
                if (paramBlockType)
                {
                    // A parameter block typed global parameter will have a VarLayout
                    // that contains an OffsetAttr(RegisterSpace, spaceId).
                    // We need to turn this VarLayout into a standard cbuffer VarLayout
                    // in the form of OffsetAttr(ConstantBuffer, 0, spaceId).
                    builder.setInsertBefore(inst);
                    IRVarLayout* varLayout = nullptr;
                    if (varLayoutInst)
                        varLayout = as<IRVarLayout>(varLayoutInst->getLayout());
                    if (varLayout)
                    {
                        auto registerSpaceOffsetAttr = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace);
                        if (registerSpaceOffsetAttr)
                        {
                            List<IRInst*> operands;
                            for (UInt i = 0; i < varLayout->getOperandCount(); i++)
                                operands.add(varLayout->getOperand(i));
                            operands.add(builder.getVarOffsetAttr(LayoutResourceKind::ConstantBuffer, 0, registerSpaceOffsetAttr->getOffset()));
                            auto newLayout = builder.getVarLayout(operands);
                            varLayoutInst->setOperand(0, newLayout);
                            varLayout->removeAndDeallocate();
                        }
                    }
                }
                else if (storageClass == SpvStorageClassPushConstant)
                {
                    // Push constant params does not need a VarLayout.
                    varLayoutInst->removeAndDeallocate();
                }
            }
            else
            {
                if (auto structuredBufferType = as<IRHLSLStructuredBufferTypeBase>(innerType))
                {
                    innerType = lowerStructuredBufferType(structuredBufferType).structType;
                    storageClass = SpvStorageClassStorageBuffer;
                    needLoad = false;
                }
            }

            auto innerElementType = innerType;
            if (arraySize)
            {
                innerType = builder.getArrayType(innerType, arraySize);
            }

            // Make a pointer type of storageClass.
            builder.setInsertBefore(inst);
            ptrType = builder.getPtrType(kIROp_PtrType, innerType, storageClass);
            inst->setFullType(ptrType);
            if (needLoad)
            {
                // Insert an explicit load at each use site.
                traverseUses(inst, [&](IRUse* use)
                    {
                        insertLoadAtLatestLocation(inst, use);
                    });
            }
            else if (arraySize)
            {
                traverseUses(inst, [&](IRUse* use)
                    {
                        auto user = use->getUser();
                        if (auto getElement = as<IRGetElement>(user))
                        {
                            // For array resources, getElement(r, index) ==> getElementPtr(r, index).
                            IRBuilder builder(getElement);
                            builder.setInsertBefore(user);
                            auto newAddr = builder.emitElementAddress(builder.getPtrType(kIROp_PtrType, innerElementType, storageClass), inst, getElement->getIndex());
                            user->replaceUsesWith(newAddr);
                            user->removeAndDeallocate();
                            return;
                        }
                    });
            }
        }
        processGlobalVar(inst);
    }

    SpvStorageClass getStorageClassFromGlobalParamResourceKind(LayoutResourceKind kind)
    {
        SpvStorageClass storageClass = SpvStorageClassMax;
        switch (kind)
        {
        case LayoutResourceKind::Uniform:
        case LayoutResourceKind::DescriptorTableSlot:
        case LayoutResourceKind::ConstantBuffer:
            storageClass = SpvStorageClassUniform;
            break;
        case LayoutResourceKind::VaryingInput:
            storageClass = SpvStorageClassInput;
            break;
        case LayoutResourceKind::VaryingOutput:
            storageClass = SpvStorageClassOutput;
            break;
        case LayoutResourceKind::ShaderResource:
        case LayoutResourceKind::UnorderedAccess:
            storageClass = SpvStorageClassStorageBuffer;
            break;
        case LayoutResourceKind::PushConstantBuffer:
            storageClass = SpvStorageClassPushConstant;
            break;
        case LayoutResourceKind::RayPayload:
            storageClass = SpvStorageClassRayPayloadKHR;
            break;
        case LayoutResourceKind::CallablePayload:
            storageClass = SpvStorageClassCallableDataKHR;
            break;
        case LayoutResourceKind::HitAttributes:
            storageClass = SpvStorageClassHitAttributeKHR;
            break;
        case LayoutResourceKind::ShaderRecord:
            storageClass = SpvStorageClassShaderRecordBufferKHR;
            break;
        default:
            break;
        }
        return storageClass;
    }

    SpvStorageClass getGlobalParamStorageClass(IRVarLayout* varLayout)
    {
        SpvStorageClass result = SpvStorageClassMax;
        for (auto rr : varLayout->getOffsetAttrs())
        {
            auto storageClass = getStorageClassFromGlobalParamResourceKind(rr->getResourceKind());
            // If we haven't inferred a storage class yet, use the one we just found.
            if (result == SpvStorageClassMax)
                result = storageClass;
            else if (result != storageClass)
            {
                // If we have inferred a storage class, and it is different from the one we just found,
                // then we have conflicting uses of the resource, and we cannot infer a storage class.
                // An exception is that a uniform storage class can be further specialized by PushConstants.
                if (result == SpvStorageClassUniform)
                    result = storageClass;
                else
                    SLANG_UNEXPECTED("Var layout contains conflicting resource uses, cannot resolve a storage class.");
            }
        }
        return result;
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
            auto cls = getGlobalParamStorageClass(varLayout);
            if (cls != SpvStorageClassMax)
                storageClass = cls;
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
            return;
        }

        // According to SPIRV spec, the if the operands of a call has pointer
        // type, then it can only be a memory-object. This means that if the
        // pointer is a result of `getElementPtr`, we cannot use it as an
        // argument. In this case, we have to allocate a temp var to pass the
        // value, and write them back to the original pointer after the call.
        // 
        // > SPIRV Spec section 2.16.1:
        // >   - Any pointer operand to an OpFunctionCall must be a memory object
        // >     declaration, or
        // >     - a pointer to an element in an array that is a memory object
        // >       declaration, where the element type is OpTypeSampler or OpTypeImage.
        //
        List<IRInst*> newArgs;
        struct WriteBackPair { IRInst* originalAddrArg; IRInst* tempVar; };
        List<WriteBackPair> writeBacks;
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        for (UInt i = 0; i < inst->getArgCount(); i++)
        {
            auto arg = inst->getArg(i);
            auto ptrType = as<IRPtrTypeBase>(arg->getDataType());
            if (!as<IRPtrTypeBase>(arg->getDataType()))
            {
                newArgs.add(arg);
                continue;
            }
            // Is the arg already a memory-object by SPIRV definition?
            // If so we don't need to allocate a temp var.
            switch (arg->getOp())
            {
            case kIROp_Var:
            case kIROp_GlobalVar:
                newArgs.add(arg);
                continue;
            case kIROp_Param:
                if (arg->getParent() == getParentFunc(arg)->getFirstBlock())
                {
                    newArgs.add(arg);
                    continue;
                }
                break;
            default:
                break;
            }
            auto root = getRootAddr(arg);
            if (root)
            {
                switch (root->getOp())
                {
                case kIROp_RWStructuredBufferGetElementPtr:
                    newArgs.add(arg);
                    continue;
                }
            }

            // If we reach here, we need to allocate a temp var.
            auto tempVar = builder.emitVar(ptrType->getValueType());
            auto load = builder.emitLoad(arg);
            builder.emitStore(tempVar, load);
            newArgs.add(tempVar);
            writeBacks.add(WriteBackPair{ arg, tempVar });
        }
        SLANG_ASSERT((UInt)newArgs.getCount() == inst->getArgCount());
        if (writeBacks.getCount())
        {
            auto newCall = builder.emitCallInst(inst->getFullType(), inst->getCallee(), newArgs);
            for (auto wb : writeBacks)
            {
                auto newVal = builder.emitLoad(wb.tempVar);
                builder.emitStore(wb.originalAddrArg, newVal);
            }
            inst->replaceUsesWith(newCall);
            inst->removeAndDeallocate();
            addUsersToWorkList(newCall);
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
        builder.setInsertBefore(inst);
        if (!m_mapArrayValueToVar.tryGetValue(x, y))
        {
            if (x->getParent()->getOp() == kIROp_Module)
                builder.setInsertBefore(inst);
            else
                setInsertAfterOrdinaryInst(&builder, x);
            y = builder.emitVar(x->getDataType(), SpvStorageClassFunction);
            builder.emitStore(y, x);
            if (x->getParent()->getOp() != kIROp_Module)
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

    void duplicateMergeBlockIfNeeded(IRUse* breakBlockUse)
    {
        auto breakBlock = as<IRBlock>(breakBlockUse->get());
        if (breakBlock->getFirstInst()->getOp() != kIROp_Unreachable)
        {
            return;
        }
        bool hasMoreThanOneUser = false;
        for (auto use = breakBlock->firstUse; use; use = use->nextUse)
        {
            if (use->getUser() != breakBlockUse->getUser())
            {
                hasMoreThanOneUser = true;
                break;
            }
        }
        if (!hasMoreThanOneUser)
            return;

        // Create a duplicate block for this use.
        IRBuilder builder(breakBlock);
        builder.setInsertBefore(breakBlock);
        auto block = builder.emitBlock();
        builder.emitUnreachable();
        breakBlockUse->set(block);
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
        duplicateMergeBlockIfNeeded(&loop->breakBlock);
    }

    void processIfElse(IRIfElse* inst)
    {
        duplicateMergeBlockIfNeeded(&inst->afterBlock);

        // SPIRV does not allow using merge block directly as true/false block,
        // so we need to create an intermediate block if this is the case.
        IRBuilder builder(inst);
        if (inst->getTrueBlock() == inst->getAfterBlock())
        {
            builder.setInsertBefore(inst->getAfterBlock());
            auto newBlock = builder.emitBlock();
            builder.emitBranch(inst->getAfterBlock());
            inst->trueBlock.set(newBlock);
        }
        if (inst->getFalseBlock() == inst->getAfterBlock())
        {
            builder.setInsertBefore(inst->getAfterBlock());
            auto newBlock = builder.emitBlock();
            builder.emitBranch(inst->getAfterBlock());
            inst->falseBlock.set(newBlock);
        }
    }

    void processSwitch(IRSwitch* inst)
    {
        duplicateMergeBlockIfNeeded(&inst->breakLabel);

        // SPIRV does not allow using merge block directly as case block,
        // so we need to create an intermediate block if this is the case.
        IRBuilder builder(inst);
        if (inst->getDefaultLabel() == inst->getBreakLabel())
        {
            builder.setInsertBefore(inst->getBreakLabel());
            auto newBlock = builder.emitBlock();
            builder.emitBranch(inst->getBreakLabel());
            inst->defaultLabel.set(newBlock);
        }
        for (UInt i = 0; i < inst->getCaseCount(); i++)
        {
            if (inst->getCaseLabel(i) == inst->getBreakLabel())
            {
                builder.setInsertBefore(inst->getBreakLabel());
                auto newBlock = builder.emitBlock();
                builder.emitBranch(inst->getBreakLabel());
                inst->getCaseLabelUse(i)->set(newBlock);
            }
        }
    }

    void processConstructor(IRInst* inst)
    {
        // If all of the operands to this instruction are global, we can hoist
        // this constructor to be a global too. This is important to make sure
        // that vectors made of constant components end up being emitted as
        // constant vectors (using OpConstantComposite).
        UIndex opIndex = 0;
        for (auto operand = inst->getOperands(); opIndex < inst->getOperandCount(); operand++, opIndex++)
            if(operand->get()->getParent() != m_module->getModuleInst())
                return;
        inst->insertAtEnd(m_module->getModuleInst());
    }

    void processModule()
    {
        // Process global params before anything else, so we don't generate inefficient
        // array marhalling code for array-typed global params.
        for (auto globalInst : m_module->getGlobalInsts())
        {
            if (auto globalParam = as<IRGlobalParam>(globalInst))
            {
                processGlobalParam(globalParam);
            }
            else
            {
                addToWorkList(globalInst);
            }
        }

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
            case kIROp_loop:
                processLoop(as<IRLoop>(inst));
                break;
            case kIROp_ifElse:
                processIfElse(as<IRIfElse>(inst));
                break;
            case kIROp_Switch:
                processSwitch(as<IRSwitch>(inst));
                break;

            case kIROp_MakeVectorFromScalar:
            case kIROp_MakeUInt64:
            case kIROp_MakeVector:
            case kIROp_MakeMatrix:
            case kIROp_MakeMatrixFromScalar:
            case kIROp_MatrixReshape:
            case kIROp_MakeArray:
            case kIROp_MakeArrayFromElement:
            case kIROp_MakeStruct:
            case kIROp_MakeTuple:
            case kIROp_MakeTargetTuple:
            case kIROp_MakeResultValue:
            case kIROp_MakeResultError:
            case kIROp_MakeOptionalValue:
            case kIROp_MakeOptionalNone:
                processConstructor(inst);
                break;

            default:
                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    addToWorkList(child);
                }
                break;
            }
        }

        // Translate types.
        List<IRHLSLStructuredBufferTypeBase*> instsToProcess;
        for (auto globalInst : m_module->getGlobalInsts())
        {
            if (auto t = as<IRHLSLStructuredBufferTypeBase>(globalInst))
            {
                instsToProcess.add(t);
            }
        }
        for (auto t : instsToProcess)
        {
            auto lowered = lowerStructuredBufferType(t);
            IRBuilder builder(t);
            builder.setInsertBefore(t);
            t->replaceUsesWith(builder.getPtrType(kIROp_PtrType, lowered.structType, SpvStorageClassStorageBuffer));
        }

        // SPIRV requires a dominator block to appear before dominated blocks.
        // After legalizing the control flow, we need to sort our blocks to ensure this is true.
        for (auto globalInst : m_module->getGlobalInsts())
        {
            if (auto func = as<IRGlobalValueWithCode>(globalInst))
                sortBlocksInFunc(func);
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
    snippet = SpvSnippet::parse(*m_grammarInfo, intrinsic->getDefinition());
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
