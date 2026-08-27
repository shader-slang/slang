#include "slang-emit-nvvm-type-lowering.h"

#include "slang-code-gen.h"
#include "slang-diagnostics.h"

namespace Slang
{

bool isNVVMSignedI32Type(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Int;
}

bool isNVVMFloat32Type(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Float;
}

bool isNVVMBoolType(IRInst* type)
{
    auto basicType = as<IRBasicType>(type);
    return basicType && basicType->getBaseType() == BaseType::Bool;
}

IRArrayType* asNVVMSupportedI32ArrayType(IRInst* type, uint32_t* outElementCount)
{
    if (outElementCount)
        *outElementCount = 0;

    auto arrayType = as<IRArrayType>(type);
    if (!arrayType || arrayType->getOp() != kIROp_ArrayType || arrayType->getOperandCount() != 2 ||
        !isNVVMSignedI32Type(arrayType->getElementType()))
    {
        return nullptr;
    }

    auto elementCount = as<IRIntLit>(arrayType->getElementCount());
    if (!elementCount || elementCount->getValue() <= 0 || elementCount->getValue() > UINT32_MAX)
        return nullptr;

    if (outElementCount)
        *outElementCount = uint32_t(elementCount->getValue());
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
    return _asNVVMSupportedDevicePointerType(type, BaseType::Int);
}

IRPtrTypeBase* asNVVMSupportedDeviceFloat32PointerType(IRInst* type)
{
    return _asNVVMSupportedDevicePointerType(type, BaseType::Float);
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

IRHLSLStructuredBufferTypeBase* asNVVMSupportedRawRWStructuredBufferI32Type(IRInst* type)
{
    auto bufferType = as<IRHLSLStructuredBufferTypeBase>(type);
    if (!bufferType || bufferType->getOp() != kIROp_HLSLRWStructuredBufferType ||
        bufferType->getOperandCount() != 3 || !isNVVMSignedI32Type(bufferType->getElementType()))
    {
        return nullptr;
    }

    IRType* dataLayout = bufferType->getDataLayout();
    return dataLayout && dataLayout->getOp() == kIROp_DefaultBufferLayoutType ? bufferType
                                                                              : nullptr;
}

IRPtrTypeBase* asNVVMSupportedRWStructuredBufferI32ElementPointerType(IRInst* type)
{
    auto ptrType = as<IRPtrTypeBase>(type);
    IRType* dataLayout = ptrType ? ptrType->getDataLayout() : nullptr;
    if (!ptrType || ptrType->getOp() != kIROp_PtrType || ptrType->getOperandCount() != 4 ||
        !isNVVMSignedI32Type(ptrType->getValueType()) ||
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
    return isNVVMSignedI32Type(type) || isNVVMFloat32Type(type) ||
           asNVVMSupportedDevicePointerType(type) ||
           asNVVMSupportedDeviceFloat32PointerType(type) ||
           asNVVMSupportedDeviceArrayPointerType(type) ||
           asNVVMSupportedRawRWStructuredBufferI32Type(type);
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
    }
    m_codeGenContext->getSink()->diagnose(
        Diagnostics::NvvmUnsupportedIr{.construct = String(construct)});
    return SLANG_E_NOT_IMPLEMENTED;
}

SlangResult NVVMTypeLoweringContext::_lowerArrayType(
    IRArrayType* type,
    SlangNVVMTypeHandle_1& outType)
{
    outType = nullptr;
    if (auto mappedType = m_typeMap.tryGetValue(type))
    {
        outType = *mappedType;
        return SLANG_OK;
    }

    uint32_t elementCount = 0;
    SLANG_RELEASE_ASSERT(asNVVMSupportedI32ArrayType(type, &elementCount));
    SlangNVVMTypeHandle_1 elementType = nullptr;
    SLANG_RETURN_ON_FAIL(lowerType(type->getElementType(), NVVMTypeUse::Value, elementType));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        "fixed i32 array type",
        m_builder.getArrayType(m_module, elementType, elementCount, outType)));
    m_typeMap[type] = outType;
    return SLANG_OK;
}

SlangResult NVVMTypeLoweringContext::_lowerPointerType(
    IRType* canonicalType,
    IRType* pointeeType,
    SlangNVVMAddressSpace_2 addressSpace,
    SlangNVVMTypeHandle_1& outType)
{
    outType = nullptr;
    SlangNVVMTypeHandle_1 loweredPointeeType = nullptr;
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
                                                    : "device i32 pointer type",
            m_builder.getPointerType(m_module, loweredPointeeType, addressSpace, outType)));
        m_pointerRepresentationMap[key] = outType;
    }
    m_typeMap[canonicalType] = outType;
    return SLANG_OK;
}

SlangResult NVVMTypeLoweringContext::lowerType(
    IRType* type,
    NVVMTypeUse use,
    SlangNVVMTypeHandle_1& outType)
{
    outType = nullptr;

    const bool isVoid = as<IRVoidType>(type) != nullptr;
    const bool isI32 = isNVVMSignedI32Type(type);
    const bool isFloat32 = isNVVMFloat32Type(type);
    const bool isBool = isNVVMBoolType(type);
    IRPtrTypeBase* devicePointer = asNVVMSupportedDevicePointerType(type);
    IRPtrTypeBase* deviceFloat32Pointer = asNVVMSupportedDeviceFloat32PointerType(type);
    IRArrayType* arrayType = nullptr;
    IRPtrTypeBase* deviceArrayPointer = asNVVMSupportedDeviceArrayPointerType(type, &arrayType);
    IRHLSLStructuredBufferTypeBase* rawResource = asNVVMSupportedRawRWStructuredBufferI32Type(type);
    IRPtrTypeBase* resourceElementPointer =
        asNVVMSupportedRWStructuredBufferI32ElementPointerType(type);

    // Preflight admits types by their producer/consumer role. Check that role before looking in the
    // cache so a handle created for a valid value cannot make the same type valid in a forbidden
    // helper signature.
    const bool isLegal = (use == NVVMTypeUse::EntryPointResult && isVoid) ||
                         (use == NVVMTypeUse::HelperResult && (isI32 || isFloat32)) ||
                         (use == NVVMTypeUse::EntryPointParameter &&
                          (isI32 || isFloat32 || devicePointer || deviceFloat32Pointer ||
                           deviceArrayPointer || rawResource)) ||
                         (use == NVVMTypeUse::HelperParameter && (isI32 || isFloat32)) ||
                         (use == NVVMTypeUse::Value &&
                          (isI32 || isFloat32 || isBool || devicePointer || deviceFloat32Pointer ||
                           deviceArrayPointer || rawResource || resourceElementPointer));
    if (!isLegal)
        return _reportUnsupportedType(use);

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
    else if (isI32 || isBool)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            isI32 ? "signed i32 type" : "Boolean type",
            m_builder.getIntegerType(m_module, isI32 ? 32u : 1u, outType)));
    }
    else if (isFloat32)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            "float32 type",
            m_builder.getFloatingPointType(m_module, 32u, outType)));
    }
    else if (devicePointer)
    {
        return _lowerPointerType(
            type,
            devicePointer->getValueType(),
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            outType);
    }
    else if (deviceFloat32Pointer)
    {
        return _lowerPointerType(
            type,
            deviceFloat32Pointer->getValueType(),
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            outType);
    }
    else if (deviceArrayPointer)
    {
        return _lowerPointerType(type, arrayType, SLANG_NVVM_ADDRESS_SPACE_GLOBAL, outType);
    }
    else if (rawResource)
    {
        SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
            "raw RWStructuredBuffer signed i32 type",
            m_builder.getRawRWStructuredBufferI32Type(m_module, outType)));
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
