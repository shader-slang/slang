#include "slang-emit-nvvm.h"

#include "compiler-core/slang-artifact-impl.h"
#include "compiler-core/slang-artifact-util.h"
#include "compiler-core/slang-nvvm-semantic-catalog.h"
#include "core/slang-dictionary.h"
#include "core/slang-math.h"
#include "slang-code-gen.h"
#include "slang-diagnostics.h"
#include "slang-emit-nvvm-type-lowering.h"
#include "slang-ir-dce.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
namespace
{

static const uint32_t kNVVMScalar32Alignment = 4;
static const IRIntegerValue kNVVMI32Min = -2147483647 - 1;
static const IRIntegerValue kNVVMI32Max = 2147483647;
static const IRIntegerValue kNVVMUInt32Max = 4294967295;
static const uint32_t kNVVMPointerAlignment = 8;

struct NVVMConventionalGlobalParams
{
    IRGlobalParam* globalParam = nullptr;
    IRStructType* elementType = nullptr;
};

// Recognizes the canonical collected CUDA parameter block. Executable resource fields and opaque
// sampler storage share this block, but sampler values remain unavailable to ordinary IR emission.
bool _getNVVMConventionalGlobalParams(IRInst* inst, NVVMConventionalGlobalParams& outParams)
{
    outParams = {};
    auto globalParam = as<IRGlobalParam>(inst);
    auto constantBufferType =
        globalParam ? as<IRConstantBufferType>(globalParam->getDataType()) : nullptr;
    auto elementType =
        constantBufferType ? as<IRStructType>(constantBufferType->getElementType()) : nullptr;
    if (!globalParam || !elementType ||
        !elementType->findDecoration<IRSynthesizedParameterGroupDecoration>())
    {
        return false;
    }

    bool hasField = false;
    for (auto field : elementType->getFields())
    {
        if (!isNVVMSupportedConventionalGlobalFieldType(field->getFieldType()))
            return false;
        hasField = true;
    }
    if (!hasField)
        return false;

    outParams = {globalParam, elementType};
    return true;
}

// Finds a field by semantic key and returns its actual ABI position. The global collector can move
// CUDA fields, so every aggregate address uses key identity instead of source declaration order.
bool _findNVVMStructField(
    IRStructType* structType,
    IRInst* key,
    IRStructField*& outField,
    uint32_t& outFieldIndex)
{
    outField = nullptr;
    outFieldIndex = 0;
    uint32_t fieldIndex = 0;
    for (auto field : structType->getFields())
    {
        if (field->getKey() == key)
        {
            outField = field;
            outFieldIndex = fieldIndex;
            return true;
        }
        ++fieldIndex;
    }
    return false;
}

// Returns whether a retained struct declaration is an exact storage type owned by the accepted
// conventional CUDA parameter block.
bool _isNVVMConventionalGlobalStorageType(const NVVMConventionalGlobalParams& params, IRInst* inst)
{
    // A raw CUDA kernel can retain by-value struct declarations without having a collected global
    // parameter block. In that case there are no conventional-global storage types to recognize.
    if (!params.elementType)
        return false;

    if (inst == params.elementType)
        return true;
    for (auto field : params.elementType->getFields())
    {
        IRStructType* parameterGroupElementType = nullptr;
        if (asNVVMSupportedScalarParameterGroupType(
                field->getFieldType(),
                &parameterGroupElementType) &&
            inst == parameterGroupElementType)
        {
            return true;
        }
    }
    return false;
}

struct NVVMStructField
{
    IRStructField* field = nullptr;
    uint32_t fieldIndex = 0;
    bool isMutableLocal = false;
};

// Resolves the aggregate-address shapes with executable representations: a field in the collected
// CUDA parameter block, a selected scalar in a loaded parameter group, or a selected numeric field
// in a local copyable struct.
bool _getNVVMStructFieldAddress(IRFieldAddress* fieldAddress, NVVMStructField& outAddress)
{
    outAddress = {};
    if (!fieldAddress)
        return false;

    IRStructType* structType = nullptr;
    NVVMConventionalGlobalParams globalParams;
    const bool isConventionalGlobal =
        _getNVVMConventionalGlobalParams(fieldAddress->getBase(), globalParams);
    if (isConventionalGlobal)
    {
        structType = globalParams.elementType;
    }
    else if (asNVVMSupportedLocalCopyableStructPointerType(
                 fieldAddress->getBase()->getDataType(),
                 &structType))
    {
        outAddress.isMutableLocal = true;
    }
    else if (asNVVMSupportedLocalScalarStructPointerType(
                 fieldAddress->getBase()->getDataType(),
                 &structType))
    {
        // A canonical BorrowInOutParam is not itself a local Ptr, but it shares the exact selected
        // scalar-struct pointee and mutable field contract established for helper parameters.
        outAddress.isMutableLocal = true;
    }
    else if (!asNVVMSupportedScalarParameterGroupType(
                 fieldAddress->getBase()->getDataType(),
                 &structType))
    {
        return false;
    }
    if (!_findNVVMStructField(
            structType,
            fieldAddress->getField(),
            outAddress.field,
            outAddress.fieldIndex))
    {
        return false;
    }

    auto pointerType = as<IRPtrTypeBase>(fieldAddress->getDataType());
    if (!pointerType || !isTypeEqual(outAddress.field->getFieldType(), pointerType->getValueType()))
    {
        return false;
    }

    IRType* fieldType = outAddress.field->getFieldType();
    if (isConventionalGlobal)
    {
        NVVMRawBufferType rawBufferType;
        // Sampler fields are ABI storage only. They intentionally have no executable value form.
        return isNVVMSupportedIntegerScalarType(fieldType) || isNVVMFloat32Type(fieldType) ||
               asNVVMSupportedScalarParameterGroupType(fieldType) ||
               getNVVMSupportedRawBufferType(fieldType, rawBufferType);
    }

    if (outAddress.isMutableLocal)
        return isNVVMSupportedNumericValueType(fieldType);

    return isNVVMSupportedIntegerScalarType(fieldType) || isNVVMFloat32Type(fieldType);
}

// Resolves one scalar field extraction by canonical struct key and verifies its result type.
bool _getNVVMStructFieldValue(IRFieldExtract* fieldExtract, NVVMStructField& outField)
{
    outField = {};
    auto structType = fieldExtract
                          ? asNVVMSupportedScalarStructType(fieldExtract->getBase()->getDataType())
                          : nullptr;
    if (!structType || !_findNVVMStructField(
                           structType,
                           fieldExtract->getField(),
                           outField.field,
                           outField.fieldIndex))
    {
        return false;
    }
    return isTypeEqual(outField.field->getFieldType(), fieldExtract->getDataType());
}

struct NVVMRawBufferDataPointer
{
    IRInst* buffer = nullptr;
    NVVMRawBufferType bufferType;
    NVVMBufferDataPointerType resultType;
};

// Resolves the canonical operation that exposes field zero of an admitted raw buffer view.
bool _getNVVMRawBufferDataPointer(IRInst* inst, NVVMRawBufferDataPointer& outPointer)
{
    outPointer = {};
    if (!inst || inst->getOperandCount() != 1 ||
        (inst->getOp() != kIROp_GetStructuredBufferPtr &&
         inst->getOp() != kIROp_GetUntypedBufferPtr))
    {
        return false;
    }

    IRInst* buffer = inst->getOperand(0);
    NVVMRawBufferType bufferType;
    NVVMBufferDataPointerType resultType;
    if (!buffer || !getNVVMSupportedRawBufferType(buffer->getDataType(), bufferType) ||
        !getNVVMSupportedBufferDataPointerType(inst->getDataType(), resultType) ||
        !isNVVMRawBufferElementType(bufferType, resultType.elementType))
    {
        return false;
    }

    const bool isStructured = bufferType.kind == NVVMRawBufferKind::Structured;
    if ((inst->getOp() == kIROp_GetStructuredBufferPtr) != isStructured)
        return false;

    outPointer.buffer = buffer;
    outPointer.bufferType = bufferType;
    outPointer.resultType = resultType;
    return true;
}

struct NVVMByteAddressAccess
{
    IRInst* buffer = nullptr;
    IRInst* byteOffset = nullptr;
    IRInst* value = nullptr;
    IRType* valueType = nullptr;
    NVVMRawBufferType bufferType;
    uint32_t alignment = 0;
    bool isStore = false;
};

// Resolves the canonical selected numeric scalar/vector byte-address load and store family. A zero
// or omitted alignment carries the ordinary four-byte contract; an explicit alignment is a
// power-of-two promise that can be forwarded unchanged to LLVM.
bool _getNVVMByteAddressAccess(IRInst* inst, NVVMByteAddressAccess& outAccess)
{
    outAccess = {};
    if (!inst)
        return false;

    const bool isLoad = inst->getOp() == kIROp_ByteAddressBufferLoad;
    const bool isStore = inst->getOp() == kIROp_ByteAddressBufferStore;
    if ((!isLoad && !isStore) ||
        (isLoad && inst->getOperandCount() != 2 && inst->getOperandCount() != 3) ||
        (isStore && inst->getOperandCount() != 4))
    {
        return false;
    }

    IRInst* buffer = inst->getOperand(0);
    IRInst* byteOffset = inst->getOperand(1);
    IRInst* alignmentOperand =
        isLoad && inst->getOperandCount() == 2 ? nullptr : inst->getOperand(2);
    IRInst* value = isStore ? inst->getOperand(3) : nullptr;
    IRType* valueType = isStore && value ? value->getDataType() : inst->getDataType();
    NVVMRawBufferType bufferType;
    if (!buffer || !byteOffset || !isNVVMUnsignedI32Type(byteOffset->getDataType()) || !valueType ||
        !isNVVMSupportedByteAddressValueType(valueType) ||
        !getNVVMSupportedRawBufferType(buffer->getDataType(), bufferType) ||
        bufferType.kind != NVVMRawBufferKind::ByteAddress ||
        (isStore && (bufferType.access != NVVMBufferAccess::ReadWrite ||
                     !as<IRVoidType>(inst->getDataType()))))
    {
        return false;
    }

    uint32_t alignment = kNVVMScalar32Alignment;
    if (alignmentOperand)
    {
        auto alignmentLiteral = as<IRIntLit>(alignmentOperand);
        if (!alignmentLiteral || !isNVVMUnsignedI32Type(alignmentLiteral->getDataType()) ||
            alignmentLiteral->getValue() < 0 || alignmentLiteral->getValue() > UINT32_MAX)
        {
            return false;
        }
        const uint32_t literalAlignment = uint32_t(alignmentLiteral->getValue());
        if (literalAlignment)
        {
            if (literalAlignment & (literalAlignment - 1))
                return false;
            alignment = literalAlignment;
        }
    }

    outAccess.buffer = buffer;
    outAccess.byteOffset = byteOffset;
    outAccess.value = value;
    outAccess.valueType = valueType;
    outAccess.bufferType = bufferType;
    outAccess.alignment = alignment;
    outAccess.isStore = isStore;
    return true;
}

struct NVVMRawBufferElementPointer
{
    IRInst* base = nullptr;
    IRInst* index = nullptr;
    IRPtrTypeBase* resultType = nullptr;
};

// Resolves one scalar element address rooted directly in an admitted raw-buffer data pointer.
bool _getNVVMRawBufferElementPointer(IRInst* inst, NVVMRawBufferElementPointer& outPointer)
{
    outPointer = {};
    if (!inst || inst->getOp() != kIROp_GetElementPtr || inst->getOperandCount() != 2)
        return false;

    IRInst* base = inst->getOperand(0);
    IRInst* index = inst->getOperand(1);
    NVVMRawBufferDataPointer baseProducer;
    auto resultType = asNVVMSupportedDeviceScalarPointerType(inst->getDataType());
    IRType* resultLayout = resultType ? resultType->getDataLayout() : nullptr;
    if (!base || !index || !_getNVVMRawBufferDataPointer(base, baseProducer) || !resultType ||
        resultType->getOperandCount() != 4 || !resultLayout ||
        resultLayout->getOp() != kIROp_ScalarBufferLayoutType ||
        !isTypeEqual(resultType->getValueType(), baseProducer.resultType.elementType) ||
        resultType->getAccessQualifier() !=
            baseProducer.resultType.pointerType->getAccessQualifier() ||
        resultType->getAddressSpace() != baseProducer.resultType.pointerType->getAddressSpace() ||
        !isNVVMInteger32Type(index->getDataType()))
    {
        return false;
    }

    outPointer.base = base;
    outPointer.index = index;
    outPointer.resultType = resultType;
    return true;
}

// Gets the natural CUDA alignment carried by one physical LLVM `byval` entry parameter.
bool _getNVVMByValueParameterAlignment(
    CodeGenContext* codeGenContext,
    IRType* type,
    uint32_t& outAlignment)
{
    outAlignment = 0;
    if (!codeGenContext || !asNVVMSupportedScalarStructType(type))
        return false;

    IRSizeAndAlignment layout;
    if (SLANG_FAILED(getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getCUDA(),
            type,
            &layout)) ||
        layout.alignment <= 0 || layout.alignment > UINT32_MAX)
    {
        return false;
    }
    outAlignment = uint32_t(layout.alignment);
    return true;
}

// Verifies that a copyable struct can use one unpadded LLVM struct for local and raw-buffer
// storage. Consider `Thing { uint pos; float radius; half4 color; }`: CUDA and LLVM give its fields
// offsets 0, 4, and 8 and the same 16-byte stride, even though their preferred aggregate alignment
// differs. Matching offsets and size are the actual memory contract; a mismatch must be handled by
// a future layout-lowering slice rather than by silently indexing a different LLVM representation.
bool _hasNVVMCompatibleCopyableStructLayout(CodeGenContext* codeGenContext, IRStructType* type)
{
    if (!codeGenContext || !asNVVMSupportedCopyableStructType(type))
        return false;

    IRSizeAndAlignment cudaLayout;
    IRSizeAndAlignment llvmLayout;
    if (SLANG_FAILED(getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getCUDA(),
            type,
            &cudaLayout)) ||
        SLANG_FAILED(getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getLLVM(),
            type,
            &llvmLayout)) ||
        cudaLayout.size <= 0 || cudaLayout.size != llvmLayout.size)
    {
        return false;
    }

    for (auto field : type->getFields())
    {
        IRIntegerValue cudaOffset = 0;
        IRIntegerValue llvmOffset = 0;
        if (SLANG_FAILED(getOffset(
                codeGenContext->getTargetReq(),
                IRTypeLayoutRules::getCUDA(),
                field,
                &cudaOffset)) ||
            SLANG_FAILED(getOffset(
                codeGenContext->getTargetReq(),
                IRTypeLayoutRules::getLLVM(),
                field,
                &llvmOffset)) ||
            cudaOffset < 0 || cudaOffset != llvmOffset)
        {
            return false;
        }
    }
    return true;
}

// Maps a canonical global produced by CUDA varying legalization to its semantic provider operation.
bool _getNVVMCUDAExecutionGlobalOperation(IRInst* inst, SlangNVVMValueOperation& outOperation)
{
    outOperation = 0;
    auto globalParam = as<IRGlobalParam>(inst);
    auto targetIntrinsic =
        globalParam ? globalParam->findDecoration<IRTargetIntrinsicDecoration>() : nullptr;
    bool isSigned = false;
    uint32_t elementCount = 0;
    if (!globalParam || !targetIntrinsic ||
        !asNVVMSupportedI32VectorType(globalParam->getDataType(), &isSigned, &elementCount) ||
        isSigned || elementCount != 3)
        return false;

    const UnownedStringSlice definition = targetIntrinsic->getDefinition();
    if (definition == toSlice("threadIdx"))
        outOperation = SLANG_NVVM_VALUE_OP_THREAD_INDEX;
    else if (definition == toSlice("blockIdx"))
        outOperation = SLANG_NVVM_VALUE_OP_BLOCK_INDEX;
    else if (definition == toSlice("blockDim"))
        outOperation = SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS;
    else if (definition == toSlice("gridDim"))
        outOperation = SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS;
    else
        return false;
    return true;
}

// Builds the complete typed descriptor shared by execution-global preflight and emission.
SlangNVVMValueOperationDesc _getNVVMCUDAExecutionGlobalOperationDesc(
    SlangNVVMValueOperation operation)
{
    return {operation, NVVMSemantics::kUnsignedI32x3, nullptr, 0};
}

IRIntLit* _asExecutableInteger32Constant(IRInst* value);

struct NVVMVectorElement
{
    IRInst* base = nullptr;
    IRInst* index = nullptr;
};

// Resolves an integer-indexed scalar read from one accepted ordinary value vector. Constant
// indices are checked here; a dynamic index retains the source IR value for provider lowering.
bool _getNVVMVectorElement(IRInst* inst, NVVMVectorElement& outElement)
{
    outElement = {};

    IRInst* base = nullptr;
    IRInst* elementIndex = nullptr;
    if (auto swizzle = as<IRSwizzle>(inst))
    {
        if (swizzle->getElementCount() != 1)
            return false;
        base = swizzle->getBase();
        elementIndex = swizzle->getElementIndex(0);
    }
    else if (auto getElement = as<IRGetElement>(inst))
    {
        base = getElement->getBase();
        elementIndex = getElement->getIndex();
    }
    else
    {
        return false;
    }

    uint32_t baseElementCount = 0;
    auto baseType =
        asNVVMSupportedValueVectorType(base ? base->getDataType() : nullptr, &baseElementCount);
    if (!baseType || !isTypeEqual(inst->getDataType(), baseType->getElementType()) ||
        !elementIndex || !isNVVMSupportedIntegerScalarType(elementIndex->getDataType()))
    {
        return false;
    }
    if (auto constantIndex = _asExecutableInteger32Constant(elementIndex))
    {
        if (constantIndex->getValue() < 0 || constantIndex->getValue() >= baseElementCount)
            return false;
    }

    outElement.base = base;
    outElement.index = elementIndex;
    return true;
}

struct NVVMVectorConstructElement
{
    IRInst* value = nullptr;
    IRInst* extractedBase = nullptr;
    uint32_t extractedIndex = 0;
};

struct NVVMVectorConstruction
{
    IRVectorType* resultType = nullptr;
    NVVMVectorConstructElement elements[4];
    uint32_t elementCount = 0;
};

// Resolves the canonical flat constructor, scalar splat, or multi-lane swizzle of one accepted
// ordinary value vector. Every output lane retains its exact scalar value or base/index source.
bool _getNVVMVectorConstruction(IRInst* inst, NVVMVectorConstruction& outConstruction)
{
    outConstruction = {};
    uint32_t elementCount = 0;
    auto resultType =
        asNVVMSupportedValueVectorType(inst ? inst->getDataType() : nullptr, &elementCount);
    if (!resultType)
        return false;

    if (inst->getOp() == kIROp_MakeVector)
    {
        if (inst->getOperandCount() != elementCount)
            return false;
        for (uint32_t i = 0; i < elementCount; ++i)
        {
            IRInst* element = inst->getOperand(i);
            if (!element || !isTypeEqual(element->getDataType(), resultType->getElementType()))
                return false;
            outConstruction.elements[i].value = element;
        }
    }
    else if (inst->getOp() == kIROp_MakeVectorFromScalar)
    {
        if (inst->getOperandCount() != 1 || !inst->getOperand(0) ||
            !isTypeEqual(inst->getOperand(0)->getDataType(), resultType->getElementType()))
        {
            return false;
        }
        for (uint32_t i = 0; i < elementCount; ++i)
            outConstruction.elements[i].value = inst->getOperand(0);
    }
    else if (auto swizzle = as<IRSwizzle>(inst))
    {
        IRInst* base = swizzle->getBase();
        uint32_t baseElementCount = 0;
        auto baseType =
            asNVVMSupportedValueVectorType(base ? base->getDataType() : nullptr, &baseElementCount);
        if (!baseType || swizzle->getElementCount() != elementCount ||
            !isTypeEqual(baseType->getElementType(), resultType->getElementType()))
        {
            return false;
        }
        for (uint32_t i = 0; i < elementCount; ++i)
        {
            auto index = _asExecutableInteger32Constant(swizzle->getElementIndex(i));
            if (!index || index->getValue() < 0 || index->getValue() >= baseElementCount)
                return false;
            outConstruction.elements[i].extractedBase = base;
            outConstruction.elements[i].extractedIndex = uint32_t(index->getValue());
        }
    }
    else if (auto swizzleSet = as<IRSwizzleSet>(inst))
    {
        IRInst* base = swizzleSet->getBase();
        IRInst* source = swizzleSet->getSource();
        uint32_t baseElementCount = 0;
        auto baseType =
            asNVVMSupportedValueVectorType(base ? base->getDataType() : nullptr, &baseElementCount);
        const uint32_t sourceElementCount = uint32_t(swizzleSet->getElementCount());
        if (!baseType || !isTypeEqual(baseType, resultType) || !source || sourceElementCount == 0 ||
            sourceElementCount > elementCount)
        {
            return false;
        }

        IRVectorType* sourceType = nullptr;
        if (sourceElementCount == 1)
        {
            if (!isTypeEqual(source->getDataType(), resultType->getElementType()))
                return false;
        }
        else
        {
            uint32_t actualSourceElementCount = 0;
            sourceType =
                asNVVMSupportedValueVectorType(source->getDataType(), &actualSourceElementCount);
            if (!sourceType || actualSourceElementCount != sourceElementCount ||
                !isTypeEqual(sourceType->getElementType(), resultType->getElementType()))
            {
                return false;
            }
        }

        for (uint32_t i = 0; i < elementCount; ++i)
        {
            outConstruction.elements[i].extractedBase = base;
            outConstruction.elements[i].extractedIndex = i;
        }

        uint32_t updatedLaneMask = 0;
        for (uint32_t sourceIndex = 0; sourceIndex < sourceElementCount; ++sourceIndex)
        {
            auto destinationIndex =
                _asExecutableInteger32Constant(swizzleSet->getElementIndex(sourceIndex));
            if (!destinationIndex || destinationIndex->getValue() < 0 ||
                destinationIndex->getValue() >= elementCount)
            {
                return false;
            }
            const uint32_t destinationLane = uint32_t(destinationIndex->getValue());
            const uint32_t laneMask = 1u << destinationLane;
            if (updatedLaneMask & laneMask)
                return false;
            updatedLaneMask |= laneMask;

            NVVMVectorConstructElement& destination = outConstruction.elements[destinationLane];
            if (sourceElementCount == 1)
            {
                destination.value = source;
                destination.extractedBase = nullptr;
            }
            else
            {
                destination.extractedBase = source;
                destination.extractedIndex = sourceIndex;
            }
        }
    }
    else
    {
        return false;
    }

    outConstruction.resultType = resultType;
    outConstruction.elementCount = elementCount;
    return true;
}

struct NVVMAggregateConstruction
{
    IRArrayType* resultType = nullptr;
    uint32_t elementCount = 0;
};

// Resolves a canonical fixed-array value whose complete ordered element sequence is explicit in
// final IR. Matrix legalization is one producer of this shape, but the provider operation remains
// aggregate-generic and does not recover matrix semantics.
bool _getNVVMAggregateConstruction(IRInst* inst, NVVMAggregateConstruction& outConstruction)
{
    outConstruction = {};
    uint32_t elementCount = 0;
    auto resultType =
        asNVVMSupportedNumericArrayType(inst ? inst->getDataType() : nullptr, &elementCount);
    if (!resultType || inst->getOp() != kIROp_MakeArray || inst->getOperandCount() != elementCount)
    {
        return false;
    }
    for (uint32_t i = 0; i < elementCount; ++i)
    {
        IRInst* element = inst->getOperand(i);
        if (!element || !isTypeEqual(element->getDataType(), resultType->getElementType()))
            return false;
    }
    outConstruction.resultType = resultType;
    outConstruction.elementCount = elementCount;
    return true;
}

struct NVVMAggregateElement
{
    IRInst* base = nullptr;
    IRArrayType* baseType = nullptr;
    uint32_t index = 0;
};

// Resolves one statically selected element from a canonical fixed-array value. LLVM aggregate
// extraction is structurally indexed, so dynamic source indexing remains outside this contract.
bool _getNVVMAggregateElement(IRInst* inst, NVVMAggregateElement& outElement)
{
    outElement = {};
    auto getElement = as<IRGetElement>(inst);
    IRInst* base = getElement ? getElement->getBase() : nullptr;
    uint32_t elementCount = 0;
    auto baseType =
        asNVVMSupportedNumericArrayType(base ? base->getDataType() : nullptr, &elementCount);
    auto index = getElement ? _asExecutableInteger32Constant(getElement->getIndex()) : nullptr;
    if (!baseType || !isTypeEqual(inst->getDataType(), baseType->getElementType()) || !index ||
        index->getValue() < 0 || index->getValue() >= elementCount)
    {
        return false;
    }
    outElement.base = base;
    outElement.baseType = baseType;
    outElement.index = uint32_t(index->getValue());
    return true;
}

struct NVVMVectorSwizzledStore
{
    IRInst* destination = nullptr;
    IRInst* source = nullptr;
    IRVectorType* destinationType = nullptr;
    IRType* elementType = nullptr;
    uint32_t sourceElementCount = 0;
    uint32_t destinationIndices[4] = {};
};

// Resolves the canonical constant-lane store to an accepted RWStructuredBuffer vector element.
// Final IR owns the exact destination mapping, so emission can consume it without reconstructing
// the source l-value swizzle.
bool _getNVVMVectorSwizzledStore(IRInst* inst, NVVMVectorSwizzledStore& outStore)
{
    outStore = {};

    auto swizzledStore = as<IRSwizzledStore>(inst);
    if (!swizzledStore || swizzledStore->getOperandCount() < 3)
        return false;

    IRInst* destination = swizzledStore->getOperand(0);
    IRInst* source = swizzledStore->getOperand(1);
    auto destinationPointerType =
        destination
            ? asNVVMSupportedRWStructuredBufferElementPointerType(destination->getDataType())
            : nullptr;
    uint32_t destinationElementCount = 0;
    auto destinationType = destinationPointerType ? asNVVMSupported32BitNumericVectorType(
                                                        destinationPointerType->getValueType(),
                                                        &destinationElementCount)
                                                  : nullptr;
    const uint32_t sourceElementCount = uint32_t(swizzledStore->getElementCount());
    if (!destinationType || !source || sourceElementCount == 0 ||
        sourceElementCount > destinationElementCount)
    {
        return false;
    }

    IRType* elementType = destinationType->getElementType();
    if (sourceElementCount == 1)
    {
        if (!isTypeEqual(source->getDataType(), elementType))
            return false;
    }
    else
    {
        uint32_t sourceVectorElementCount = 0;
        auto sourceType =
            asNVVMSupported32BitNumericVectorType(source->getDataType(), &sourceVectorElementCount);
        if (!sourceType || sourceVectorElementCount != sourceElementCount ||
            !isTypeEqual(sourceType->getElementType(), elementType))
        {
            return false;
        }
    }

    uint32_t usedDestinationLanes = 0;
    for (uint32_t sourceIndex = 0; sourceIndex < sourceElementCount; ++sourceIndex)
    {
        auto destinationIndex =
            _asExecutableInteger32Constant(swizzledStore->getElementIndex(sourceIndex));
        if (!destinationIndex || destinationIndex->getValue() < 0 ||
            destinationIndex->getValue() >= destinationElementCount)
        {
            return false;
        }
        const uint32_t lane = uint32_t(destinationIndex->getValue());
        const uint32_t laneMask = 1u << lane;
        if (usedDestinationLanes & laneMask)
            return false;
        usedDestinationLanes |= laneMask;
        outStore.destinationIndices[sourceIndex] = lane;
    }

    outStore.destination = destination;
    outStore.source = source;
    outStore.destinationType = destinationType;
    outStore.elementType = elementType;
    outStore.sourceElementCount = sourceElementCount;
    return true;
}

struct ScopedNVVMModule
{
    const NVVMIRBuilder* builder = nullptr;
    SlangNVVMModuleHandle module = nullptr;

    ~ScopedNVVMModule()
    {
        if (builder && module)
            builder->destroyModule(module);
    }
};

SlangResult _diagnoseUnsupportedIR(
    CodeGenContext* codeGenContext,
    const UnownedStringSlice& construct)
{
    codeGenContext->getSink()->diagnose(
        Diagnostics::NvvmUnsupportedIr{.construct = String(construct)});
    return SLANG_E_NOT_IMPLEMENTED;
}

SlangResult _requireBuilderOperation(
    CodeGenContext* codeGenContext,
    const char* operation,
    SlangResult result)
{
    if (SLANG_SUCCEEDED(result))
        return result;

    codeGenContext->getSink()->diagnose(Diagnostics::NvvmIrBuilderOperationFailed{
        .operation = String(operation),
        .resultCode = result,
    });
    return result;
}

// Returns an executable signed-i32 literal, excluding layout and other module constants.
IRIntLit* _asExecutableI32Constant(IRInst* value)
{
    auto intLit = as<IRIntLit>(value);
    if (!intLit || !isNVVMSignedI32Type(intLit->getDataType()))
        return nullptr;

    const IRIntegerValue intValue = intLit->getValue();
    return intValue >= kNVVMI32Min && intValue <= kNVVMI32Max ? intLit : nullptr;
}

// Returns an executable signed or unsigned 32-bit literal, excluding module/layout constants.
IRIntLit* _asExecutableInteger32Constant(IRInst* value)
{
    if (auto intLit = _asExecutableI32Constant(value))
        return intLit;

    auto intLit = as<IRIntLit>(value);
    if (!intLit || !isNVVMUnsignedI32Type(intLit->getDataType()))
        return nullptr;

    const IRIntegerValue intValue = intLit->getValue();
    return intValue >= 0 && intValue <= kNVVMUInt32Max ? intLit : nullptr;
}

// Returns an executable literal in one selected integer width. Canonical UInt64 uses the signed
// storage bits of IRIntegerValue when its high bit is set; the provider preserves that bit pattern.
IRIntLit* _asExecutableSelectedIntegerConstant(IRInst* value)
{
    auto intLit = as<IRIntLit>(value);
    uint32_t bitWidth = 0;
    bool isSigned = false;
    if (!intLit || !isNVVMSupportedIntegerScalarType(intLit->getDataType(), &bitWidth, &isSigned))
    {
        return nullptr;
    }

    const IRIntegerValue integerValue = intLit->getValue();
    if (bitWidth == 64)
        return intLit;
    if (isSigned)
    {
        const IRIntegerValue minimum = -(IRIntegerValue(1) << (bitWidth - 1));
        const IRIntegerValue maximum = (IRIntegerValue(1) << (bitWidth - 1)) - 1;
        return integerValue >= minimum && integerValue <= maximum ? intLit : nullptr;
    }
    const IRIntegerValue maximum = (IRIntegerValue(1) << bitWidth) - 1;
    return integerValue >= 0 && integerValue <= maximum ? intLit : nullptr;
}

// Returns a canonical executable Boolean literal.
IRBoolLit* _asExecutableBoolConstant(IRInst* value)
{
    auto boolLit = as<IRBoolLit>(value);
    return boolLit && isNVVMBoolType(boolLit->getDataType()) ? boolLit : nullptr;
}

// Returns an executable selected floating-point literal, excluding layout and module constants.
IRFloatLit* _asExecutableFloatingPointConstant(IRInst* value)
{
    auto floatLit = as<IRFloatLit>(value);
    return floatLit && isNVVMSupportedFloatingPointScalarType(floatLit->getDataType()) ? floatLit
                                                                                       : nullptr;
}

// Matches one canonical Slang type against a provider-owned semantic type role.
bool _isNVVMSemanticType(IRType* type, const SlangNVVMValueTypeDesc& semanticType)
{
    if (!type)
        return false;

    if (semanticType.kind == SLANG_NVVM_VALUE_TYPE_VOID)
    {
        return semanticType.bitWidth == 0 && semanticType.laneCount == 0 && as<IRVoidType>(type);
    }
    if (semanticType.laneCount >= 2 && semanticType.laneCount <= 4)
    {
        bool isSigned = false;
        uint32_t elementCount = 0;
        return semanticType.bitWidth == 32 &&
               asNVVMSupportedI32VectorType(type, &isSigned, &elementCount) &&
               semanticType.laneCount == elementCount &&
               (semanticType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER) == isSigned;
    }
    if (semanticType.laneCount != 1)
        return false;

    switch (semanticType.kind)
    {
    case SLANG_NVVM_VALUE_TYPE_BOOL:
        return semanticType.bitWidth == 1 && isNVVMBoolType(type);
    case SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER:
    case SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER:
        {
            uint32_t bitWidth = 0;
            bool isSigned = false;
            return isNVVMSupportedIntegerScalarType(type, &bitWidth, &isSigned) &&
                   semanticType.bitWidth == bitWidth &&
                   (semanticType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER) == isSigned;
        }
    case SLANG_NVVM_VALUE_TYPE_FLOATING_POINT:
        return semanticType.bitWidth == 32 && isNVVMFloat32Type(type);
    default:
        return false;
    }
}

// Checks the complete canonical helper signature against one typed semantic catalog row.
bool _isNVVMGenericAsmSemanticSignature(
    IRFunc* function,
    const NVVMSemantics::CatalogEntry& semantic)
{
    if (!function || function->getParamCount() != semantic.operandCount ||
        !_isNVVMSemanticType(function->getResultType(), semantic.resultType))
    {
        return false;
    }

    for (uint32_t i = 0; i < semantic.operandCount; ++i)
    {
        if (!_isNVVMSemanticType(function->getParamType(i), semantic.operandTypes[i]))
            return false;
    }
    return true;
}

// Maps an exact CUDA-selected GenericAsm terminator to one typed provider semantic.
const NVVMSemantics::CatalogEntry* _findNVVMGenericAsmSemantic(
    IRGenericAsm* genericAsm,
    IRFunc* function)
{
    if (!genericAsm || !function)
        return nullptr;

    for (const NVVMSemantics::CatalogEntry& semantic : NVVMSemantics::kCatalog)
    {
        if (semantic.genericAsm &&
            genericAsm->getAsm() == UnownedStringSlice(semantic.genericAsm) &&
            _isNVVMGenericAsmSemanticSignature(function, semantic))
        {
            return &semantic;
        }
    }
    return nullptr;
}

// Recognizes the canonical scalar `all(bool)` implementation selected by the CUDA prelude. Its
// `bool($0)` body is an identity operation, so the direct backend can preserve the checked
// parameter value without asking the provider to manufacture redundant IR.
IRParam* _getNVVMBoolIdentityGenericAsmParameter(IRGenericAsm* genericAsm, IRFunc* function)
{
    if (!genericAsm || !function || genericAsm->getAsm() != toSlice("bool($0)") ||
        genericAsm->getOperandCount() != 1 || function->getParamCount() != 1 ||
        !isNVVMBoolType(function->getResultType()) || !isNVVMBoolType(function->getParamType(0)))
    {
        return nullptr;
    }
    return function->getFirstParam();
}

enum class NVVMCUDALayoutQueryKind
{
    None,
    Size,
    Alignment,
    Offset,
};

struct NVVMCUDALayoutQuery
{
    NVVMCUDALayoutQueryKind kind = NVVMCUDALayoutQueryKind::None;
    IRType* explicitType = nullptr;
};

// Recognizes one exact CUDA-prelude layout-query helper. These helpers describe compile-time
// metadata; their aggregate parameters are not part of the direct backend's runtime value ABI.
bool _getNVVMCUDALayoutQuery(IRFunc* function, NVVMCUDALayoutQuery& outQuery)
{
    outQuery = {};
    if (!function || !isNVVMSignedI32Type(function->getResultType()))
        return false;

    IRBlock* block = function->getFirstBlock();
    if (!block || block->getNextBlock())
        return false;
    auto genericAsm = as<IRGenericAsm>(block->getTerminator());
    if (!genericAsm || genericAsm->getOperandCount() == 0 ||
        !as<IRStringLit>(genericAsm->getOperand(0)))
        return false;
    for (auto inst : block->getOrdinaryInsts())
    {
        if (inst != genericAsm)
            return false;
    }

    const UnownedStringSlice assembly = genericAsm->getAsm();
    if (assembly == toSlice("sizeof($[0])") || assembly == toSlice("alignof($[0])"))
    {
        if (function->getParamCount() != 0 || genericAsm->getOperandCount() != 2)
            return false;
        auto explicitType = as<IRType>(genericAsm->getOperand(1));
        if (!explicitType)
            return false;
        outQuery.kind = assembly == toSlice("sizeof($[0])") ? NVVMCUDALayoutQueryKind::Size
                                                            : NVVMCUDALayoutQueryKind::Alignment;
        outQuery.explicitType = explicitType;
        return true;
    }
    if (assembly == toSlice("sizeof($T0)") || assembly == toSlice("alignof($T0)"))
    {
        if (function->getParamCount() != 1 || genericAsm->getOperandCount() != 1)
            return false;
        outQuery.kind = assembly == toSlice("sizeof($T0)") ? NVVMCUDALayoutQueryKind::Size
                                                           : NVVMCUDALayoutQueryKind::Alignment;
        return true;
    }
    if (assembly == toSlice("int(((char*)&($1)) - ((char*)&($0)))"))
    {
        if (function->getParamCount() != 2 || genericAsm->getOperandCount() != 1)
            return false;
        outQuery.kind = NVVMCUDALayoutQueryKind::Offset;
        return true;
    }
    return false;
}

// Resolves one canonical query call through the shared CUDA layout rules. In particular, an
// offset is owned by the exact struct-field key already present in IR, never by positional or
// structural matching in the emitter.
bool _getNVVMCUDALayoutQueryValue(
    CodeGenContext* codeGenContext,
    IRCall* call,
    IRFunc* function,
    const NVVMCUDALayoutQuery& query,
    IRIntegerValue& outValue)
{
    outValue = 0;
    if (!call || !function || !isNVVMSignedI32Type(call->getDataType()) ||
        call->getArgCount() != function->getParamCount())
    {
        return false;
    }

    for (UInt argumentIndex = 0; argumentIndex < call->getArgCount(); ++argumentIndex)
    {
        IRInst* argument = call->getArg(argumentIndex);
        if (!argument ||
            !isTypeEqual(argument->getDataType(), function->getParamType(argumentIndex)))
        {
            return false;
        }
    }

    if (query.kind == NVVMCUDALayoutQueryKind::Offset)
    {
        auto aggregateType =
            call->getArgCount() == 2 ? as<IRStructType>(call->getArg(0)->getDataType()) : nullptr;
        auto fieldExtract =
            call->getArgCount() == 2 ? as<IRFieldExtract>(call->getArg(1)) : nullptr;
        if (!aggregateType || !fieldExtract || fieldExtract->getBase() != call->getArg(0))
            return false;

        IRStructField* selectedField = nullptr;
        for (auto field : aggregateType->getFields())
        {
            if (field->getKey() == fieldExtract->getField())
            {
                selectedField = field;
                break;
            }
        }
        if (!selectedField ||
            !isTypeEqual(selectedField->getFieldType(), fieldExtract->getDataType()))
        {
            return false;
        }

        IRIntegerValue offset = 0;
        if (SLANG_FAILED(getOffset(
                codeGenContext->getTargetReq(),
                IRTypeLayoutRules::getCUDA(),
                selectedField,
                &offset)) ||
            offset < 0 || offset > kNVVMI32Max)
        {
            return false;
        }
        outValue = offset;
        return true;
    }

    IRType* queriedType = query.explicitType;
    if (!queriedType && call->getArgCount() == 1)
        queriedType = function->getParamType(0);
    if (!queriedType)
        return false;

    IRSizeAndAlignment layout;
    if (SLANG_FAILED(getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getCUDA(),
            queriedType,
            &layout)))
    {
        return false;
    }

    const IRIntegerValue value =
        query.kind == NVVMCUDALayoutQueryKind::Alignment ? layout.alignment : layout.size;
    if (value <= 0 || value > kNVVMI32Max)
        return false;

    outValue = value;
    return true;
}

// Converts one canonical Slang type to its stable provider semantic role.
bool _getNVVMSemanticType(IRType* type, SlangNVVMValueTypeDesc& outType)
{
    if (as<IRVoidType>(type))
        outType = NVVMSemantics::kVoid;
    else if (auto vectorType = asNVVMSupportedValueVectorType(type))
    {
        IRType* elementType = vectorType->getElementType();
        uint32_t bitWidth = 0;
        bool isSigned = false;
        uint32_t elementCount = 0;
        SLANG_RELEASE_ASSERT(asNVVMSupportedValueVectorType(type, &elementCount));
        if (isNVVMSupportedIntegerScalarType(elementType, &bitWidth, &isSigned))
            outType = {
                isSigned ? SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                         : SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
                bitWidth,
                elementCount,
            };
        else if (uint32_t floatingPointBitWidth = 0;
                 isNVVMSupportedFloatingPointScalarType(elementType, &floatingPointBitWidth))
            outType = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, floatingPointBitWidth, elementCount};
        else
        {
            SLANG_RELEASE_ASSERT(isNVVMBoolType(elementType));
            outType = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, elementCount};
        }
    }
    else if (isNVVMBoolType(type))
        outType = NVVMSemantics::kBool;
    else
    {
        uint32_t bitWidth = 0;
        bool isSigned = false;
        if (isNVVMSupportedIntegerScalarType(type, &bitWidth, &isSigned))
        {
            outType = {
                isSigned ? SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                         : SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
                bitWidth,
                1,
            };
        }
        else if (uint32_t floatingPointBitWidth = 0;
                 isNVVMSupportedFloatingPointScalarType(type, &floatingPointBitWidth))
            outType = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, floatingPointBitWidth, 1};
        else
            return false;
    }
    return true;
}

bool _getNVVMValueOperation(IROp op, SlangNVVMValueOperation& outOperation)
{
    switch (op)
    {
    case kIROp_Add:
        outOperation = SLANG_NVVM_VALUE_OP_ADD;
        return true;
    case kIROp_Sub:
        outOperation = SLANG_NVVM_VALUE_OP_SUBTRACT;
        return true;
    case kIROp_Mul:
        outOperation = SLANG_NVVM_VALUE_OP_MULTIPLY;
        return true;
    case kIROp_Div:
        outOperation = SLANG_NVVM_VALUE_OP_DIVIDE;
        return true;
    case kIROp_IRem:
    case kIROp_FRem:
        outOperation = SLANG_NVVM_VALUE_OP_REMAINDER;
        return true;
    case kIROp_Lsh:
        outOperation = SLANG_NVVM_VALUE_OP_SHIFT_LEFT;
        return true;
    case kIROp_Rsh:
        outOperation = SLANG_NVVM_VALUE_OP_SHIFT_RIGHT;
        return true;
    case kIROp_BitAnd:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_AND;
        return true;
    case kIROp_BitOr:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_OR;
        return true;
    case kIROp_BitXor:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_XOR;
        return true;
    case kIROp_BitNot:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_NOT;
        return true;
    case kIROp_And:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_AND;
        return true;
    case kIROp_Or:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_OR;
        return true;
    case kIROp_Not:
        outOperation = SLANG_NVVM_VALUE_OP_BIT_NOT;
        return true;
    case kIROp_Neg:
        outOperation = SLANG_NVVM_VALUE_OP_NEGATE;
        return true;
    case kIROp_Eql:
        outOperation = SLANG_NVVM_VALUE_OP_EQUAL;
        return true;
    case kIROp_Neq:
        outOperation = SLANG_NVVM_VALUE_OP_NOT_EQUAL;
        return true;
    case kIROp_Less:
        outOperation = SLANG_NVVM_VALUE_OP_LESS_THAN;
        return true;
    case kIROp_Greater:
        outOperation = SLANG_NVVM_VALUE_OP_GREATER_THAN;
        return true;
    case kIROp_Leq:
        outOperation = SLANG_NVVM_VALUE_OP_LESS_EQUAL;
        return true;
    case kIROp_Geq:
        outOperation = SLANG_NVVM_VALUE_OP_GREATER_EQUAL;
        return true;
    case kIROp_IntCast:
        outOperation = SLANG_NVVM_VALUE_OP_INTEGER_CONVERT;
        return true;
    case kIROp_CastIntToFloat:
        outOperation = SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT;
        return true;
    case kIROp_CastFloatToInt:
        outOperation = SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER;
        return true;
    case kIROp_FloatCast:
        outOperation = SLANG_NVVM_VALUE_OP_FLOAT_CONVERT;
        return true;
    case kIROp_WaveMaskBallot:
        outOperation = SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT;
        return true;
    default:
        return false;
    }
}

struct NVVMResolvedValueOperation
{
    SlangNVVMValueTypeDesc operandTypes[3] = {};
    SlangNVVMValueOperationDesc desc = {};
    const NVVMSemantics::CatalogEntry* staticEntry = nullptr;
    NVVMSemantics::ValueOperationFamilyResolution family;
    const char* diagnosticName = nullptr;
};

// Records one exact typed provider operation, deduplicating identical overloads.
void _requireValueOperation(
    NVVMValueOperationRequirements& requirements,
    const SlangNVVMValueOperationDesc& desc,
    const char* diagnosticName)
{
    for (const auto& requirement : requirements)
    {
        const SlangNVVMValueOperationDesc existing = requirement.getDesc();
        if (existing.operation != desc.operation || existing.operandCount != desc.operandCount ||
            !NVVMSemantics::areSameType(existing.resultType, desc.resultType))
        {
            continue;
        }

        bool operandsMatch = true;
        for (uint32_t i = 0; i < existing.operandCount; ++i)
        {
            operandsMatch =
                operandsMatch &&
                NVVMSemantics::areSameType(existing.operandTypes[i], desc.operandTypes[i]);
        }
        if (operandsMatch)
            return;
    }

    NVVMValueOperationRequirement requirement;
    requirement.operation = desc.operation;
    requirement.resultType = desc.resultType;
    requirement.operandCount = uint32_t(desc.operandCount);
    requirement.diagnosticName = diagnosticName;
    for (uint32_t i = 0; i < requirement.operandCount; ++i)
        requirement.operandTypes[i] = desc.operandTypes[i];
    requirements.add(requirement);
}

// Resolves canonical Slang value operations to either a fixed exact row or one bounded family.
bool _resolveNVVMValueOperation(IRInst* inst, NVVMResolvedValueOperation& outOperation)
{
    outOperation = {};
    if (!inst || inst->getOperandCount() > 3)
        return false;

    SlangNVVMValueOperation operation = 0;
    SlangNVVMValueTypeDesc resultType = {};
    if (!_getNVVMValueOperation(inst->getOp(), operation) ||
        !_getNVVMSemanticType(inst->getDataType(), resultType))
    {
        return false;
    }
    for (UInt i = 0; i < inst->getOperandCount(); ++i)
    {
        IRInst* operand = inst->getOperand(i);
        if (!operand || !_getNVVMSemanticType(operand->getDataType(), outOperation.operandTypes[i]))
            return false;
    }

    outOperation.desc = {
        operation,
        resultType,
        inst->getOperandCount() ? outOperation.operandTypes : nullptr,
        inst->getOperandCount(),
    };
    outOperation.staticEntry = NVVMSemantics::find(outOperation.desc);
    if (outOperation.staticEntry)
    {
        outOperation.diagnosticName = outOperation.staticEntry->diagnosticName;
        return true;
    }
    if (!NVVMSemantics::resolveValueOperationFamily(outOperation.desc, outOperation.family))
        return false;
    outOperation.diagnosticName = outOperation.family.diagnosticName;
    return true;
}

// Checks that an executable operand has an accepted definition that dominates its use.
SlangResult _validateAvailableValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    // Canonical module-owned storage and CUDA execution globals exist before every function body
    // and therefore do not participate in instruction dominance. All other executable values
    // remain SSA-ordered.
    NVVMConventionalGlobalParams globalParams;
    SlangNVVMValueOperation executionOperation = 0;
    if (value && consumer && value->getModule() == consumer->getModule() &&
        (asNVVMSupportedSharedI32ArrayGlobal(value) ||
         _getNVVMConventionalGlobalParams(value, globalParams) ||
         _getNVVMCUDAExecutionGlobalOperation(value, executionOperation)))
    {
        return SLANG_OK;
    }
    if (value && consumer && dominatorTree && availableValues.contains(value) &&
        dominatorTree->dominates(value, consumer))
    {
        return SLANG_OK;
    }

    return _diagnoseUnsupportedIR(
        codeGenContext,
        value ? UnownedStringSlice(getIROpInfo(value->getOp()).name) : toSlice("missing operand"));
}

// Checks that an executable operand is an available signed 32-bit value.
SlangResult _validateI32Value(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (!value || !isNVVMSignedI32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("signed i32 value"));

    if (_asExecutableI32Constant(value))
    {
        return SLANG_OK;
    }

    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks sign-independent transport of a canonical 32-bit integer value.
SlangResult _validateInteger32Value(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (value && isNVVMSignedI32Type(value->getDataType()))
    {
        return _validateI32Value(codeGenContext, value, consumer, availableValues, dominatorTree);
    }
    if (!value || !isNVVMUnsignedI32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("32-bit integer value"));
    if (_asExecutableSelectedIntegerConstant(value))
        return SLANG_OK;
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks one selected integer value, including an exact-width executable literal.
SlangResult _validateSelectedIntegerValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (!value || !isNVVMSupportedIntegerScalarType(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("selected integer value"));
    if (_asExecutableSelectedIntegerConstant(value))
    {
        return SLANG_OK;
    }
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks a canonical UInt value, including its operation-defined 32-bit literal form.
SlangResult _validateUnsignedI32Value(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree,
    UnownedStringSlice diagnosticRole)
{
    if (!value || !isNVVMUnsignedI32Type(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, diagnosticRole);
    if (_asExecutableInteger32Constant(value))
    {
        return SLANG_OK;
    }
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks a canonical UInt wave mask, including its operation-defined 32-bit literal form.
SlangResult _validateWaveMaskValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    return _validateUnsignedI32Value(
        codeGenContext,
        value,
        consumer,
        availableValues,
        dominatorTree,
        toSlice("wave mask value"));
}

// Checks transport of a canonical Boolean value or materializes its literal through i1.
SlangResult _validateBooleanValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (!value || !isNVVMBoolType(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("Boolean value"));
    if (_asExecutableBoolConstant(value))
    {
        return SLANG_OK;
    }
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks that an executable operand is an available selected floating-point value.
SlangResult _validateFloatingPointValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (!value || !isNVVMSupportedFloatingPointScalarType(value->getDataType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("floating-point value"));

    if (_asExecutableFloatingPointConstant(value))
    {
        return SLANG_OK;
    }

    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks an available canonical scalar value using its semantic type.
SlangResult _validateScalarValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (value && isNVVMBoolType(value->getDataType()))
    {
        return _validateBooleanValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }
    if (value && isNVVMSupportedFloatingPointScalarType(value->getDataType()))
    {
        return _validateFloatingPointValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }
    if (value && isNVVMSupportedIntegerScalarType(value->getDataType()))
    {
        return _validateSelectedIntegerValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    }
    return _diagnoseUnsupportedIR(codeGenContext, toSlice("scalar value"));
}

// Checks a selected scalar, fixed value vector, or fixed numeric aggregate admitted by preflight.
SlangResult _validateSelectedValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (value && (asNVVMSupportedValueVectorType(value->getDataType()) ||
                  asNVVMSupportedNumericArrayType(value->getDataType())))
        return _validateAvailableValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    return _validateScalarValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks a selected byte payload. A fixed array is already an exact first-class SSA value, so it
// needs availability validation rather than the scalar/vector semantic validator.
SlangResult _validateByteAddressValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    if (value && asNVVMSupportedNumericArrayType(value->getDataType()))
        return _validateAvailableValue(
            codeGenContext,
            value,
            consumer,
            availableValues,
            dominatorTree);
    return _validateSelectedValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks an available scalar pointer and enforces the source access qualifier for stores.
SlangResult _validatePointerValue(
    CodeGenContext* codeGenContext,
    IRInst* value,
    IRInst* consumer,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree,
    bool requireWriteAccess,
    IRType* expectedPointeeType)
{
    auto numericPtrType =
        value ? asNVVMSupportedDeviceNumericPointerType(value->getDataType()) : nullptr;
    auto resourceElementPtrType =
        value ? asNVVMSupportedRWStructuredBufferElementPointerType(value->getDataType()) : nullptr;
    auto sharedElementPtrType =
        value ? asNVVMSupportedSharedI32ElementPointerType(value->getDataType()) : nullptr;
    auto localStructPtrType =
        value ? asNVVMSupportedLocalCopyableStructPointerType(value->getDataType()) : nullptr;
    auto borrowedStructPtrType =
        value ? asNVVMSupportedLocalScalarStructPointerType(value->getDataType()) : nullptr;
    auto fieldPtrType = value ? as<IRPtrTypeBase>(value->getDataType()) : nullptr;
    NVVMStructField fieldAddress;
    if (!fieldPtrType || value->getOp() != kIROp_FieldAddress ||
        !_getNVVMStructFieldAddress(as<IRFieldAddress>(value), fieldAddress))
    {
        fieldPtrType = nullptr;
    }
    IRPtrTypeBase* devicePtrType = numericPtrType;
    IRPtrTypeBase* acceptedPtrType = devicePtrType            ? devicePtrType
                                     : sharedElementPtrType   ? sharedElementPtrType
                                     : resourceElementPtrType ? resourceElementPtrType
                                     : localStructPtrType     ? localStructPtrType
                                     : borrowedStructPtrType  ? borrowedStructPtrType
                                                              : fieldPtrType;
    if (!acceptedPtrType)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("device scalar pointer"));
    IRType* actualPointeeType = acceptedPtrType->getValueType();
    if (!expectedPointeeType || !isTypeEqual(actualPointeeType, expectedPointeeType))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("device pointer pointee type"));
    if (resourceElementPtrType && consumer->getOp() != kIROp_Load &&
        consumer->getOp() != kIROp_Store && consumer->getOp() != kIROp_SwizzledStore)
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            toSlice("raw RWStructuredBuffer numeric load or store consumer"));
    }
    if (fieldPtrType && !fieldAddress.isMutableLocal &&
        (consumer->getOp() != kIROp_Load || requireWriteAccess))
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            toSlice("read-only conventional parameter field load"));
    }
    if (requireWriteAccess && acceptedPtrType->getAccessQualifier() != AccessQualifier::ReadWrite)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("read-only pointer store"));
    return _validateAvailableValue(codeGenContext, value, consumer, availableValues, dominatorTree);
}

// Checks that a branch destination is a block declared by the selected function.
SlangResult _validateBlockTarget(
    CodeGenContext* codeGenContext,
    IRBlock* block,
    const HashSet<IRBlock*>& functionBlocks)
{
    if (block && functionBlocks.contains(block))
        return SLANG_OK;
    return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch target"));
}

// Orders reachable bodies by CFG dominance, then preserves physical order for unreachable bodies.
List<IRBlock*> _getNVVMBodyOrder(IRFunc* function, IRDominatorTree* dominatorTree)
{
    List<IRBlock*> result;
    HashSet<IRBlock*> addedBlocks;
    for (auto block : getReversePostorder(function))
    {
        if (!dominatorTree->isUnreachable(block) && addedBlocks.add(block))
            result.add(block);
    }
    for (auto block : function->getBlocks())
    {
        if (addedBlocks.add(block))
            result.add(block);
    }
    return result;
}

// Counts the positional SSA values a branch to `block` must provide.
UInt _getBlockParamCount(IRBlock* block)
{
    UInt count = 0;
    for (auto param : block->getParams())
    {
        SLANG_UNUSED(param);
        ++count;
    }
    return count;
}

// Validates the positional SSA values carried by an actual branch edge.
SlangResult _validateBranchArguments(
    CodeGenContext* codeGenContext,
    IRUnconditionalBranch* branch,
    IRBlock* entryBlock,
    const HashSet<IRBlock*>& functionBlocks,
    const HashSet<IRInst*>& availableValues,
    IRDominatorTree* dominatorTree)
{
    IRBlock* targetBlock = branch->getTargetBlock();
    SLANG_RETURN_ON_FAIL(_validateBlockTarget(codeGenContext, targetBlock, functionBlocks));
    if (targetBlock == entryBlock)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-block branch target"));

    const UInt argumentCount = branch->getArgCount();
    if (argumentCount != _getBlockParamCount(targetBlock))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch argument count"));

    IRParam* targetParam = targetBlock->getFirstParam();
    for (UInt argumentIndex = 0; argumentIndex < argumentCount;
         ++argumentIndex, targetParam = targetParam->getNextParam())
    {
        IRInst* argument = branch->getArg(argumentIndex);
        SLANG_ASSERT(targetParam);
        if (!argument || !isTypeEqual(argument->getDataType(), targetParam->getDataType()))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch argument type"));
        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
            codeGenContext,
            argument,
            branch,
            availableValues,
            dominatorTree));
    }
    return SLANG_OK;
}

// Returns the LLVM symbol chosen from the canonical linked IR for an accepted function.
UnownedStringSlice _getNVVMFunctionName(IRFunc* function, IRFunc* entryPoint)
{
    if (function == entryPoint)
    {
        auto entryPointDecoration = function->findDecoration<IREntryPointDecoration>();
        SLANG_RELEASE_ASSERT(entryPointDecoration);
        return entryPointDecoration->getName()->getStringSlice();
    }
    if (auto exportDecoration = function->findDecorationImpl(kIROp_CudaDeviceExportDecoration))
    {
        SLANG_RELEASE_ASSERT(exportDecoration->getOperandCount() == 1);
        auto exportName = as<IRStringLit>(exportDecoration->getOperand(0));
        SLANG_RELEASE_ASSERT(exportName);
        return exportName->getStringSlice();
    }
    return getMangledName(function);
}

// Returns whether a type is an accepted canonical value in a helper result.
bool _isSupportedNVVMHelperResultType(IRInst* type)
{
    return as<IRVoidType>(type) || isNVVMSupportedValueType(type) ||
           asNVVMSupportedScalarStructType(type);
}

// Returns whether one exact canonical type can cross a selected helper parameter boundary.
bool _isSupportedNVVMHelperParameterType(IRInst* type)
{
    return isNVVMSupportedValueType(type) || asNVVMSupportedLocalScalarStructPointerType(type);
}

// Returns whether one canonical call argument satisfies an exact helper parameter. A mutable
// borrow deliberately has a distinct source type from the local pointer passed to it, while both
// preserve the same selected aggregate and lower to one typed generic pointer.
bool _isSupportedNVVMHelperArgumentType(IRType* argumentType, IRType* parameterType)
{
    if (isTypeEqual(argumentType, parameterType))
        return true;
    IRStructType* argumentValueType = nullptr;
    IRStructType* parameterValueType = nullptr;
    auto argumentPointer =
        asNVVMSupportedLocalScalarStructPointerType(argumentType, &argumentValueType);
    auto parameterPointer =
        asNVVMSupportedLocalScalarStructPointerType(parameterType, &parameterValueType);
    return argumentPointer && argumentPointer->getOp() == kIROp_PtrType && parameterPointer &&
           parameterPointer->getOp() == kIROp_BorrowInOutParamType &&
           isTypeEqual(argumentValueType, parameterValueType);
}

// Returns whether a canonical helper signature needs the generic construction path.
bool _usesGenericNVVMFunctions(IRFunc* helper)
{
    SLANG_RELEASE_ASSERT(helper);
    SLANG_RELEASE_ASSERT(_isSupportedNVVMHelperResultType(helper->getResultType()));
    if (!isNVVMSignedI32Type(helper->getResultType()))
        return true;
    for (UInt parameterIndex = 0; parameterIndex < helper->getParamCount(); ++parameterIndex)
    {
        IRType* parameterType = helper->getParamType(parameterIndex);
        SLANG_RELEASE_ASSERT(_isSupportedNVVMHelperParameterType(parameterType));
        if (!isNVVMSignedI32Type(parameterType))
            return true;
    }
    return false;
}

// Checks the exact helper ABI before adding a direct callee to the accepted closure.
SlangResult _validateNVVMHelperTarget(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    IRFunc* entryPoint,
    IRFunc* helper)
{
    if (!helper)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));
    if (helper == entryPoint || helper->findDecoration<IREntryPointDecoration>() ||
        helper->findDecoration<IRCudaKernelDecoration>())
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));
    }
    if (helper->getParent() != linkedIR.module->getModuleInst() || !helper->isDefinition())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));
    if (!_isSupportedNVVMHelperResultType(helper->getResultType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("helper function result type"));
    for (UInt parameterIndex = 0; parameterIndex < helper->getParamCount(); ++parameterIndex)
    {
        if (!_isSupportedNVVMHelperParameterType(helper->getParamType(parameterIndex)))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("helper function parameter"));
    }
    return SLANG_OK;
}

// Visits the exact direct-call graph and records each reachable function once in preorder.
SlangResult _visitNVVMFunction(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    IRFunc* entryPoint,
    IRFunc* function,
    List<IRFunc*>& functions,
    HashSet<IRFunc*>& functionSet,
    HashSet<IRFunc*>& activeFunctions,
    HashSet<IRFunc*>& completedFunctions)
{
    if (completedFunctions.contains(function))
        return SLANG_OK;
    if (!activeFunctions.add(function))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("recursive function call"));
    if (functionSet.add(function))
        functions.add(function);

    for (auto block : function->getBlocks())
    {
        for (auto inst : block->getOrdinaryInsts())
        {
            auto call = as<IRCall>(inst);
            if (!call)
                continue;
            if (!call->getOperandCount())
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("call"));

            auto helper = as<IRFunc>(call->getOperand(0));
            SLANG_RETURN_ON_FAIL(
                _validateNVVMHelperTarget(codeGenContext, linkedIR, entryPoint, helper));
            if (activeFunctions.contains(helper))
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("recursive function call"));
            SLANG_RETURN_ON_FAIL(_visitNVVMFunction(
                codeGenContext,
                linkedIR,
                entryPoint,
                helper,
                functions,
                functionSet,
                activeFunctions,
                completedFunctions));
        }
    }

    activeFunctions.remove(function);
    completedFunctions.add(function);
    return SLANG_OK;
}

// Collects the finite direct-call closure rooted at the sole selected entry point.
SlangResult _collectNVVMFunctions(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    IRFunc* entryPoint,
    List<IRFunc*>& functions,
    HashSet<IRFunc*>& functionSet)
{
    HashSet<IRFunc*> activeFunctions;
    HashSet<IRFunc*> completedFunctions;
    return _visitNVVMFunction(
        codeGenContext,
        linkedIR,
        entryPoint,
        entryPoint,
        functions,
        functionSet,
        activeFunctions,
        completedFunctions);
}

// Checks that function values remain direct callees rather than becoming first-class data.
SlangResult _validateNVVMFunctionUses(
    CodeGenContext* codeGenContext,
    const List<IRFunc*>& functions)
{
    for (auto function : functions)
    {
        for (auto use = function->firstUse; use; use = use->nextUse)
        {
            auto call = as<IRCall>(use->getUser());
            if (!call || use != call->getCalleeUse())
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("function value use"));
        }
    }
    return SLANG_OK;
}

// Checks that every emitted function and storage object has a distinct canonical symbol before
// provider discovery.
SlangResult _validateNVVMSymbolNames(
    CodeGenContext* codeGenContext,
    IRModule* module,
    IRFunc* entryPoint,
    const List<IRFunc*>& functions)
{
    HashSet<String> names;
    for (auto function : functions)
    {
        UnownedStringSlice name = _getNVVMFunctionName(function, entryPoint);
        if (!name.getLength() || !names.add(String(name)))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("function name"));
    }
    for (auto globalInst : module->getGlobalInsts())
    {
        NVVMConventionalGlobalParams globalParams;
        if (_getNVVMConventionalGlobalParams(globalInst, globalParams))
        {
            if (!names.add(String("SLANG_globalParams")))
                return _diagnoseUnsupportedIR(codeGenContext, toSlice("global storage name"));
            continue;
        }
        auto globalVar = asNVVMSupportedSharedI32ArrayGlobal(globalInst);
        if (!globalVar)
            continue;
        const UnownedStringSlice name = getMangledName(globalVar);
        if (!name.getLength() || !names.add(String(name)))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("global storage name"));
    }
    return SLANG_OK;
}

// Checks one function body using the same block and SSA order that emission will use.
SlangResult _validateNVVMFunction(
    CodeGenContext* codeGenContext,
    IRFunc* entryPoint,
    IRFunc* function,
    const HashSet<IRFunc*>& functionSet,
    NVVMValueOperationRequirements& requirements)
{
    const bool isEntryPoint = function == entryPoint;
    IRBlock* entryBlock = function->getFirstBlock();
    if (!entryBlock)
        return _diagnoseUnsupportedIR(
            codeGenContext,
            isEntryPoint ? toSlice("entry block") : toSlice("helper entry block"));

    HashSet<IRBlock*> functionBlocks;
    for (auto block : function->getBlocks())
        functionBlocks.add(block);
    RefPtr<IRDominatorTree> dominatorTree = computeDominatorTree(function);
    List<IRBlock*> bodyOrder = _getNVVMBodyOrder(function, dominatorTree);
    for (auto block : bodyOrder)
    {
        if (!functionBlocks.contains(block))
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch target"));
    }

    HashSet<IRInst*> availableValues;
    UInt actualParamCount = 0;
    for (auto param : function->getParams())
    {
        const bool isSupportedType =
            isEntryPoint ? isNVVMSupportedParameterType(param->getDataType())
                         : _isSupportedNVVMHelperParameterType(param->getDataType());
        if (actualParamCount >= function->getParamCount() || !isSupportedType ||
            !isTypeEqual(param->getDataType(), function->getParamType(actualParamCount)))
        {
            return _diagnoseUnsupportedIR(
                codeGenContext,
                isEntryPoint ? toSlice("entry-point parameter")
                             : toSlice("helper function parameter"));
        }
        NVVMRawBufferType rawBufferType;
        if (isEntryPoint && getNVVMSupportedRawBufferType(param->getDataType(), rawBufferType) &&
            rawBufferType.kind == NVVMRawBufferKind::Structured)
        {
            if (auto elementStruct =
                    asNVVMSupportedCopyableStructType(rawBufferType.structuredElementType))
            {
                if (!_hasNVVMCompatibleCopyableStructLayout(codeGenContext, elementStruct))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("structured-buffer aggregate layout"));
                }
            }
        }
        if (isEntryPoint && asNVVMSupportedScalarStructType(param->getDataType()))
        {
            uint32_t alignment = 0;
            if (!_getNVVMByValueParameterAlignment(codeGenContext, param->getDataType(), alignment))
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("entry-point parameter layout"));
            }
        }
        availableValues.add(param);
        ++actualParamCount;
    }
    if (actualParamCount != function->getParamCount())
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            isEntryPoint ? toSlice("entry-point parameter count")
                         : toSlice("helper parameter count"));
    }
    // Register every accepted block parameter before checking uses because emission creates all
    // phi placeholders before any body. Ordinary values join this set in the second pass, in the
    // same order in which their LLVM instructions will be emitted.
    for (auto block : function->getBlocks())
    {
        if (block != entryBlock)
        {
            for (auto param : block->getParams())
            {
                if (!isNVVMSupportedValueType(param->getDataType()) &&
                    !asNVVMSupportedNumericArrayType(param->getDataType()))
                {
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block parameter"));
                }
                availableValues.add(param);
            }
        }

        IRTerminatorInst* terminator = block->getTerminator();
        if (!terminator)
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("missing terminator"));

        for (auto inst : block->getOrdinaryInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_Var:
                {
                    IRStructType* valueType = nullptr;
                    if (!asNVVMSupportedLocalCopyableStructPointerType(
                            inst->getDataType(),
                            &valueType))
                    {
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("var"));
                    }
                    if (!_hasNVVMCompatibleCopyableStructLayout(codeGenContext, valueType))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("local copyable-struct layout"));
                    }
                }
                break;

            case kIROp_Load:
                {
                    NVVMRawBufferType rawBufferType;
                    if (!isNVVMSupportedNumericValueType(inst->getDataType()) &&
                        !getNVVMSupportedRawBufferType(inst->getDataType(), rawBufferType) &&
                        !asNVVMSupportedScalarParameterGroupType(inst->getDataType()) &&
                        !asNVVMSupportedCopyableStructType(inst->getDataType()))
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("load result type"));
                }
                break;

            case kIROp_Store:
                if (inst->getOperandCount() != 2 || !inst->getOperand(0))
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("store"));
                if (isPointerToImmutableLocation(getRootAddr(inst->getOperand(0))))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("store to immutable location"));
                }
                break;

            case kIROp_SwizzledStore:
                {
                    NVVMVectorSwizzledStore store;
                    if (!_getNVVMVectorSwizzledStore(inst, store))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("RWStructuredBuffer numeric vector swizzled store"));
                    }
                }
                break;

            case kIROp_Add:
            case kIROp_Sub:
            case kIROp_Mul:
            case kIROp_Div:
            case kIROp_IRem:
            case kIROp_FRem:
            case kIROp_Lsh:
            case kIROp_Rsh:
            case kIROp_BitAnd:
            case kIROp_BitOr:
            case kIROp_BitXor:
            case kIROp_BitNot:
            case kIROp_And:
            case kIROp_Or:
            case kIROp_Not:
            case kIROp_Neg:
            case kIROp_IntCast:
            case kIROp_CastIntToFloat:
            case kIROp_CastFloatToInt:
            case kIROp_FloatCast:
                {
                    NVVMResolvedValueOperation operation;
                    if (!_resolveNVVMValueOperation(inst, operation))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    _requireValueOperation(requirements, operation.desc, operation.diagnosticName);
                }
                break;

            case kIROp_AtomicAdd:
                {
                    if (inst->getOperandCount() != 3 || !isNVVMSignedI32Type(inst->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("relaxed global signed i32 atomic add"));
                    }
                    auto memoryOrder = _asExecutableI32Constant(inst->getOperand(2));
                    if (!memoryOrder || memoryOrder->getValue() != kIRMemoryOrder_Relaxed)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("relaxed atomic-add memory order"));
                    }
                }
                break;

            case kIROp_Less:
            case kIROp_Eql:
            case kIROp_Neq:
            case kIROp_Greater:
            case kIROp_Leq:
            case kIROp_Geq:
                {
                    NVVMResolvedValueOperation operation;
                    if (!_resolveNVVMValueOperation(inst, operation))
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            UnownedStringSlice(getIROpInfo(inst->getOp()).name));
                    _requireValueOperation(requirements, operation.desc, operation.diagnosticName);
                }
                break;

            case kIROp_Call:
                {
                    auto call = as<IRCall>(inst);
                    auto callee =
                        call && call->getOperandCount() ? as<IRFunc>(call->getOperand(0)) : nullptr;
                    if (!callee || !_isSupportedNVVMHelperResultType(inst->getDataType()))
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("value call"));
                }
                break;

            case kIROp_MakeVector:
            case kIROp_MakeVectorFromScalar:
            case kIROp_MakeArray:
            case kIROp_Swizzle:
            case kIROp_SwizzleSet:
            case kIROp_GetElement:
                {
                    NVVMVectorElement element;
                    NVVMVectorConstruction construction;
                    NVVMAggregateElement aggregateElement;
                    NVVMAggregateConstruction aggregateConstruction;
                    if (!_getNVVMVectorElement(inst, element) &&
                        !_getNVVMVectorConstruction(inst, construction) &&
                        !_getNVVMAggregateElement(inst, aggregateElement) &&
                        !_getNVVMAggregateConstruction(inst, aggregateConstruction))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("selected value construction or extraction"));
                    }
                }
                break;

            case kIROp_GenericAsm:
                {
                    auto genericAsm = as<IRGenericAsm>(inst);
                    if (isEntryPoint || genericAsm != terminator || functionBlocks.getCount() != 1)
                    {
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("GenericAsm"));
                    }
                    const NVVMSemantics::CatalogEntry* semantic =
                        _findNVVMGenericAsmSemantic(genericAsm, function);
                    if (_getNVVMBoolIdentityGenericAsmParameter(genericAsm, function))
                        break;
                    if (genericAsm->getOperandCount() != 1 || !semantic)
                    {
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("GenericAsm"));
                    }
                    const SlangNVVMValueOperationDesc operation =
                        NVVMSemantics::getOperationDesc(*semantic);
                    _requireValueOperation(requirements, operation, semantic->diagnosticName);
                }
                break;

            case kIROp_WaveMaskBallot:
                {
                    NVVMResolvedValueOperation operation;
                    if (!_resolveNVVMValueOperation(inst, operation) || !operation.staticEntry)
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("wave-mask ballot"));
                    _requireValueOperation(requirements, operation.desc, operation.diagnosticName);
                }
                break;

            case kIROp_GetOffsetPtr:
                if (inst->getOperandCount() != 2 ||
                    !asNVVMSupportedDeviceNumericPointerType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("device scalar pointer offset"));
                }
                break;

            case kIROp_GetElementPtr:
                {
                    NVVMRawBufferElementPointer bufferElementPointer;
                    if (inst->getOperandCount() != 2 ||
                        (!asNVVMSupportedDevicePointerType(inst->getDataType()) &&
                         !asNVVMSupportedSharedI32ElementPointerType(inst->getDataType()) &&
                         !_getNVVMRawBufferElementPointer(inst, bufferElementPointer)))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("device i32 array element pointer"));
                    }
                }
                break;

            case kIROp_GetStructuredBufferPtr:
            case kIROp_GetUntypedBufferPtr:
                {
                    NVVMRawBufferDataPointer dataPointer;
                    if (!_getNVVMRawBufferDataPointer(inst, dataPointer))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("raw buffer data pointer"));
                    }
                }
                break;

            case kIROp_RWStructuredBufferGetElementPtr:
                if (inst->getOperandCount() != 2 ||
                    !asNVVMSupportedRWStructuredBufferElementPointerType(inst->getDataType()))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("raw RWStructuredBuffer numeric element pointer"));
                }
                break;

            case kIROp_StructuredBufferLoad:
            case kIROp_RWStructuredBufferLoad:
                {
                    NVVMRawBufferType bufferType;
                    if (inst->getOperandCount() != 2 ||
                        !isNVVMSupportedNumericValueType(inst->getDataType()) ||
                        !inst->getOperand(0) ||
                        !getNVVMSupportedRawBufferType(
                            inst->getOperand(0)->getDataType(),
                            bufferType) ||
                        bufferType.kind != NVVMRawBufferKind::Structured)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("raw structured-buffer numeric load"));
                    }
                }
                break;

            case kIROp_ByteAddressBufferLoad:
            case kIROp_ByteAddressBufferStore:
                {
                    NVVMByteAddressAccess access;
                    if (!_getNVVMByteAddressAccess(inst, access))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("core byte-address buffer access"));
                    }
                }
                break;

            case kIROp_FieldExtract:
                {
                    NVVMStructField field;
                    if (!_getNVVMStructFieldValue(as<IRFieldExtract>(inst), field))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("scalar struct field value"));
                    }
                }
                break;

            case kIROp_FieldAddress:
                {
                    NVVMStructField fieldAddress;
                    if (!_getNVVMStructFieldAddress(as<IRFieldAddress>(inst), fieldAddress))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conventional global parameter field address"));
                    }
                }
                break;

            case kIROp_Return:
                break;

            case kIROp_UnconditionalBranch:
            case kIROp_Loop:
            case kIROp_IfElse:
                break;

            default:
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    UnownedStringSlice(getIROpInfo(inst->getOp()).name));
            }
        }
    }

    bool hasHelperReturn = false;
    // Reachable reverse postorder puts every dominating ordinary producer before its consumer
    // without making physical sibling order part of legality. Unreachable blocks retain physical
    // order, and phi definitions are already available in every block.
    for (auto block : bodyOrder)
    {
        IRTerminatorInst* terminator = block->getTerminator();
        SLANG_ASSERT(terminator);

        for (auto inst : block->getOrdinaryInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_Var:
                availableValues.add(inst);
                break;

            case kIROp_Load:
                {
                    auto load = cast<IRLoad>(inst);
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        load->getPtr(),
                        load,
                        availableValues,
                        dominatorTree,
                        false,
                        load->getDataType()));
                    availableValues.add(load);
                }
                break;

            case kIROp_Store:
                {
                    auto store = cast<IRStore>(inst);
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        store->getPtr(),
                        store,
                        availableValues,
                        dominatorTree,
                        true,
                        store->getVal()->getDataType()));
                    if (asNVVMSupportedCopyableStructType(store->getVal()->getDataType()))
                    {
                        SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                            codeGenContext,
                            store->getVal(),
                            store,
                            availableValues,
                            dominatorTree));
                    }
                    else
                    {
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            store->getVal(),
                            store,
                            availableValues,
                            dominatorTree));
                    }
                }
                break;

            case kIROp_SwizzledStore:
                {
                    NVVMVectorSwizzledStore store;
                    SLANG_RELEASE_ASSERT(_getNVVMVectorSwizzledStore(inst, store));
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        store.destination,
                        inst,
                        availableValues,
                        dominatorTree,
                        true,
                        store.destinationType));
                    SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                        codeGenContext,
                        store.source,
                        inst,
                        availableValues,
                        dominatorTree));
                }
                break;

            case kIROp_Add:
            case kIROp_Sub:
            case kIROp_Mul:
            case kIROp_Div:
            case kIROp_IRem:
            case kIROp_FRem:
            case kIROp_Lsh:
            case kIROp_Rsh:
            case kIROp_BitAnd:
            case kIROp_BitOr:
            case kIROp_BitXor:
            case kIROp_BitNot:
            case kIROp_And:
            case kIROp_Or:
            case kIROp_Not:
            case kIROp_Neg:
            case kIROp_Less:
            case kIROp_Eql:
            case kIROp_Neq:
            case kIROp_Greater:
            case kIROp_Leq:
            case kIROp_Geq:
            case kIROp_IntCast:
            case kIROp_CastIntToFloat:
            case kIROp_CastFloatToInt:
            case kIROp_FloatCast:
                {
                    NVVMResolvedValueOperation operation;
                    SLANG_RELEASE_ASSERT(_resolveNVVMValueOperation(inst, operation));
                    for (UInt operandIndex = 0; operandIndex < inst->getOperandCount();
                         ++operandIndex)
                    {
                        SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                            codeGenContext,
                            inst->getOperand(operandIndex),
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    availableValues.add(inst);
                }
                break;

            case kIROp_AtomicAdd:
                // Operand two is the literal Relaxed policy validated in the shape pass, not an
                // SSA value that the provider should receive.
                SLANG_RETURN_ON_FAIL(_validatePointerValue(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree,
                    true,
                    inst->getDataType()));
                SLANG_RETURN_ON_FAIL(_validateI32Value(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree));
                availableValues.add(inst);
                break;

            case kIROp_Call:
                {
                    auto call = cast<IRCall>(inst);
                    auto callee = as<IRFunc>(call->getOperand(0));
                    if (!callee || callee == entryPoint || !functionSet.contains(callee) ||
                        !isTypeEqual(call->getDataType(), callee->getResultType()) ||
                        call->getArgCount() != callee->getParamCount())
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("direct scalar call"));
                    }
                    for (UInt argumentIndex = 0; argumentIndex < call->getArgCount();
                         ++argumentIndex)
                    {
                        IRInst* argument = call->getArg(argumentIndex);
                        if (!argument || !_isSupportedNVVMHelperArgumentType(
                                             argument->getDataType(),
                                             callee->getParamType(argumentIndex)))
                        {
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("call argument type"));
                        }
                        if (asNVVMSupportedLocalScalarStructPointerType(argument->getDataType()))
                        {
                            SLANG_RETURN_ON_FAIL(_validatePointerValue(
                                codeGenContext,
                                argument,
                                call,
                                availableValues,
                                dominatorTree,
                                false,
                                cast<IRPtrTypeBase>(argument->getDataType())->getValueType()));
                        }
                        else
                        {
                            SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                                codeGenContext,
                                argument,
                                call,
                                availableValues,
                                dominatorTree));
                        }
                    }
                    if (!as<IRVoidType>(call->getDataType()))
                        availableValues.add(call);
                }
                break;

            case kIROp_MakeVector:
            case kIROp_MakeVectorFromScalar:
            case kIROp_MakeArray:
            case kIROp_Swizzle:
            case kIROp_SwizzleSet:
            case kIROp_GetElement:
                {
                    NVVMVectorElement element;
                    NVVMVectorConstruction construction;
                    NVVMAggregateElement aggregateElement;
                    NVVMAggregateConstruction aggregateConstruction;
                    if (_getNVVMVectorElement(inst, element))
                    {
                        SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                            codeGenContext,
                            element.base,
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    else if (_getNVVMVectorConstruction(inst, construction))
                    {
                        for (uint32_t i = 0; i < construction.elementCount; ++i)
                        {
                            const NVVMVectorConstructElement& source = construction.elements[i];
                            if (source.value)
                            {
                                SLANG_RETURN_ON_FAIL(_validateScalarValue(
                                    codeGenContext,
                                    source.value,
                                    inst,
                                    availableValues,
                                    dominatorTree));
                            }
                            else
                            {
                                SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                                    codeGenContext,
                                    source.extractedBase,
                                    inst,
                                    availableValues,
                                    dominatorTree));
                            }
                        }
                    }
                    else if (_getNVVMAggregateElement(inst, aggregateElement))
                    {
                        SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                            codeGenContext,
                            aggregateElement.base,
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    else
                    {
                        SLANG_RELEASE_ASSERT(
                            _getNVVMAggregateConstruction(inst, aggregateConstruction));
                        for (uint32_t i = 0; i < aggregateConstruction.elementCount; ++i)
                        {
                            SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                                codeGenContext,
                                inst->getOperand(i),
                                inst,
                                availableValues,
                                dominatorTree));
                        }
                    }
                    availableValues.add(inst);
                }
                break;

            case kIROp_GenericAsm:
                SLANG_ASSERT(inst == terminator);
                hasHelperReturn = true;
                break;

            case kIROp_WaveMaskBallot:
                SLANG_RETURN_ON_FAIL(_validateWaveMaskValue(
                    codeGenContext,
                    inst->getOperand(0),
                    inst,
                    availableValues,
                    dominatorTree));
                SLANG_RETURN_ON_FAIL(_validateBooleanValue(
                    codeGenContext,
                    inst->getOperand(1),
                    inst,
                    availableValues,
                    dominatorTree));
                availableValues.add(inst);
                break;

            case kIROp_GetOffsetPtr:
                {
                    IRInst* basePointer = inst->getOperand(0);
                    IRInst* elementOffset = inst->getOperand(1);
                    auto basePointerType =
                        basePointer
                            ? asNVVMSupportedDeviceNumericPointerType(basePointer->getDataType())
                            : nullptr;
                    if (!basePointerType ||
                        !isTypeEqual(inst->getDataType(), basePointer->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("pointer offset result type"));
                    }
                    SLANG_RETURN_ON_FAIL(_validatePointerValue(
                        codeGenContext,
                        basePointer,
                        inst,
                        availableValues,
                        dominatorTree,
                        false,
                        basePointerType->getValueType()));
                    SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                        codeGenContext,
                        elementOffset,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_GetElementPtr:
                {
                    IRInst* basePointer = inst->getOperand(0);
                    IRInst* elementIndex = inst->getOperand(1);
                    NVVMRawBufferElementPointer bufferElementPointer;
                    if (_getNVVMRawBufferElementPointer(inst, bufferElementPointer))
                    {
                        SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                            codeGenContext,
                            basePointer,
                            inst,
                            availableValues,
                            dominatorTree));
                        SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                            codeGenContext,
                            elementIndex,
                            inst,
                            availableValues,
                            dominatorTree));
                        availableValues.add(inst);
                        break;
                    }
                    IRArrayType* arrayType = nullptr;
                    auto basePointerType = basePointer ? asNVVMSupportedDeviceArrayPointerType(
                                                             basePointer->getDataType(),
                                                             &arrayType)
                                                       : nullptr;
                    IRArrayType* sharedArrayType = nullptr;
                    auto sharedGlobal =
                        asNVVMSupportedSharedI32ArrayGlobal(basePointer, &sharedArrayType);
                    auto resultPointerType = asNVVMSupportedDevicePointerType(inst->getDataType());
                    auto sharedResultPointerType =
                        asNVVMSupportedSharedI32ElementPointerType(inst->getDataType());
                    const bool isDeviceArrayElement =
                        basePointerType && resultPointerType && arrayType &&
                        basePointerType->getAddressSpace() ==
                            resultPointerType->getAddressSpace() &&
                        basePointerType->getAccessQualifier() ==
                            resultPointerType->getAccessQualifier() &&
                        isTypeEqual(arrayType->getElementType(), resultPointerType->getValueType());
                    const bool isSharedArrayElement = sharedGlobal && sharedArrayType &&
                                                      sharedResultPointerType &&
                                                      isTypeEqual(
                                                          sharedArrayType->getElementType(),
                                                          sharedResultPointerType->getValueType());
                    if (!isDeviceArrayElement && !isSharedArrayElement)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("array element pointer relation"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        basePointer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                        codeGenContext,
                        elementIndex,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_GetStructuredBufferPtr:
            case kIROp_GetUntypedBufferPtr:
                {
                    NVVMRawBufferDataPointer dataPointer;
                    SLANG_RELEASE_ASSERT(_getNVVMRawBufferDataPointer(inst, dataPointer));
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        dataPointer.buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_FieldAddress:
                SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                    codeGenContext,
                    cast<IRFieldAddress>(inst)->getBase(),
                    inst,
                    availableValues,
                    dominatorTree));
                availableValues.add(inst);
                break;

            case kIROp_FieldExtract:
                {
                    auto fieldExtract = cast<IRFieldExtract>(inst);
                    auto parameter = as<IRParam>(fieldExtract->getBase());
                    if (!isEntryPoint || !parameter || parameter->getParent() != entryBlock)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("scalar struct field base"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        parameter,
                        inst,
                        availableValues,
                        dominatorTree));
                }
                availableValues.add(inst);
                break;

            case kIROp_StructuredBufferLoad:
            case kIROp_RWStructuredBufferLoad:
                {
                    IRInst* buffer = inst->getOperand(0);
                    IRInst* elementIndex = inst->getOperand(1);
                    NVVMRawBufferType bufferType;
                    if (!buffer ||
                        !getNVVMSupportedRawBufferType(buffer->getDataType(), bufferType) ||
                        bufferType.kind != NVVMRawBufferKind::Structured ||
                        bufferType.access != (inst->getOp() == kIROp_StructuredBufferLoad
                                                  ? NVVMBufferAccess::ReadOnly
                                                  : NVVMBufferAccess::ReadWrite) ||
                        !isNVVMRawBufferElementType(bufferType, inst->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("raw structured-buffer numeric relation"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                        codeGenContext,
                        elementIndex,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_ByteAddressBufferLoad:
            case kIROp_ByteAddressBufferStore:
                {
                    NVVMByteAddressAccess access;
                    SLANG_RELEASE_ASSERT(_getNVVMByteAddressAccess(inst, access));
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        access.buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateUnsignedI32Value(
                        codeGenContext,
                        access.byteOffset,
                        inst,
                        availableValues,
                        dominatorTree,
                        toSlice("byte-address offset")));
                    if (access.isStore)
                    {
                        SLANG_RETURN_ON_FAIL(_validateByteAddressValue(
                            codeGenContext,
                            access.value,
                            inst,
                            availableValues,
                            dominatorTree));
                    }
                    else
                    {
                        availableValues.add(inst);
                    }
                }
                break;

            case kIROp_RWStructuredBufferGetElementPtr:
                {
                    IRInst* buffer = inst->getOperand(0);
                    IRInst* elementIndex = inst->getOperand(1);
                    NVVMRawBufferType bufferType;
                    auto resultPointerType =
                        asNVVMSupportedRWStructuredBufferElementPointerType(inst->getDataType());
                    if (!buffer ||
                        !getNVVMSupportedRawBufferType(buffer->getDataType(), bufferType) ||
                        bufferType.kind != NVVMRawBufferKind::Structured ||
                        bufferType.access != NVVMBufferAccess::ReadWrite || !resultPointerType ||
                        !isNVVMRawBufferElementType(bufferType, resultPointerType->getValueType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("raw RWStructuredBuffer numeric relation"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        buffer,
                        inst,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateInteger32Value(
                        codeGenContext,
                        elementIndex,
                        inst,
                        availableValues,
                        dominatorTree));
                    availableValues.add(inst);
                }
                break;

            case kIROp_Return:
                {
                    auto returnInst = cast<IRReturn>(inst);
                    if (returnInst != terminator || !returnInst->getVal())
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("return value"));
                    if (isEntryPoint)
                    {
                        if (returnInst->getVal()->getOp() != kIROp_VoidLit)
                            return _diagnoseUnsupportedIR(codeGenContext, toSlice("return value"));
                    }
                    else
                    {
                        if (!isTypeEqual(
                                returnInst->getVal()->getDataType(),
                                function->getResultType()))
                        {
                            return _diagnoseUnsupportedIR(
                                codeGenContext,
                                toSlice("helper return type"));
                        }
                        if (as<IRVoidType>(function->getResultType()))
                        {
                            if (returnInst->getVal()->getOp() != kIROp_VoidLit)
                            {
                                return _diagnoseUnsupportedIR(
                                    codeGenContext,
                                    toSlice("void helper return"));
                            }
                        }
                        else
                        {
                            if (asNVVMSupportedScalarStructType(function->getResultType()))
                            {
                                SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                                    codeGenContext,
                                    returnInst->getVal(),
                                    returnInst,
                                    availableValues,
                                    dominatorTree));
                            }
                            else
                            {
                                SLANG_RETURN_ON_FAIL(_validateSelectedValue(
                                    codeGenContext,
                                    returnInst->getVal(),
                                    returnInst,
                                    availableValues,
                                    dominatorTree));
                            }
                        }
                        hasHelperReturn = true;
                    }
                }
                break;

            case kIROp_UnconditionalBranch:
                {
                    auto branch = cast<IRUnconditionalBranch>(inst);
                    if (branch != terminator)
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("branch position"));
                    SLANG_RETURN_ON_FAIL(_validateBranchArguments(
                        codeGenContext,
                        branch,
                        entryBlock,
                        functionBlocks,
                        availableValues,
                        dominatorTree));
                }
                break;

            case kIROp_Loop:
                {
                    auto loop = cast<IRLoop>(inst);
                    if (loop != terminator)
                        return _diagnoseUnsupportedIR(codeGenContext, toSlice("loop position"));
                    SLANG_RETURN_ON_FAIL(_validateBranchArguments(
                        codeGenContext,
                        loop,
                        entryBlock,
                        functionBlocks,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        loop->getBreakBlock(),
                        functionBlocks));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        loop->getContinueBlock(),
                        functionBlocks));
                }
                break;

            case kIROp_IfElse:
                {
                    auto ifElse = cast<IRIfElse>(inst);
                    if (ifElse != terminator)
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch position"));
                    }
                    if (!ifElse->getCondition() ||
                        !isNVVMBoolType(ifElse->getCondition()->getDataType()))
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch condition"));
                    }
                    SLANG_RETURN_ON_FAIL(_validateAvailableValue(
                        codeGenContext,
                        ifElse->getCondition(),
                        ifElse,
                        availableValues,
                        dominatorTree));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        ifElse->getTrueBlock(),
                        functionBlocks));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        ifElse->getFalseBlock(),
                        functionBlocks));
                    SLANG_RETURN_ON_FAIL(_validateBlockTarget(
                        codeGenContext,
                        ifElse->getAfterBlock(),
                        functionBlocks));
                    if (ifElse->getTrueBlock()->getFirstParam() ||
                        ifElse->getFalseBlock()->getFirstParam())
                    {
                        return _diagnoseUnsupportedIR(
                            codeGenContext,
                            toSlice("conditional branch target parameter"));
                    }
                }
                break;

            default:
                SLANG_UNEXPECTED("NVVM validation reached an unclassified instruction");
            }
        }
    }

    if (!isEntryPoint && !hasHelperReturn)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("helper return"));

    // Every non-entry phi needs at least one actual CFG predecessor. Structural `IRLoop`
    // break/continue and `IRIfElse::afterBlock` operands are deliberately absent from this list.
    for (auto block : function->getBlocks())
    {
        if (block == entryBlock || !block->getFirstParam())
            continue;

        auto predecessors = block->getPredecessors();
        if (predecessors.isEmpty())
            return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block predecessor"));
        for (auto predecessor : predecessors)
        {
            auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
            if (!branch || branch->getTargetBlock() != block)
            {
                return _diagnoseUnsupportedIR(
                    codeGenContext,
                    toSlice("parameterized predecessor edge"));
            }
        }
    }
    return SLANG_OK;
}

using NVVMValueMap = Dictionary<IRInst*, SlangNVVMValueHandle>;

// Returns an already-lowered SSA value or materializes an exact preflighted scalar literal.
SlangResult _getLoweredNVVMValue(
    CodeGenContext* codeGenContext,
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    IRInst* irValue,
    NVVMValueMap& valueMap,
    NVVMTypeLoweringContext& typeContext,
    SlangNVVMValueHandle& outValue)
{
    outValue = nullptr;
    if (auto mappedValue = valueMap.tryGetValue(irValue))
    {
        outValue = *mappedValue;
        return SLANG_OK;
    }

    SlangNVVMValueOperation executionOperation = 0;
    if (_getNVVMCUDAExecutionGlobalOperation(irValue, executionOperation))
    {
        const SlangNVVMValueOperationDesc operation =
            _getNVVMCUDAExecutionGlobalOperationDesc(executionOperation);
        const NVVMSemantics::CatalogEntry* semantic = NVVMSemantics::find(operation);
        SLANG_RELEASE_ASSERT(semantic);
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            semantic->diagnosticName,
            builder.emitValueOperation(module, operation, nullptr, 0, outValue)));
        valueMap[irValue] = outValue;
        return SLANG_OK;
    }

    if (auto intLit = _asExecutableSelectedIntegerConstant(irValue))
    {
        SlangNVVMTypeHandle integerType = nullptr;
        IRIntegerValue integerValue = intLit->getValue();
        uint32_t bitWidth = 0;
        bool isSigned = false;
        SLANG_RELEASE_ASSERT(
            isNVVMSupportedIntegerScalarType(intLit->getDataType(), &bitWidth, &isSigned));
        if (!isSigned && bitWidth < 64 && integerValue >= (IRIntegerValue(1) << (bitWidth - 1)))
            integerValue -= IRIntegerValue(1) << bitWidth;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(intLit->getDataType(), NVVMTypeUse::Value, integerType));
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "selected integer constant",
            builder.getIntegerConstant(module, integerType, int64_t(integerValue), outValue)));
        valueMap[irValue] = outValue;
        return SLANG_OK;
    }

    if (auto boolLit = _asExecutableBoolConstant(irValue))
    {
        SlangNVVMTypeHandle boolType = nullptr;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(boolLit->getDataType(), NVVMTypeUse::Value, boolType));
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "Boolean constant",
            builder.getIntegerConstant(module, boolType, boolLit->getValue() ? 1 : 0, outValue)));
        valueMap[irValue] = outValue;
        return SLANG_OK;
    }

    auto floatLit = _asExecutableFloatingPointConstant(irValue);
    SLANG_RELEASE_ASSERT(floatLit);
    SlangNVVMTypeHandle floatingPointType = nullptr;
    SLANG_RETURN_ON_FAIL(
        typeContext.lowerType(floatLit->getDataType(), NVVMTypeUse::Value, floatingPointType));
    uint32_t bitWidth = 0;
    SLANG_RELEASE_ASSERT(
        isNVVMSupportedFloatingPointScalarType(floatLit->getDataType(), &bitWidth));
    const uint64_t bitPattern = bitWidth == 16
                                    ? uint64_t(FloatToHalf(float(floatLit->getValue())))
                                    : uint64_t(uint32_t(FloatAsInt(float(floatLit->getValue()))));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        bitWidth == 16 ? "float16 constant" : "float32 constant",
        builder
            .getFloatingPointConstant(module, floatingPointType, bitWidth, bitPattern, outValue)));
    valueMap[irValue] = outValue;
    return SLANG_OK;
}

} // namespace

SlangResult foldNVVMCompileTimeLayoutQueries(CodeGenContext* codeGenContext, LinkedIR& linkedIR)
{
    if (!linkedIR.module)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("CUDA layout query module"));

    struct Fold
    {
        IRCall* call = nullptr;
        IRIntegerValue value = 0;
    };
    List<Fold> folds;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                auto call = as<IRCall>(inst);
                auto callee = call ? as<IRFunc>(call->getCallee()) : nullptr;
                NVVMCUDALayoutQuery query;
                if (!callee || !_getNVVMCUDALayoutQuery(callee, query))
                    continue;

                IRIntegerValue value = 0;
                if (!_getNVVMCUDALayoutQueryValue(codeGenContext, call, callee, query, value))
                {
                    return _diagnoseUnsupportedIR(codeGenContext, toSlice("CUDA layout query"));
                }
                folds.add({call, value});
            }
        }
    }

    if (!folds.getCount())
        return SLANG_OK;

    IRBuilder builder(linkedIR.module);
    for (const Fold& fold : folds)
    {
        IRInst* constant = builder.getIntValue(fold.call->getDataType(), fold.value);
        fold.call->replaceUsesWith(constant);
        fold.call->removeAndDeallocate();
    }

    IRDeadCodeEliminationOptions options;
    options.keepLayoutsAlive = true;
    eliminateDeadCode(linkedIR.module, options);

    // A value-form query can be the only consumer of an aggregate initializer. For example,
    // `__offsetOf(gp, gp.block)` leaves the zero-initializing `TestGlobalParams.$init(0, null)`
    // call dead after the query is folded. The initializer is marked read-none, but generic DCE
    // conservatively retains it because a pointer-typed argument might normally expose writable
    // memory. A literal null default cannot expose storage, so this exact dead call has no
    // observable effect and can be removed without lowering its temporary aggregate construction.
    List<IRCall*> deadAggregateCalls;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                auto call = as<IRCall>(inst);
                if (!call || call->hasUses() || !as<IRStructType>(call->getDataType()))
                    continue;

                auto callee = getResolvedInstForDecorations(call->getCallee());
                auto constructor =
                    callee ? callee->findDecoration<IRConstructorDecoration>() : nullptr;
                if (!callee || !callee->findDecoration<IRReadNoneDecoration>() || !constructor ||
                    !constructor->getSynthesizedStatus())
                    continue;

                bool hasOnlySideEffectFreeArguments = true;
                for (UInt i = 0; i < call->getArgCount(); ++i)
                {
                    auto argument = call->getArg(i);
                    if (isValueType(argument->getDataType()))
                        continue;
                    auto pointerLiteral = as<IRPtrLit>(argument);
                    if (!pointerLiteral || pointerLiteral->getValue())
                    {
                        hasOnlySideEffectFreeArguments = false;
                        break;
                    }
                }
                if (hasOnlySideEffectFreeArguments)
                    deadAggregateCalls.add(call);
            }
        }
    }
    for (auto call : deadAggregateCalls)
        call->removeAndDeallocate();
    if (deadAggregateCalls.getCount())
    {
        eliminateDeadCode(linkedIR.module, options);
    }
    return SLANG_OK;
}

SlangResult validateNVVMSupportedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    NVVMValueOperationRequirements& outRequirements)
{
    outRequirements = {};
    if (!linkedIR.module || linkedIR.entryPoints.getCount() != 1)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point count"));

    IRFunc* entryPoint = linkedIR.entryPoints[0];
    if (!entryPoint || entryPoint->getParent() != linkedIR.module->getModuleInst() ||
        !entryPoint->isDefinition())
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point definition"));
    }

    auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
    if (!entryPointDecoration)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point decoration"));
    if (entryPointDecoration->getProfile().getStage() != Stage::Compute)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point stage"));
    if (!entryPointDecoration->getName()->getStringSlice().getLength())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point name"));
    if (!as<IRVoidType>(entryPoint->getResultType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point result type"));

    List<IRFunc*> functions;
    HashSet<IRFunc*> functionSet;
    SLANG_RETURN_ON_FAIL(
        _collectNVVMFunctions(codeGenContext, linkedIR, entryPoint, functions, functionSet));
    SLANG_RETURN_ON_FAIL(
        _validateNVVMSymbolNames(codeGenContext, linkedIR.module, entryPoint, functions));
    SLANG_RETURN_ON_FAIL(_validateNVVMFunctionUses(codeGenContext, functions));

    for (auto function : functions)
    {
        SLANG_RETURN_ON_FAIL(_validateNVVMFunction(
            codeGenContext,
            entryPoint,
            function,
            functionSet,
            outRequirements));
    }

    // Scalar CUDA launch parameters and executable scalar operations are meaningful only for a
    // CUDA kernel. Preserve Slice 6's conventional zero-parameter empty compute entry point, but
    // do not invent a raw CUDA launch ABI for an ordinary shader entry point.
    bool requiresCUDAKernel = functions.getCount() > 1 || entryPoint->getParamCount() != 0;
    for (auto function : functions)
    {
        for (auto block : function->getBlocks())
        {
            requiresCUDAKernel = requiresCUDAKernel || block != function->getFirstBlock();
            for (auto inst : block->getOrdinaryInsts())
                requiresCUDAKernel = requiresCUDAKernel || inst->getOp() != kIROp_Return;
        }
    }
    bool hasConventionalCUDAABI = false;
    NVVMConventionalGlobalParams conventionalGlobalParams;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        NVVMConventionalGlobalParams globalParams;
        SlangNVVMValueOperation executionOperation = 0;
        if (_getNVVMConventionalGlobalParams(globalInst, globalParams))
            conventionalGlobalParams = globalParams;
        hasConventionalCUDAABI =
            hasConventionalCUDAABI || globalParams.globalParam ||
            _getNVVMCUDAExecutionGlobalOperation(globalInst, executionOperation);
    }
    if (requiresCUDAKernel && !entryPoint->findDecoration<IRCudaKernelDecoration>() &&
        !hasConventionalCUDAABI)
    {
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("CUDA kernel decoration"));
    }

    HashSet<IRInst*> selectedReachableStructTypes;
    if (conventionalGlobalParams.elementType)
    {
        for (auto field : conventionalGlobalParams.elementType->getFields())
        {
            NVVMRawBufferType rawBufferType;
            if (!getNVVMSupportedRawBufferType(field->getFieldType(), rawBufferType) ||
                rawBufferType.kind != NVVMRawBufferKind::Structured)
            {
                continue;
            }
            if (auto elementStruct =
                    asNVVMSupportedCopyableStructType(rawBufferType.structuredElementType))
            {
                if (!_hasNVVMCompatibleCopyableStructLayout(codeGenContext, elementStruct))
                {
                    return _diagnoseUnsupportedIR(
                        codeGenContext,
                        toSlice("structured-buffer aggregate layout"));
                }
                selectedReachableStructTypes.add(elementStruct);
            }
        }
    }

    for (auto function : functions)
    {
        if (auto resultType = asNVVMSupportedScalarStructType(function->getResultType()))
            selectedReachableStructTypes.add(resultType);
        for (auto parameter : function->getParams())
        {
            if (auto parameterType = asNVVMSupportedScalarStructType(parameter->getDataType()))
                selectedReachableStructTypes.add(parameterType);
            IRStructType* pointerValueType = nullptr;
            if (asNVVMSupportedLocalScalarStructPointerType(
                    parameter->getDataType(),
                    &pointerValueType))
            {
                selectedReachableStructTypes.add(pointerValueType);
            }
        }
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                IRStructType* localValueType = nullptr;
                if (inst->getOp() == kIROp_Var && asNVVMSupportedLocalCopyableStructPointerType(
                                                      inst->getDataType(),
                                                      &localValueType))
                {
                    selectedReachableStructTypes.add(localValueType);
                }
            }
        }
    }
    // Linking can retain module-scope types, layouts, capabilities, and constants needed to spell
    // the reachable functions. IRStructKey is also layout-only identity retained for raw CUDA
    // parameter layouts. A selected struct used by a reachable signature, local, or raw structured
    // buffer is its canonical value type, not an unrelated dropped global. Reject every other
    // semantic global so this emitter cannot silently drop a function, parameter, initializer, or
    // storage object.
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        if (auto globalFunction = as<IRFunc>(globalInst))
        {
            if (functionSet.contains(globalFunction))
                continue;
            return _diagnoseUnsupportedIR(
                codeGenContext,
                UnownedStringSlice(getIROpInfo(globalInst->getOp()).name));
        }
        if (as<IRGlobalVar>(globalInst))
        {
            if (asNVVMSupportedSharedI32ArrayGlobal(globalInst))
                continue;
            return _diagnoseUnsupportedIR(
                codeGenContext,
                UnownedStringSlice(getIROpInfo(globalInst->getOp()).name));
        }
        NVVMConventionalGlobalParams globalParams;
        if (_getNVVMConventionalGlobalParams(globalInst, globalParams))
            continue;
        SlangNVVMValueOperation executionOperation = 0;
        if (_getNVVMCUDAExecutionGlobalOperation(globalInst, executionOperation))
        {
            const SlangNVVMValueOperationDesc desc =
                _getNVVMCUDAExecutionGlobalOperationDesc(executionOperation);
            const NVVMSemantics::CatalogEntry* semantic = NVVMSemantics::find(desc);
            SLANG_RELEASE_ASSERT(semantic);
            _requireValueOperation(outRequirements, desc, semantic->diagnosticName);
            continue;
        }
        if (as<IRDecoration>(globalInst) || as<IRConstant>(globalInst) ||
            as<IRStructKey>(globalInst) || getIROpInfo(globalInst->getOp()).isHoistable())
        {
            continue;
        }
        if (selectedReachableStructTypes.contains(globalInst))
            continue;
        if (_isNVVMConventionalGlobalStorageType(conventionalGlobalParams, globalInst))
            continue;
        return _diagnoseUnsupportedIR(
            codeGenContext,
            UnownedStringSlice(getIROpInfo(globalInst->getOp()).name));
    }

    return SLANG_OK;
}
SlangResult emitNVVMIRFromLinkedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    const NVVMIRBuilder& builder,
    const NVVMValueOperationRequirements& requirements,
    ComPtr<IArtifact>& outArtifact)
{
    outArtifact.setNull();
    SLANG_RELEASE_ASSERT(linkedIR.entryPoints.getCount() == 1);

    // Capability queries are pure. Complete this exact typed preflight before module creation so
    // an unsupported overload cannot leave partial provider state behind.
    for (const auto& requirement : requirements)
    {
        if (!builder.supportsValueOperation(requirement.getDesc()))
        {
            return _requireBuilderOperation(
                codeGenContext,
                requirement.diagnosticName,
                SLANG_E_NOT_AVAILABLE);
        }
    }

    IRFunc* entryPoint = linkedIR.entryPoints[0];
    auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
    SLANG_RELEASE_ASSERT(entryPointDecoration);

    // Reuse preflight's exact closure walk so the accepted and emitted function sets cannot drift.
    List<IRFunc*> functions;
    HashSet<IRFunc*> functionSet;
    SLANG_RETURN_ON_FAIL(
        _collectNVVMFunctions(codeGenContext, linkedIR, entryPoint, functions, functionSet));

    ScopedNVVMModule moduleScope;
    moduleScope.builder = &builder;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "module creation",
        builder.createModule(toSlice("slang-direct-nvvm"), moduleScope.module)));

    NVVMTypeLoweringContext typeContext(codeGenContext, builder, moduleScope.module);
    Dictionary<IRFunc*, SlangNVVMValueHandle> functionMap;
    NVVMValueMap valueMap;
    Dictionary<IRBlock*, SlangNVVMBlockHandle> blockMap;

    // The canonical global owns storage class, value type, extent, and name. Lower those facts once
    // before any function declaration; ordinary body uses then resolve through the shared value
    // map.
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        NVVMConventionalGlobalParams globalParams;
        if (_getNVVMConventionalGlobalParams(globalInst, globalParams))
        {
            SlangNVVMTypeHandle loweredStructType = nullptr;
            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                globalParams.elementType,
                NVVMTypeUse::Storage,
                loweredStructType));
            SlangNVVMValueHandle loweredStorage = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "conventional global parameter storage declaration",
                builder.declareGlobalStorage(
                    moduleScope.module,
                    loweredStructType,
                    SLANG_NVVM_LINKAGE_EXTERNAL,
                    SLANG_NVVM_ADDRESS_SPACE_CONSTANT,
                    kNVVMPointerAlignment,
                    toSlice("SLANG_globalParams"),
                    loweredStorage)));
            valueMap[globalParams.globalParam] = loweredStorage;
            continue;
        }

        IRArrayType* arrayType = nullptr;
        auto globalVar = asNVVMSupportedSharedI32ArrayGlobal(globalInst, &arrayType);
        if (!globalVar)
            continue;

        SlangNVVMTypeHandle loweredArrayType = nullptr;
        SLANG_RETURN_ON_FAIL(
            typeContext.lowerType(arrayType, NVVMTypeUse::Value, loweredArrayType));
        SlangNVVMValueHandle loweredStorage = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "shared global storage declaration",
            builder.declareGlobalStorage(
                moduleScope.module,
                loweredArrayType,
                SLANG_NVVM_LINKAGE_INTERNAL,
                SLANG_NVVM_ADDRESS_SPACE_SHARED,
                kNVVMScalar32Alignment,
                getMangledName(globalVar),
                loweredStorage)));
        valueMap[globalVar] = loweredStorage;
    }

    // Every function is declared before any body is emitted. A call can therefore target a helper
    // that appears later in linked-IR order without turning physical order into a legality rule.
    for (auto function : functions)
    {
        const bool isEntryPoint = function == entryPoint;
        SlangNVVMTypeHandle resultType = nullptr;
        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
            function->getResultType(),
            isEntryPoint ? NVVMTypeUse::EntryPointResult : NVVMTypeUse::HelperResult,
            resultType));

        List<SlangNVVMTypeHandle> parameterTypes;
        for (auto param : function->getParams())
        {
            SlangNVVMTypeHandle parameterType = nullptr;
            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                param->getDataType(),
                isEntryPoint ? NVVMTypeUse::EntryPointParameter : NVVMTypeUse::HelperParameter,
                parameterType));
            parameterTypes.add(parameterType);
        }

        SlangNVVMTypeHandle functionType = nullptr;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "function type",
            builder.getFunctionType(
                moduleScope.module,
                resultType,
                parameterTypes.getCount() ? parameterTypes.getBuffer() : nullptr,
                size_t(parameterTypes.getCount()),
                functionType)));

        SlangNVVMValueHandle loweredFunction = nullptr;
        const bool isExported =
            isEntryPoint || function->findDecorationImpl(kIROp_CudaDeviceExportDecoration);
        const SlangNVVMLinkage linkage =
            isExported ? SLANG_NVVM_LINKAGE_EXTERNAL : SLANG_NVVM_LINKAGE_INTERNAL;
        SlangNVVMFunctionFlags flags = SLANG_NVVM_FUNCTION_FLAG_NONE;
        if (!isEntryPoint && function->findDecoration<IRNoInlineDecoration>())
            flags |= SLANG_NVVM_FUNCTION_FLAG_NO_INLINE;
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            codeGenContext,
            "function declaration",
            builder.declareFunction(
                moduleScope.module,
                functionType,
                linkage,
                flags,
                _getNVVMFunctionName(function, entryPoint),
                loweredFunction)));
        if (isEntryPoint)
        {
            size_t parameterIndex = 0;
            for (auto parameter : function->getParams())
            {
                if (asNVVMSupportedScalarStructType(parameter->getDataType()))
                {
                    SlangNVVMTypeHandle aggregateType = nullptr;
                    SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                        parameter->getDataType(),
                        NVVMTypeUse::Value,
                        aggregateType));
                    uint32_t alignment = 0;
                    SLANG_RELEASE_ASSERT(_getNVVMByValueParameterAlignment(
                        codeGenContext,
                        parameter->getDataType(),
                        alignment));
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        "by-value aggregate parameter attributes",
                        builder.setFunctionParameterAttributes(
                            moduleScope.module,
                            loweredFunction,
                            parameterIndex,
                            SLANG_NVVM_PARAMETER_FLAG_BY_VALUE,
                            aggregateType,
                            alignment)));
                }
                ++parameterIndex;
            }
        }
        functionMap[function] = loweredFunction;
    }

    for (auto function : functions)
    {
        size_t parameterIndex = 0;
        for (auto param : function->getParams())
        {
            SlangNVVMValueHandle parameter = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "function parameter",
                builder.getFunctionParameter(
                    moduleScope.module,
                    functionMap.getValue(function),
                    parameterIndex,
                    parameter)));
            valueMap[param] = parameter;
            ++parameterIndex;
        }
    }

    for (auto function : functions)
    {
        // LLVM branches can refer to blocks declared later, so create this function's complete CFG
        // before emitting any body instruction.
        Index blockIndex = 0;
        for (auto block : function->getBlocks())
        {
            StringBuilder nameBuilder;
            if (blockIndex == 0)
                nameBuilder << "entry";
            else
                nameBuilder << "block" << blockIndex;
            String blockName = nameBuilder.produceString();

            SlangNVVMBlockHandle loweredBlock = nullptr;
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "basic-block creation",
                builder.createBlock(
                    moduleScope.module,
                    functionMap.getValue(function),
                    blockName.getUnownedSlice(),
                    loweredBlock)));
            blockMap[block] = loweredBlock;
            ++blockIndex;
        }

        // Consider the loop header header(i, sum). Its phis must exist before the compare and body
        // use them, while their backedge values are not emitted until later blocks. Create every
        // phi placeholder now; incoming pairs are attached after all bodies and terminators exist.
        IRBlock* entryBlock = function->getFirstBlock();
        for (auto block : function->getBlocks())
        {
            if (block == entryBlock)
                continue;

            for (auto param : block->getParams())
            {
                SlangNVVMTypeHandle parameterType = nullptr;
                SLANG_RETURN_ON_FAIL(
                    typeContext.lowerType(param->getDataType(), NVVMTypeUse::Value, parameterType));
                SlangNVVMValueHandle loweredPhi = nullptr;
                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                    codeGenContext,
                    isNVVMSignedI32Type(param->getDataType()) ? "signed i32 phi"
                                                              : "generic value phi",
                    isNVVMSignedI32Type(param->getDataType()) ? builder.emitIntegerPhi(
                                                                    moduleScope.module,
                                                                    blockMap.getValue(block),
                                                                    parameterType,
                                                                    loweredPhi)
                                                              : builder.emitPhi(
                                                                    moduleScope.module,
                                                                    blockMap.getValue(block),
                                                                    parameterType,
                                                                    loweredPhi)));
                valueMap[param] = loweredPhi;
            }
        }

        RefPtr<IRDominatorTree> dominatorTree = computeDominatorTree(function);
        List<IRBlock*> bodyOrder = _getNVVMBodyOrder(function, dominatorTree);
        for (auto block : bodyOrder)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                codeGenContext,
                "insertion-block selection",
                builder.setInsertBlock(moduleScope.module, blockMap.getValue(block))));

            for (auto inst : block->getOrdinaryInsts())
            {
                switch (inst->getOp())
                {
                case kIROp_Var:
                    {
                        IRStructType* valueType = nullptr;
                        SLANG_RELEASE_ASSERT(asNVVMSupportedLocalCopyableStructPointerType(
                            inst->getDataType(),
                            &valueType));
                        SlangNVVMTypeHandle loweredValueType = nullptr;
                        SLANG_RETURN_ON_FAIL(
                            typeContext.lowerType(valueType, NVVMTypeUse::Value, loweredValueType));
                        const uint32_t alignment = getNVVMCopyableValueAlignment(valueType);
                        SLANG_RELEASE_ASSERT(alignment);
                        SlangNVVMValueHandle loweredStorage = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "local copyable-struct storage",
                            builder.emitLocalStorage(
                                moduleScope.module,
                                loweredValueType,
                                alignment,
                                toSlice("slangLocal"),
                                loweredStorage)));
                        valueMap[inst] = loweredStorage;
                    }
                    break;

                case kIROp_Load:
                    {
                        auto load = cast<IRLoad>(inst);
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            load->getPtr(),
                            valueMap,
                            typeContext,
                            loweredPointer));
                        SlangNVVMValueHandle loweredValue = nullptr;
                        uint32_t alignment = getNVVMCopyableValueAlignment(load->getDataType());
                        NVVMRawBufferType rawBufferType;
                        if (getNVVMSupportedRawBufferType(load->getDataType(), rawBufferType) ||
                            asNVVMSupportedScalarParameterGroupType(load->getDataType()))
                        {
                            alignment = kNVVMPointerAlignment;
                        }
                        SLANG_RELEASE_ASSERT(alignment);
                        const SlangNVVMLoadFlags loadFlags =
                            isPointerToImmutableLocation(getRootAddr(load->getPtr()))
                                ? SLANG_NVVM_LOAD_FLAG_INVARIANT
                                : SLANG_NVVM_LOAD_FLAG_NONE;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "value load",
                            builder.emitLoad(
                                moduleScope.module,
                                loweredPointer,
                                alignment,
                                loadFlags,
                                loweredValue)));
                        valueMap[load] = loweredValue;
                    }
                    break;

                case kIROp_Store:
                    {
                        auto store = cast<IRStore>(inst);
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store->getVal(),
                            valueMap,
                            typeContext,
                            loweredValue));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store->getPtr(),
                            valueMap,
                            typeContext,
                            loweredPointer));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "value store",
                            builder.emitStore(
                                moduleScope.module,
                                loweredValue,
                                loweredPointer,
                                getNVVMCopyableValueAlignment(store->getVal()->getDataType()))));
                    }
                    break;

                case kIROp_SwizzledStore:
                    {
                        NVVMVectorSwizzledStore store;
                        SLANG_RELEASE_ASSERT(_getNVVMVectorSwizzledStore(inst, store));
                        SlangNVVMValueHandle loweredDestination = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store.destination,
                            valueMap,
                            typeContext,
                            loweredDestination));
                        SlangNVVMValueHandle loweredSource = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            store.source,
                            valueMap,
                            typeContext,
                            loweredSource));
                        SlangNVVMTypeHandle loweredElementType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            store.elementType,
                            NVVMTypeUse::Value,
                            loweredElementType));
                        SlangNVVMTypeHandle loweredIndexType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            cast<IRSwizzledStore>(inst)->getElementIndex(0)->getDataType(),
                            NVVMTypeUse::Value,
                            loweredIndexType));

                        for (uint32_t sourceIndex = 0; sourceIndex < store.sourceElementCount;
                             ++sourceIndex)
                        {
                            SlangNVVMValueHandle loweredSourceElement = loweredSource;
                            if (store.sourceElementCount > 1)
                            {
                                SlangNVVMValueHandle loweredSourceIndex = nullptr;
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "numeric vector swizzled-store source index",
                                    builder.getIntegerConstant(
                                        moduleScope.module,
                                        loweredIndexType,
                                        sourceIndex,
                                        loweredSourceIndex)));
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "numeric vector swizzled-store extraction",
                                    builder.emitVectorElementExtract(
                                        moduleScope.module,
                                        loweredSource,
                                        loweredSourceIndex,
                                        loweredSourceElement)));
                            }

                            SlangNVVMValueHandle loweredByteOffset = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "numeric vector swizzled-store byte offset",
                                builder.getIntegerConstant(
                                    moduleScope.module,
                                    loweredIndexType,
                                    int64_t(store.destinationIndices[sourceIndex] * 4),
                                    loweredByteOffset)));
                            SlangNVVMValueHandle loweredElementPointer = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "numeric vector swizzled-store element pointer",
                                builder.emitByteOffsetPointer(
                                    moduleScope.module,
                                    loweredDestination,
                                    loweredByteOffset,
                                    loweredElementType,
                                    loweredElementPointer)));
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "numeric vector swizzled-store element",
                                builder.emitStore(
                                    moduleScope.module,
                                    loweredSourceElement,
                                    loweredElementPointer,
                                    kNVVMScalar32Alignment)));
                        }
                    }
                    break;

                case kIROp_Add:
                case kIROp_Sub:
                case kIROp_Mul:
                case kIROp_Div:
                case kIROp_IRem:
                case kIROp_FRem:
                case kIROp_Lsh:
                case kIROp_Rsh:
                case kIROp_BitAnd:
                case kIROp_BitOr:
                case kIROp_BitXor:
                case kIROp_BitNot:
                case kIROp_And:
                case kIROp_Or:
                case kIROp_Not:
                case kIROp_Neg:
                case kIROp_Less:
                case kIROp_Eql:
                case kIROp_Neq:
                case kIROp_Greater:
                case kIROp_Leq:
                case kIROp_Geq:
                case kIROp_IntCast:
                case kIROp_CastIntToFloat:
                case kIROp_CastFloatToInt:
                case kIROp_FloatCast:
                case kIROp_WaveMaskBallot:
                    {
                        NVVMResolvedValueOperation operation;
                        SLANG_RELEASE_ASSERT(_resolveNVVMValueOperation(inst, operation));
                        SlangNVVMValueHandle loweredOperands[3] = {};
                        for (UInt operandIndex = 0; operandIndex < inst->getOperandCount();
                             ++operandIndex)
                        {
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                inst->getOperand(operandIndex),
                                valueMap,
                                typeContext,
                                loweredOperands[operandIndex]));
                        }

                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            operation.diagnosticName,
                            builder.emitValueOperation(
                                moduleScope.module,
                                operation.desc,
                                inst->getOperandCount() ? loweredOperands : nullptr,
                                inst->getOperandCount(),
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_AtomicAdd:
                    {
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            typeContext,
                            loweredPointer));
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredValue));
                        SlangNVVMValueHandle loweredOriginalValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "relaxed global signed i32 atomic add",
                            builder.emitRelaxedGlobalI32AtomicAdd(
                                moduleScope.module,
                                loweredPointer,
                                loweredValue,
                                loweredOriginalValue)));
                        valueMap[inst] = loweredOriginalValue;
                    }
                    break;


                case kIROp_Call:
                    {
                        auto call = cast<IRCall>(inst);
                        auto callee = cast<IRFunc>(call->getOperand(0));
                        List<SlangNVVMValueHandle> loweredArguments;
                        for (UInt argumentIndex = 0; argumentIndex < call->getArgCount();
                             ++argumentIndex)
                        {
                            SlangNVVMValueHandle loweredArgument = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                call->getArg(argumentIndex),
                                valueMap,
                                typeContext,
                                loweredArgument));
                            loweredArguments.add(loweredArgument);
                        }

                        SlangNVVMValueHandle loweredValue = nullptr;
                        const bool usesGenericFunctions = _usesGenericNVVMFunctions(callee);
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            usesGenericFunctions ? "generic value call" : "signed i32 call",
                            usesGenericFunctions
                                ? builder.emitCall(
                                      moduleScope.module,
                                      functionMap.getValue(callee),
                                      loweredArguments.getCount() ? loweredArguments.getBuffer()
                                                                  : nullptr,
                                      size_t(loweredArguments.getCount()),
                                      loweredValue)
                                : builder.emitIntegerCall(
                                      moduleScope.module,
                                      functionMap.getValue(callee),
                                      loweredArguments.getCount() ? loweredArguments.getBuffer()
                                                                  : nullptr,
                                      size_t(loweredArguments.getCount()),
                                      loweredValue)));
                        valueMap[call] = loweredValue;
                    }
                    break;

                case kIROp_MakeVector:
                case kIROp_MakeVectorFromScalar:
                case kIROp_MakeArray:
                case kIROp_Swizzle:
                case kIROp_SwizzleSet:
                case kIROp_GetElement:
                    {
                        NVVMAggregateElement aggregateElement;
                        if (_getNVVMAggregateElement(inst, aggregateElement))
                        {
                            SlangNVVMValueHandle loweredBase = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                aggregateElement.base,
                                valueMap,
                                typeContext,
                                loweredBase));
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "fixed aggregate element extraction",
                                builder.emitAggregateElementExtract(
                                    moduleScope.module,
                                    loweredBase,
                                    aggregateElement.index,
                                    loweredValue)));
                            valueMap[inst] = loweredValue;
                            break;
                        }

                        NVVMAggregateConstruction aggregateConstruction;
                        if (_getNVVMAggregateConstruction(inst, aggregateConstruction))
                        {
                            List<SlangNVVMValueHandle> loweredElements;
                            for (uint32_t i = 0; i < aggregateConstruction.elementCount; ++i)
                            {
                                SlangNVVMValueHandle loweredElement = nullptr;
                                SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    inst->getOperand(i),
                                    valueMap,
                                    typeContext,
                                    loweredElement));
                                loweredElements.add(loweredElement);
                            }
                            SlangNVVMTypeHandle loweredResultType = nullptr;
                            SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                                aggregateConstruction.resultType,
                                NVVMTypeUse::Value,
                                loweredResultType));
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "fixed aggregate construction",
                                builder.emitAggregateConstruct(
                                    moduleScope.module,
                                    loweredResultType,
                                    loweredElements.getBuffer(),
                                    size_t(loweredElements.getCount()),
                                    loweredValue)));
                            valueMap[inst] = loweredValue;
                            break;
                        }

                        NVVMVectorElement element;
                        NVVMVectorConstruction construction;
                        if (_getNVVMVectorElement(inst, element))
                        {
                            SlangNVVMValueHandle loweredBase = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                element.base,
                                valueMap,
                                typeContext,
                                loweredBase));
                            SlangNVVMValueHandle loweredIndex = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                element.index,
                                valueMap,
                                typeContext,
                                loweredIndex));
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "value vector element extraction",
                                builder.emitVectorElementExtract(
                                    moduleScope.module,
                                    loweredBase,
                                    loweredIndex,
                                    loweredValue)));
                            valueMap[inst] = loweredValue;
                            break;
                        }

                        SLANG_RELEASE_ASSERT(_getNVVMVectorConstruction(inst, construction));
                        SlangNVVMValueHandle loweredElements[4] = {};
                        for (uint32_t i = 0; i < construction.elementCount; ++i)
                        {
                            const NVVMVectorConstructElement& source = construction.elements[i];
                            if (source.value)
                            {
                                SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    source.value,
                                    valueMap,
                                    typeContext,
                                    loweredElements[i]));
                            }
                            else
                            {
                                SlangNVVMValueHandle loweredBase = nullptr;
                                SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                    codeGenContext,
                                    builder,
                                    moduleScope.module,
                                    source.extractedBase,
                                    valueMap,
                                    typeContext,
                                    loweredBase));
                                SlangNVVMTypeHandle loweredIndexType = nullptr;
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "vector extraction index type",
                                    builder
                                        .getIntegerType(moduleScope.module, 32, loweredIndexType)));
                                SlangNVVMValueHandle loweredIndex = nullptr;
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "vector extraction index",
                                    builder.getIntegerConstant(
                                        moduleScope.module,
                                        loweredIndexType,
                                        source.extractedIndex,
                                        loweredIndex)));
                                SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                    codeGenContext,
                                    "numeric vector swizzle extraction",
                                    builder.emitVectorElementExtract(
                                        moduleScope.module,
                                        loweredBase,
                                        loweredIndex,
                                        loweredElements[i])));
                            }
                        }
                        SlangNVVMTypeHandle loweredResultType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            construction.resultType,
                            NVVMTypeUse::Value,
                            loweredResultType));
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "value vector construction",
                            builder.emitVectorConstruct(
                                moduleScope.module,
                                loweredResultType,
                                loweredElements,
                                construction.elementCount,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_GenericAsm:
                    {
                        auto genericAsm = as<IRGenericAsm>(inst);
                        if (auto identityParameter =
                                _getNVVMBoolIdentityGenericAsmParameter(genericAsm, function))
                        {
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                identityParameter,
                                valueMap,
                                typeContext,
                                loweredValue));
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "Boolean identity return",
                                builder.emitValueReturn(moduleScope.module, loweredValue)));
                            break;
                        }

                        const NVVMSemantics::CatalogEntry* semantic =
                            _findNVVMGenericAsmSemantic(genericAsm, function);
                        SLANG_RELEASE_ASSERT(semantic);
                        List<SlangNVVMValueHandle> loweredArguments;
                        for (auto parameter : function->getParams())
                        {
                            SlangNVVMValueHandle loweredArgument = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                parameter,
                                valueMap,
                                typeContext,
                                loweredArgument));
                            loweredArguments.add(loweredArgument);
                        }
                        SlangNVVMValueHandle loweredValue = nullptr;
                        const SlangNVVMValueOperationDesc operation =
                            NVVMSemantics::getOperationDesc(*semantic);
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            semantic->diagnosticName,
                            builder.emitValueOperation(
                                moduleScope.module,
                                operation,
                                loweredArguments.getCount() ? loweredArguments.getBuffer()
                                                            : nullptr,
                                size_t(loweredArguments.getCount()),
                                loweredValue)));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            semantic->resultType.kind == SLANG_NVVM_VALUE_TYPE_VOID
                                ? "void return"
                                : "generic value return",
                            semantic->resultType.kind == SLANG_NVVM_VALUE_TYPE_VOID
                                ? builder.emitReturnVoid(moduleScope.module)
                                : builder.emitValueReturn(moduleScope.module, loweredValue)));
                    }
                    break;


                case kIROp_GetOffsetPtr:
                    {
                        SlangNVVMValueHandle loweredBasePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            typeContext,
                            loweredBasePointer));
                        SlangNVVMValueHandle loweredElementOffset = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredElementOffset));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "device scalar pointer offset",
                            builder.emitPointerOffset(
                                moduleScope.module,
                                loweredBasePointer,
                                loweredElementOffset,
                                loweredPointer)));
                        valueMap[inst] = loweredPointer;
                    }
                    break;

                case kIROp_GetElementPtr:
                    {
                        NVVMRawBufferElementPointer bufferElementPointer;
                        const bool isBufferElement =
                            _getNVVMRawBufferElementPointer(inst, bufferElementPointer);
                        SlangNVVMValueHandle loweredBasePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            typeContext,
                            loweredBasePointer));
                        SlangNVVMValueHandle loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredElementIndex));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        const SlangResult pointerResult = isBufferElement
                                                              ? builder.emitPointerOffset(
                                                                    moduleScope.module,
                                                                    loweredBasePointer,
                                                                    loweredElementIndex,
                                                                    loweredPointer)
                                                              : builder.emitArrayElementPointer(
                                                                    moduleScope.module,
                                                                    loweredBasePointer,
                                                                    loweredElementIndex,
                                                                    loweredPointer);
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            asNVVMSupportedSharedI32ArrayGlobal(inst->getOperand(0))
                                ? "shared i32 array element pointer"
                            : isBufferElement ? "raw buffer scalar element pointer"
                                              : "device i32 array element pointer",
                            pointerResult));
                        valueMap[inst] = loweredPointer;
                    }
                    break;

                case kIROp_GetStructuredBufferPtr:
                case kIROp_GetUntypedBufferPtr:
                    {
                        NVVMRawBufferDataPointer dataPointer;
                        SLANG_RELEASE_ASSERT(_getNVVMRawBufferDataPointer(inst, dataPointer));
                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            dataPointer.buffer,
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw buffer data pointer",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                0,
                                loweredPointer)));
                        valueMap[inst] = loweredPointer;
                    }
                    break;

                case kIROp_FieldAddress:
                    {
                        auto fieldAddress = cast<IRFieldAddress>(inst);
                        NVVMStructField resolvedAddress;
                        SLANG_RELEASE_ASSERT(
                            _getNVVMStructFieldAddress(fieldAddress, resolvedAddress));
                        SlangNVVMValueHandle loweredBasePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            fieldAddress->getBase(),
                            valueMap,
                            typeContext,
                            loweredBasePointer));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            resolvedAddress.isMutableLocal ? "local copyable-struct field address"
                                                           : "global parameter field address",
                            builder.emitStructFieldPointer(
                                moduleScope.module,
                                loweredBasePointer,
                                resolvedAddress.fieldIndex,
                                loweredPointer)));
                        valueMap[inst] = loweredPointer;
                    }
                    break;

                case kIROp_FieldExtract:
                    {
                        auto fieldExtract = cast<IRFieldExtract>(inst);
                        NVVMStructField resolvedField;
                        SLANG_RELEASE_ASSERT(_getNVVMStructFieldValue(fieldExtract, resolvedField));
                        SlangNVVMValueHandle loweredBase = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            fieldExtract->getBase(),
                            valueMap,
                            typeContext,
                            loweredBase));
                        SlangNVVMValueHandle loweredFieldPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "by-value aggregate field pointer",
                            builder.emitStructFieldPointer(
                                moduleScope.module,
                                loweredBase,
                                resolvedField.fieldIndex,
                                loweredFieldPointer)));
                        const uint32_t alignment =
                            getNVVMNumericValueAlignment(fieldExtract->getDataType());
                        SLANG_RELEASE_ASSERT(alignment);
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "by-value aggregate field load",
                            builder.emitLoad(
                                moduleScope.module,
                                loweredFieldPointer,
                                alignment,
                                SLANG_NVVM_LOAD_FLAG_INVARIANT,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_StructuredBufferLoad:
                case kIROp_RWStructuredBufferLoad:
                    {
                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredDataPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw StructuredBuffer data pointer",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                0,
                                loweredDataPointer)));
                        SlangNVVMValueHandle loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredElementIndex));
                        SlangNVVMValueHandle loweredElementPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw StructuredBuffer numeric element pointer",
                            builder.emitPointerOffset(
                                moduleScope.module,
                                loweredDataPointer,
                                loweredElementIndex,
                                loweredElementPointer)));
                        const uint32_t alignment =
                            getNVVMNumericValueAlignment(inst->getDataType());
                        SLANG_RELEASE_ASSERT(alignment);
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw structured-buffer numeric load",
                            builder.emitLoad(
                                moduleScope.module,
                                loweredElementPointer,
                                alignment,
                                inst->getOp() == kIROp_StructuredBufferLoad
                                    ? SLANG_NVVM_LOAD_FLAG_INVARIANT
                                    : SLANG_NVVM_LOAD_FLAG_NONE,
                                loweredValue)));
                        valueMap[inst] = loweredValue;
                    }
                    break;

                case kIROp_ByteAddressBufferLoad:
                case kIROp_ByteAddressBufferStore:
                    {
                        NVVMByteAddressAccess access;
                        SLANG_RELEASE_ASSERT(_getNVVMByteAddressAccess(inst, access));

                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            access.buffer,
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredDataPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw byte-address buffer data pointer",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                0,
                                loweredDataPointer)));

                        SlangNVVMValueHandle loweredByteOffset = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            access.byteOffset,
                            valueMap,
                            typeContext,
                            loweredByteOffset));
                        SlangNVVMTypeHandle loweredValueType = nullptr;
                        SLANG_RETURN_ON_FAIL(typeContext.lowerType(
                            access.valueType,
                            NVVMTypeUse::Value,
                            loweredValueType));
                        SlangNVVMValueHandle loweredValuePointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw byte-address buffer byte offset",
                            builder.emitByteOffsetPointer(
                                moduleScope.module,
                                loweredDataPointer,
                                loweredByteOffset,
                                loweredValueType,
                                loweredValuePointer)));

                        if (access.isStore)
                        {
                            SlangNVVMValueHandle loweredValue = nullptr;
                            SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                                codeGenContext,
                                builder,
                                moduleScope.module,
                                access.value,
                                valueMap,
                                typeContext,
                                loweredValue));
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "raw byte-address buffer store",
                                builder.emitStore(
                                    moduleScope.module,
                                    loweredValue,
                                    loweredValuePointer,
                                    access.alignment)));
                        }
                        else
                        {
                            SlangNVVMValueHandle loweredValue = nullptr;
                            const SlangNVVMLoadFlags flags =
                                access.bufferType.access == NVVMBufferAccess::ReadOnly
                                    ? SLANG_NVVM_LOAD_FLAG_INVARIANT
                                    : SLANG_NVVM_LOAD_FLAG_NONE;
                            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                                codeGenContext,
                                "raw byte-address buffer load",
                                builder.emitLoad(
                                    moduleScope.module,
                                    loweredValuePointer,
                                    access.alignment,
                                    flags,
                                    loweredValue)));
                            valueMap[inst] = loweredValue;
                        }
                    }
                    break;

                case kIROp_RWStructuredBufferGetElementPtr:
                    {
                        SlangNVVMValueHandle loweredBuffer = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(0),
                            valueMap,
                            typeContext,
                            loweredBuffer));
                        SlangNVVMValueHandle loweredElementIndex = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            inst->getOperand(1),
                            valueMap,
                            typeContext,
                            loweredElementIndex));
                        SlangNVVMValueHandle loweredPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw RWStructuredBuffer data pointer",
                            builder.emitAggregateElementExtract(
                                moduleScope.module,
                                loweredBuffer,
                                0,
                                loweredPointer)));
                        SlangNVVMValueHandle loweredElementPointer = nullptr;
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "raw RWStructuredBuffer numeric element pointer",
                            builder.emitPointerOffset(
                                moduleScope.module,
                                loweredPointer,
                                loweredElementIndex,
                                loweredElementPointer)));
                        valueMap[inst] = loweredElementPointer;
                    }
                    break;

                case kIROp_Return:
                    if (function == entryPoint || as<IRVoidType>(function->getResultType()))
                    {
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "void return",
                            builder.emitReturnVoid(moduleScope.module)));
                    }
                    else
                    {
                        auto returnInst = cast<IRReturn>(inst);
                        SlangNVVMValueHandle loweredValue = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            returnInst->getVal(),
                            valueMap,
                            typeContext,
                            loweredValue));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            _usesGenericNVVMFunctions(function) ? "generic value return"
                                                                : "signed i32 return",
                            _usesGenericNVVMFunctions(function)
                                ? builder.emitValueReturn(moduleScope.module, loweredValue)
                                : builder.emitIntegerReturn(moduleScope.module, loweredValue)));
                    }
                    break;

                case kIROp_UnconditionalBranch:
                case kIROp_Loop:
                    {
                        auto branch = cast<IRUnconditionalBranch>(inst);
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            inst->getOp() == kIROp_Loop ? "loop entry branch"
                                                        : "unconditional branch",
                            builder.emitBranch(
                                moduleScope.module,
                                blockMap.getValue(branch->getTargetBlock()))));
                    }
                    break;

                case kIROp_IfElse:
                    {
                        auto ifElse = cast<IRIfElse>(inst);
                        SlangNVVMValueHandle loweredCondition = nullptr;
                        SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                            codeGenContext,
                            builder,
                            moduleScope.module,
                            ifElse->getCondition(),
                            valueMap,
                            typeContext,
                            loweredCondition));
                        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                            codeGenContext,
                            "conditional branch",
                            builder.emitConditionalBranch(
                                moduleScope.module,
                                loweredCondition,
                                blockMap.getValue(ifElse->getTrueBlock()),
                                blockMap.getValue(ifElse->getFalseBlock()))));
                    }
                    break;

                default:
                    SLANG_UNEXPECTED("NVVM emission received IR that was not preflighted");
                }
            }
        }

        // Slang block parameters are the phi source of truth: argument N on each actual predecessor
        // edge feeds parameter N. At this point even loop backedge instructions exist, so every
        // pair can be attached without reconstructing a local variable or searching an operand
        // graph.
        for (auto block : function->getBlocks())
        {
            if (block == entryBlock || !block->getFirstParam())
                continue;

            for (auto predecessor : block->getPredecessors())
            {
                auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
                SLANG_RELEASE_ASSERT(branch && branch->getTargetBlock() == block);

                UInt phiParameterIndex = 0;
                for (auto param : block->getParams())
                {
                    SlangNVVMValueHandle loweredArgument = nullptr;
                    SLANG_RETURN_ON_FAIL(_getLoweredNVVMValue(
                        codeGenContext,
                        builder,
                        moduleScope.module,
                        branch->getArg(phiParameterIndex),
                        valueMap,
                        typeContext,
                        loweredArgument));
                    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                        codeGenContext,
                        isNVVMSignedI32Type(param->getDataType())
                            ? "signed i32 phi incoming value"
                            : "generic value phi incoming value",
                        isNVVMSignedI32Type(param->getDataType())
                            ? builder.addIntegerPhiIncoming(
                                  moduleScope.module,
                                  valueMap.getValue(param),
                                  loweredArgument,
                                  blockMap.getValue(predecessor))
                            : builder.addPhiIncoming(
                                  moduleScope.module,
                                  valueMap.getValue(param),
                                  loweredArgument,
                                  blockMap.getValue(predecessor))));
                    ++phiParameterIndex;
                }
            }
        }
    }

    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "kernel annotation",
        builder.markFunctionAsKernel(moduleScope.module, functionMap.getValue(entryPoint))));

    ComPtr<ISlangBlob> serializedIR;
    String verifierDiagnostics;
    SlangResult serializationResult = builder.serializeModule(
        moduleScope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        serializedIR,
        verifierDiagnostics);
    if (SLANG_FAILED(serializationResult))
    {
        _requireBuilderOperation(
            codeGenContext,
            "verified NVVM IR 2.0 assembly serialization",
            serializationResult);
        if (verifierDiagnostics.getLength())
        {
            codeGenContext->getSink()->diagnoseRaw(
                Severity::Note,
                verifierDiagnostics.getUnownedSlice());
        }
        return serializationResult;
    }
    if (verifierDiagnostics.getLength())
    {
        codeGenContext->getSink()->diagnoseRaw(
            Severity::Note,
            verifierDiagnostics.getUnownedSlice());
    }
    if (!serializedIR || !serializedIR->getBufferSize())
    {
        return _requireBuilderOperation(
            codeGenContext,
            "verified NVVM IR 2.0 assembly serialization",
            SLANG_FAIL);
    }

    auto artifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(ArtifactKind::Assembly, ArtifactPayload::LLVMIR, ArtifactStyle::Kernel));
    artifact->addRepresentationUnknown(serializedIR);
    ArtifactUtil::addAssociated(artifact, linkedIR.metadata);
    outArtifact = artifact;
    return SLANG_OK;
}

} // namespace Slang
