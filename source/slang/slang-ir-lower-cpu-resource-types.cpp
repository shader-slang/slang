// slang-ir-lower-cpu-resource-types.cpp

#include "slang-ir-lower-cpu-resource-types.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-layout.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir.h"

// This pass lowers resource types and the instructions accessing them into
// concrete types. The lowering rules are outlined below.
//
// ByteAddressBuffer/RWByteAddressBuffer/StructuredBuffer/RWStructuredBuffer:
//
//     struct LoweredType {
//         T* data;
//         size_t size;
//     }
//
//     Instructions that access `LoweredType` are lowered immediately in this
//     pass such that they index `data` as needed.
//
// ConstantBufferType/ParameterBlockType:
//
//     typealias LoweredType = T*;
//
//     Instructions that access `LoweredType` are lowered immediately in this
//     pass such that they access members through the pointer as needed.
//
// All other resource types (Textures, Samplers, RaytracingAccelerationStructures):
//
//     typealias LoweredType = void*;
//
//     These types are generally accessed only via functions defined in the
//     core module using inline assembly. Once support for these operations on
//     the LLVM target is added, the inline assembly should expect these types
//     to be pointers in the LLVM IR.
//
//     This pass itself doesn't deal with such function calls, they should
//     eventually be fully handled in the core module and the LLVM emitter.

namespace Slang
{

struct ResourceTypeLoweringContext : InstPassBase
{
    DiagnosticSink* diagnosticSink;
    CodeGenContext* codeGenContext;
    Dictionary<IRType*, IRType*> loweredResourceTypes;

    ResourceTypeLoweringContext(CodeGenContext* codeGenContext, IRModule* inModule)
        : InstPassBase(inModule), codeGenContext(codeGenContext)
    {
    }

    void lowerType(IRBuilder& builder, IRType* type)
    {
        if (loweredResourceTypes.containsKey(type))
            return;

        codeGenContext->getTargetProgram();

        IRType* loweredType = nullptr;
        switch (type->getOp())
        {
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
            {
                auto layoutType =
                    getTypeLayoutTypeForBuffer(codeGenContext->getTargetProgram(), builder, type);
                loweredType = builder.getPtrType(
                    as<IRType>(type->getOperand(0)),
                    AccessQualifier::ReadWrite,
                    AddressSpace::Generic,
                    layoutType);
            }
            break;
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
            {
                auto bufferType = as<IRHLSLStructuredBufferTypeBase>(type);
                auto layoutType =
                    getTypeLayoutTypeForBuffer(codeGenContext->getTargetProgram(), builder, type);

                IRStructType* s = builder.createStructType();
                auto ptrKey = builder.createStructKey();
                auto sizeKey = builder.createStructKey();
                auto ptrType = builder.getPtrType(
                    bufferType->getElementType(),
                    AccessQualifier::ReadWrite,
                    AddressSpace::Generic,
                    layoutType);
                builder.createStructField(s, ptrKey, ptrType);
                builder.createStructField(s, sizeKey, builder.getType(kIROp_UIntPtrType));
                loweredType = s;
            }
            break;
        case kIROp_HLSLByteAddressBufferType:
        case kIROp_HLSLRWByteAddressBufferType:
            {
                auto layoutType =
                    getTypeLayoutTypeForBuffer(codeGenContext->getTargetProgram(), builder, type);

                IRStructType* s = builder.createStructType();
                auto ptrKey = builder.createStructKey();
                auto sizeKey = builder.createStructKey();
                auto ptrType = builder.getPtrType(
                    builder.getUInt8Type(),
                    AccessQualifier::ReadWrite,
                    AddressSpace::Generic,
                    layoutType);
                builder.createStructField(s, ptrKey, ptrType);
                builder.createStructField(s, sizeKey, builder.getType(kIROp_UIntPtrType));
                loweredType = s;
            }
            break;
        case kIROp_TextureType:
        case kIROp_SamplerStateType:
        case kIROp_RaytracingAccelerationStructureType:
        case kIROp_RayQueryType:
            {
                loweredType = builder.getPtrType(builder.getVoidType());
            }
            break;
        default:
            break;
        }

        if (loweredType)
            loweredResourceTypes[type] = loweredType;
    }

    IRInst* getBufferPtr(IRBuilder& builder, IRInst* buffer)
    {
        auto structType = cast<IRStructType>(buffer->getDataType());
        return builder.emitFieldExtract(buffer, structType->getFields().getFirst()->getKey());
    }

    IRInst* getBufferSize(IRBuilder& builder, IRInst* buffer)
    {
        auto structType = cast<IRStructType>(buffer->getDataType());
        return builder.emitFieldExtract(buffer, structType->getFields().getLast()->getKey());
    }

    void processInst(IRBuilder& builder, IRInst* inst)
    {
        IRInst* loweredInst = nullptr;
        builder.setInsertBefore(inst);
        switch (inst->getOp())
        {
        case kIROp_RWStructuredBufferGetElementPtr:
            {
                auto gepInst = static_cast<IRRWStructuredBufferGetElementPtr*>(inst);
                auto index = gepInst->getIndex();
                auto ptr = getBufferPtr(builder, gepInst->getBase());
                loweredInst = builder.emitGetOffsetPtr(ptr, index);
            }
            break;
        case kIROp_StructuredBufferLoad:
        case kIROp_RWStructuredBufferLoad:
            {
                auto base = inst->getOperand(0);
                auto index = inst->getOperand(1);
                auto ptr = getBufferPtr(builder, base);
                auto offsetPtr = builder.emitGetOffsetPtr(ptr, index);
                loweredInst = builder.emitLoad(offsetPtr);
            }
            break;
        case kIROp_RWStructuredBufferStore:
            {
                auto base = inst->getOperand(0);
                auto index = inst->getOperand(1);
                auto val = inst->getOperand(2);
                auto ptr = getBufferPtr(builder, base);
                auto offsetPtr = builder.emitGetOffsetPtr(ptr, index);
                loweredInst = builder.emitStore(offsetPtr, val);
            }
            break;
        case kIROp_ByteAddressBufferLoad:
            {
                auto base = inst->getOperand(0);
                auto index = inst->getOperand(1);
                auto ptr = getBufferPtr(builder, base);
                auto offsetPtr = builder.emitGetOffsetPtr(ptr, index);
                auto typedPtr =
                    builder.emitCast(builder.getPtrType(inst->getDataType()), offsetPtr);
                loweredInst = builder.emitLoad(inst->getDataType(), typedPtr);
            }
            break;
        case kIROp_ByteAddressBufferStore:
            {
                auto base = inst->getOperand(0);
                auto index = inst->getOperand(1);
                auto val = inst->getOperand(inst->getOperandCount() - 1);
                auto ptr = getBufferPtr(builder, base);
                auto offsetPtr = builder.emitGetOffsetPtr(ptr, index);
                auto typedPtr = builder.emitCast(builder.getPtrType(val->getDataType()), offsetPtr);
                loweredInst = builder.emitStore(typedPtr, val);
            }
            break;

        case kIROp_StructuredBufferGetDimensions:
            {
                auto getDimensionsInst = cast<IRStructuredBufferGetDimensions>(inst);
                auto buffer = getDimensionsInst->getBuffer();
                auto ptr = getBufferPtr(builder, buffer);
                auto size = getBufferSize(builder, buffer);
                auto intType = builder.getIntType();
                auto vecType = builder.getVectorType(intType, 2);

                auto rules = getTypeLayoutRuleForBuffer(
                    codeGenContext->getTargetProgram(),
                    ptr->getDataType());
                IRSizeAndAlignment sizeAlignment;
                Slang::getSizeAndAlignment(
                    codeGenContext->getTargetReq(),
                    rules,
                    cast<IRPtrType>(ptr->getDataType())->getValueType(),
                    &sizeAlignment);

                loweredInst = builder.emitMakeVector(
                    vecType,
                    {builder.emitCast(intType, size),
                     builder.getIntValue(sizeAlignment.getStride())});
            }
            break;

        case kIROp_GetEquivalentStructuredBuffer:
            {
                auto bufferType = inst->getDataType();
                auto byteBuffer = inst->getOperand(0);

                auto structType = cast<IRStructType>(bufferType);
                auto ptrType = as<IRPtrType>(structType->getFields().getFirst()->getFieldType());
                auto rules =
                    getTypeLayoutRuleForBuffer(codeGenContext->getTargetProgram(), ptrType);
                auto elementType = ptrType->getValueType();

                IRSizeAndAlignment sizeAlignment;
                Slang::getSizeAndAlignment(
                    codeGenContext->getTargetReq(),
                    rules,
                    elementType,
                    &sizeAlignment);

                auto ptr = getBufferPtr(builder, byteBuffer);
                auto bytes = getBufferSize(builder, byteBuffer);

                auto size = builder.emitDiv(
                    builder.getIntPtrType(),
                    bytes,
                    builder.getIntValue(builder.getIntPtrType(), sizeAlignment.getStride()));

                loweredInst =
                    builder.emitMakeStruct(structType, {builder.emitCast(ptrType, ptr), size});
            }
            break;
        }

        if (loweredInst)
        {
            inst->replaceUsesWith(loweredInst);
            inst->removeAndDeallocate();
        }
    }

    void processModule()
    {
        IRBuilder builder(module);

        processAllInsts(
            [&](IRInst* inst)
            {
                if (IRType* type = as<IRType>(inst))
                    lowerType(builder, type);
            });

        // Replace all resource types with lowered types.
        for (const auto& [type, loweredType] : loweredResourceTypes)
            type->replaceUsesWith(loweredType);

        processAllInsts([&](IRInst* inst) { processInst(builder, inst); });
    }
};

void lowerCPUResourceTypes(IRModule* module, CodeGenContext* codeGenContext)
{
    ResourceTypeLoweringContext context(codeGenContext, module);
    context.diagnosticSink = codeGenContext->getSink();
    context.processModule();
}

} // namespace Slang
