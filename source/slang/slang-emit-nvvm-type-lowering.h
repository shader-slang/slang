#pragma once

#include "compiler-core/slang-nvvm-ir-builder.h"
#include "core/slang-dictionary.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct CodeGenContext;

/// Returns whether `type` is the canonical signed 32-bit integer accepted by direct NVVM.
bool isNVVMSignedI32Type(IRInst* type);

/// Returns whether `type` is the canonical Boolean result produced by an accepted comparison.
bool isNVVMBoolType(IRInst* type);

/// Returns an accepted nonempty fixed i32 array and optionally its exact element count.
IRArrayType* asNVVMSupportedI32ArrayType(IRInst* type, uint32_t* outElementCount = nullptr);

/// Returns an accepted CUDA device pointer to i32, including its source access qualifier.
IRPtrTypeBase* asNVVMSupportedDevicePointerType(IRInst* type);

/// Returns an accepted CUDA device pointer to a fixed i32 array.
IRPtrTypeBase* asNVVMSupportedDeviceArrayPointerType(
    IRInst* type,
    IRArrayType** outArrayType = nullptr,
    uint32_t* outElementCount = nullptr);

/// Returns the exact raw CUDA `RWStructuredBuffer<int, DefaultLayout>` launch-value type.
IRHLSLStructuredBufferTypeBase* asNVVMSupportedRawRWStructuredBufferI32Type(IRInst* type);

/// Returns the canonical pointer produced by accepted structured-buffer element addressing.
IRPtrTypeBase* asNVVMSupportedRWStructuredBufferI32ElementPointerType(IRInst* type);

/// Returns whether `type` has an accepted direct CUDA launch-parameter representation.
bool isNVVMSupportedParameterType(IRInst* type);

/// Identifies the producer/consumer contract under which a canonical linked-IR type is lowered.
enum class NVVMTypeUse
{
    EntryPointResult,
    HelperResult,
    EntryPointParameter,
    HelperParameter,
    Value,
};

/// Maps canonical linked-IR types to module-owned provider handles and caches each representation.
class NVVMTypeLoweringContext
{
public:
    NVVMTypeLoweringContext(
        CodeGenContext* codeGenContext,
        const NVVMIRBuilder& builder,
        SlangNVVMModuleHandle_1 module)
        : m_codeGenContext(codeGenContext), m_builder(builder), m_module(module)
    {
    }

    SlangResult lowerType(IRType* type, NVVMTypeUse use, SlangNVVMTypeHandle_1& outType);

private:
    struct PointerTypeKey
    {
        IRType* pointeeType = nullptr;
        SlangNVVMAddressSpace_2 addressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;

        HashCode getHashCode() const
        {
            return combineHash(
                Slang::getHashCode(pointeeType),
                Slang::getHashCode(uint32_t(addressSpace)));
        }
        bool operator==(const PointerTypeKey& other) const
        {
            return pointeeType == other.pointeeType && addressSpace == other.addressSpace;
        }
    };

    SlangResult _lowerArrayType(IRArrayType* type, SlangNVVMTypeHandle_1& outType);
    SlangResult _lowerPointerType(
        IRType* canonicalType,
        IRType* pointeeType,
        SlangNVVMAddressSpace_2 addressSpace,
        SlangNVVMTypeHandle_1& outType);
    SlangResult _reportUnsupportedType(NVVMTypeUse use) const;
    SlangResult _requireBuilderOperation(const char* operation, SlangResult result) const;

    CodeGenContext* m_codeGenContext = nullptr;
    const NVVMIRBuilder& m_builder;
    SlangNVVMModuleHandle_1 m_module = nullptr;
    Dictionary<IRType*, SlangNVVMTypeHandle_1> m_typeMap;
    Dictionary<PointerTypeKey, SlangNVVMTypeHandle_1> m_pointerRepresentationMap;
};

} // namespace Slang
