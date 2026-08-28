#pragma once

#include "compiler-core/slang-nvvm-ir-builder.h"
#include "core/slang-dictionary.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct CodeGenContext;

/// Returns whether `type` is the canonical signed 32-bit integer accepted by direct NVVM.
bool isNVVMSignedI32Type(IRInst* type);

/// Returns whether `type` is the canonical unsigned 32-bit integer accepted by direct NVVM.
bool isNVVMUnsignedI32Type(IRInst* type);

/// Returns whether `type` is either canonical 32-bit integer representation.
bool isNVVMInteger32Type(IRInst* type);

/// Returns whether `type` is one selected 8/16/32/64-bit integer scalar.
bool isNVVMSupportedIntegerScalarType(
    IRInst* type,
    uint32_t* outBitWidth = nullptr,
    bool* outIsSigned = nullptr);

/// Returns whether `type` is the canonical IEEE 32-bit float accepted by direct NVVM.
bool isNVVMFloat32Type(IRInst* type);

/// Returns whether `type` is the canonical Boolean result produced by an accepted comparison.
bool isNVVMBoolType(IRInst* type);

/// Returns an exact two- through four-lane 32-bit Int, UInt, or Float vector.
IRVectorType* asNVVMSupported32BitNumericVectorType(
    IRInst* type,
    uint32_t* outElementCount = nullptr);

/// Returns an exact two- through four-lane signed/unsigned 32-bit integer vector.
IRVectorType* asNVVMSupportedI32VectorType(
    IRInst* type,
    bool* outIsSigned = nullptr,
    uint32_t* outElementCount = nullptr);

/// Returns whether `type` is a selected scalar or established fixed numeric vector.
bool isNVVMSupportedNumericValueType(IRInst* type);

/// Returns whether `type` is an exact selected byte-address scalar or vector value.
bool isNVVMSupportedByteAddressValueType(IRInst* type);

/// Returns an exact nonempty struct whose fields are all selected scalar values.
IRStructType* asNVVMSupportedScalarStructType(IRInst* type);

/// Returns the natural byte alignment of one selected numeric value, or zero when unsupported.
uint32_t getNVVMNumericValueAlignment(IRInst* type);

/// Returns an accepted nonempty fixed i32 array and optionally its exact element count.
IRArrayType* asNVVMSupportedI32ArrayType(IRInst* type, uint32_t* outElementCount = nullptr);

/// Returns an accepted CUDA device pointer to signed or unsigned i32.
IRPtrTypeBase* asNVVMSupportedDevicePointerType(IRInst* type);

/// Returns an accepted CUDA device pointer to float32, including its source access qualifier.
IRPtrTypeBase* asNVVMSupportedDeviceFloat32PointerType(IRInst* type);

/// Returns an accepted CUDA device pointer to any established scalar value type.
IRPtrTypeBase* asNVVMSupportedDeviceScalarPointerType(IRInst* type);

/// Returns an accepted CUDA device pointer to a selected scalar or signed-i32x2 value.
IRPtrTypeBase* asNVVMSupportedDeviceNumericPointerType(IRInst* type);

/// Returns an accepted CUDA device pointer to a fixed i32 array.
IRPtrTypeBase* asNVVMSupportedDeviceArrayPointerType(
    IRInst* type,
    IRArrayType** outArrayType = nullptr,
    uint32_t* outElementCount = nullptr);

/// Returns the exact canonical uninitialized `groupshared` fixed-i32-array global.
IRGlobalVar* asNVVMSupportedSharedI32ArrayGlobal(
    IRInst* inst,
    IRArrayType** outArrayType = nullptr,
    uint32_t* outElementCount = nullptr);

/// Returns the canonical shared-address-space pointer produced for one i32 array element.
IRPtrTypeBase* asNVVMSupportedSharedI32ElementPointerType(IRInst* type);

/// Describes which operations a canonical raw buffer view permits.
enum class NVVMBufferAccess
{
    ReadOnly,
    ReadWrite,
};

/// Identifies which canonical source family owns a raw buffer view.
enum class NVVMRawBufferKind
{
    Structured,
    ByteAddress,
};

/// Describes one exact raw CUDA buffer view and its physical scalar element.
struct NVVMRawBufferType
{
    IRType* canonicalType = nullptr;
    IRType* structuredElementType = nullptr;
    NVVMRawBufferKind kind = NVVMRawBufferKind::Structured;
    NVVMBufferAccess access = NVVMBufferAccess::ReadOnly;
};

/// Resolves an exact structured or byte-address raw CUDA buffer view.
bool getNVVMSupportedRawBufferType(IRInst* type, NVVMRawBufferType& outType);

/// Returns whether `elementType` is the physical scalar selected by `bufferType`.
bool isNVVMRawBufferElementType(const NVVMRawBufferType& bufferType, IRType* elementType);

/// Describes the exact pointer-to-unsized-array spelling produced for raw buffer data.
struct NVVMBufferDataPointerType
{
    IRPtrTypeBase* pointerType = nullptr;
    IRUnsizedArrayType* arrayType = nullptr;
    IRType* elementType = nullptr;
};

/// Resolves an exact selected-scalar raw-buffer data pointer.
bool getNVVMSupportedBufferDataPointerType(IRInst* type, NVVMBufferDataPointerType& outType);

/// Returns an accepted storage-only CUDA sampler placeholder.
IRSamplerStateTypeBase* asNVVMSupportedSamplerStorageType(IRInst* type);

/// Returns an accepted storage-only unsized CUDA sampler array.
IRUnsizedArrayType* asNVVMSupportedUnsizedSamplerArrayStorageType(IRInst* type);

/// Returns an exact parameter block or constant buffer with a nonempty scalar-only element struct.
IRParameterGroupType* asNVVMSupportedScalarParameterGroupType(
    IRInst* type,
    IRStructType** outElementType = nullptr);

/// Returns whether `type` is one field admitted in the conventional CUDA parameter block.
bool isNVVMSupportedConventionalGlobalFieldType(IRInst* type);

/// Returns the canonical pointer produced by selected-scalar structured-buffer element addressing.
IRPtrTypeBase* asNVVMSupportedRWStructuredBufferElementPointerType(IRInst* type);

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
    Storage,
};

/// Maps canonical linked-IR types to module-owned provider handles and caches each representation.
class NVVMTypeLoweringContext
{
public:
    NVVMTypeLoweringContext(
        CodeGenContext* codeGenContext,
        const NVVMIRBuilder& builder,
        SlangNVVMModuleHandle module)
        : m_codeGenContext(codeGenContext), m_builder(builder), m_module(module)
    {
    }

    SlangResult lowerType(IRType* type, NVVMTypeUse use, SlangNVVMTypeHandle& outType);

private:
    struct PointerTypeKey
    {
        IRType* pointeeType = nullptr;
        SlangNVVMAddressSpace addressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;

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

    SlangResult _lowerArrayType(IRArrayType* type, SlangNVVMTypeHandle& outType);
    SlangResult _lowerStructType(IRStructType* type, SlangNVVMTypeHandle& outType);
    SlangResult _lowerRawBufferType(const NVVMRawBufferType& type, SlangNVVMTypeHandle& outType);
    SlangResult _lowerScalarParameterGroupType(
        IRParameterGroupType* type,
        IRStructType* elementType,
        SlangNVVMTypeHandle& outType);
    SlangResult _lowerUnsizedSamplerArrayStorageType(
        IRUnsizedArrayType* type,
        SlangNVVMTypeHandle& outType);
    SlangResult _lowerPointerType(
        IRType* canonicalType,
        IRType* pointeeType,
        SlangNVVMAddressSpace addressSpace,
        SlangNVVMTypeHandle& outType);
    SlangResult _reportUnsupportedType(NVVMTypeUse use) const;
    SlangResult _requireBuilderOperation(const char* operation, SlangResult result) const;

    CodeGenContext* m_codeGenContext = nullptr;
    const NVVMIRBuilder& m_builder;
    SlangNVVMModuleHandle m_module = nullptr;
    Dictionary<IRType*, SlangNVVMTypeHandle> m_typeMap;
    Dictionary<IRType*, SlangNVVMTypeHandle> m_entryParameterRepresentationMap;
    Dictionary<PointerTypeKey, SlangNVVMTypeHandle> m_pointerRepresentationMap;
};

} // namespace Slang
