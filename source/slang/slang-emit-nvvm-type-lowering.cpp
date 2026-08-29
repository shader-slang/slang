#include "slang-emit-nvvm-type-lowering.h"

#include "slang-base-type-info.h"
#include "slang-code-gen.h"
#include "slang-diagnostics.h"

namespace Slang
{

bool isNVVMSignedI32Type(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Int;
}

bool isNVVMUnsignedI32Type(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::UInt;
}

bool isNVVMInteger32Type(IRInst* type)
{
    return isNVVMSignedI32Type(type) || isNVVMUnsignedI32Type(type);
}

bool isNVVMSupportedIntegerScalarType(IRInst* type, uint32_t* outBitWidth, bool* outIsSigned)
{
    if (outBitWidth)
        *outBitWidth = 0;
    if (outIsSigned)
        *outIsSigned = false;

    auto basicType = as<IRBasicType>(type);
    if (!basicType)
        return false;
    const BaseType baseType = basicType->getBaseType();
    switch (baseType)
    {
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
        break;
    default:
        return false;
    }

    const BaseTypeInfo& info = BaseTypeInfo::getInfo(baseType);
    SLANG_RELEASE_ASSERT(info.flags & BaseTypeInfo::Flag::Integer);
    if (outBitWidth)
        *outBitWidth = uint32_t(info.sizeInBytes) * 8;
    if (outIsSigned)
        *outIsSigned = (info.flags & BaseTypeInfo::Flag::Signed) != 0;
    return true;
}

bool isNVVMFloat32Type(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Float;
}

bool isNVVMFloat16Type(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Half;
}

bool isNVVMSupportedFloatingPointScalarType(IRInst* type, uint32_t* outBitWidth)
{
    if (outBitWidth)
        *outBitWidth = 0;
    if (isNVVMFloat16Type(type))
    {
        if (outBitWidth)
            *outBitWidth = 16;
        return true;
    }
    if (isNVVMFloat32Type(type))
    {
        if (outBitWidth)
            *outBitWidth = 32;
        return true;
    }
    return false;
}

bool isNVVMBoolType(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Bool;
}

static IRVectorType* _asNVVMSupportedVectorType(
    IRInst* type,
    bool allowBool,
    uint32_t* outElementCount)
{
    if (outElementCount)
        *outElementCount = 0;

    auto vectorType = as<IRVectorType>(type);
    auto elementCount = vectorType ? as<IRIntLit>(vectorType->getElementCount()) : nullptr;
    IRType* elementType = vectorType ? vectorType->getElementType() : nullptr;
    if (!vectorType ||
        (!isNVVMSupportedIntegerScalarType(elementType) &&
         !isNVVMSupportedFloatingPointScalarType(elementType) &&
         !(allowBool && isNVVMBoolType(elementType))) ||
        !elementCount || elementCount->getValue() < 2 || elementCount->getValue() > 4)
    {
        return nullptr;
    }
    if (outElementCount)
        *outElementCount = uint32_t(elementCount->getValue());
    return vectorType;
}

IRVectorType* asNVVMSupportedValueVectorType(IRInst* type, uint32_t* outElementCount)
{
    return _asNVVMSupportedVectorType(type, true, outElementCount);
}

bool isNVVMSupportedValueType(IRInst* type)
{
    return isNVVMSupportedIntegerScalarType(type) || isNVVMSupportedFloatingPointScalarType(type) ||
           isNVVMBoolType(type) || asNVVMSupportedValueVectorType(type);
}

IRVectorType* asNVVMSupportedNumericVectorType(IRInst* type, uint32_t* outElementCount)
{
    return _asNVVMSupportedVectorType(type, false, outElementCount);
}

IRVectorType* asNVVMSupported32BitNumericVectorType(IRInst* type, uint32_t* outElementCount)
{
    if (outElementCount)
        *outElementCount = 0;

    uint32_t elementCount = 0;
    auto vectorType = asNVVMSupportedNumericVectorType(type, &elementCount);
    if (!vectorType || (!isNVVMInteger32Type(vectorType->getElementType()) &&
                        !isNVVMFloat32Type(vectorType->getElementType())))
        return nullptr;
    if (outElementCount)
        *outElementCount = elementCount;
    return vectorType;
}

IRVectorType* asNVVMSupportedI32VectorType(
    IRInst* type,
    bool* outIsSigned,
    uint32_t* outElementCount)
{
    if (outIsSigned)
        *outIsSigned = false;
    if (outElementCount)
        *outElementCount = 0;

    uint32_t elementCount = 0;
    auto vectorType = asNVVMSupported32BitNumericVectorType(type, &elementCount);
    const bool isSigned = vectorType && isNVVMSignedI32Type(vectorType->getElementType());
    if (!vectorType || (!isSigned && !isNVVMUnsignedI32Type(vectorType->getElementType())))
        return nullptr;
    if (outIsSigned)
        *outIsSigned = isSigned;
    if (outElementCount)
        *outElementCount = elementCount;
    return vectorType;
}

bool isNVVMSupportedNumericValueType(IRInst* type)
{
    return isNVVMSupportedIntegerScalarType(type) || isNVVMSupportedFloatingPointScalarType(type) ||
           asNVVMSupportedNumericVectorType(type);
}

// Returns the non-aggregate byte payload family shared by direct values and array elements.
static bool _isNVVMSupportedByteAddressLeafValueType(IRInst* type)
{
    uint32_t integerBitWidth = 0;
    const bool isSupportedInteger = isNVVMSupportedIntegerScalarType(type, &integerBitWidth) &&
                                    (integerBitWidth == 32 || integerBitWidth == 64);
    return isSupportedInteger || isNVVMFloat32Type(type) ||
           asNVVMSupported32BitNumericVectorType(type);
}

IRArrayType* asNVVMSupportedNumericArrayType(IRInst* type, uint32_t* outElementCount)
{
    if (outElementCount)
        *outElementCount = 0;

    auto arrayType = as<IRArrayType>(type);
    if (!arrayType || arrayType->getOp() != kIROp_ArrayType ||
        (arrayType->getOperandCount() != 2 && arrayType->getOperandCount() != 3) ||
        !_isNVVMSupportedByteAddressLeafValueType(arrayType->getElementType()))
    {
        return nullptr;
    }

    auto elementCount = as<IRIntLit>(arrayType->getElementCount());
    if (!elementCount || elementCount->getValue() <= 0 || elementCount->getValue() > UINT32_MAX)
        return nullptr;

    if (IRInst* stride = arrayType->getArrayStride())
    {
        auto strideValue = as<IRIntLit>(stride);
        const uint32_t naturalStride = getNVVMNumericValueAlignment(arrayType->getElementType());
        if (!strideValue || strideValue->getValue() != naturalStride)
            return nullptr;
    }

    if (outElementCount)
        *outElementCount = uint32_t(elementCount->getValue());
    return arrayType;
}

bool isNVVMSupportedByteAddressValueType(IRInst* type)
{
    return _isNVVMSupportedByteAddressLeafValueType(type) || asNVVMSupportedNumericArrayType(type);
}

IRStructType* asNVVMSupportedScalarStructType(IRInst* type)
{
    auto structType = as<IRStructType>(type);
    if (!structType)
        return nullptr;

    bool hasField = false;
    for (auto field : structType->getFields())
    {
        if (!isNVVMSupportedIntegerScalarType(field->getFieldType()) &&
            !isNVVMFloat32Type(field->getFieldType()))
        {
            return nullptr;
        }
        hasField = true;
    }
    return hasField ? structType : nullptr;
}

IRStructType* asNVVMSupportedPhysicalArrayStructType(IRInst* type)
{
    auto structType = as<IRStructType>(type);
    if (!structType || !structType->findDecoration<IRPhysicalTypeDecoration>())
        return nullptr;

    IRStructField* onlyField = nullptr;
    for (auto field : structType->getFields())
    {
        if (onlyField || !asNVVMSupportedNumericArrayType(field->getFieldType()))
            return nullptr;
        onlyField = field;
    }
    return onlyField ? structType : nullptr;
}

IRStructType* asNVVMSupportedParameterGroupStructType(IRInst* type)
{
    if (auto scalarStructType = asNVVMSupportedScalarStructType(type))
        return scalarStructType;
    return asNVVMSupportedPhysicalArrayStructType(type);
}

IRStructType* asNVVMSupportedCopyableStructType(IRInst* type)
{
    auto structType = as<IRStructType>(type);
    if (!structType)
        return nullptr;

    bool hasField = false;
    for (auto field : structType->getFields())
    {
        IRType* fieldType = field->getFieldType();
        if (!isNVVMSupportedNumericValueType(fieldType) &&
            !asNVVMSupportedCopyableStructType(fieldType))
        {
            return nullptr;
        }
        hasField = true;
    }
    return hasField ? structType : nullptr;
}

IRPtrTypeBase* asNVVMSupportedLocalNumericPointerType(IRInst* type, IRType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    IRType* valueType = pointerType ? pointerType->getValueType() : nullptr;
    if (!pointerType || !isNVVMSupportedNumericValueType(valueType) ||
        (pointerType->getOp() != kIROp_PtrType && pointerType->getOp() != kIROp_OutParamType &&
         pointerType->getOp() != kIROp_BorrowInOutParamType) ||
        pointerType->getOperandCount() != 1 ||
        pointerType->getAddressSpace() != AddressSpace::Generic)
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedLocalNumericArrayPointerType(
    IRInst* type,
    IRArrayType** outValueType,
    uint32_t* outElementCount)
{
    if (outValueType)
        *outValueType = nullptr;
    if (outElementCount)
        *outElementCount = 0;

    auto pointerType = as<IRPtrTypeBase>(type);
    uint32_t elementCount = 0;
    auto valueType =
        pointerType ? asNVVMSupportedNumericArrayType(pointerType->getValueType(), &elementCount)
                    : nullptr;
    if (!pointerType || !valueType ||
        (pointerType->getOp() != kIROp_PtrType && pointerType->getOp() != kIROp_OutParamType &&
         pointerType->getOp() != kIROp_BorrowInOutParamType) ||
        pointerType->getOperandCount() != 1 ||
        pointerType->getAddressSpace() != AddressSpace::Generic)
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    if (outElementCount)
        *outElementCount = elementCount;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedLocalScalarStructPointerType(
    IRInst* type,
    IRStructType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    auto valueType =
        pointerType ? asNVVMSupportedScalarStructType(pointerType->getValueType()) : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    const bool isLocalPointer =
        pointerType && pointerType->getOp() == kIROp_PtrType && pointerType->getOperandCount() == 1;
    const bool isMutableBorrow = pointerType &&
                                 pointerType->getOp() == kIROp_BorrowInOutParamType &&
                                 pointerType->getOperandCount() == 1;
    // Explicit-global-context lowering spells its helper parameter with the complete CUDA local
    // pointer contract, while the entry-point `var` passed to it retains the compact local-pointer
    // spelling. Both are the canonical producer shapes for the same per-invocation storage.
    const bool isThreadLocalContextPointer =
        pointerType && pointerType->getOp() == kIROp_PtrType &&
        pointerType->getOperandCount() == 4 &&
        pointerType->getAccessQualifier() == AccessQualifier::ReadWrite &&
        pointerType->getAddressSpace() == AddressSpace::ThreadLocal && dataLayout &&
        dataLayout->getOp() == kIROp_DefaultBufferLayoutType;
    if (!valueType || (!isLocalPointer && !isMutableBorrow && !isThreadLocalContextPointer))
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedLocalCopyableStructPointerType(
    IRInst* type,
    IRStructType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    auto valueType =
        pointerType ? asNVVMSupportedCopyableStructType(pointerType->getValueType()) : nullptr;
    if (!pointerType || !valueType || pointerType->getOp() != kIROp_PtrType ||
        pointerType->getOperandCount() != 1)
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

uint32_t getNVVMNumericValueAlignment(IRInst* type)
{
    uint32_t bitWidth = 0;
    if (isNVVMSupportedIntegerScalarType(type, &bitWidth))
        return bitWidth / 8;
    if (isNVVMSupportedFloatingPointScalarType(type, &bitWidth))
        return bitWidth / 8;
    uint32_t elementCount = 0;
    auto vectorType = asNVVMSupportedNumericVectorType(type, &elementCount);
    if (vectorType)
    {
        const uint32_t elementAlignment =
            getNVVMNumericValueAlignment(vectorType->getElementType());
        SLANG_RELEASE_ASSERT(elementAlignment);
        return elementAlignment * (elementCount == 3 ? 4 : elementCount);
    }
    return 0;
}

uint32_t getNVVMCopyableValueAlignment(IRInst* type)
{
    if (const uint32_t numericAlignment = getNVVMNumericValueAlignment(type))
        return numericAlignment;
    if (auto arrayType = asNVVMSupportedNumericArrayType(type))
        return getNVVMNumericValueAlignment(arrayType->getElementType());
    auto structType = asNVVMSupportedCopyableStructType(type);
    if (!structType)
        return 0;
    uint32_t alignment = 0;
    for (auto field : structType->getFields())
    {
        const uint32_t fieldAlignment = getNVVMCopyableValueAlignment(field->getFieldType());
        SLANG_RELEASE_ASSERT(fieldAlignment);
        alignment = Math::Max(alignment, fieldAlignment);
    }
    return alignment;
}

IRArrayType* asNVVMSupportedI32ArrayType(IRInst* type, uint32_t* outElementCount)
{
    if (outElementCount)
        *outElementCount = 0;

    uint32_t elementCount = 0;
    auto arrayType = asNVVMSupportedNumericArrayType(type, &elementCount);
    if (!arrayType || !isNVVMSignedI32Type(arrayType->getElementType()))
        return nullptr;

    if (outElementCount)
        *outElementCount = elementCount;
    return arrayType;
}

static IRPtrTypeBase* _asNVVMSupportedDevicePointerType(IRInst* type, BaseType valueBaseType)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    auto valueType = ptrType ? as<IRBasicType>(ptrType->getValueType()) : nullptr;
    if (!ptrType || ptrType->getOp() != kIROp_PtrType || !valueType ||
        valueType->getBaseType() != valueBaseType ||
        ptrType->getAddressSpace() != AddressSpace::UserPointer)
    {
        return nullptr;
    }

    const AccessQualifier access = ptrType->getAccessQualifier();
    return access == AccessQualifier::Read || access == AccessQualifier::ReadWrite ? ptrType
                                                                                   : nullptr;
}

IRPtrTypeBase* asNVVMSupportedDevicePointerType(IRInst* type)
{
    auto pointer = _asNVVMSupportedDevicePointerType(type, BaseType::Int);
    return pointer ? pointer : _asNVVMSupportedDevicePointerType(type, BaseType::UInt);
}

IRPtrTypeBase* asNVVMSupportedDeviceFloat32PointerType(IRInst* type)
{
    return _asNVVMSupportedDevicePointerType(type, BaseType::Float);
}

IRPtrTypeBase* asNVVMSupportedDeviceScalarPointerType(IRInst* type)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    if (!ptrType || ptrType->getOp() != kIROp_PtrType ||
        !(isNVVMSupportedIntegerScalarType(ptrType->getValueType()) ||
          isNVVMFloat32Type(ptrType->getValueType())) ||
        ptrType->getAddressSpace() != AddressSpace::UserPointer)
    {
        return nullptr;
    }
    const AccessQualifier access = ptrType->getAccessQualifier();
    return access == AccessQualifier::Read || access == AccessQualifier::ReadWrite ? ptrType
                                                                                   : nullptr;
}

IRPtrTypeBase* asNVVMSupportedDeviceNumericPointerType(IRInst* type)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    bool vectorIsSigned = false;
    uint32_t vectorElementCount = 0;
    const bool isEstablishedVectorPointer = ptrType &&
                                            asNVVMSupportedI32VectorType(
                                                ptrType->getValueType(),
                                                &vectorIsSigned,
                                                &vectorElementCount) &&
                                            vectorIsSigned && vectorElementCount == 2;
    if (!ptrType || ptrType->getOp() != kIROp_PtrType ||
        !(isNVVMSupportedIntegerScalarType(ptrType->getValueType()) ||
          isNVVMFloat32Type(ptrType->getValueType()) || isEstablishedVectorPointer) ||
        ptrType->getAddressSpace() != AddressSpace::UserPointer)
    {
        return nullptr;
    }
    const AccessQualifier access = ptrType->getAccessQualifier();
    return access == AccessQualifier::Read || access == AccessQualifier::ReadWrite ? ptrType
                                                                                   : nullptr;
}

IRPtrTypeBase* asNVVMSupportedDeviceArrayPointerType(
    IRInst* type,
    IRArrayType** outArrayType,
    uint32_t* outElementCount)
{
    if (outArrayType)
        *outArrayType = nullptr;
    if (outElementCount)
        *outElementCount = 0;

    auto ptrType = as<IRPtrTypeBase>(type);
    IRArrayType* arrayType = nullptr;
    uint32_t elementCount = 0;
    if (!ptrType || ptrType->getOp() != kIROp_PtrType ||
        !(arrayType = asNVVMSupportedI32ArrayType(ptrType->getValueType(), &elementCount)) ||
        ptrType->getAddressSpace() != AddressSpace::UserPointer)
    {
        return nullptr;
    }

    const AccessQualifier access = ptrType->getAccessQualifier();
    if (access != AccessQualifier::Read && access != AccessQualifier::ReadWrite)
        return nullptr;

    if (outArrayType)
        *outArrayType = arrayType;
    if (outElementCount)
        *outElementCount = elementCount;
    return ptrType;
}

IRGlobalVar* asNVVMSupportedSharedI32ArrayGlobal(
    IRInst* inst,
    IRArrayType** outArrayType,
    uint32_t* outElementCount)
{
    if (outArrayType)
        *outArrayType = nullptr;
    if (outElementCount)
        *outElementCount = 0;

    auto globalVar = as<IRGlobalVar>(inst);
    auto ptrType = globalVar ? globalVar->getDataType() : nullptr;
    IRArrayType* arrayType = nullptr;
    uint32_t elementCount = 0;
    if (!globalVar || !as<IRGroupSharedRate>(globalVar->getRate()) || globalVar->getFirstBlock() ||
        !ptrType || ptrType->getOp() != kIROp_PtrType || ptrType->getOperandCount() != 1 ||
        !(arrayType = asNVVMSupportedI32ArrayType(ptrType->getValueType(), &elementCount)))
    {
        return nullptr;
    }

    if (outArrayType)
        *outArrayType = arrayType;
    if (outElementCount)
        *outElementCount = elementCount;
    return globalVar;
}

IRPtrTypeBase* asNVVMSupportedSharedI32ElementPointerType(IRInst* type)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    IRType* dataLayout = ptrType ? ptrType->getDataLayout() : nullptr;
    if (!ptrType || ptrType->getOp() != kIROp_PtrType || ptrType->getOperandCount() != 4 ||
        !isNVVMSignedI32Type(ptrType->getValueType()) ||
        ptrType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        ptrType->getAddressSpace() != AddressSpace::GroupShared || !dataLayout ||
        dataLayout->getOp() != kIROp_ScalarBufferLayoutType)
    {
        return nullptr;
    }
    return ptrType;
}

static bool _isNVVMSupportedResourceElementType(IRInst* type)
{
    return isNVVMSupportedIntegerScalarType(type) || isNVVMFloat32Type(type) ||
           asNVVMSupported32BitNumericVectorType(type) || asNVVMSupportedCopyableStructType(type) ||
           asNVVMSupportedPhysicalArrayStructType(type);
}

bool getNVVMSupportedRawBufferType(IRInst* type, NVVMRawBufferType& outType)
{
    outType = {};

    auto bufferType = as<IRHLSLStructuredBufferTypeBase>(type);
    if (bufferType &&
        (bufferType->getOp() == kIROp_HLSLStructuredBufferType ||
         bufferType->getOp() == kIROp_HLSLRWStructuredBufferType) &&
        bufferType->getOperandCount() == 3 &&
        _isNVVMSupportedResourceElementType(bufferType->getElementType()))
    {
        IRType* dataLayout = bufferType->getDataLayout();
        if (!dataLayout || dataLayout->getOp() != kIROp_DefaultBufferLayoutType)
            return false;
        outType.canonicalType = bufferType;
        outType.structuredElementType = bufferType->getElementType();
        outType.kind = NVVMRawBufferKind::Structured;
        outType.access = bufferType->getOp() == kIROp_HLSLRWStructuredBufferType
                             ? NVVMBufferAccess::ReadWrite
                             : NVVMBufferAccess::ReadOnly;
        return true;
    }

    auto byteAddressType = as<IRByteAddressBufferTypeBase>(type);
    if (!byteAddressType || byteAddressType->getOperandCount() != 0 ||
        (byteAddressType->getOp() != kIROp_HLSLByteAddressBufferType &&
         byteAddressType->getOp() != kIROp_HLSLRWByteAddressBufferType))
    {
        return false;
    }

    outType.canonicalType = byteAddressType;
    outType.kind = NVVMRawBufferKind::ByteAddress;
    outType.access = byteAddressType->getOp() == kIROp_HLSLRWByteAddressBufferType
                         ? NVVMBufferAccess::ReadWrite
                         : NVVMBufferAccess::ReadOnly;
    return true;
}

bool isNVVMRawBufferElementType(const NVVMRawBufferType& bufferType, IRType* elementType)
{
    return bufferType.kind == NVVMRawBufferKind::ByteAddress
               ? isNVVMUnsignedI32Type(elementType)
               : isTypeEqual(bufferType.structuredElementType, elementType);
}

bool getNVVMSupportedBufferDataPointerType(IRInst* type, NVVMBufferDataPointerType& outType)
{
    outType = {};
    auto pointerType = as<IRPtrTypeBase>(type);
    auto arrayType = pointerType ? as<IRUnsizedArrayType>(pointerType->getValueType()) : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    if (!pointerType || pointerType->getOp() != kIROp_PtrType ||
        pointerType->getOperandCount() != 4 || !arrayType || arrayType->getOperandCount() != 1 ||
        !_isNVVMSupportedResourceElementType(arrayType->getElementType()) ||
        pointerType->getAddressSpace() != AddressSpace::UserPointer || !dataLayout ||
        dataLayout->getOp() != kIROp_DefaultBufferLayoutType)
    {
        return false;
    }

    if (pointerType->getAccessQualifier() != AccessQualifier::ReadWrite)
        return false;

    outType.pointerType = pointerType;
    outType.arrayType = arrayType;
    outType.elementType = arrayType->getElementType();
    return true;
}

static bool _getNVVMSelected32BitNumericElementType(IRType* type, SlangNVVMValueTypeDesc& outType)
{
    outType = {};
    IRType* scalarType = type;
    uint32_t laneCount = 1;
    uint32_t vectorLaneCount = 0;
    if (auto vectorType = asNVVMSupported32BitNumericVectorType(type, &vectorLaneCount))
    {
        if (vectorLaneCount != 2 && vectorLaneCount != 4)
            return false;
        scalarType = vectorType->getElementType();
        laneCount = vectorLaneCount;
    }

    uint32_t bitWidth = 0;
    bool isSigned = false;
    if (isNVVMSupportedIntegerScalarType(scalarType, &bitWidth, &isSigned) && bitWidth == 32)
    {
        outType = {
            isSigned ? SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER
                     : SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
            32,
            laneCount,
        };
        return true;
    }
    if (!isNVVMFloat32Type(scalarType))
        return false;
    outType = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, 32, laneCount};
    return true;
}

bool getNVVMSupportedSurfaceType(IRInst* type, NVVMSurfaceType& outType)
{
    outType = {};
    auto textureType = as<IRTextureTypeBase>(type);
    if (!textureType || textureType->getOp() != kIROp_TextureType ||
        textureType->getOperandCount() < 9 || textureType->isMultisample() ||
        textureType->isShadow() || textureType->isCombined() ||
        textureType->getAccess() != SLANG_RESOURCE_ACCESS_READ_WRITE)
    {
        return false;
    }

    SlangNVVMTextureShape shape = 0;
    uint32_t coordinateLaneCount = 0;
    switch (textureType->GetBaseShape())
    {
    case SLANG_TEXTURE_1D:
        shape = SLANG_NVVM_TEXTURE_SHAPE_1D;
        coordinateLaneCount = 1;
        break;
    case SLANG_TEXTURE_2D:
        shape = SLANG_NVVM_TEXTURE_SHAPE_2D;
        coordinateLaneCount = 2;
        break;
    case SLANG_TEXTURE_3D:
        shape = SLANG_NVVM_TEXTURE_SHAPE_3D;
        coordinateLaneCount = 3;
        break;
    default:
        return false;
    }

    const bool isArray = textureType->isArray();
    if (isArray && shape != SLANG_NVVM_TEXTURE_SHAPE_2D)
        return false;

    SlangNVVMValueTypeDesc elementType = {};
    if (_getNVVMSelected32BitNumericElementType(textureType->getElementType(), elementType))
    {
        outType.textureType = textureType;
        outType.shape = shape;
        outType.isArray = isArray;
        outType.coordinateLaneCount = coordinateLaneCount + (isArray ? 1u : 0u);
        outType.elementType = elementType;
        return true;
    }

    IRType* scalarType = textureType->getElementType();
    uint32_t laneCount = 1;
    if (auto vectorType = as<IRVectorType>(scalarType))
    {
        auto count = as<IRIntLit>(vectorType->getElementCount());
        if (!count || (count->getValue() != 2 && count->getValue() != 4))
            return false;
        scalarType = vectorType->getElementType();
        laneCount = uint32_t(count->getValue());
    }
    uint32_t bitWidth = 0;
    if (!isNVVMSupportedFloatingPointScalarType(scalarType, &bitWidth) || bitWidth != 16 ||
        isArray || shape == SLANG_NVVM_TEXTURE_SHAPE_3D)
        return false;

    outType.textureType = textureType;
    outType.shape = shape;
    outType.coordinateLaneCount = coordinateLaneCount;
    outType.elementType = {SLANG_NVVM_VALUE_TYPE_FLOATING_POINT, bitWidth, laneCount};
    return true;
}

bool getNVVMSupportedSurfaceField(
    IRStructField* field,
    NVVMSurfaceType& outType,
    SlangNVVMSurfaceStorageFormat& outStorageFormat)
{
    outType = {};
    outStorageFormat = SLANG_NVVM_SURFACE_STORAGE_NATIVE;
    if (!field || !getNVVMSupportedSurfaceType(field->getFieldType(), outType))
        return false;

    auto formatDecoration = field->getKey()->findDecoration<IRFormatDecoration>();
    if (!formatDecoration)
        return true;

    const ImageFormatInfo& formatInfo = getImageFormatInfo(formatDecoration->getFormat());
    if (formatInfo.channelCount != outType.elementType.laneCount)
        return false;

    switch (outType.elementType.kind)
    {
    case SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER:
        return formatInfo.scalarType == SLANG_SCALAR_TYPE_INT32;
    case SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER:
        return formatInfo.scalarType == SLANG_SCALAR_TYPE_UINT32;
    case SLANG_NVVM_VALUE_TYPE_FLOATING_POINT:
        break;
    default:
        return false;
    }

    if ((outType.elementType.bitWidth == 16 &&
         formatInfo.scalarType == SLANG_SCALAR_TYPE_FLOAT16) ||
        (outType.elementType.bitWidth == 32 && formatInfo.scalarType == SLANG_SCALAR_TYPE_FLOAT32))
    {
        return true;
    }
    if (outType.elementType.bitWidth != 32 || formatInfo.scalarType != SLANG_SCALAR_TYPE_FLOAT16 ||
        outType.isArray || outType.shape == SLANG_NVVM_TEXTURE_SHAPE_3D)
    {
        return false;
    }

    outStorageFormat = SLANG_NVVM_SURFACE_STORAGE_FLOAT16;
    return true;
}

bool getNVVMSupportedReadOnlyTextureType(IRInst* type, NVVMReadOnlyTextureType& outType)
{
    outType = {};
    auto textureType = as<IRTextureTypeBase>(type);
    if (!textureType || textureType->getOp() != kIROp_TextureType ||
        textureType->getOperandCount() < 9 || textureType->isMultisample() ||
        textureType->isShadow() || textureType->isCombined() ||
        textureType->getAccess() != SLANG_RESOURCE_ACCESS_READ)
    {
        return false;
    }

    SlangNVVMValueTypeDesc elementType = {};
    if (!_getNVVMSelected32BitNumericElementType(textureType->getElementType(), elementType))
        return false;

    SlangNVVMTextureShape shape = 0;
    uint32_t coordinateLaneCount = 0;
    switch (textureType->GetBaseShape())
    {
    case SLANG_TEXTURE_1D:
        shape = SLANG_NVVM_TEXTURE_SHAPE_1D;
        coordinateLaneCount = 1;
        break;
    case SLANG_TEXTURE_2D:
        shape = SLANG_NVVM_TEXTURE_SHAPE_2D;
        coordinateLaneCount = 2;
        break;
    case SLANG_TEXTURE_3D:
        shape = SLANG_NVVM_TEXTURE_SHAPE_3D;
        coordinateLaneCount = 3;
        break;
    case SLANG_TEXTURE_CUBE:
        shape = SLANG_NVVM_TEXTURE_SHAPE_CUBE;
        coordinateLaneCount = 3;
        break;
    default:
        return false;
    }

    const bool isArray = textureType->isArray();
    if (isArray && shape == SLANG_NVVM_TEXTURE_SHAPE_3D)
        return false;

    outType.textureType = textureType;
    outType.shape = shape;
    outType.isArray = isArray;
    outType.coordinateLaneCount = coordinateLaneCount + (isArray ? 1u : 0u);
    outType.elementType = elementType;
    return true;
}

IRSamplerStateTypeBase* asNVVMSupportedSamplerValueType(IRInst* type)
{
    return type && type->getOp() == kIROp_SamplerStateType ? as<IRSamplerStateTypeBase>(type)
                                                           : nullptr;
}

IRSamplerStateTypeBase* asNVVMSupportedSamplerStorageType(IRInst* type)
{
    return as<IRSamplerStateTypeBase>(type);
}

IRUnsizedArrayType* asNVVMSupportedUnsizedSamplerArrayStorageType(IRInst* type)
{
    auto arrayType = as<IRUnsizedArrayType>(type);
    return arrayType && asNVVMSupportedSamplerStorageType(arrayType->getElementType()) ? arrayType
                                                                                       : nullptr;
}

IRParameterGroupType* asNVVMSupportedParameterGroupType(IRInst* type, IRType** outElementType)
{
    if (outElementType)
        *outElementType = nullptr;

    auto parameterGroupType = as<IRParameterGroupType>(type);
    if (!parameterGroupType || (parameterGroupType->getOp() != kIROp_ParameterBlockType &&
                                parameterGroupType->getOp() != kIROp_ConstantBufferType))
    {
        return nullptr;
    }

    IRType* elementType = parameterGroupType->getElementType();
    if (!asNVVMSupportedParameterGroupStructType(elementType) &&
        !asNVVMSupportedNumericArrayType(elementType))
        return nullptr;

    if (outElementType)
        *outElementType = elementType;
    return parameterGroupType;
}

bool isNVVMSupportedConventionalGlobalFieldType(IRStructField* field)
{
    NVVMRawBufferType rawBufferType;
    NVVMSurfaceType surfaceType;
    NVVMReadOnlyTextureType sampledTextureType;
    SlangNVVMSurfaceStorageFormat storageFormat = SLANG_NVVM_SURFACE_STORAGE_NATIVE;
    IRType* type = field ? field->getFieldType() : nullptr;
    return isNVVMSupportedIntegerScalarType(type) || isNVVMFloat32Type(type) ||
           asNVVMSupportedParameterGroupType(type) ||
           getNVVMSupportedRawBufferType(type, rawBufferType) ||
           getNVVMSupportedSurfaceField(field, surfaceType, storageFormat) ||
           getNVVMSupportedReadOnlyTextureType(type, sampledTextureType) ||
           asNVVMSupportedSamplerStorageType(type) ||
           asNVVMSupportedUnsizedSamplerArrayStorageType(type);
}

IRPtrTypeBase* asNVVMSupportedRWStructuredBufferElementPointerType(IRInst* type)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    IRType* dataLayout = ptrType ? ptrType->getDataLayout() : nullptr;
    if (!ptrType || ptrType->getOp() != kIROp_PtrType || ptrType->getOperandCount() != 4 ||
        !_isNVVMSupportedResourceElementType(ptrType->getValueType()) ||
        ptrType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        ptrType->getAddressSpace() != AddressSpace::Generic || !dataLayout ||
        dataLayout->getOp() != kIROp_ScalarBufferLayoutType)
    {
        return nullptr;
    }
    return ptrType;
}

bool isNVVMSupportedParameterType(IRInst* type)
{
    NVVMRawBufferType rawBufferType;
    return isNVVMSupportedIntegerScalarType(type) || isNVVMFloat32Type(type) ||
           asNVVMSupportedScalarStructType(type) || asNVVMSupportedDeviceNumericPointerType(type) ||
           asNVVMSupportedDeviceArrayPointerType(type) ||
           getNVVMSupportedRawBufferType(type, rawBufferType);
}

SlangResult NVVMTypeLoweringContext::_requireBuilderOperation(
    const char* operation,
    SlangResult result) const
{
    if (SLANG_SUCCEEDED(result))
        return result;

    m_codeGenContext->getSink()->diagnose(Diagnostics::NvvmIrBuilderOperationFailed{
        .operation = String(operation),
        .resultCode = result,
    });
    return result;
}

SlangResult NVVMTypeLoweringContext::_reportUnsupportedType(NVVMTypeUse use) const
{
    const char* construct = "value type";
    switch (use)
    {
    case NVVMTypeUse::EntryPointResult:
        construct = "entry-point result type";
        break;
    case NVVMTypeUse::HelperResult:
        construct = "helper function result type";
        break;
    case NVVMTypeUse::EntryPointParameter:
        construct = "entry-point parameter";
        break;
    case NVVMTypeUse::HelperParameter:
        construct = "helper function parameter";
        break;
    case NVVMTypeUse::Value:
        break;
    case NVVMTypeUse::Storage:
        construct = "conventional global storage field type";
        break;
    }
    m_codeGenContext->getSink()->diagnose(
        Diagnostics::NvvmUnsupportedIr{.construct = String(construct)});
    return SLANG_E_NOT_IMPLEMENTED;
}

SlangResult NVVMTypeLoweringContext::_lowerArrayType(
    IRArrayType* type,
    SlangNVVMTypeHandle& outType)
{
    outType = nullptr;
    if (auto mappedType = m_typeMap.tryGetValue(type))
    {
        outType = *mappedType;
        return SLANG_OK;
    }

    uint32_t elementCount = 0;
    SLANG_RELEASE_ASSERT(asNVVMSupportedNumericArrayType(type, &elementCount));
    SlangNVVMTypeHandle elementType = nullptr;
    SLANG_RETURN_ON_FAIL(lowerType(type->getElementType(), NVVMTypeUse::Value, elementType));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        "fixed numeric array type",
        m_builder.getArrayType(m_module, elementType, elementCount, outType)));
    m_typeMap[type] = outType;
    return SLANG_OK;
}

SlangResult NVVMTypeLoweringContext::_lowerStructType(
    IRStructType* type,
    SlangNVVMTypeHandle& outType)
{
    outType = nullptr;
    const NVVMTypeUse fieldUse =
        asNVVMSupportedCopyableStructType(type) ? NVVMTypeUse::Value : NVVMTypeUse::Storage;
    List<SlangNVVMTypeHandle> fieldTypes;
    for (auto field : type->getFields())
    {
        SlangNVVMTypeHandle fieldType = nullptr;
        SLANG_RETURN_ON_FAIL(lowerType(field->getFieldType(), fieldUse, fieldType));
        fieldTypes.add(fieldType);
    }
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        "struct type",
        m_builder.getStructType(
            m_module,
            fieldTypes.getCount() ? fieldTypes.getBuffer() : nullptr,
            size_t(fieldTypes.getCount()),
            outType)));
    m_typeMap[type] = outType;
    return SLANG_OK;
}

SlangResult NVVMTypeLoweringContext::_lowerUnsizedSamplerArrayStorageType(
    IRUnsizedArrayType* type,
    SlangNVVMTypeHandle& outType)
{
    outType = nullptr;
    SLANG_RELEASE_ASSERT(asNVVMSupportedUnsizedSamplerArrayStorageType(type));

    SlangNVVMTypeHandle elementType = nullptr;
    SLANG_RETURN_ON_FAIL(lowerType(type->getElementType(), NVVMTypeUse::Storage, elementType));
    SlangNVVMTypeHandle dataPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        "unsized CUDA sampler-array data-pointer type",
        m_builder.getPointerType(
            m_module,
            elementType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            dataPointerType)));
    SlangNVVMTypeHandle countType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        "unsized CUDA sampler-array count type",
        m_builder.getIntegerType(m_module, 64, countType)));
    const SlangNVVMTypeHandle fieldTypes[] = {dataPointerType, countType};
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        "unsized CUDA sampler-array storage type",
        m_builder.getStructType(m_module, fieldTypes, SLANG_COUNT_OF(fieldTypes), outType)));
    m_typeMap[type] = outType;
    return SLANG_OK;
}

SlangResult NVVMTypeLoweringContext::_lowerRawBufferType(
    const NVVMRawBufferType& type,
    SlangNVVMTypeHandle& outType)
{
    outType = nullptr;
    SLANG_RELEASE_ASSERT(type.canonicalType);

    SlangNVVMTypeHandle loweredElementType = nullptr;
    if (type.kind == NVVMRawBufferKind::ByteAddress)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            "raw byte-address-buffer element type",
            m_builder.getIntegerType(m_module, 32, loweredElementType)));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(
            lowerType(type.structuredElementType, NVVMTypeUse::Value, loweredElementType));
    }
    SlangNVVMTypeHandle dataPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        "raw buffer data-pointer type",
        m_builder.getPointerType(
            m_module,
            loweredElementType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            dataPointerType)));
    SlangNVVMTypeHandle countType = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        "raw buffer count type",
        m_builder.getIntegerType(m_module, 64, countType)));
    const SlangNVVMTypeHandle fieldTypes[] = {dataPointerType, countType};
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        "raw buffer view type",
        m_builder.getStructType(m_module, fieldTypes, SLANG_COUNT_OF(fieldTypes), outType)));
    m_typeMap[type.canonicalType] = outType;
    return SLANG_OK;
}

SlangResult NVVMTypeLoweringContext::_lowerParameterGroupType(
    IRParameterGroupType* type,
    IRType* elementType,
    SlangNVVMTypeHandle& outType)
{
    outType = nullptr;
    SLANG_RELEASE_ASSERT(type && elementType);

    SlangNVVMTypeHandle loweredElementType = nullptr;
    SLANG_RETURN_ON_FAIL(lowerType(elementType, NVVMTypeUse::Storage, loweredElementType));

    const PointerTypeKey key = {elementType, SLANG_NVVM_ADDRESS_SPACE_GLOBAL};
    if (auto mappedRepresentation = m_pointerRepresentationMap.tryGetValue(key))
    {
        outType = *mappedRepresentation;
    }
    else
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            "parameter-group pointer type",
            m_builder.getPointerType(
                m_module,
                loweredElementType,
                SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
                outType)));
        m_pointerRepresentationMap[key] = outType;
    }
    m_typeMap[type] = outType;
    return SLANG_OK;
}

SlangResult NVVMTypeLoweringContext::_lowerPointerType(
    IRType* canonicalType,
    IRType* pointeeType,
    SlangNVVMAddressSpace addressSpace,
    SlangNVVMTypeHandle& outType)
{
    outType = nullptr;
    SlangNVVMTypeHandle loweredPointeeType = nullptr;
    if (auto arrayType = as<IRArrayType>(pointeeType))
    {
        SLANG_RETURN_ON_FAIL(_lowerArrayType(arrayType, loweredPointeeType));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(lowerType(pointeeType, NVVMTypeUse::Value, loweredPointeeType));
    }

    // Consider a kernel that copies from `Ptr<int, Read, Device>` to
    // `Ptr<int, ReadWrite, Device>`. Those are distinct canonical Slang types because stores are
    // legal through only one of them, but LLVM represents both as the same `i32 addrspace(1)*`.
    // Cache that provider representation by exact pointee identity and address space, then record
    // the resulting handle separately for each canonical source type.
    const PointerTypeKey key = {pointeeType, addressSpace};
    if (auto mappedRepresentation = m_pointerRepresentationMap.tryGetValue(key))
    {
        outType = *mappedRepresentation;
    }
    else
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            pointeeType->getOp() == kIROp_ArrayType ? "device fixed i32 array pointer type"
                                                    : "device numeric pointer type",
            m_builder.getPointerType(m_module, loweredPointeeType, addressSpace, outType)));
        m_pointerRepresentationMap[key] = outType;
    }
    m_typeMap[canonicalType] = outType;
    return SLANG_OK;
}

SlangResult NVVMTypeLoweringContext::lowerType(
    IRType* type,
    NVVMTypeUse use,
    SlangNVVMTypeHandle& outType)
{
    outType = nullptr;

    const bool isVoid = as<IRVoidType>(type) != nullptr;
    uint32_t integerBitWidth = 0;
    const bool isInteger = isNVVMSupportedIntegerScalarType(type, &integerBitWidth);
    uint32_t floatingPointBitWidth = 0;
    const bool isFloatingPoint =
        isNVVMSupportedFloatingPointScalarType(type, &floatingPointBitWidth);
    const bool isFloat32 = floatingPointBitWidth == 32;
    const bool isBool = isNVVMBoolType(type);
    uint32_t valueVectorElementCount = 0;
    IRVectorType* valueVectorType = asNVVMSupportedValueVectorType(type, &valueVectorElementCount);
    IRStructType* structType = as<IRStructType>(type);
    IRStructType* scalarStructType = asNVVMSupportedScalarStructType(type);
    IRStructType* copyableStructType = asNVVMSupportedCopyableStructType(type);
    IRStructType* physicalArrayStructType = asNVVMSupportedPhysicalArrayStructType(type);
    IRStructType* localScalarStructValueType = nullptr;
    IRPtrTypeBase* localScalarStructPointer =
        asNVVMSupportedLocalScalarStructPointerType(type, &localScalarStructValueType);
    IRType* localNumericPointerValueType = nullptr;
    IRPtrTypeBase* localNumericPointer =
        asNVVMSupportedLocalNumericPointerType(type, &localNumericPointerValueType);
    IRArrayType* localNumericArrayPointerValueType = nullptr;
    IRPtrTypeBase* localNumericArrayPointer =
        asNVVMSupportedLocalNumericArrayPointerType(type, &localNumericArrayPointerValueType);
    IRPtrTypeBase* deviceNumericPointer = asNVVMSupportedDeviceNumericPointerType(type);
    IRArrayType* fixedNumericArrayType = asNVVMSupportedNumericArrayType(type);
    IRArrayType* deviceArrayType = nullptr;
    IRPtrTypeBase* deviceArrayPointer =
        asNVVMSupportedDeviceArrayPointerType(type, &deviceArrayType);
    NVVMRawBufferType rawBufferType;
    const bool isRawBuffer = getNVVMSupportedRawBufferType(type, rawBufferType);
    NVVMSurfaceType surfaceType;
    const bool isSurface = getNVVMSupportedSurfaceType(type, surfaceType);
    NVVMReadOnlyTextureType sampledTextureType;
    const bool isSampledTexture = getNVVMSupportedReadOnlyTextureType(type, sampledTextureType);
    NVVMBufferDataPointerType bufferDataPointerType;
    const bool isBufferDataPointer =
        getNVVMSupportedBufferDataPointerType(type, bufferDataPointerType);
    IRType* parameterGroupElementType = nullptr;
    IRParameterGroupType* parameterGroup =
        asNVVMSupportedParameterGroupType(type, &parameterGroupElementType);
    IRSamplerStateTypeBase* samplerStorage = asNVVMSupportedSamplerStorageType(type);
    IRSamplerStateTypeBase* samplerValue = asNVVMSupportedSamplerValueType(type);
    IRUnsizedArrayType* unsizedSamplerArrayStorage =
        asNVVMSupportedUnsizedSamplerArrayStorageType(type);
    IRPtrTypeBase* resourceElementPointer =
        asNVVMSupportedRWStructuredBufferElementPointerType(type);
    IRPtrTypeBase* sharedElementPointer = asNVVMSupportedSharedI32ElementPointerType(type);

    // Preflight admits types by their producer/consumer role. Check that role before looking in the
    // cache so a handle created for a valid value cannot make the same type valid in a forbidden
    // helper signature.
    const bool isLegal =
        (use == NVVMTypeUse::EntryPointResult && isVoid) ||
        (use == NVVMTypeUse::HelperResult &&
         (isVoid || isNVVMSupportedValueType(type) || copyableStructType)) ||
        (use == NVVMTypeUse::EntryPointParameter &&
         (isInteger || isFloat32 || scalarStructType || deviceNumericPointer ||
          deviceArrayPointer || isRawBuffer)) ||
        (use == NVVMTypeUse::HelperParameter &&
         (isNVVMSupportedValueType(type) || fixedNumericArrayType || copyableStructType ||
          localScalarStructPointer || localNumericPointer || localNumericArrayPointer ||
          isSurface || isSampledTexture || samplerValue)) ||
        (use == NVVMTypeUse::Value &&
         (isInteger || isFloatingPoint || isBool || valueVectorType || copyableStructType ||
          physicalArrayStructType || fixedNumericArrayType || deviceNumericPointer ||
          deviceArrayPointer || isRawBuffer || isBufferDataPointer || parameterGroup || isSurface ||
          isSampledTexture || samplerValue || resourceElementPointer || sharedElementPointer)) ||
        (use == NVVMTypeUse::Storage &&
         (isInteger || isFloat32 || structType || fixedNumericArrayType || isRawBuffer ||
          parameterGroup || isSurface || isSampledTexture || samplerStorage ||
          unsizedSamplerArrayStorage));
    if (!isLegal)
        return _reportUnsupportedType(use);

    // NVPTX represents an aggregate kernel parameter as a generic pointer carrying `byval`, while
    // the same canonical Slang struct remains a first-class LLVM struct in ordinary value roles.
    // Keep this physical ABI representation separate from the canonical value-type cache.
    if (use == NVVMTypeUse::EntryPointParameter && scalarStructType)
    {
        if (auto mappedType = m_entryParameterRepresentationMap.tryGetValue(type))
        {
            outType = *mappedType;
            return SLANG_OK;
        }

        SlangNVVMTypeHandle aggregateType = nullptr;
        SLANG_RETURN_ON_FAIL(lowerType(type, NVVMTypeUse::Value, aggregateType));
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            "by-value aggregate parameter type",
            m_builder.getPointerType(
                m_module,
                aggregateType,
                SLANG_NVVM_ADDRESS_SPACE_GENERIC,
                outType)));
        m_entryParameterRepresentationMap[type] = outType;
        return SLANG_OK;
    }

    if (use == NVVMTypeUse::HelperParameter && localScalarStructPointer)
    {
        return _lowerPointerType(
            type,
            localScalarStructValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType);
    }

    if (use == NVVMTypeUse::HelperParameter && localNumericPointer)
    {
        return _lowerPointerType(
            type,
            localNumericPointerValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType);
    }

    if (use == NVVMTypeUse::HelperParameter && localNumericArrayPointer)
    {
        return _lowerPointerType(
            type,
            localNumericArrayPointerValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType);
    }

    if (auto mappedType = m_typeMap.tryGetValue(type))
    {
        outType = *mappedType;
        return SLANG_OK;
    }

    if (isVoid)
    {
        SLANG_RETURN_ON_FAIL(
            _requireBuilderOperation("void type", m_builder.getVoidType(m_module, outType)));
    }
    else if (isInteger || isBool)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            isInteger ? "selected integer type" : "Boolean type",
            m_builder.getIntegerType(m_module, isInteger ? integerBitWidth : 1u, outType)));
    }
    else if (isFloatingPoint)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            floatingPointBitWidth == 16 ? "float16 type" : "float32 type",
            m_builder.getFloatingPointType(m_module, floatingPointBitWidth, outType)));
    }
    else if (valueVectorType)
    {
        SlangNVVMTypeHandle elementType = nullptr;
        SLANG_RETURN_ON_FAIL(
            lowerType(valueVectorType->getElementType(), NVVMTypeUse::Value, elementType));
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            "selected value vector type",
            m_builder.getVectorType(m_module, elementType, valueVectorElementCount, outType)));
    }
    else if (fixedNumericArrayType)
    {
        return _lowerArrayType(fixedNumericArrayType, outType);
    }
    else if (structType)
    {
        return _lowerStructType(structType, outType);
    }
    else if (deviceNumericPointer)
    {
        return _lowerPointerType(
            type,
            deviceNumericPointer->getValueType(),
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            outType);
    }
    else if (deviceArrayPointer)
    {
        return _lowerPointerType(type, deviceArrayType, SLANG_NVVM_ADDRESS_SPACE_GLOBAL, outType);
    }
    else if (isRawBuffer)
    {
        return _lowerRawBufferType(rawBufferType, outType);
    }
    else if (isBufferDataPointer)
    {
        return _lowerPointerType(
            type,
            bufferDataPointerType.elementType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            outType);
    }
    else if (parameterGroup)
    {
        return _lowerParameterGroupType(parameterGroup, parameterGroupElementType, outType);
    }
    else if (isSurface || isSampledTexture)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            isSurface ? "CUDA surface handle type" : "CUDA texture handle type",
            m_builder.getIntegerType(m_module, 64, outType)));
    }
    else if (samplerStorage)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            "CUDA sampler placeholder storage type",
            m_builder.getIntegerType(m_module, 64, outType)));
    }
    else if (unsizedSamplerArrayStorage)
    {
        return _lowerUnsizedSamplerArrayStorageType(unsizedSamplerArrayStorage, outType);
    }
    else if (sharedElementPointer)
    {
        return _lowerPointerType(
            type,
            sharedElementPointer->getValueType(),
            SLANG_NVVM_ADDRESS_SPACE_SHARED,
            outType);
    }
    else
    {
        SLANG_RELEASE_ASSERT(resourceElementPointer);
        return _lowerPointerType(
            type,
            resourceElementPointer->getValueType(),
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            outType);
    }

    m_typeMap[type] = outType;
    return SLANG_OK;
}

} // namespace Slang
