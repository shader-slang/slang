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

bool isNVVMFloat64Type(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Double;
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
    if (isNVVMFloat64Type(type))
    {
        if (outBitWidth)
            *outBitWidth = 64;
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
    uint32_t floatingPointBitWidth = 0;
    const bool isSupportedVectorFloat =
        isNVVMSupportedFloatingPointScalarType(elementType, &floatingPointBitWidth);
    if (!vectorType ||
        (!isNVVMSupportedIntegerScalarType(elementType) && !isSupportedVectorFloat &&
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
    const bool isSupportedInteger =
        isNVVMSupportedIntegerScalarType(type, &integerBitWidth) &&
        (integerBitWidth == 16 || integerBitWidth == 32 || integerBitWidth == 64);
    if (isSupportedInteger || isNVVMFloat16Type(type) || isNVVMFloat32Type(type))
        return true;

    auto vectorType = asNVVMSupportedNumericVectorType(type);
    return vectorType && _isNVVMSupportedByteAddressLeafValueType(vectorType->getElementType());
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
        if (onlyField || (!asNVVMSupportedNumericArrayType(field->getFieldType()) &&
                          !asNVVMSupportedAggregateStorageArrayType(field->getFieldType())))
            return nullptr;
        onlyField = field;
    }
    return onlyField ? structType : nullptr;
}

IRVectorType* asNVVMSupportedCompactParameterGroupVectorType(IRInst* type)
{
    uint32_t elementCount = 0;
    auto vectorType = asNVVMSupportedNumericVectorType(type, &elementCount);
    if (!vectorType)
        return nullptr;

    // CUDA's native three-lane 32-bit vectors have scalar alignment and no tail padding, whereas
    // LLVM gives the corresponding value vector four-lane storage. Slang's CUDA prelude spells
    // half3 and half4 as 8-byte, 4-byte-aligned structs, which also differs from LLVM's value
    // vectors. These are the exact selected vector families that need a distinct storage type.
    if (elementCount == 3 && (isNVVMInteger32Type(vectorType->getElementType()) ||
                              isNVVMFloat32Type(vectorType->getElementType())))
    {
        return vectorType;
    }
    return isNVVMFloat16Type(vectorType->getElementType()) && elementCount >= 3 ? vectorType
                                                                                : nullptr;
}

static bool _isNVVMSupportedAggregateStorageType(IRInst* type, HashSet<IRInst*>& activeTypes);

static IRArrayType* _asNVVMSupportedAggregateStorageArrayType(
    IRInst* type,
    uint32_t* outElementCount,
    HashSet<IRInst*>& activeTypes)
{
    if (outElementCount)
        *outElementCount = 0;
    if (auto arrayType = asNVVMSupportedNumericArrayType(type, outElementCount))
        return arrayType;

    auto arrayType = as<IRArrayType>(type);
    auto elementCount = arrayType ? as<IRIntLit>(arrayType->getElementCount()) : nullptr;
    auto stride = arrayType ? as<IRIntLit>(arrayType->getArrayStride()) : nullptr;
    if (!arrayType || arrayType->getOp() != kIROp_ArrayType || !elementCount ||
        elementCount->getValue() <= 0 || elementCount->getValue() > UINT32_MAX)
    {
        return nullptr;
    }

    if (auto compactVector =
            asNVVMSupportedCompactParameterGroupVectorType(arrayType->getElementType()))
    {
        const IRIntegerValue compactStride =
            isNVVMFloat16Type(compactVector->getElementType()) ? 8 : 12;
        if (!stride || stride->getValue() != compactStride)
            return nullptr;
    }
    else
    {
        // Generic LLVM arrays select their natural element stride. An explicit non-numeric stride
        // would require a padded physical element type, which this storage algebra does not invent.
        if (arrayType->getOperandCount() != 2 || activeTypes.contains(arrayType))
            return nullptr;
        activeTypes.add(arrayType);
        if (!_isNVVMSupportedAggregateStorageType(arrayType->getElementType(), activeTypes))
        {
            activeTypes.remove(arrayType);
            return nullptr;
        }
        activeTypes.remove(arrayType);
    }

    if (outElementCount)
        *outElementCount = uint32_t(elementCount->getValue());
    return arrayType;
}

IRArrayType* asNVVMSupportedAggregateStorageArrayType(IRInst* type, uint32_t* outElementCount)
{
    HashSet<IRInst*> activeTypes;
    return _asNVVMSupportedAggregateStorageArrayType(type, outElementCount, activeTypes);
}

static bool _isNVVMSupportedAggregateStorageType(IRInst* type, HashSet<IRInst*>& activeTypes)
{
    if (isNVVMSupportedIntegerScalarType(type) || isNVVMBoolType(type) || isNVVMFloat16Type(type) ||
        isNVVMFloat32Type(type) || asNVVMSupported32BitNumericVectorType(type) ||
        asNVVMSupportedCompactParameterGroupVectorType(type))
    {
        return true;
    }

    NVVMRawBufferType rawBufferType;
    NVVMSurfaceType surfaceType;
    NVVMReadOnlyTextureType sampledTextureType;
    if (getNVVMSupportedRawBufferType(type, rawBufferType) ||
        getNVVMSupportedSurfaceType(type, surfaceType) ||
        getNVVMSupportedReadOnlyTextureType(type, sampledTextureType) ||
        asNVVMSupportedDescriptorHandleType(type) ||
        asNVVMSupportedSamplerStorageType(type) ||
        asNVVMSupportedDeviceCopyableValuePointerType(type))
    {
        return true;
    }

    if (activeTypes.contains(type))
        return false;

    // A nested parameter group is one pointer-sized storage leaf, but its pointee still needs an
    // executable parameter-group representation. Consider this example:
    //
    //     struct Scene { ParameterBlock<Material> material; }
    //     ParameterBlock<Scene> scene;
    //
    // Entry-point-uniform lowering stores the `Material` parameter block pointer in `Scene` and
    // preserves the specialized `Material` element type on that pointer. Prove the complete
    // pointee here while the active set prevents recursive parameter-group declarations from
    // creating an infinite storage type.
    if (auto parameterGroupType = as<IRParameterGroupType>(type))
    {
        if (parameterGroupType->getOp() != kIROp_ParameterBlockType &&
            parameterGroupType->getOp() != kIROp_ConstantBufferType)
        {
            return false;
        }
        activeTypes.add(type);
        const bool isSupported =
            _isNVVMSupportedAggregateStorageType(parameterGroupType->getElementType(), activeTypes);
        activeTypes.remove(type);
        return isSupported;
    }

    if (auto arrayType = as<IRArrayType>(type))
    {
        uint32_t elementCount = 0;
        auto supportedArray =
            _asNVVMSupportedAggregateStorageArrayType(arrayType, &elementCount, activeTypes);
        return supportedArray && elementCount;
    }

    auto structType = as<IRStructType>(type);
    if (!structType)
        return false;

    activeTypes.add(type);
    bool hasField = false;
    for (auto field : structType->getFields())
    {
        if (!_isNVVMSupportedAggregateStorageType(field->getFieldType(), activeTypes))
        {
            activeTypes.remove(type);
            return false;
        }
        hasField = true;
    }
    activeTypes.remove(type);
    return hasField;
}

bool isNVVMSupportedAggregateStorageType(IRInst* type)
{
    HashSet<IRInst*> activeTypes;
    return _isNVVMSupportedAggregateStorageType(type, activeTypes);
}

IRStructType* asNVVMSupportedAggregateStorageStructType(IRInst* type)
{
    auto structType = as<IRStructType>(type);
    return structType && isNVVMSupportedAggregateStorageType(type) ? structType : nullptr;
}

static bool _isNVVMSupportedCopyableValueType(IRInst* type, HashSet<IRInst*>& activeTypes)
{
    if (isNVVMSupportedValueType(type))
        return true;

    if (activeTypes.contains(type))
        return false;

    if (auto arrayType = as<IRArrayType>(type))
    {
        auto elementCount = as<IRIntLit>(arrayType->getElementCount());
        if (arrayType->getOp() != kIROp_ArrayType ||
            (arrayType->getOperandCount() != 2 && !asNVVMSupportedNumericArrayType(arrayType)) ||
            !elementCount || elementCount->getValue() <= 0 || elementCount->getValue() > UINT32_MAX)
        {
            return false;
        }

        activeTypes.add(type);
        const bool isSupported =
            _isNVVMSupportedCopyableValueType(arrayType->getElementType(), activeTypes);
        activeTypes.remove(type);
        return isSupported;
    }

    auto structType = as<IRStructType>(type);
    if (!structType)
        return false;

    activeTypes.add(type);
    bool hasField = false;
    for (auto field : structType->getFields())
    {
        if (!_isNVVMSupportedCopyableValueType(field->getFieldType(), activeTypes))
        {
            activeTypes.remove(type);
            return false;
        }
        hasField = true;
    }
    activeTypes.remove(type);
    return hasField;
}

bool isNVVMSupportedCopyableValueType(IRInst* type)
{
    HashSet<IRInst*> activeTypes;
    return _isNVVMSupportedCopyableValueType(type, activeTypes);
}

IRStructType* asNVVMSupportedCopyableStructType(IRInst* type)
{
    auto structType = as<IRStructType>(type);
    return structType && isNVVMSupportedCopyableValueType(type) ? structType : nullptr;
}

IRArrayType* asNVVMSupportedCopyableArrayType(IRInst* type, uint32_t* outElementCount)
{
    if (outElementCount)
        *outElementCount = 0;
    auto arrayType = as<IRArrayType>(type);
    auto elementCount = arrayType ? as<IRIntLit>(arrayType->getElementCount()) : nullptr;
    if (!arrayType || !isNVVMSupportedCopyableValueType(arrayType) || !elementCount)
    {
        return nullptr;
    }

    if (outElementCount)
        *outElementCount = uint32_t(elementCount->getValue());
    return arrayType;
}

IRPtrTypeBase* asNVVMSupportedDeviceCopyableValuePointerType(IRInst* type, IRType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    IRType* valueType = pointerType ? pointerType->getValueType() : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    if (!pointerType || pointerType->getOp() != kIROp_PtrType ||
        pointerType->getOperandCount() != 4 ||
        pointerType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        pointerType->getAddressSpace() != AddressSpace::UserPointer || !dataLayout ||
        dataLayout->getOp() != kIROp_DefaultBufferLayoutType ||
        !isNVVMSupportedCopyableValueType(valueType))
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

static bool _isNVVMSupportedHelperValueType(IRInst* type, HashSet<IRInst*>& activeTypes);

// Recognizes one canonical CUDA user-pointer leaf while proving its complete finite pointee. For
// example, dynamic-dispatch lowering turns `IFoo**` into `Ptr<Ptr<Tuple>>`: the outer pointer is a
// helper value only because the inner pointer and its existential tuple are helper values too.
// Keeping the active set here rejects recursive pointee graphs instead of assigning them a
// provider type that cannot have finite size.
static IRPtrTypeBase* _asNVVMSupportedDeviceHelperValuePointerType(
    IRInst* type,
    IRType** outValueType,
    HashSet<IRInst*>& activeTypes)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    IRType* valueType = pointerType ? pointerType->getValueType() : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    if (!pointerType || pointerType->getOp() != kIROp_PtrType ||
        pointerType->getOperandCount() != 4 ||
        pointerType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        pointerType->getAddressSpace() != AddressSpace::UserPointer || !dataLayout ||
        dataLayout->getOp() != kIROp_DefaultBufferLayoutType || activeTypes.contains(type))
    {
        return nullptr;
    }

    activeTypes.add(type);
    const bool isSupported = _isNVVMSupportedHelperValueType(valueType, activeTypes);
    activeTypes.remove(type);
    if (!isSupported)
        return nullptr;
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

static bool _isNVVMSupportedHelperValueType(IRInst* type, HashSet<IRInst*>& activeTypes)
{
    if (isNVVMSupportedCopyableValueType(type) || asNVVMSupportedDescriptorHandleType(type))
    {
        return true;
    }
    if (!type || activeTypes.contains(type))
        return false;
    if (_asNVVMSupportedDeviceHelperValuePointerType(type, nullptr, activeTypes))
        return true;

    if (auto arrayType = as<IRArrayType>(type))
    {
        auto elementCount = as<IRIntLit>(arrayType->getElementCount());
        if (arrayType->getOp() != kIROp_ArrayType || arrayType->getOperandCount() != 2 ||
            !elementCount || elementCount->getValue() <= 0 || elementCount->getValue() > UINT32_MAX)
            return false;
        activeTypes.add(type);
        const bool result =
            _isNVVMSupportedHelperValueType(arrayType->getElementType(), activeTypes);
        activeTypes.remove(type);
        return result;
    }

    auto structType = as<IRStructType>(type);
    if (!structType)
        return false;
    activeTypes.add(type);
    bool hasField = false;
    for (auto field : structType->getFields())
    {
        if (!_isNVVMSupportedHelperValueType(field->getFieldType(), activeTypes))
        {
            activeTypes.remove(type);
            return false;
        }
        hasField = true;
    }
    activeTypes.remove(type);
    return hasField;
}

bool isNVVMSupportedHelperValueType(IRInst* type)
{
    HashSet<IRInst*> activeTypes;
    return _isNVVMSupportedHelperValueType(type, activeTypes);
}

IRPtrTypeBase* asNVVMSupportedDeviceHelperValuePointerType(
    IRInst* type,
    IRType** outValueType)
{
    HashSet<IRInst*> activeTypes;
    return _asNVVMSupportedDeviceHelperValuePointerType(type, outValueType, activeTypes);
}

static uint32_t _getNVVMHelperValueAlignment(IRInst* type, HashSet<IRInst*>& activeTypes)
{
    if (const uint32_t copyableAlignment = getNVVMCopyableValueAlignment(type))
        return copyableAlignment;
    if (asNVVMSupportedDeviceHelperValuePointerType(type))
        return 8;
    if (asNVVMSupportedDescriptorHandleType(type))
        return getNVVMResourceValueAlignment(type);
    if (!type || activeTypes.contains(type))
        return 0;

    activeTypes.add(type);
    uint32_t alignment = 0;
    bool hasElement = false;
    if (auto arrayType = as<IRArrayType>(type))
    {
        alignment = _getNVVMHelperValueAlignment(arrayType->getElementType(), activeTypes);
        hasElement = alignment != 0;
    }
    else if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            const uint32_t fieldAlignment =
                _getNVVMHelperValueAlignment(field->getFieldType(), activeTypes);
            if (!fieldAlignment)
            {
                alignment = 0;
                hasElement = false;
                break;
            }
            alignment = Math::Max(alignment, fieldAlignment);
            hasElement = true;
        }
    }
    activeTypes.remove(type);
    return hasElement ? alignment : 0;
}

uint32_t getNVVMHelperValueAlignment(IRInst* type)
{
    if (!isNVVMSupportedHelperValueType(type))
        return 0;
    HashSet<IRInst*> activeTypes;
    return _getNVVMHelperValueAlignment(type, activeTypes);
}

IRArrayType* asNVVMSupportedHelperArrayType(IRInst* type, uint32_t* outElementCount)
{
    if (outElementCount)
        *outElementCount = 0;
    auto arrayType = as<IRArrayType>(type);
    auto elementCount = arrayType ? as<IRIntLit>(arrayType->getElementCount()) : nullptr;
    if (!arrayType || !elementCount || elementCount->getValue() <= 0 ||
        !isNVVMSupportedHelperValueType(type))
    {
        return nullptr;
    }
    if (outElementCount)
        *outElementCount = uint32_t(elementCount->getValue());
    return arrayType;
}

IRStructType* asNVVMSupportedHelperStructType(IRInst* type)
{
    auto structType = as<IRStructType>(type);
    return structType && isNVVMSupportedHelperValueType(type) ? structType : nullptr;
}

IRAtomicType* asNVVMSupportedAtomicType(IRInst* type, IRType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto atomicType = as<IRAtomicType>(type);
    IRType* valueType = atomicType ? atomicType->getElementType() : nullptr;
    uint32_t integerBitWidth = 0;
    const bool isSelectedInteger = isNVVMSupportedIntegerScalarType(valueType, &integerBitWidth) &&
                                   (integerBitWidth == 32 || integerBitWidth == 64);
    if (!atomicType ||
        (!isSelectedInteger && !isNVVMFloat32Type(valueType) && !isNVVMFloat64Type(valueType)))
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return atomicType;
}

IRPtrTypeBase* asNVVMSupportedHelperReferencePointerType(IRInst* type, IRType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    IRType* valueType = pointerType ? pointerType->getValueType() : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    const bool isMutableReference = pointerType && pointerType->getOp() == kIROp_RefParamType &&
                                    pointerType->getAccessQualifier() == AccessQualifier::ReadWrite;
    const bool isImmutableReference = pointerType &&
                                      pointerType->getOp() == kIROp_BorrowInParamType &&
                                      pointerType->getAccessQualifier() == AccessQualifier::Read;
    if (!pointerType || pointerType->getOperandCount() != 4 ||
        (!isMutableReference && !isImmutableReference) ||
        pointerType->getAddressSpace() != AddressSpace::Generic || !dataLayout ||
        dataLayout->getOp() != kIROp_DefaultBufferLayoutType ||
        (!isNVVMSupportedHelperValueType(valueType) && !asNVVMSupportedAtomicType(valueType)))
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRStructType* asNVVMSupportedPhysicalAggregateStorageStructType(IRInst* type)
{
    auto structType = asNVVMSupportedAggregateStorageStructType(type);
    return structType && structType->findDecoration<IRPhysicalTypeDecoration>() ? structType
                                                                                : nullptr;
}

IRPtrTypeBase* asNVVMSupportedPhysicalStorageReferencePointerType(
    IRInst* type,
    IRStructType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;

    auto pointerType = as<IRPtrTypeBase>(type);
    auto valueType =
        pointerType ? asNVVMSupportedPhysicalAggregateStorageStructType(pointerType->getValueType())
                    : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    if (!pointerType || pointerType->getOp() != kIROp_BorrowInParamType ||
        pointerType->getOperandCount() != 4 ||
        pointerType->getAccessQualifier() != AccessQualifier::Read ||
        pointerType->getAddressSpace() != AddressSpace::Generic || !dataLayout ||
        dataLayout->getOp() != kIROp_DefaultBufferLayoutType || !valueType)
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedLocalPhysicalStoragePointerType(
    IRInst* type,
    IRStructType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;

    auto pointerType = as<IRPtrTypeBase>(type);
    auto valueType =
        pointerType ? asNVVMSupportedPhysicalAggregateStorageStructType(pointerType->getValueType())
                    : nullptr;
    if (!pointerType || pointerType->getOp() != kIROp_PtrType ||
        pointerType->getOperandCount() != 1 ||
        pointerType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        pointerType->getAddressSpace() != AddressSpace::Generic || !valueType)
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedDevicePhysicalStoragePointerType(
    IRInst* type,
    IRStructType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;

    auto pointerType = as<IRPtrTypeBase>(type);
    auto valueType =
        pointerType ? asNVVMSupportedPhysicalAggregateStorageStructType(pointerType->getValueType())
                    : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    if (!pointerType || pointerType->getOp() != kIROp_PtrType ||
        pointerType->getOperandCount() != 4 ||
        pointerType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        pointerType->getAddressSpace() != AddressSpace::UserPointer || !dataLayout ||
        dataLayout->getOp() != kIROp_DefaultBufferLayoutType || !valueType)
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedSharedHelperPointerType(IRInst* type, IRType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    IRType* valueType = pointerType ? pointerType->getValueType() : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    if (!pointerType || pointerType->getOp() != kIROp_PtrType ||
        pointerType->getOperandCount() != 4 ||
        pointerType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        pointerType->getAddressSpace() != AddressSpace::GroupShared || !dataLayout ||
        dataLayout->getOp() != kIROp_DefaultBufferLayoutType ||
        !isNVVMSupportedHelperValueType(valueType))
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedLocalHelperValuePointerType(IRInst* type, IRType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    IRType* valueType = pointerType ? pointerType->getValueType() : nullptr;
    const bool isPlainLocalPointer =
        pointerType && pointerType->getOp() == kIROp_PtrType && pointerType->getOperandCount() == 1;
    const bool isMutableParameter = pointerType &&
                                    (pointerType->getOp() == kIROp_OutParamType ||
                                     pointerType->getOp() == kIROp_BorrowInOutParamType) &&
                                    pointerType->getOperandCount() == 1;
    if (!pointerType || isNVVMSupportedCopyableValueType(valueType) ||
        !isNVVMSupportedHelperValueType(valueType) ||
        (!isPlainLocalPointer && !isMutableParameter) ||
        pointerType->getAddressSpace() != AddressSpace::Generic)
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedLocalCopyableValuePointerType(IRInst* type, IRType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    IRType* valueType = pointerType ? pointerType->getValueType() : nullptr;
    const bool isPlainLocalPointer =
        pointerType && pointerType->getOp() == kIROp_PtrType && pointerType->getOperandCount() == 1;
    const bool isMutableParameter = pointerType &&
                                    (pointerType->getOp() == kIROp_OutParamType ||
                                     pointerType->getOp() == kIROp_BorrowInOutParamType) &&
                                    pointerType->getOperandCount() == 1;
    if (!pointerType || !isNVVMSupportedCopyableValueType(valueType) ||
        (!isPlainLocalPointer && !isMutableParameter) ||
        pointerType->getAddressSpace() != AddressSpace::Generic)
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedDerivedCopyableValuePointerType(IRInst* type, IRType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    IRType* valueType = pointerType ? pointerType->getValueType() : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    const bool hasCanonicalLayout =
        pointerType && ((pointerType->getOperandCount() == 3 && !dataLayout) ||
                        (pointerType->getOperandCount() == 4 && dataLayout &&
                         dataLayout->getOp() == kIROp_ScalarBufferLayoutType));
    if (!pointerType || pointerType->getOp() != kIROp_PtrType || !hasCanonicalLayout ||
        pointerType->getAddressSpace() != AddressSpace::Generic ||
        !isNVVMSupportedCopyableValueType(valueType))
    {
        return nullptr;
    }
    if (outValueType)
        *outValueType = valueType;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedLocalNumericPointerType(IRInst* type, IRType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    IRType* valueType = nullptr;
    auto pointerType = asNVVMSupportedLocalCopyableValuePointerType(type, &valueType);
    if (!pointerType || !isNVVMSupportedNumericValueType(valueType))
        return nullptr;
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

    IRType* pointerValueType = nullptr;
    auto pointerType = asNVVMSupportedLocalCopyableValuePointerType(type, &pointerValueType);
    uint32_t elementCount = 0;
    auto valueType = asNVVMSupportedNumericArrayType(pointerValueType, &elementCount);
    if (!pointerType || !valueType)
        return nullptr;
    if (outValueType)
        *outValueType = valueType;
    if (outElementCount)
        *outElementCount = elementCount;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedLocalCopyableArrayPointerType(
    IRInst* type,
    IRArrayType** outValueType,
    uint32_t* outElementCount)
{
    if (outValueType)
        *outValueType = nullptr;
    if (outElementCount)
        *outElementCount = 0;

    IRType* pointerValueType = nullptr;
    auto pointerType = asNVVMSupportedLocalCopyableValuePointerType(type, &pointerValueType);
    uint32_t elementCount = 0;
    auto valueType = asNVVMSupportedCopyableArrayType(pointerValueType, &elementCount);
    if (!pointerType || !valueType)
        return nullptr;
    if (outValueType)
        *outValueType = valueType;
    if (outElementCount)
        *outElementCount = elementCount;
    return pointerType;
}

IRPtrTypeBase* asNVVMSupportedLocalResourceStructPointerType(
    IRInst* type,
    IRStructType** outValueType)
{
    if (outValueType)
        *outValueType = nullptr;
    auto pointerType = as<IRPtrTypeBase>(type);
    auto valueType =
        pointerType ? asNVVMSupportedResourceStructType(pointerType->getValueType()) : nullptr;
    IRType* dataLayout = pointerType ? pointerType->getDataLayout() : nullptr;
    const bool isLocalPointer =
        pointerType && pointerType->getOp() == kIROp_PtrType && pointerType->getOperandCount() == 1;
    const bool isMutableBorrow = pointerType &&
                                 pointerType->getOp() == kIROp_BorrowInOutParamType &&
                                 pointerType->getOperandCount() == 1;
    // Consider `void set(inout Outer value)`: the helper receives `BorrowInOutParam<Outer>`, while
    // its caller passes the `Ptr<Outer>` produced by a local `var`. Both point at the same selected
    // resource-capable aggregate representation. Explicit-global-context lowering adds the complete
    // CUDA thread-local pointer spelling, but that established producer remains scalar-struct-only.
    const bool isThreadLocalContextPointer =
        asNVVMSupportedScalarStructType(valueType) && pointerType &&
        pointerType->getOp() == kIROp_PtrType && pointerType->getOperandCount() == 4 &&
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
    if (isNVVMBoolType(type))
        return 1;
    uint32_t vectorElementCount = 0;
    if (auto vectorType = asNVVMSupportedValueVectorType(type, &vectorElementCount))
    {
        SLANG_RELEASE_ASSERT(isNVVMBoolType(vectorType->getElementType()));
        return 1;
    }
    if (auto arrayType = asNVVMSupportedCopyableArrayType(type))
        return getNVVMCopyableValueAlignment(arrayType->getElementType());
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

bool getNVVMSupportedSharedGlobal(IRInst* inst, NVVMSharedGlobal* outGlobal)
{
    if (outGlobal)
        *outGlobal = {};

    auto globalVar = as<IRGlobalVar>(inst);
    auto ptrType = globalVar ? globalVar->getDataType() : nullptr;
    IRType* valueType = ptrType ? ptrType->getValueType() : nullptr;
    IRType* dataLayout = ptrType ? ptrType->getDataLayout() : nullptr;
    IRType* atomicValueType = nullptr;
    const bool isAtomic = asNVVMSupportedAtomicType(valueType, &atomicValueType) != nullptr;
    const bool hasCanonicalPointerType =
        ptrType && ptrType->getOp() == kIROp_PtrType &&
        (ptrType->getOperandCount() == 1 ||
         (ptrType->getOperandCount() == 3 &&
          ptrType->getAccessQualifier() == AccessQualifier::ReadWrite &&
          ptrType->getAddressSpace() == AddressSpace::Generic && !dataLayout));
    if (!globalVar || !as<IRGroupSharedRate>(globalVar->getRate()) || globalVar->getFirstBlock() ||
        !hasCanonicalPointerType ||
        (!isAtomic && !isNVVMSupportedHelperValueType(valueType)))
    {
        return false;
    }

    if (outGlobal)
    {
        outGlobal->globalVar = globalVar;
        outGlobal->storageType = valueType;
        outGlobal->alignmentType = isAtomic ? atomicValueType : valueType;
    }
    return true;
}

IRPtrTypeBase* asNVVMSupportedSharedElementPointerType(IRInst* type)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    IRType* dataLayout = ptrType ? ptrType->getDataLayout() : nullptr;
    if (!ptrType || ptrType->getOp() != kIROp_PtrType || ptrType->getOperandCount() != 4 ||
        !isNVVMSupportedHelperValueType(ptrType->getValueType()) ||
        ptrType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        ptrType->getAddressSpace() != AddressSpace::GroupShared || !dataLayout ||
        dataLayout->getOp() != kIROp_ScalarBufferLayoutType)
    {
        return nullptr;
    }
    return ptrType;
}

static bool _getNVVMSupportedRawBufferType(
    IRInst* type,
    NVVMRawBufferType& outType,
    HashSet<IRInst*>& activeTypes);
static bool _hasNVVMParameterGroupStorageValueRepresentation(
    IRInst* type,
    HashSet<IRInst*>& activeTypes);

// Returns the natural alignment of one canonical resource-capable value, or zero when the type is
// outside the contract. The active set makes resource indirection cycle-safe. Consider this
// example:
//
//     struct Node { StructuredBuffer<Node> children; }
//
// Classifying `Node` reaches the buffer element `Node` again. Such a recursive value is not an
// executable finite LLVM aggregate, so reject it at the declaration boundary instead of recursing
// indefinitely or relying on a later provider failure.
static uint32_t _getNVVMResourceValueAlignment(IRInst* type, HashSet<IRInst*>& activeTypes)
{
    if (const uint32_t copyableAlignment = getNVVMCopyableValueAlignment(type))
        return copyableAlignment;
    IRType* atomicValueType = nullptr;
    if (asNVVMSupportedAtomicType(type, &atomicValueType))
        return getNVVMCopyableValueAlignment(atomicValueType);
    if (isNVVMBoolType(type))
        return 1;
    if (asNVVMSupportedDevicePhysicalStoragePointerType(type))
        return 8;

    IRType* parameterGroupElementType = nullptr;
    if (asNVVMSupportedParameterGroupType(type, &parameterGroupElementType))
    {
        if (activeTypes.contains(type))
            return 0;
        activeTypes.add(type);
        const bool hasValueRepresentation = _hasNVVMParameterGroupStorageValueRepresentation(
            parameterGroupElementType,
            activeTypes);
        activeTypes.remove(type);
        return hasValueRepresentation ? 8 : 0;
    }

    IRType* descriptorResourceType = nullptr;
    if (asNVVMSupportedDescriptorHandleType(type, &descriptorResourceType))
        return _getNVVMResourceValueAlignment(descriptorResourceType, activeTypes);

    NVVMRawBufferType rawBufferType;
    NVVMSurfaceType surfaceType;
    NVVMReadOnlyTextureType sampledTextureType;
    if (_getNVVMSupportedRawBufferType(type, rawBufferType, activeTypes) ||
        getNVVMSupportedSurfaceType(type, surfaceType) ||
        getNVVMSupportedReadOnlyTextureType(type, sampledTextureType) ||
        asNVVMSupportedSamplerValueType(type))
    {
        return 8;
    }

    // Consider `Texture2D textures[2]` in the synthesized CUDA parameter block. Uniform lowering
    // retains a fixed ArrayType, then ordinary IR loads the complete value before selecting one
    // texture. Each texture is already the canonical i64 CUDA handle, so the array has the same
    // natural LLVM and CUDA stride. Require the exact two-operand fixed-array producer here; an
    // explicit stride belongs to a physical-storage representation and must be proved separately.
    if (auto arrayType = as<IRArrayType>(type))
    {
        auto elementCount = as<IRIntLit>(arrayType->getElementCount());
        if (arrayType->getOp() != kIROp_ArrayType || arrayType->getOperandCount() != 2 ||
            !elementCount || elementCount->getValue() <= 0 ||
            elementCount->getValue() > UINT32_MAX || activeTypes.contains(type))
        {
            return 0;
        }
        activeTypes.add(type);
        const uint32_t alignment =
            _getNVVMResourceValueAlignment(arrayType->getElementType(), activeTypes);
        activeTypes.remove(type);
        return alignment;
    }

    auto structType = as<IRStructType>(type);
    if (!structType || activeTypes.contains(type))
        return 0;

    activeTypes.add(type);
    uint32_t alignment = 0;
    bool hasField = false;
    for (auto field : structType->getFields())
    {
        const uint32_t fieldAlignment =
            _getNVVMResourceValueAlignment(field->getFieldType(), activeTypes);
        if (!fieldAlignment)
        {
            activeTypes.remove(type);
            return 0;
        }
        alignment = Math::Max(alignment, fieldAlignment);
        hasField = true;
    }
    activeTypes.remove(type);
    return hasField ? alignment : 0;
}

IRStructType* asNVVMSupportedResourceStructType(IRInst* type)
{
    auto structType = as<IRStructType>(type);
    if (!structType)
        return nullptr;
    HashSet<IRInst*> activeTypes;
    return _getNVVMResourceValueAlignment(type, activeTypes) ? structType : nullptr;
}

uint32_t getNVVMResourceValueAlignment(IRInst* type)
{
    HashSet<IRInst*> activeTypes;
    return _getNVVMResourceValueAlignment(type, activeTypes);
}

IRArrayType* asNVVMSupportedResourceArrayType(IRInst* type, uint32_t* outElementCount)
{
    if (outElementCount)
        *outElementCount = 0;
    auto arrayType = as<IRArrayType>(type);
    auto elementCount = arrayType ? as<IRIntLit>(arrayType->getElementCount()) : nullptr;
    if (!arrayType || !elementCount || isNVVMSupportedHelperValueType(arrayType) ||
        !getNVVMResourceValueAlignment(arrayType))
        return nullptr;
    if (outElementCount)
        *outElementCount = uint32_t(elementCount->getValue());
    return arrayType;
}

static bool _isNVVMSupportedResourceElementType(IRInst* type, HashSet<IRInst*>& activeTypes)
{
    // Resource lowering preserves the exact specialized element type in the raw view and every
    // typed element pointer. Reuse the value algebra that generic type/memory emission already
    // supports instead of maintaining the older integer/Float32 subset here.
    return isNVVMSupportedStructuredBufferStorageType(type) ||
           isNVVMSupportedNumericValueType(type) || asNVVMSupportedAtomicType(type) ||
           asNVVMSupportedPhysicalArrayStructType(type) ||
           (as<IRStructType>(type) && _getNVVMResourceValueAlignment(type, activeTypes));
}

static bool _getNVVMSupportedRawBufferType(
    IRInst* type,
    NVVMRawBufferType& outType,
    HashSet<IRInst*>& activeTypes)
{
    outType = {};

    // The ordinary buffer-element legalization may append a layout-conformance operand, while
    // `lowerStructuredBufferType` deliberately constructs the element and atomic-counter views of
    // an Append/Consume aggregate from only the semantic element and explicit data layout. Both
    // are canonical structured-buffer types; the required layout is checked below in either form.
    auto bufferType = as<IRHLSLStructuredBufferTypeBase>(type);
    if (bufferType &&
        (bufferType->getOp() == kIROp_HLSLStructuredBufferType ||
         bufferType->getOp() == kIROp_HLSLRWStructuredBufferType) &&
        (bufferType->getOperandCount() == 2 || bufferType->getOperandCount() == 3) &&
        _isNVVMSupportedResourceElementType(bufferType->getElementType(), activeTypes))
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

bool getNVVMSupportedRawBufferType(IRInst* type, NVVMRawBufferType& outType)
{
    HashSet<IRInst*> activeTypes;
    return _getNVVMSupportedRawBufferType(type, outType, activeTypes);
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
    HashSet<IRInst*> activeTypes;
    if (!pointerType || pointerType->getOp() != kIROp_PtrType ||
        pointerType->getOperandCount() != 4 || !arrayType || arrayType->getOperandCount() != 1 ||
        !_isNVVMSupportedResourceElementType(arrayType->getElementType(), activeTypes) ||
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

IRDescriptorHandleType* asNVVMSupportedDescriptorHandleType(
    IRInst* type,
    IRType** outResourceType)
{
    if (outResourceType)
        *outResourceType = nullptr;

    auto handleType = as<IRDescriptorHandleType>(type);
    IRType* resourceType = handleType ? handleType->getResourceType() : nullptr;
    NVVMRawBufferType rawBufferType;
    NVVMReadOnlyTextureType sampledTextureType;
    if (!handleType ||
        (!getNVVMSupportedRawBufferType(resourceType, rawBufferType) &&
         !getNVVMSupportedReadOnlyTextureType(resourceType, sampledTextureType) &&
         !asNVVMSupportedSamplerValueType(resourceType)))
    {
        return nullptr;
    }

    if (outResourceType)
        *outResourceType = resourceType;
    return handleType;
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
    if (!asNVVMSupportedAggregateStorageStructType(elementType) &&
        !asNVVMSupportedAggregateStorageArrayType(elementType))
        return nullptr;

    if (outElementType)
        *outElementType = elementType;
    return parameterGroupType;
}

static bool _hasNVVMParameterGroupStorageValueRepresentation(
    IRInst* type,
    HashSet<IRInst*>& activeTypes)
{
    if (isNVVMSupportedIntegerScalarType(type) || isNVVMBoolType(type) || isNVVMFloat16Type(type) ||
        isNVVMFloat32Type(type))
        return true;

    if (asNVVMSupported32BitNumericVectorType(type))
        return !asNVVMSupportedCompactParameterGroupVectorType(type);

    if (asNVVMSupportedCompactParameterGroupVectorType(type))
        return false;

    // Physical matrix wrappers deliberately delegate parameter-group storage lowering to ordinary
    // value lowering. Keep that producer-owned identity explicit instead of trying to infer
    // provider type equality from its children.
    if (asNVVMSupportedPhysicalArrayStructType(type))
        return true;

    NVVMRawBufferType rawBufferType;
    NVVMSurfaceType surfaceType;
    NVVMReadOnlyTextureType sampledTextureType;
    if (getNVVMSupportedRawBufferType(type, rawBufferType) ||
        getNVVMSupportedSurfaceType(type, surfaceType) ||
        getNVVMSupportedReadOnlyTextureType(type, sampledTextureType) ||
        asNVVMSupportedDescriptorHandleType(type) ||
        asNVVMSupportedSamplerValueType(type))
    {
        return true;
    }

    IRType* parameterGroupElementType = nullptr;
    if (asNVVMSupportedParameterGroupType(type, &parameterGroupElementType))
    {
        if (activeTypes.contains(type))
            return false;
        activeTypes.add(type);
        const bool result = _hasNVVMParameterGroupStorageValueRepresentation(
            parameterGroupElementType,
            activeTypes);
        activeTypes.remove(type);
        return result;
    }

    // A UserPointer leaf is intentionally global in parameter-group storage and generic as an
    // ordinary value. Whole-value loads need an explicit conversion before that family is legal.
    if (!type || asNVVMSupportedDeviceCopyableValuePointerType(type) || activeTypes.contains(type))
    {
        return false;
    }

    if (auto arrayType = as<IRArrayType>(type))
    {
        if (!asNVVMSupportedAggregateStorageArrayType(arrayType))
            return false;
        activeTypes.add(type);
        const bool result = _hasNVVMParameterGroupStorageValueRepresentation(
            arrayType->getElementType(),
            activeTypes);
        activeTypes.remove(type);
        return result;
    }

    auto structType = asNVVMSupportedAggregateStorageStructType(type);
    if (!structType)
        return false;

    activeTypes.add(type);
    bool hasField = false;
    for (auto field : structType->getFields())
    {
        if (!_hasNVVMParameterGroupStorageValueRepresentation(field->getFieldType(), activeTypes))
        {
            activeTypes.remove(type);
            return false;
        }
        hasField = true;
    }
    activeTypes.remove(type);
    return hasField;
}

bool hasNVVMParameterGroupStorageValueRepresentation(IRInst* type)
{
    if (!isNVVMSupportedAggregateStorageType(type))
        return false;
    HashSet<IRInst*> activeTypes;
    return _hasNVVMParameterGroupStorageValueRepresentation(type, activeTypes);
}

bool isNVVMSupportedConventionalGlobalFieldType(IRStructField* field)
{
    NVVMRawBufferType rawBufferType;
    NVVMSurfaceType surfaceType;
    NVVMReadOnlyTextureType sampledTextureType;
    SlangNVVMSurfaceStorageFormat storageFormat = SLANG_NVVM_SURFACE_STORAGE_NATIVE;
    IRType* type = field ? field->getFieldType() : nullptr;
    return isNVVMSupportedIntegerScalarType(type) || isNVVMFloat32Type(type) ||
           asNVVMSupportedResourceStructType(type) ||
           asNVVMSupportedDeviceCopyableValuePointerType(type) ||
           asNVVMSupportedDevicePhysicalStoragePointerType(type) ||
           asNVVMSupportedParameterGroupType(type) ||
           getNVVMSupportedRawBufferType(type, rawBufferType) ||
           getNVVMSupportedSurfaceField(field, surfaceType, storageFormat) ||
           getNVVMSupportedReadOnlyTextureType(type, sampledTextureType) ||
           asNVVMSupportedDescriptorHandleType(type) || asNVVMSupportedSamplerStorageType(type) ||
           asNVVMSupportedUnsizedSamplerArrayStorageType(type) ||
           asNVVMSupportedAggregateStorageArrayType(type);
}

IRPtrTypeBase* asNVVMSupportedRWStructuredBufferElementPointerType(IRInst* type)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    IRType* dataLayout = ptrType ? ptrType->getDataLayout() : nullptr;
    HashSet<IRInst*> activeTypes;
    // Address-space specialization retains StorageBuffer on physical aggregate elements while
    // ordinary scalar and vector elements keep Generic. Both are producer-side spellings for the
    // same CUDA global-memory pointer returned by RWStructuredBufferGetElementPtr.
    const bool hasResourceAddressSpace =
        ptrType && (ptrType->getAddressSpace() == AddressSpace::Generic ||
                    ptrType->getAddressSpace() == AddressSpace::StorageBuffer);
    if (!ptrType || ptrType->getOp() != kIROp_PtrType || ptrType->getOperandCount() != 4 ||
        !_isNVVMSupportedResourceElementType(ptrType->getValueType(), activeTypes) ||
        ptrType->getAccessQualifier() != AccessQualifier::ReadWrite ||
        !hasResourceAddressSpace || !dataLayout ||
        dataLayout->getOp() != kIROp_ScalarBufferLayoutType)
    {
        return nullptr;
    }
    return ptrType;
}

bool isNVVMSupportedParameterType(IRInst* type)
{
    NVVMRawBufferType rawBufferType;
    IRType* parameterGroupElementType = nullptr;
    return isNVVMSupportedIntegerScalarType(type) || isNVVMFloat32Type(type) ||
           asNVVMSupportedResourceStructType(type) ||
           (asNVVMSupportedParameterGroupType(type, &parameterGroupElementType) &&
            hasNVVMParameterGroupStorageValueRepresentation(parameterGroupElementType)) ||
           asNVVMSupportedDeviceNumericPointerType(type) ||
           asNVVMSupportedDeviceArrayPointerType(type) ||
           getNVVMSupportedRawBufferType(type, rawBufferType);
}

static bool _isNVVMSupportedStructuredBufferStorageType(IRInst* type, HashSet<IRInst*>& activeTypes)
{
    if (isNVVMSupportedNumericValueType(type) || isNVVMBoolType(type) ||
        asNVVMSupportedAtomicType(type) || asNVVMSupportedDescriptorHandleType(type))
        return true;

    if (auto vectorType = asNVVMSupportedValueVectorType(type))
        return isNVVMBoolType(vectorType->getElementType());

    if (!type || activeTypes.contains(type))
        return false;

    activeTypes.add(type);
    if (auto arrayType = as<IRArrayType>(type))
    {
        auto count = as<IRIntLit>(arrayType->getElementCount());
        const bool isSupported =
            arrayType->getOp() == kIROp_ArrayType &&
            (arrayType->getOperandCount() == 2 || arrayType->getOperandCount() == 3) && count &&
            count->getValue() > 0 && count->getValue() <= UINT32_MAX &&
            _isNVVMSupportedStructuredBufferStorageType(arrayType->getElementType(), activeTypes);
        activeTypes.remove(type);
        return isSupported;
    }

    auto structType = as<IRStructType>(type);
    if (!structType)
    {
        activeTypes.remove(type);
        return false;
    }

    bool hasField = false;
    for (auto field : structType->getFields())
    {
        if (!_isNVVMSupportedStructuredBufferStorageType(field->getFieldType(), activeTypes))
        {
            activeTypes.remove(type);
            return false;
        }
        hasField = true;
    }
    activeTypes.remove(type);
    return hasField;
}

bool isNVVMSupportedStructuredBufferStorageType(IRInst* type)
{
    HashSet<IRInst*> activeTypes;
    return _isNVVMSupportedStructuredBufferStorageType(type, activeTypes);
}

// Selects the provider pointee representation for one canonical structured-buffer element.
// Numeric/Boolean aggregates need the external CUDA storage algebra, while an established
// resource-containing element such as `MyImpl { Texture2D tex; }` already has an exact ordinary
// value representation. The resource-element classifier proves that the latter family is legal;
// its layout is checked separately before this role is consumed.
static NVVMTypeUse _getNVVMStructuredBufferElementTypeUse(IRType* type)
{
    return isNVVMSupportedStructuredBufferStorageType(type) ? NVVMTypeUse::StructuredBufferStorage
                                                            : NVVMTypeUse::Value;
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
    case NVVMTypeUse::HelperValue:
        construct = "helper value type";
        break;
    case NVVMTypeUse::Value:
        break;
    case NVVMTypeUse::Storage:
    case NVVMTypeUse::ParameterGroupStorage:
        construct = "aggregate storage type";
        break;
    case NVVMTypeUse::StructuredBufferStorage:
        construct = "structured-buffer storage type";
        break;
    }
    m_codeGenContext->getSink()->diagnose(
        Diagnostics::NvvmUnsupportedIr{.construct = String(construct)});
    return SLANG_E_NOT_IMPLEMENTED;
}

SlangResult NVVMTypeLoweringContext::_lowerArrayType(
    IRArrayType* type,
    NVVMTypeUse use,
    SlangNVVMTypeHandle& outType)
{
    outType = nullptr;
    const bool isAggregateStorage =
        use == NVVMTypeUse::Storage || use == NVVMTypeUse::ParameterGroupStorage;
    auto& typeMap = use == NVVMTypeUse::HelperValue               ? m_helperABIRepresentationMap
                    : use == NVVMTypeUse::StructuredBufferStorage ? m_structuredBufferStorageTypeMap
                    : isAggregateStorage                          ? m_aggregateStorageTypeMap
                                                                  : m_typeMap;
    if (auto mappedType = typeMap.tryGetValue(type))
    {
        outType = *mappedType;
        return SLANG_OK;
    }

    uint32_t elementCount = 0;
    IRArrayType* supportedType = nullptr;
    if (use == NVVMTypeUse::StructuredBufferStorage)
    {
        auto count = as<IRIntLit>(type->getElementCount());
        if (isNVVMSupportedStructuredBufferStorageType(type) && count)
        {
            supportedType = type;
            elementCount = uint32_t(count->getValue());
        }
    }
    else
    {
        supportedType = use == NVVMTypeUse::Storage || use == NVVMTypeUse::ParameterGroupStorage
                            ? asNVVMSupportedAggregateStorageArrayType(type, &elementCount)
                            : asNVVMSupportedHelperArrayType(type, &elementCount);
        if (!supportedType && use == NVVMTypeUse::Value)
            supportedType = asNVVMSupportedResourceArrayType(type, &elementCount);
    }
    SLANG_RELEASE_ASSERT(supportedType);
    SlangNVVMTypeHandle elementType = nullptr;
    const NVVMTypeUse elementUse =
        use == NVVMTypeUse::HelperValue               ? NVVMTypeUse::HelperValue
        : use == NVVMTypeUse::StructuredBufferStorage ? NVVMTypeUse::StructuredBufferStorage
        : use == NVVMTypeUse::ParameterGroupStorage   ? NVVMTypeUse::ParameterGroupStorage
        : use == NVVMTypeUse::Storage && isNVVMSupportedHelperValueType(type) &&
                !isNVVMSupportedCopyableValueType(type)
            ? NVVMTypeUse::HelperValue
        : use == NVVMTypeUse::Storage ? NVVMTypeUse::Storage
                                      : NVVMTypeUse::Value;
    SLANG_RETURN_ON_FAIL(lowerType(type->getElementType(), elementUse, elementType));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        use == NVVMTypeUse::StructuredBufferStorage ? "structured-buffer storage array type"
        : isAggregateStorage                        ? "fixed aggregate storage array type"
                                                    : "fixed copyable array type",
        m_builder.getArrayType(m_module, elementType, elementCount, outType)));
    typeMap[type] = outType;
    return SLANG_OK;
}

SlangResult NVVMTypeLoweringContext::_lowerStructType(
    IRStructType* type,
    NVVMTypeUse use,
    SlangNVVMTypeHandle& outType)
{
    outType = nullptr;
    const bool isAggregateStorage =
        use == NVVMTypeUse::Storage || use == NVVMTypeUse::ParameterGroupStorage;
    auto& typeMap = use == NVVMTypeUse::HelperValue               ? m_helperABIRepresentationMap
                    : use == NVVMTypeUse::StructuredBufferStorage ? m_structuredBufferStorageTypeMap
                    : isAggregateStorage                          ? m_aggregateStorageTypeMap
                                                                  : m_typeMap;
    if (auto mappedType = typeMap.tryGetValue(type))
    {
        outType = *mappedType;
        return SLANG_OK;
    }

    const NVVMTypeUse fieldUse =
        use == NVVMTypeUse::HelperValue               ? NVVMTypeUse::HelperValue
        : use == NVVMTypeUse::StructuredBufferStorage ? NVVMTypeUse::StructuredBufferStorage
        : use == NVVMTypeUse::ParameterGroupStorage   ? NVVMTypeUse::ParameterGroupStorage
        // Conventional global storage is producer-proven device memory even when the aggregate
        // also has a valid helper-value representation. Preserve that storage role recursively so
        // UserPointer fields remain AS1 until they cross an executable helper-value boundary.
        : use == NVVMTypeUse::Storage ? NVVMTypeUse::Storage
        : (asNVVMSupportedHelperStructType(type) && !isNVVMSupportedCopyableValueType(type))
            ? NVVMTypeUse::HelperValue
        : asNVVMSupportedHelperStructType(type) ||
                (use != NVVMTypeUse::Storage && asNVVMSupportedResourceStructType(type))
            ? NVVMTypeUse::Value
            : NVVMTypeUse::Storage;
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
    typeMap[type] = outType;
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
        SLANG_RETURN_ON_FAIL(lowerType(
            type.structuredElementType,
            _getNVVMStructuredBufferElementTypeUse(type.structuredElementType),
            loweredElementType));
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
    SLANG_RETURN_ON_FAIL(
        lowerType(elementType, NVVMTypeUse::ParameterGroupStorage, loweredElementType));

    const PointerTypeKey key = {
        elementType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        NVVMTypeUse::ParameterGroupStorage};
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
    SlangNVVMTypeHandle& outType,
    NVVMTypeUse pointeeUse,
    bool cacheCanonicalType)
{
    outType = nullptr;
    SlangNVVMTypeHandle loweredPointeeType = nullptr;
    if (auto arrayType = as<IRArrayType>(pointeeType))
    {
        SLANG_RETURN_ON_FAIL(_lowerArrayType(arrayType, pointeeUse, loweredPointeeType));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(lowerType(pointeeType, pointeeUse, loweredPointeeType));
    }

    // Consider a kernel that copies from `Ptr<int, Read, Device>` to
    // `Ptr<int, ReadWrite, Device>`. Those are distinct canonical Slang types because stores are
    // legal through only one of them, but LLVM represents both as the same `i32 addrspace(1)*`.
    // Cache that provider representation by exact pointee identity and address space, then record
    // the resulting handle separately for each canonical source type.
    const PointerTypeKey key = {pointeeType, addressSpace, pointeeUse};
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
    if (cacheCanonicalType)
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
    IRStructType* resourceStructType = asNVVMSupportedResourceStructType(type);
    IRStructType* physicalArrayStructType = asNVVMSupportedPhysicalArrayStructType(type);
    IRStructType* localResourceStructValueType = nullptr;
    IRPtrTypeBase* localResourceStructPointer =
        asNVVMSupportedLocalResourceStructPointerType(type, &localResourceStructValueType);
    IRType* localCopyablePointerValueType = nullptr;
    IRPtrTypeBase* localCopyablePointer =
        asNVVMSupportedLocalCopyableValuePointerType(type, &localCopyablePointerValueType);
    IRType* localHelperPointerValueType = nullptr;
    IRPtrTypeBase* localHelperPointer =
        asNVVMSupportedLocalHelperValuePointerType(type, &localHelperPointerValueType);
    IRType* helperReferenceValueType = nullptr;
    IRPtrTypeBase* helperReferencePointer =
        asNVVMSupportedHelperReferencePointerType(type, &helperReferenceValueType);
    IRStructType* physicalStorageReferenceValueType = nullptr;
    IRPtrTypeBase* physicalStorageReferencePointer =
        asNVVMSupportedPhysicalStorageReferencePointerType(
            type,
            &physicalStorageReferenceValueType);
    IRStructType* localPhysicalStorageValueType = nullptr;
    IRPtrTypeBase* localPhysicalStoragePointer =
        asNVVMSupportedLocalPhysicalStoragePointerType(type, &localPhysicalStorageValueType);
    IRType* sharedHelperPointerValueType = nullptr;
    IRPtrTypeBase* sharedHelperPointer =
        asNVVMSupportedSharedHelperPointerType(type, &sharedHelperPointerValueType);
    IRType* deviceCopyablePointerValueType = nullptr;
    IRPtrTypeBase* deviceCopyablePointer =
        asNVVMSupportedDeviceCopyableValuePointerType(type, &deviceCopyablePointerValueType);
    IRType* deviceHelperPointerValueType = nullptr;
    IRPtrTypeBase* deviceHelperPointer =
        asNVVMSupportedDeviceHelperValuePointerType(type, &deviceHelperPointerValueType);
    IRStructType* devicePhysicalStorageValueType = nullptr;
    IRPtrTypeBase* devicePhysicalStoragePointer =
        asNVVMSupportedDevicePhysicalStoragePointerType(type, &devicePhysicalStorageValueType);
    const bool isHelperValue = isNVVMSupportedHelperValueType(type);
    const bool isPointerBearingHelperValue =
        isHelperValue && !isNVVMSupportedCopyableValueType(type);
    IRPtrTypeBase* deviceNumericPointer = asNVVMSupportedDeviceNumericPointerType(type);
    IRArrayType* fixedCopyableArrayType = asNVVMSupportedCopyableArrayType(type);
    IRArrayType* fixedHelperArrayType = asNVVMSupportedHelperArrayType(type);
    IRArrayType* fixedResourceArrayType = asNVVMSupportedResourceArrayType(type);
    IRArrayType* aggregateStorageArrayType = asNVVMSupportedAggregateStorageArrayType(type);
    IRVectorType* compactParameterGroupVectorType =
        asNVVMSupportedCompactParameterGroupVectorType(type);
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
    IRType* descriptorResourceType = nullptr;
    IRDescriptorHandleType* descriptorHandle =
        asNVVMSupportedDescriptorHandleType(type, &descriptorResourceType);
    IRUnsizedArrayType* unsizedSamplerArrayStorage =
        asNVVMSupportedUnsizedSamplerArrayStorageType(type);
    IRPtrTypeBase* resourceElementPointer =
        asNVVMSupportedRWStructuredBufferElementPointerType(type);
    IRPtrTypeBase* sharedElementPointer = asNVVMSupportedSharedElementPointerType(type);
    IRType* atomicValueType = nullptr;
    IRAtomicType* atomicType = asNVVMSupportedAtomicType(type, &atomicValueType);
    const bool isStructuredBufferStorage = isNVVMSupportedStructuredBufferStorageType(type);

    // Preflight admits types by their producer/consumer role. Check that role before looking in the
    // cache so a handle created for a valid value cannot make the same type valid in a forbidden
    // helper signature.
    const bool isLegal =
        (use == NVVMTypeUse::EntryPointResult && isVoid) ||
        (use == NVVMTypeUse::HelperResult &&
         (isVoid || isHelperValue || resourceStructType || localCopyablePointer ||
          localHelperPointer || isRawBuffer || isSampledTexture)) ||
        (use == NVVMTypeUse::EntryPointParameter &&
         (isInteger || isFloat32 || resourceStructType || deviceNumericPointer ||
          deviceCopyablePointer || devicePhysicalStoragePointer || deviceArrayPointer ||
          isRawBuffer ||
          (parameterGroup &&
           hasNVVMParameterGroupStorageValueRepresentation(parameterGroupElementType)))) ||
        (use == NVVMTypeUse::HelperParameter &&
         (isHelperValue || resourceStructType || localResourceStructPointer ||
          localCopyablePointer || localHelperPointer || helperReferencePointer ||
          physicalStorageReferencePointer || localPhysicalStoragePointer || sharedHelperPointer ||
          isRawBuffer || isSurface || isSampledTexture || samplerValue)) ||
        (use == NVVMTypeUse::HelperValue && isHelperValue) ||
        (use == NVVMTypeUse::Value &&
         (isHelperValue || resourceStructType || fixedResourceArrayType ||
          physicalArrayStructType || deviceNumericPointer || devicePhysicalStoragePointer ||
          deviceArrayPointer || isRawBuffer || isBufferDataPointer || parameterGroup || isSurface ||
          isSampledTexture || samplerValue || resourceElementPointer || sharedElementPointer ||
          sharedHelperPointer || atomicType)) ||
        (use == NVVMTypeUse::Storage &&
         (isInteger || isFloat32 || isNVVMFloat16Type(type) ||
          asNVVMSupported32BitNumericVectorType(type) || compactParameterGroupVectorType ||
          structType || aggregateStorageArrayType || deviceCopyablePointer ||
          devicePhysicalStoragePointer || isRawBuffer || parameterGroup || isSurface ||
          isSampledTexture || samplerStorage || unsizedSamplerArrayStorage || atomicType ||
          descriptorHandle)) ||
        (use == NVVMTypeUse::ParameterGroupStorage && isNVVMSupportedAggregateStorageType(type)) ||
        (use == NVVMTypeUse::StructuredBufferStorage && isStructuredBufferStorage);
    if (!isLegal)
        return _reportUnsupportedType(use);

    // CUDA's canonical layout producer defines `DescriptorHandle<T>` to have exactly the layout
    // of `T` on bindless targets. The selected handle families also carry the same SSA value as
    // their resource: the two descriptor conversion instructions below are therefore identities.
    // Preserve that producer-owned representation instead of inventing an integer handle ABI.
    if (descriptorHandle)
    {
        if (auto mappedType = m_typeMap.tryGetValue(type))
        {
            outType = *mappedType;
            return SLANG_OK;
        }
        SLANG_RETURN_ON_FAIL(lowerType(descriptorResourceType, NVVMTypeUse::Value, outType));
        m_typeMap[type] = outType;
        return SLANG_OK;
    }

    // `Atomic<T>` is a storage semantic wrapper. CUDA and LLVM both represent its physical
    // payload as `T`; only atomic operations may access pointers to that storage.
    if (atomicType)
    {
        if (auto mappedType = m_typeMap.tryGetValue(type))
        {
            outType = *mappedType;
            return SLANG_OK;
        }
        SLANG_RETURN_ON_FAIL(lowerType(atomicValueType, NVVMTypeUse::Value, outType));
        m_typeMap[type] = outType;
        return SLANG_OK;
    }

    // A finite pointer-bearing helper aggregate has one executable representation whose
    // UserPointer leaves are LLVM generic pointers. A helper can receive either a kernel device
    // pointer or `__getAddress` of local storage, so global-only leaves would be incorrect. Keep
    // this representation separate from launch and conventional-global storage, where the
    // canonical producer proves global-memory provenance.
    if ((use == NVVMTypeUse::HelperParameter || use == NVVMTypeUse::HelperResult ||
         use == NVVMTypeUse::Value) &&
        isPointerBearingHelperValue && !deviceCopyablePointer)
    {
        return lowerType(type, NVVMTypeUse::HelperValue, outType);
    }

    if (use == NVVMTypeUse::HelperValue && isNVVMSupportedCopyableValueType(type))
        return lowerType(type, NVVMTypeUse::Value, outType);

    // Matrix legalization marks its one-field wrapper as PhysicalType. Its canonical value is
    // already the external storage spelling, so use the same provider handle as every typed
    // structured-buffer pointer instead of creating a parallel ordinary-value struct.
    if (use == NVVMTypeUse::Value && physicalArrayStructType)
        return lowerType(type, NVVMTypeUse::StructuredBufferStorage, outType);

    if (use == NVVMTypeUse::EntryPointParameter && deviceCopyablePointer)
    {
        if (auto mappedType = m_entryParameterRepresentationMap.tryGetValue(type))
        {
            outType = *mappedType;
            return SLANG_OK;
        }
        SLANG_RETURN_ON_FAIL(_lowerPointerType(
            type,
            deviceCopyablePointerValueType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            outType,
            NVVMTypeUse::Value,
            false));
        m_entryParameterRepresentationMap[type] = outType;
        return SLANG_OK;
    }

    if (use == NVVMTypeUse::Storage && deviceCopyablePointer)
    {
        if (auto mappedType = m_aggregateStorageTypeMap.tryGetValue(type))
        {
            outType = *mappedType;
            return SLANG_OK;
        }
        SLANG_RETURN_ON_FAIL(_lowerPointerType(
            type,
            deviceCopyablePointerValueType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            outType,
            NVVMTypeUse::Value,
            false));
        m_aggregateStorageTypeMap[type] = outType;
        return SLANG_OK;
    }

    if ((use == NVVMTypeUse::EntryPointParameter || use == NVVMTypeUse::Storage ||
         use == NVVMTypeUse::Value) &&
        devicePhysicalStoragePointer)
    {
        return _lowerPointerType(
            type,
            devicePhysicalStorageValueType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            outType,
            NVVMTypeUse::ParameterGroupStorage,
            false);
    }

    if ((use == NVVMTypeUse::HelperParameter || use == NVVMTypeUse::HelperResult ||
         use == NVVMTypeUse::HelperValue || use == NVVMTypeUse::Value) &&
        deviceHelperPointer)
    {
        return _lowerPointerType(
            type,
            deviceHelperPointerValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType,
            deviceCopyablePointer ? NVVMTypeUse::Value : NVVMTypeUse::HelperValue);
    }

    // Keep canonical Half values in LLVM's `half` type inside helper bodies, but transport a Half
    // helper parameter or result as i16. libNVVM's O3 NVPTX lowering can otherwise omit the caller
    // parameter store for a direct `half` argument, leaving the callee's value uninitialized.
    if ((use == NVVMTypeUse::HelperParameter || use == NVVMTypeUse::HelperResult) &&
        isNVVMFloat16Type(type))
    {
        if (auto mappedType = m_helperABIRepresentationMap.tryGetValue(type))
        {
            outType = *mappedType;
            return SLANG_OK;
        }
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            "physical Half helper ABI type",
            m_builder.getIntegerType(m_module, 16, outType)));
        m_helperABIRepresentationMap[type] = outType;
        return SLANG_OK;
    }

    // NVPTX represents an aggregate kernel parameter as a generic pointer carrying `byval`, while
    // the same canonical Slang struct remains a first-class LLVM struct in ordinary value roles.
    // Keep this physical ABI representation separate from the canonical value-type cache.
    if (use == NVVMTypeUse::EntryPointParameter && resourceStructType)
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

    if ((use == NVVMTypeUse::HelperParameter || use == NVVMTypeUse::HelperResult) &&
        localCopyablePointer)
    {
        return _lowerPointerType(
            type,
            localCopyablePointerValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType);
    }

    if ((use == NVVMTypeUse::HelperParameter || use == NVVMTypeUse::HelperResult) &&
        localHelperPointer)
    {
        if (auto mappedType = m_helperABIRepresentationMap.tryGetValue(type))
        {
            outType = *mappedType;
            return SLANG_OK;
        }
        SLANG_RETURN_ON_FAIL(_lowerPointerType(
            type,
            localHelperPointerValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType,
            NVVMTypeUse::HelperValue,
            false));
        m_helperABIRepresentationMap[type] = outType;
        return SLANG_OK;
    }

    if (use == NVVMTypeUse::HelperParameter && helperReferencePointer)
    {
        return _lowerPointerType(
            type,
            helperReferenceValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType,
            NVVMTypeUse::Value,
            false);
    }

    if (use == NVVMTypeUse::HelperParameter && physicalStorageReferencePointer)
    {
        return _lowerPointerType(
            type,
            physicalStorageReferenceValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType,
            NVVMTypeUse::ParameterGroupStorage,
            false);
    }

    if (use == NVVMTypeUse::HelperParameter && localPhysicalStoragePointer)
    {
        return _lowerPointerType(
            type,
            localPhysicalStorageValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType,
            NVVMTypeUse::ParameterGroupStorage,
            false);
    }

    if ((use == NVVMTypeUse::HelperParameter || use == NVVMTypeUse::Value) && sharedHelperPointer)
    {
        return _lowerPointerType(
            type,
            sharedHelperPointerValueType,
            SLANG_NVVM_ADDRESS_SPACE_SHARED,
            outType,
            NVVMTypeUse::Value,
            false);
    }

    if (use == NVVMTypeUse::HelperParameter && localResourceStructPointer)
    {
        return _lowerPointerType(
            type,
            localResourceStructValueType,
            SLANG_NVVM_ADDRESS_SPACE_GENERIC,
            outType);
    }

    if (use == NVVMTypeUse::ParameterGroupStorage &&
        (isInteger || isFloat32 || isNVVMFloat16Type(type) ||
         (asNVVMSupported32BitNumericVectorType(type) && !compactParameterGroupVectorType) ||
         asNVVMSupportedScalarStructType(type) || asNVVMSupportedPhysicalArrayStructType(type)))
    {
        return lowerType(type, NVVMTypeUse::Value, outType);
    }

    const bool isAggregateStorage =
        use == NVVMTypeUse::Storage || use == NVVMTypeUse::ParameterGroupStorage;
    auto& typeMap = use == NVVMTypeUse::HelperValue               ? m_helperABIRepresentationMap
                    : use == NVVMTypeUse::StructuredBufferStorage ? m_structuredBufferStorageTypeMap
                    : isAggregateStorage                          ? m_aggregateStorageTypeMap
                                                                  : m_typeMap;
    if (auto mappedType = typeMap.tryGetValue(type))
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
            isInteger                                     ? "selected integer type"
            : use == NVVMTypeUse::StructuredBufferStorage ? "structured-buffer Boolean storage type"
                                                          : "Boolean type",
            m_builder.getIntegerType(
                m_module,
                isInteger                                     ? integerBitWidth
                : use == NVVMTypeUse::StructuredBufferStorage ? 8u
                                                              : 1u,
                outType)));
    }
    else if (isFloatingPoint)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            floatingPointBitWidth == 16   ? "float16 type"
            : floatingPointBitWidth == 32 ? "float32 type"
                                          : "float64 type",
            m_builder.getFloatingPointType(m_module, floatingPointBitWidth, outType)));
    }
    else if (valueVectorType)
    {
        SlangNVVMTypeHandle elementType = nullptr;
        SLANG_RETURN_ON_FAIL(lowerType(
            valueVectorType->getElementType(),
            use == NVVMTypeUse::StructuredBufferStorage ? NVVMTypeUse::StructuredBufferStorage
                                                        : NVVMTypeUse::Value,
            elementType));
        const bool useStructuredBufferArray =
            use == NVVMTypeUse::StructuredBufferStorage && valueVectorElementCount == 3;
        const bool useCompactHalfChunks =
            compactParameterGroupVectorType &&
            isNVVMFloat16Type(compactParameterGroupVectorType->getElementType()) &&
            (use == NVVMTypeUse::Storage || use == NVVMTypeUse::ParameterGroupStorage);
        SlangNVVMTypeHandle compactHalfChunkType = nullptr;
        if (useCompactHalfChunks)
        {
            SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
                "compact half-vector storage chunk type",
                m_builder.getVectorType(m_module, elementType, 2, compactHalfChunkType)));
        }
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            useStructuredBufferArray ? "structured-buffer vector storage type"
            : compactParameterGroupVectorType &&
                    (use == NVVMTypeUse::Storage || use == NVVMTypeUse::ParameterGroupStorage)
                ? "compact aggregate vector storage type"
                : "selected value vector type",
            useCompactHalfChunks
                ? m_builder.getArrayType(m_module, compactHalfChunkType, 2, outType)
            : useStructuredBufferArray
                ? m_builder.getArrayType(m_module, elementType, valueVectorElementCount, outType)
            : compactParameterGroupVectorType &&
                    (use == NVVMTypeUse::Storage || use == NVVMTypeUse::ParameterGroupStorage)
                ? m_builder.getArrayType(m_module, elementType, valueVectorElementCount, outType)
                : m_builder
                      .getVectorType(m_module, elementType, valueVectorElementCount, outType)));
    }
    else if (
        fixedCopyableArrayType || fixedHelperArrayType || fixedResourceArrayType ||
        (use == NVVMTypeUse::StructuredBufferStorage && isStructuredBufferStorage &&
         as<IRArrayType>(type)) ||
        ((use == NVVMTypeUse::Storage || use == NVVMTypeUse::ParameterGroupStorage) &&
         aggregateStorageArrayType))
    {
        return _lowerArrayType(cast<IRArrayType>(type), use, outType);
    }
    else if (structType)
    {
        return _lowerStructType(structType, use, outType);
    }
    else if (deviceNumericPointer)
    {
        return _lowerPointerType(
            type,
            deviceNumericPointer->getValueType(),
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            outType);
    }
    else if (deviceCopyablePointer)
    {
        return _lowerPointerType(
            type,
            deviceCopyablePointerValueType,
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
            outType,
            _getNVVMStructuredBufferElementTypeUse(bufferDataPointerType.elementType));
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
            outType,
            _getNVVMStructuredBufferElementTypeUse(resourceElementPointer->getValueType()));
    }

    typeMap[type] = outType;
    return SLANG_OK;
}

} // namespace Slang
