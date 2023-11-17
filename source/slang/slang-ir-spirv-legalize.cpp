// slang-ir-spirv-legalize.cpp
#include "slang-ir-spirv-legalize.h"

#include "slang-ir-glsl-legalize.h"

#include "slang-ir-clone.h"
#include "slang-ir-legalize-mesh-outputs.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-emit-base.h"
#include "slang-glsl-extension-tracker.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir-dominators.h"
#include "slang-ir-composite-reg-to-mem.h"
#include "slang-ir-sccp.h"
#include "slang-ir-dce.h"
#include "slang-ir-simplify-cfg.h"
#include "slang-ir-peephole.h"
#include "slang-ir-redundancy-removal.h"

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
        getSizeAndAlignment(m_sharedContext->m_targetRequest, layoutRules, elementType, &elementSize);
        elementSize = layoutRules->alignCompositeElement(elementSize);

        const auto arrayType = builder.getUnsizedArrayType(inst->getElementType(), builder.getIntValue(builder.getIntType(), elementSize.getStride()));
        const auto structType = builder.createStructType();
        const auto arrayKey = builder.createStructKey();
        builder.createStructField(structType, arrayKey, arrayType);
        IRSizeAndAlignment structSize;
        getSizeAndAlignment(m_sharedContext->m_targetRequest, layoutRules, structType, &structSize);

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
        getSizeAndAlignment(m_sharedContext->m_targetRequest, rules, structType, &sizeAlignment);
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
                auto loadedValue = builder.emitLoad(addr);
                builder.setInsertBefore(spirvAsmOperand);
                auto loadedValueOperand = builder.emitSPIRVAsmOperandInst(loadedValue);
                spirvAsmOperand->replaceUsesWith(loadedValueOperand);
                spirvAsmOperand->removeAndDeallocate();
            }
            else
            {
                if (!as<IRDecoration>(use->getUser()))
                {
                    auto val = builder.emitLoad(addr);
                    builder.replaceOperand(use, val);
                }
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
        if (const auto arr = as<IRArrayTypeBase>(type))
            return isSpirvUniformConstantType(arr->getElementType());
        switch (type->getOp())
        {
        case kIROp_RaytracingAccelerationStructureType:
        case kIROp_RayQueryType:
            return true;
        default:
            return false;
        }
    }

    Stage getReferencingEntryPointStage(IRInst* inst)
    {
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            if (auto f = getParentFunc(use->getUser()))
            {
                if (auto d = f->findDecoration<IREntryPointDecoration>())
                    return d->getProfile().getStage();
            }
        }
        return Stage::Unknown;
    }

    bool translatePerVertexInputType(IRInst* param)
    {
        if (auto interpolationModeDecor = param->findDecoration<IRInterpolationModeDecoration>())
        {
            if (interpolationModeDecor->getMode() == IRInterpolationMode::PerVertex)
            {
                if (getReferencingEntryPointStage(param) == Stage::Fragment)
                {
                    auto originalType = param->getFullType();
                    IRBuilder builder(param);
                    builder.setInsertBefore(param);
                    auto arrayType = builder.getArrayType(originalType, builder.getIntValue(builder.getIntType(), 3));
                    param->setFullType(arrayType);
                    return true;
                }
            }
        }
        return false;
    }

    static IRType* replaceImageElementType(IRInst* originalType, IRInst* newElementType)
    {
        switch(originalType->getOp())
        {
        case kIROp_ArrayType:
        case kIROp_UnsizedArrayType:
        case kIROp_PtrType:
        case kIROp_OutType:
        case kIROp_RefType:
        case kIROp_ConstRefType:
        case kIROp_InOutType:
            {
                auto newInnerType = replaceImageElementType(originalType->getOperand(0), newElementType);
                if (newInnerType != originalType->getOperand(0))
                {
                    IRBuilder builder(originalType);
                    builder.setInsertBefore(originalType);
                    IRCloneEnv cloneEnv;
                    cloneEnv.mapOldValToNew.add(originalType->getOperand(0), newInnerType);
                    return (IRType*)cloneInst(&cloneEnv, &builder, originalType);
                }
                return (IRType*)originalType;
            }
            
        default:
            if (as<IRResourceTypeBase>(originalType))
                return (IRType*)newElementType;
            return (IRType*)originalType;
        }
    }

    static void inferTextureFormat(IRInst* textureInst, IRTextureTypeBase* textureType)
    {
        ImageFormat format = (ImageFormat)(textureType->getFormat());
        if (auto decor = textureInst->findDecoration<IRFormatDecoration>())
        {
            format = decor->getFormat();
        }
        if (format == ImageFormat::unknown)
        {
            // If the texture has no format decoration, try to infer it from the type.
            auto elementType = textureType->getElementType();
            Int vectorWidth = 1;
            if (auto elementVecType = as<IRVectorType>(elementType))
            {
                if (auto intLitVal = as<IRIntLit>(elementVecType->getElementCount()))
                {
                    vectorWidth = (Int)intLitVal->getValue();
                }
                else
                {
                    vectorWidth = 0;
                }
                elementType = elementVecType->getElementType();
            }
            switch (elementType->getOp())
            {
            case kIROp_UIntType:
                switch (vectorWidth)
                {
                case 1: format = ImageFormat::r32ui; break;
                case 2: format = ImageFormat::rg32ui; break;
                case 4: format = ImageFormat::rgba32ui; break;
                }
                break;
            case kIROp_IntType:
                switch (vectorWidth)
                {
                case 1: format = ImageFormat::r32i; break;
                case 2: format = ImageFormat::rg32i; break;
                case 4: format = ImageFormat::rgba32i; break;
                }
                break;
            case kIROp_UInt16Type:
                switch (vectorWidth)
                {
                case 1: format = ImageFormat::r16ui; break;
                case 2: format = ImageFormat::rg16ui; break;
                case 4: format = ImageFormat::rgba16ui; break;
                }
                break;
            case kIROp_Int16Type:
                switch (vectorWidth)
                {
                case 1: format = ImageFormat::r16i; break;
                case 2: format = ImageFormat::rg16i; break;
                case 4: format = ImageFormat::rgba16i; break;
                }
                break;
            case kIROp_UInt8Type:
                switch (vectorWidth)
                {
                case 1: format = ImageFormat::r8ui; break;
                case 2: format = ImageFormat::rg8ui; break;
                case 4: format = ImageFormat::rgba8ui; break;
                }
                break;
            case kIROp_Int8Type:
                switch (vectorWidth)
                {
                case 1: format = ImageFormat::r8i; break;
                case 2: format = ImageFormat::rg8i; break;
                case 4: format = ImageFormat::rgba8i; break;
                }
                break;
            case kIROp_Int64Type:
                switch (vectorWidth)
                {
                case 1: format = ImageFormat::r64i; break;
                default: break;
                }
                break;
            case kIROp_UInt64Type:
                switch (vectorWidth)
                {
                case 1: format = ImageFormat::r64ui; break;
                default: break;
                }
                break;
            }
        }
        if (format != ImageFormat::unknown)
        {
            IRBuilder builder(textureInst->getModule());
            builder.setInsertBefore(textureInst);
            auto formatArg = builder.getIntValue(builder.getUIntType(), IRIntegerValue(format));

            auto newType = builder.getTextureType(
                textureType->getElementType(),
                textureType->getShapeInst(),
                textureType->getIsArrayInst(),
                textureType->getIsMultisampleInst(),
                textureType->getSampleCountInst(),
                textureType->getAccessInst(),
                textureType->getIsShadowInst(),
                textureType->getIsCombinedInst(),
                formatArg);

            if (textureInst->getFullType() == textureType)
            {
                // Simple texture typed global param.
                textureInst->setFullType(newType);
            }
            else
            {
                // Array typed global param. We need to replace the type and the types of all getElement insts.
                auto newInstType = (IRType*)replaceImageElementType(textureInst->getFullType(), newType);
                textureInst->setFullType(newInstType);
                List<IRUse*> typeReplacementWorkList;
                HashSet<IRUse*> typeReplacementWorkListSet;
                for (auto use = textureInst->firstUse; use; use = use->nextUse)
                {
                    if (typeReplacementWorkListSet.add(use))
                        typeReplacementWorkList.add(use);
                }
                for (Index i = 0; i < typeReplacementWorkList.getCount(); i++)
                {
                    auto use = typeReplacementWorkList[i];
                    auto user = use->getUser();
                    switch (user->getOp())
                    {
                    case kIROp_GetElementPtr:
                    case kIROp_GetElement:
                    case kIROp_Load:
                        {
                            auto newUserType = (IRType*)replaceImageElementType(user->getFullType(), newType);
                            user->setFullType(newUserType);
                            for (auto u = user->firstUse; u; u = u->nextUse)
                            {
                                if (typeReplacementWorkListSet.add(u))
                                    typeReplacementWorkList.add(u);
                            }
                            break;
                        };
                    }
                }
            }
        }
    }

    void processGlobalParam(IRGlobalParam* inst)
    {
        // If the param is a texture, infer its format.
        if (auto textureType = as<IRTextureTypeBase>(unwrapArray(inst->getDataType())))
        {
            inferTextureFormat(inst, textureType);
        }

        // If the global param is not a pointer type, make it so and insert explicit load insts.
        auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
        if (!ptrType)
        {
            bool needLoad = true;

            if (translatePerVertexInputType(inst))
                needLoad = false;

            auto innerType = inst->getFullType();

            auto arrayType = as<IRArrayTypeBase>(inst->getDataType());
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
            if (isSpirvUniformConstantType(inst->getDataType()))
            {
                storageClass = SpvStorageClassUniformConstant;
            }

            // Strip any HLSL wrappers
            IRBuilder builder(m_sharedContext->m_irModule);
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
                        auto registerSpaceOffsetAttr = varLayout->findOffsetAttr(LayoutResourceKind::SubElementRegisterSpace);
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
            if (arrayType)
            {
                Array<IRInst*, 2> arrayTypeArgs;
                arrayTypeArgs.add(innerType);
                if (arraySize)
                    arrayTypeArgs.add(arraySize);
                innerType = (IRType*)builder.emitIntrinsicInst(builder.getTypeKind(), arrayType->getOp(), (UInt)arrayTypeArgs.getCount(), arrayTypeArgs.getBuffer());
                if (!arraySize)
                {
                    builder.addRequireSPIRVDescriptorIndexingExtensionDecoration(inst);
                }
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
            else if (arrayType)
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
            storageClass = SpvStorageClassIncomingRayPayloadKHR;
            break;
        case LayoutResourceKind::CallablePayload:
            storageClass = SpvStorageClassIncomingCallableDataKHR;
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
        auto typeLayout = varLayout->getTypeLayout()->unwrapArray();
        if (auto parameterGroupTypeLayout = as<IRParameterGroupTypeLayout>(typeLayout))
        {
            varLayout = parameterGroupTypeLayout->getContainerVarLayout();
        }

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
        for (auto decor : inst->getDecorations())
        {
            switch (decor->getOp())
            {
            case kIROp_VulkanRayPayloadDecoration:
                storageClass = SpvStorageClassRayPayloadKHR;
                break;
            case kIROp_VulkanCallablePayloadDecoration:
                storageClass = SpvStorageClassCallableDataKHR;
                break;
            case kIROp_VulkanHitObjectAttributesDecoration:
                storageClass = SpvStorageClassHitObjectAttributeNV;
                break;
            case kIROp_VulkanHitAttributesDecoration:
                storageClass = SpvStorageClassHitAttributeKHR;
                break;
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
        auto funcType = as<IRFuncType>(funcValue->getFullType());
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
                    if (funcType)
                    {
                        if (funcType->getParamCount() > i && as<IRRefType>(funcType->getParamType(i)))
                        {
                            // If we are passing an address from a structured buffer as a
                            // ref argument, pass the original pointer as is.
                            // This is to support stdlib atomic functions.
                            newArgs.add(arg);
                            continue;
                        }
                    }
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

    void processImageSubscript(IRImageSubscript* subscript)
    {
        if (auto ptrType = as<IRPtrTypeBase>(subscript->getDataType()))
        {
            if (ptrType->hasAddressSpace())
                return;
            IRBuilder builder(m_sharedContext->m_irModule);
            builder.setInsertBefore(subscript);
            auto newPtrType = builder.getPtrType(
                ptrType->getOp(),
                ptrType->getValueType(),
                SpvStorageClassImage);
            subscript->setFullType(newPtrType);

            // HACK: assumes the image operand is a load and replace it with
            // the pointer to satisfy SPIRV requirements.
            // We should consider changing the front-end to pass `this` by ref
            // for the __ref accessor so we will be guaranteed to have a pointer
            // image operand here.
            auto image = subscript->getImage();
            if (auto load = as<IRLoad>(image))
                subscript->setOperand(0, load->getPtr());

            addUsersToWorkList(subscript);
        }
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

    void maybeHoistConstructInstToGlobalScope(IRInst* inst)
    {
        // If all of the operands to this instruction are global, we can hoist
        // this constructor to be a global too. This is important to make sure
        // that vectors made of constant components end up being emitted as
        // constant vectors (using OpConstantComposite).
        UIndex opIndex = 0;
        for (auto operand = inst->getOperands(); opIndex < inst->getOperandCount(); operand++, opIndex++)
            if (operand->get()->getParent() != m_module->getModuleInst())
                return;
        inst->insertAtEnd(m_module->getModuleInst());
    }

    void processConstructor(IRInst* inst)
    {
        maybeHoistConstructInstToGlobalScope(inst);

        if (inst->getOp() == kIROp_MakeVector
            && inst->getParent()->getOp() == kIROp_Module
            && inst->getOperandCount() != (UInt)getIntVal(as<IRVectorType>(inst->getDataType())->getElementCount()))
        {
            // SPIRV's OpConstantComposite inst requires the number of operands to
            // exactly match the number of elements of the composite, so the general
            // form of vector construction will not work, and we need to convert it.
            //
            List<IRInst*> args;
            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                auto operand = inst->getOperand(i);
                if (auto operandVecType = as<IRVectorType>(operand->getDataType()))
                {
                    auto operandVecSize = getIntVal(operandVecType->getElementCount());
                    for (IRIntegerValue j = 0; j < operandVecSize; j++)
                    {
                        args.add(builder.emitElementExtract(operand, j));
                    }
                }
                else
                {
                    args.add(operand);
                }
            }
            auto newMakeVector = builder.emitMakeVector(inst->getDataType(), args);
            inst->replaceUsesWith(newMakeVector);
        }
    }

    static bool isAsmInst(IRInst* inst)
    {
        return (as<IRSPIRVAsmInst>(inst) || as<IRSPIRVAsmOperand>(inst));
    }

    void processSPIRVAsm(IRSPIRVAsm* inst)
    {
        // Move anything that is not an spirv instruction to the outer parent.
        for (auto child : inst->getModifiableChildren())
        {
            if (!isAsmInst(child))
                child->insertBefore(inst);
        }
    }

    void legalizeSPIRVEntryPoint(IRFunc* func, IREntryPointDecoration* entryPointDecor)
    {
        auto stage = entryPointDecor->getProfile().getStage();
        switch (stage)
        {
        case Stage::Geometry:
            if (!func->findDecoration<IRInstanceDecoration>())
            {
                IRBuilder builder(func);
                builder.addDecoration(func, kIROp_InstanceDecoration, builder.getIntValue(builder.getUIntType(), 1));
            }
            break;
        case Stage::Compute:
            if (!func->findDecoration<IRNumThreadsDecoration>())
            {
                IRBuilder builder(func);
                auto one = builder.getIntValue(builder.getUIntType(), 1);
                IRInst* args[3] = { one, one, one };
                builder.addDecoration(func, kIROp_NumThreadsDecoration, args, 3);
            }
            break;
        }

    }

    // Opcodes that can exist in global scope, as long as the operands are.
    bool isLegalGlobalInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_MakeStruct:
        case kIROp_MakeArray:
        case kIROp_MakeArrayFromElement:
        case kIROp_MakeVector:
        case kIROp_MakeMatrix:
        case kIROp_MakeMatrixFromScalar:
        case kIROp_MakeVectorFromScalar:
            return true;
        default:
            return false;
        }
    }

    // Opcodes that can be inlined into function bodies.
    bool isInlinableGlobalInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_FRem:
        case kIROp_IRem:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_Not:
        case kIROp_Neg:
        case kIROp_FieldExtract:
        case kIROp_FieldAddress:
        case kIROp_GetElement:
        case kIROp_GetElementPtr:
        case kIROp_UpdateElement:
        case kIROp_MakeTuple:
        case kIROp_GetTupleElement:
        case kIROp_MakeStruct:
        case kIROp_MakeArray:
        case kIROp_MakeArrayFromElement:
        case kIROp_MakeVector:
        case kIROp_MakeMatrix:
        case kIROp_MakeMatrixFromScalar:
        case kIROp_MakeVectorFromScalar:
        case kIROp_swizzle:
        case kIROp_swizzleSet:
        case kIROp_MatrixReshape:
        case kIROp_MakeString:
        case kIROp_MakeResultError:
        case kIROp_MakeResultValue:
        case kIROp_GetResultError:
        case kIROp_GetResultValue:
        case kIROp_CastFloatToInt:
        case kIROp_CastIntToFloat:
        case kIROp_CastIntToPtr:
        case kIROp_CastPtrToBool:
        case kIROp_CastPtrToInt:
        case kIROp_BitAnd:
        case kIROp_BitNot:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_BitCast:
        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_Greater:
        case kIROp_Less:
        case kIROp_Geq:
        case kIROp_Leq:
        case kIROp_Neq:
        case kIROp_Eql:
        case kIROp_Call:
            return true;
        default:
            return false;
        }
    }

    bool shouldInlineInst(IRInst* inst)
    {
        if (!isInlinableGlobalInst(inst))
            return false;
        if (isLegalGlobalInst(inst))
        {
            for (UInt i = 0; i < inst->getOperandCount(); i++)
                if (shouldInlineInst(inst->getOperand(i)))
                    return true;
            return false;
        }
        return true;
    }

    /// Inline `inst` in the local function body so they can be emitted as a local inst.
    ///
    IRInst* maybeInlineGlobalValue(IRBuilder& builder, IRInst* inst, IRCloneEnv& cloneEnv)
    {
        if (!shouldInlineInst(inst))
        {
            switch (inst->getOp())
            {
            case kIROp_Func:
            case kIROp_Specialize:
            case kIROp_Generic:
            case kIROp_LookupWitness:
                return inst;
            }
            if (as<IRType>(inst))
                return inst;

            // If we encounter a global value that shouldn't be inlined, e.g. a const literal,
            // we should insert a GlobalValueRef() inst to wrap around it, so all the dependent uses
            // can be pinned to the function body.
            auto result = builder.emitGlobalValueRef(inst);
            cloneEnv.mapOldValToNew[inst] = result;
            return result;
        }
        
        // If the global value is inlinable, we make all its operands avaialble locally, and then copy it
        // to the local scope.
        ShortList<IRInst*> args;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto operand = inst->getOperand(i);
            auto inlinedOperand = maybeInlineGlobalValue(builder, operand, cloneEnv);
            args.add(inlinedOperand);
        }
        auto result = cloneInst(&cloneEnv, &builder, inst);
        cloneEnv.mapOldValToNew[inst] = result;
        return result;
    }

    void processWorkList()
    {

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
            case kIROp_ImageSubscript:
                processImageSubscript(as<IRImageSubscript>(inst));
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
            case kIROp_SPIRVAsm:
                processSPIRVAsm(as<IRSPIRVAsm>(inst));
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

    void setInsertBeforeOutsideASM(IRBuilder& builder, IRInst* beforeInst)
    {
        auto parent = beforeInst->getParent();
        while (parent)
        {
            if (as<IRSPIRVAsm>(parent))
            {
                builder.setInsertBefore(parent);
                return;
            }
            parent = parent->getParent();
        }
        builder.setInsertBefore(beforeInst);
    }

    void processModule()
    {
        convertCompositeTypeParametersToPointers(m_module);

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
        processWorkList();

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

        List<IRUse*> globalInstUsesToInline;

        for (auto globalInst : m_module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(globalInst))
            {
                if (auto entryPointDecor = func->findDecoration<IREntryPointDecoration>())
                {
                    legalizeSPIRVEntryPoint(func, entryPointDecor);
                }
                // SPIRV requires a dominator block to appear before dominated blocks.
                // After legalizing the control flow, we need to sort our blocks to ensure this is true.
                sortBlocksInFunc(func);
            }

            if (isInlinableGlobalInst(globalInst))
            {
                for (auto use = globalInst->firstUse; use; use = use->nextUse)
                {
                    if (getParentFunc(use->getUser()) != nullptr)
                        globalInstUsesToInline.add(use);
                }
            }
        }

        for (auto use : globalInstUsesToInline)
        {
            auto user = use->getUser();
            IRBuilder builder(user);
            setInsertBeforeOutsideASM(builder, user);
            IRCloneEnv cloneEnv;
            auto val = maybeInlineGlobalValue(builder, use->get(), cloneEnv);
            if (val != use->get())
                builder.replaceOperand(use, val);
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

void buildEntryPointReferenceGraph(SPIRVEmitSharedContext* context, IRModule* module)
{
    struct WorkItem
    {
        IRFunc* entryPoint; IRInst* inst; 
    
        HashCode getHashCode() const
        {
            return combineHash(Slang::getHashCode(entryPoint), Slang::getHashCode(inst));
        }
        bool operator == (const WorkItem& other) const
        {
            return entryPoint == other.entryPoint && inst == other.inst;
        }
    };
    HashSet<WorkItem> workListSet;
    List<WorkItem> workList;
    auto addToWorkList = [&](WorkItem item)
    {
        if (workListSet.add(item))
            workList.add(item);
    };

    auto registerEntryPointReference = [&](IRFunc* entryPoint, IRInst* inst)
        {
            if (auto set = context->m_referencingEntryPoints.tryGetValue(inst))
                set->add(entryPoint);
            else
            {
                HashSet<IRFunc*> newSet;
                newSet.add(entryPoint);
                context->m_referencingEntryPoints.add(inst, _Move(newSet));
            }
        };
    auto visit = [&](IRFunc* entryPoint, IRInst* inst)
        {
            if (auto code = as<IRGlobalValueWithCode>(inst))
            {
                registerEntryPointReference(entryPoint, inst);
                for (auto child : code->getChildren())
                {
                    addToWorkList({ entryPoint, child });
                }
                return;
            }
            switch (inst->getOp())
            {
            case kIROp_GlobalParam:
            case kIROp_SPIRVAsmOperandBuiltinVar:
                registerEntryPointReference(entryPoint, inst);
                break;
            case kIROp_Block:
            case kIROp_SPIRVAsm:
                for (auto child : inst->getChildren())
                {
                    addToWorkList({ entryPoint, child });
                }
                break;
            case kIROp_Call:
                {
                    auto call = as<IRCall>(inst);
                    addToWorkList({ entryPoint, call->getCallee() });
                }
                break;
            case kIROp_SPIRVAsmOperandInst:
                {
                    auto operand = as<IRSPIRVAsmOperandInst>(inst);
                    addToWorkList({ entryPoint, operand->getValue() });
                }
                break;
            }
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                auto operand = inst->getOperand(i);
                switch (operand->getOp())
                {
                case kIROp_GlobalParam:
                case kIROp_GlobalVar:
                case kIROp_SPIRVAsmOperandBuiltinVar:
                    addToWorkList({ entryPoint, operand });
                    break;
                }
            }
        };

    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->getOp() == kIROp_Func && globalInst->findDecoration<IREntryPointDecoration>())
        {
            visit(as<IRFunc>(globalInst), globalInst);
        }
    }
    for (Index i = 0; i < workList.getCount(); i++)
        visit(workList[i].entryPoint, workList[i].inst);
}

void simplifyIRForSpirvLegalization(DiagnosticSink* sink, IRModule* module)
{
    bool changed = true;
    const int kMaxIterations = 8;
    const int kMaxFuncIterations = 16;
    int iterationCounter = 0;

    while (changed && iterationCounter < kMaxIterations)
    {
        if (sink && sink->getErrorCount())
            break;

        changed = false;

        changed |= applySparseConditionalConstantPropagationForGlobalScope(module, sink);
        changed |= peepholeOptimizeGlobalScope(module);

        for (auto inst : module->getGlobalInsts())
        {
            auto func = as<IRGlobalValueWithCode>(inst);
            if (!func)
                continue;
            bool funcChanged = true;
            int funcIterationCount = 0;
            while (funcChanged && funcIterationCount < kMaxFuncIterations)
            {
                funcChanged = false;
                funcChanged |= applySparseConditionalConstantPropagation(func, sink);
                funcChanged |= peepholeOptimize(func);
                funcChanged |= removeRedundancyInFunc(func);
                funcChanged |= simplifyCFG(func, CFGSimplificationOptions::getFast());
                eliminateDeadCode(func);
            }
        }
    }
}

void legalizeIRForSPIRV(
    SPIRVEmitSharedContext* context,
    IRModule* module,
    const List<IRFunc*>& entryPoints,
    CodeGenContext* codeGenContext)
{
    SLANG_UNUSED(entryPoints);
    legalizeSPIRV(context, module);
    simplifyIRForSpirvLegalization(codeGenContext->getSink(), module);
    buildEntryPointReferenceGraph(context, module);
}

} // namespace Slang
