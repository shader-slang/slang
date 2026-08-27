#ifndef SLANG_NVVM_IR_BUILDER_H
#define SLANG_NVVM_IR_BUILDER_H

#include "core/slang-shared-library.h"
#include "slang-com-ptr.h"
#include "slang-nvvm-ir-builder-api.h"

namespace Slang
{

/// Owns and validates the optional LLVM 14 NVVM builder ABI.
///
/// This wrapper and its retained provider library must outlive every module handle created through
/// it. Individual modules are thread-confined by the underlying ABI.
class NVVMIRBuilder
{
public:
    /// Loads `slang-llvm-nvvm` from an explicit path or by its logical name.
    static SlangResult load(
        const String& path,
        ISlangSharedLibraryLoader* loader,
        NVVMIRBuilder& outBuilder);

    /// Validates an already queried API table and retains its non-null owning library.
    static SlangResult initialize(
        const SlangNVVMBuilderAPI_V1& api,
        ISlangSharedLibrary* library,
        NVVMIRBuilder& outBuilder);

    /// Validates the known V2 prefixes and nested V1 API, then retains its owning library.
    static SlangResult initialize(
        const SlangNVVMBuilderAPI_V2& api,
        ISlangSharedLibrary* library,
        NVVMIRBuilder& outBuilder);

    bool isInitialized() const { return m_api.createModule != nullptr; }
    bool supportsSerializationDiagnostics() const
    {
        return m_apiV2.structureSize >= SLANG_NVVM_BUILDER_API_V2_MIN_SIZE &&
               m_apiV2.serializeModuleWithDiagnostics != nullptr;
    }
    /// Returns whether the provider advertised the complete Slice 4 scalar-memory prefix.
    bool supportsScalarOperations() const;
    /// Returns whether the provider advertised the complete Slice 7 scalar-control-flow prefix.
    bool supportsScalarControlFlow() const;
    /// Returns whether the provider advertised the complete Slice 8 scalar-SSA prefix.
    bool supportsScalarSSA() const;
    /// Returns whether the provider advertised the complete Slice 9 scalar-function prefix.
    bool supportsScalarFunctions() const;
    /// Returns whether the provider advertised the complete Slice 10 pointer-arithmetic prefix.
    bool supportsScalarPointerArithmetic() const;
    /// Returns whether the provider advertised the complete Slice 11 array-addressing prefix.
    bool supportsScalarArrayAddressing() const;
    /// Returns whether the provider advertised the complete Slice 12 integer-multiply prefix.
    bool supportsScalarIntegerMultiply() const;
    /// Returns whether the provider advertised the complete Slice 13 integer-bit-AND prefix.
    bool supportsScalarIntegerBitAnd() const;
    /// Returns whether the provider advertised the complete Slice 14 integer-bit-OR prefix.
    bool supportsScalarIntegerBitOr() const;
    /// Returns whether the provider advertised the complete Slice 15 integer-bit-XOR prefix.
    bool supportsScalarIntegerBitXor() const;
    /// Returns whether the provider advertised the complete Slice 16 integer-bit-NOT prefix.
    bool supportsScalarIntegerBitNot() const;
    /// Returns whether the provider advertised the complete Slice 17 integer-negate prefix.
    bool supportsScalarIntegerNegate() const;
    /// Returns whether the provider can serialize the audited NVVM IR 2.0 text dialect.
    bool supportsNVVMIR20Assembly() const;
    /// Returns whether the provider advertised the complete Slice 19 atomic-add prefix.
    bool supportsRelaxedGlobalI32AtomicAdd() const;
    /// Returns whether the provider advertised the complete Slice 21 integer-equality prefix.
    bool supportsScalarIntegerEqual() const;
    /// Returns whether the provider advertised the complete Slice 22 integer-inequality prefix.
    bool supportsScalarIntegerNotEqual() const;
    /// Returns whether the provider advertised the complete Slice 23 signed-greater-than prefix.
    bool supportsScalarIntegerSignedGreaterThan() const;
    /// Returns whether the provider advertised the complete Slice 24 signed-less-equal prefix.
    bool supportsScalarIntegerSignedLessEqual() const;
    /// Returns whether the provider advertised the complete Slice 25 signed-greater-equal prefix.
    bool supportsScalarIntegerSignedGreaterEqual() const;
    /// Returns the provider identity that affects generated IR and shader-cache keys.
    String getVersionString() const;
    const SlangNVVMBuilderAPI_V1& getAPI() const { return m_api; }
    /// Returns the locally supported V2 prefix, with `structureSize` clamped to that prefix.
    const SlangNVVMBuilderAPI_V2* getAPIV2() const
    {
        return supportsSerializationDiagnostics() ? &m_apiV2 : nullptr;
    }

    /// Creates a module whose LLVM objects remain owned by the returned module handle.
    SlangResult createModule(
        const UnownedStringSlice& moduleName,
        SlangNVVMModuleHandle_1& outModule) const;

    /// Destroys a module and every opaque handle created from it.
    void destroyModule(SlangNVVMModuleHandle_1 module) const;

    /// Gets the module's context-owned void type.
    SlangResult getVoidType(SlangNVVMModuleHandle_1 module, SlangNVVMTypeHandle_1& outType) const;

    /// Gets a module-context-owned signless integer type.
    SlangResult getIntegerType(
        SlangNVVMModuleHandle_1 module,
        uint32_t bitWidth,
        SlangNVVMTypeHandle_1& outType) const;

    /// Gets a typed pointer with the requested NVVM address space.
    SlangResult getPointerType(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 pointeeType,
        SlangNVVMAddressSpace_2 addressSpace,
        SlangNVVMTypeHandle_1& outType) const;

    /// Creates a non-variadic function type from module-owned type handles.
    SlangResult getFunctionType(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 resultType,
        const SlangNVVMTypeHandle_1* parameterTypes,
        size_t parameterCount,
        SlangNVVMTypeHandle_1& outType) const;

    /// Declares a function in the module with the exact caller-provided name.
    SlangResult declareFunction(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 functionType,
        const UnownedStringSlice& name,
        SlangNVVMValueHandle_1& outFunction) const;

    /// Gets a declared function's parameter by its zero-based ABI position.
    SlangResult getFunctionParameter(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 function,
        size_t parameterIndex,
        SlangNVVMValueHandle_1& outValue) const;

    /// Appends a basic block to a function owned by the module.
    SlangResult createBlock(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 function,
        const UnownedStringSlice& name,
        SlangNVVMBlockHandle_1& outBlock) const;

    /// Selects a module-owned block as the destination for subsequent instructions.
    SlangResult setInsertBlock(SlangNVVMModuleHandle_1 module, SlangNVVMBlockHandle_1 block) const;

    /// Emits a non-volatile aligned load into the current insertion block.
    SlangResult emitLoad(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 pointer,
        uint32_t alignment,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits a non-volatile aligned store into the current insertion block.
    SlangResult emitStore(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1 pointer,
        uint32_t alignment) const;

    /// Emits ADD or SUB for same-typed scalar integer values.
    SlangResult emitIntegerBinary(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMIntegerBinaryOp_2 operation,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits a signed integer less-than comparison and returns its i1 result.
    SlangResult emitIntegerSignedLessThan(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Terminates the current insertion block with an unconditional branch.
    SlangResult emitBranch(SlangNVVMModuleHandle_1 module, SlangNVVMBlockHandle_1 targetBlock)
        const;

    /// Terminates the current insertion block with an i1 conditional branch.
    SlangResult emitConditionalBranch(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 condition,
        SlangNVVMBlockHandle_1 trueBlock,
        SlangNVVMBlockHandle_1 falseBlock) const;

    /// Gets an exactly representable signed integer constant of the requested type.
    SlangResult getIntegerConstant(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 integerType,
        int64_t value,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits an integer phi at the start of the explicit target block.
    SlangResult emitIntegerPhi(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMBlockHandle_1 targetBlock,
        SlangNVVMTypeHandle_1 integerType,
        SlangNVVMValueHandle_1& outValue) const;

    /// Adds a validated integer phi input from one predecessor edge.
    SlangResult addIntegerPhiIncoming(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 phi,
        SlangNVVMValueHandle_1 value,
        SlangNVVMBlockHandle_1 predecessorBlock) const;

    /// Emits a direct call to an integer function and returns its integer result.
    SlangResult emitIntegerCall(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 callee,
        const SlangNVVMValueHandle_1* arguments,
        size_t argumentCount,
        SlangNVVMValueHandle_1& outValue) const;

    /// Terminates the current insertion block with an integer return value.
    SlangResult emitIntegerReturn(SlangNVVMModuleHandle_1 module, SlangNVVMValueHandle_1 value)
        const;

    /// Emits a non-inbounds element offset from a typed pointer.
    SlangResult emitPointerOffset(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 basePointer,
        SlangNVVMValueHandle_1 elementOffset,
        SlangNVVMValueHandle_1& outPointer) const;

    /// Gets a fixed, nonempty array type with the requested element type.
    SlangResult getArrayType(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 elementType,
        uint32_t elementCount,
        SlangNVVMTypeHandle_1& outType) const;

    /// Emits a non-inbounds address of one element of a typed array pointer.
    SlangResult emitArrayElementPointer(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 baseArrayPointer,
        SlangNVVMValueHandle_1 elementIndex,
        SlangNVVMValueHandle_1& outPointer) const;

    /// Emits multiplication for same-typed scalar integer values.
    SlangResult emitIntegerMultiply(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits bitwise AND for same-typed scalar integer values.
    SlangResult emitIntegerBitAnd(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits bitwise OR for same-typed scalar integer values.
    SlangResult emitIntegerBitOr(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits bitwise XOR for same-typed scalar integer values.
    SlangResult emitIntegerBitXor(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits bitwise NOT for a scalar integer value.
    SlangResult emitIntegerBitNot(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits wrapping arithmetic negation for a scalar integer value.
    SlangResult emitIntegerNegate(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits relaxed device-scope atomic add through a naturally aligned global i32 pointer.
    SlangResult emitRelaxedGlobalI32AtomicAdd(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 pointer,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1& outOriginalValue) const;

    /// Emits scalar integer equality and returns an i1 value.
    SlangResult emitIntegerEqual(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits scalar integer inequality and returns an i1 value.
    SlangResult emitIntegerNotEqual(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits scalar signed-integer greater-than and returns an i1 value.
    SlangResult emitIntegerSignedGreaterThan(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits scalar signed-integer less-than-or-equal and returns an i1 value.
    SlangResult emitIntegerSignedLessEqual(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Emits scalar signed-integer greater-than-or-equal and returns an i1 value.
    SlangResult emitIntegerSignedGreaterEqual(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1& outValue) const;

    /// Terminates the current void-returning insertion block.
    SlangResult emitReturnVoid(SlangNVVMModuleHandle_1 module) const;

    /// Adds the NVVM kernel annotation for a module-owned function.
    SlangResult markFunctionAsKernel(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 function) const;

    /// Serializes into a host-owned blob using the ABI's size-query/write protocol.
    SlangResult serializeModule(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMSerializationFormat_1 format,
        ComPtr<ISlangBlob>& outBlob) const;

    /// Serializes through V2 and copies the corresponding verifier bytes into host storage.
    ///
    /// Returns `SLANG_E_NOT_AVAILABLE` for a V1-only provider. LLVM verification failure returns
    /// `SLANG_FAIL`, leaves `outBlob` null, and preserves the verifier text in `outDiagnostics`.
    SlangResult serializeModule(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMSerializationFormat_1 format,
        ComPtr<ISlangBlob>& outBlob,
        String& outDiagnostics) const;

private:
    SlangNVVMBuilderAPI_V1 m_api = {};
    SlangNVVMBuilderAPI_V2 m_apiV2 = {};
    ComPtr<ISlangSharedLibrary> m_library;
};

} // namespace Slang

#endif
