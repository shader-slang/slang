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

    /// Validates the exact current root and every required subinterface.
    static SlangResult initialize(
        const SlangNVVMBuilderAPI& api,
        ISlangSharedLibrary* library,
        NVVMIRBuilder& outBuilder);

    bool isInitialized() const { return m_foundation.createModule != nullptr; }
    /// Returns the provider identity that affects generated IR and shader-cache keys.
    String getVersionString() const;
    const SlangNVVMBuilderAPI& getAPI() const { return m_api; }
    const SlangNVVMBuilderFoundationAPI* getFoundationAPI() const { return &m_foundation; }
    const SlangNVVMBuilderConstructionAPI* getConstructionAPI() const { return &m_construction; }
    const SlangNVVMBuilderValueOperationsAPI* getValueOperationsAPI() const
    {
        return &m_valueOperations;
    }
    const SlangNVVMBuilderSurfaceOperationsAPI* getSurfaceOperationsAPI() const
    {
        return &m_surfaceOperations;
    }
    const SlangNVVMBuilderTextureOperationsAPI* getTextureOperationsAPI() const
    {
        return &m_textureOperations;
    }

    /// Queries one complete typed operation.
    bool supportsValueOperation(const SlangNVVMValueOperationDesc& operation) const;

    /// Emits one complete typed operation.
    SlangResult emitValueOperation(
        SlangNVVMModuleHandle module,
        const SlangNVVMValueOperationDesc& operation,
        const SlangNVVMValueHandle* operands,
        size_t operandCount,
        SlangNVVMValueHandle& outValue) const;

    /// Queries one complete typed surface-resource operation.
    bool supportsSurfaceOperation(const SlangNVVMSurfaceOperationDesc& operation) const;

    /// Emits one complete typed surface-resource operation.
    SlangResult emitSurfaceOperation(
        SlangNVVMModuleHandle module,
        const SlangNVVMSurfaceOperationDesc& operation,
        const SlangNVVMValueHandle* operands,
        size_t operandCount,
        SlangNVVMValueHandle& outValue) const;

    /// Queries one complete typed sampled-texture operation.
    bool supportsTextureOperation(const SlangNVVMTextureOperationDesc& operation) const;

    /// Emits one complete typed sampled-texture operation.
    SlangResult emitTextureOperation(
        SlangNVVMModuleHandle module,
        const SlangNVVMTextureOperationDesc& operation,
        const SlangNVVMValueHandle* operands,
        size_t operandCount,
        SlangNVVMValueHandle& outValue) const;

    /// Creates a module whose LLVM objects remain owned by the returned module handle.
    SlangResult createModule(const UnownedStringSlice& moduleName, SlangNVVMModuleHandle& outModule)
        const;

    /// Destroys a module and every opaque handle created from it.
    void destroyModule(SlangNVVMModuleHandle module) const;

    /// Gets the module's context-owned void type.
    SlangResult getVoidType(SlangNVVMModuleHandle module, SlangNVVMTypeHandle& outType) const;

    /// Gets a module-context-owned signless integer type.
    SlangResult getIntegerType(
        SlangNVVMModuleHandle module,
        uint32_t bitWidth,
        SlangNVVMTypeHandle& outType) const;

    /// Gets a module-context-owned IEEE floating-point type.
    SlangResult getFloatingPointType(
        SlangNVVMModuleHandle module,
        uint32_t bitWidth,
        SlangNVVMTypeHandle& outType) const;

    /// Gets a typed pointer with the requested NVVM address space.
    SlangResult getPointerType(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle pointeeType,
        SlangNVVMAddressSpace addressSpace,
        SlangNVVMTypeHandle& outType) const;

    /// Creates a non-variadic function type from module-owned type handles.
    SlangResult getFunctionType(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle resultType,
        const SlangNVVMTypeHandle* parameterTypes,
        size_t parameterCount,
        SlangNVVMTypeHandle& outType) const;

    /// Declares a function with explicit linkage, attributes, and caller-provided name.
    SlangResult declareFunction(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle functionType,
        SlangNVVMLinkage linkage,
        SlangNVVMFunctionFlags flags,
        const UnownedStringSlice& name,
        SlangNVVMValueHandle& outFunction) const;

    /// Gets a declared function's parameter by its zero-based ABI position.
    SlangResult getFunctionParameter(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle function,
        size_t parameterIndex,
        SlangNVVMValueHandle& outValue) const;

    /// Sets validated ABI properties on one physical function parameter.
    SlangResult setFunctionParameterAttributes(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle function,
        size_t parameterIndex,
        SlangNVVMParameterFlags flags,
        SlangNVVMTypeHandle pointeeType,
        uint32_t alignment) const;

    /// Appends a basic block to a function owned by the module.
    SlangResult createBlock(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle function,
        const UnownedStringSlice& name,
        SlangNVVMBlockHandle& outBlock) const;

    /// Selects a module-owned block as the destination for subsequent instructions.
    SlangResult setInsertBlock(SlangNVVMModuleHandle module, SlangNVVMBlockHandle block) const;

    /// Emits a non-volatile aligned load with explicit memory semantics.
    SlangResult emitLoad(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle pointer,
        uint32_t alignment,
        SlangNVVMLoadFlags flags,
        SlangNVVMValueHandle& outValue) const;

    /// Emits a non-volatile aligned store into the current insertion block.
    SlangResult emitStore(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle value,
        SlangNVVMValueHandle pointer,
        uint32_t alignment) const;

    /// Emits entry-block generic local storage for one complete provider value type.
    SlangResult emitLocalStorage(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle valueType,
        uint32_t alignment,
        const UnownedStringSlice& name,
        SlangNVVMValueHandle& outStorage) const;

    /// Emits ADD or SUB for same-typed scalar integer values.
    SlangResult emitIntegerBinary(
        SlangNVVMModuleHandle module,
        SlangNVVMValueOperation operation,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits one scalar-integer unary operation through the current semantic table.
    SlangResult emitIntegerUnary(
        SlangNVVMModuleHandle module,
        SlangNVVMValueOperation operation,
        SlangNVVMValueHandle value,
        SlangNVVMValueHandle& outValue) const;

    /// Emits one scalar-integer binary operation through the current semantic table.
    SlangResult emitIntegerBinaryOperation(
        SlangNVVMModuleHandle module,
        SlangNVVMValueOperation operation,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits one scalar-integer comparison through the current semantic table.
    SlangResult emitIntegerCompare(
        SlangNVVMModuleHandle module,
        SlangNVVMValueOperation operation,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits one scalar floating-point binary operation through the current semantic table.
    SlangResult emitFloatingBinary(
        SlangNVVMModuleHandle module,
        SlangNVVMValueOperation operation,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits one scalar floating-point unary operation through the current semantic table.
    SlangResult emitFloatingUnary(
        SlangNVVMModuleHandle module,
        SlangNVVMValueOperation operation,
        SlangNVVMValueHandle value,
        SlangNVVMValueHandle& outValue) const;

    /// Emits one scalar floating-point comparison through the current semantic table.
    SlangResult emitFloatingCompare(
        SlangNVVMModuleHandle module,
        SlangNVVMValueOperation operation,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Gets an exact scalar floating-point constant from its width-bounded IEEE-754 bits.
    SlangResult getFloatingPointConstant(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle floatingPointType,
        uint32_t bitWidth,
        uint64_t bitPattern,
        SlangNVVMValueHandle& outValue) const;

    /// Emits a typed scalar phi at the start of the explicit target block.
    SlangResult emitPhi(
        SlangNVVMModuleHandle module,
        SlangNVVMBlockHandle targetBlock,
        SlangNVVMTypeHandle type,
        SlangNVVMValueHandle& outValue) const;

    /// Adds one exact-typed scalar phi input from a predecessor edge.
    SlangResult addPhiIncoming(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle phi,
        SlangNVVMValueHandle value,
        SlangNVVMBlockHandle predecessorBlock) const;

    /// Emits a direct call to a same-module typed function.
    SlangResult emitCall(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle callee,
        const SlangNVVMValueHandle* arguments,
        size_t argumentCount,
        SlangNVVMValueHandle& outValue) const;

    /// Emits a typed valued return in the current function.
    SlangResult emitValueReturn(SlangNVVMModuleHandle module, SlangNVVMValueHandle value) const;

    /// Emits a signed integer less-than comparison and returns its i1 result.
    SlangResult emitIntegerSignedLessThan(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Terminates the current insertion block with an unconditional branch.
    SlangResult emitBranch(SlangNVVMModuleHandle module, SlangNVVMBlockHandle targetBlock) const;

    /// Terminates the current insertion block with an i1 conditional branch.
    SlangResult emitConditionalBranch(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle condition,
        SlangNVVMBlockHandle trueBlock,
        SlangNVVMBlockHandle falseBlock) const;

    /// Terminates the current block with a typed integer switch.
    SlangResult emitSwitch(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle condition,
        const SlangNVVMValueHandle* caseValues,
        const SlangNVVMBlockHandle* caseBlocks,
        size_t caseCount,
        SlangNVVMBlockHandle defaultBlock) const;

    /// Gets an exactly representable signed integer constant of the requested type.
    SlangResult getIntegerConstant(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle integerType,
        int64_t value,
        SlangNVVMValueHandle& outValue) const;

    /// Emits an integer phi at the start of the explicit target block.
    SlangResult emitIntegerPhi(
        SlangNVVMModuleHandle module,
        SlangNVVMBlockHandle targetBlock,
        SlangNVVMTypeHandle integerType,
        SlangNVVMValueHandle& outValue) const;

    /// Adds a validated integer phi input from one predecessor edge.
    SlangResult addIntegerPhiIncoming(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle phi,
        SlangNVVMValueHandle value,
        SlangNVVMBlockHandle predecessorBlock) const;

    /// Emits a direct call to an integer function and returns its integer result.
    SlangResult emitIntegerCall(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle callee,
        const SlangNVVMValueHandle* arguments,
        size_t argumentCount,
        SlangNVVMValueHandle& outValue) const;

    /// Terminates the current insertion block with an integer return value.
    SlangResult emitIntegerReturn(SlangNVVMModuleHandle module, SlangNVVMValueHandle value) const;

    /// Emits a non-inbounds element offset from a typed pointer.
    SlangResult emitPointerOffset(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle basePointer,
        SlangNVVMValueHandle elementOffset,
        SlangNVVMValueHandle& outPointer) const;

    /// Emits a non-inbounds byte offset and returns a pointer to `resultPointeeType`.
    SlangResult emitByteOffsetPointer(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle basePointer,
        SlangNVVMValueHandle byteOffset,
        SlangNVVMTypeHandle resultPointeeType,
        SlangNVVMValueHandle& outPointer) const;

    /// Gets a fixed, nonempty array type with the requested element type.
    SlangResult getArrayType(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle elementType,
        uint32_t elementCount,
        SlangNVVMTypeHandle& outType) const;

    /// Gets a fixed-vector type through construction interface version 2.
    SlangResult getVectorType(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle elementType,
        uint32_t elementCount,
        SlangNVVMTypeHandle& outType) const;

    /// Gets an unpacked literal struct type from an ordered field-type sequence.
    SlangResult getStructType(
        SlangNVVMModuleHandle module,
        const SlangNVVMTypeHandle* fieldTypes,
        size_t fieldCount,
        SlangNVVMTypeHandle& outType) const;

    /// Declares uninitialized storage with exact linkage, provider type, and NVVM address space.
    SlangResult declareGlobalStorage(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle valueType,
        SlangNVVMLinkage linkage,
        SlangNVVMAddressSpace addressSpace,
        uint32_t alignment,
        const UnownedStringSlice& name,
        SlangNVVMValueHandle& outStorage) const;

    /// Constructs a fixed vector from its complete ordered scalar element sequence.
    SlangResult emitVectorConstruct(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle vectorType,
        const SlangNVVMValueHandle* elements,
        size_t elementCount,
        SlangNVVMValueHandle& outValue) const;

    /// Extracts the integer-indexed element of a fixed vector.
    SlangResult emitVectorElementExtract(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle vector,
        SlangNVVMValueHandle elementIndex,
        SlangNVVMValueHandle& outValue) const;

    /// Emits a non-inbounds address of one element of a typed fixed-array or vector pointer.
    SlangResult emitSequentialElementPointer(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle baseSequentialPointer,
        SlangNVVMValueHandle elementIndex,
        SlangNVVMValueHandle& outPointer) const;

    /// Emits the address of one statically selected struct field.
    SlangResult emitStructFieldPointer(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle baseStructPointer,
        uint32_t fieldIndex,
        SlangNVVMValueHandle& outPointer) const;

    /// Constructs an array or struct from its complete ordered element sequence.
    SlangResult emitAggregateConstruct(
        SlangNVVMModuleHandle module,
        SlangNVVMTypeHandle aggregateType,
        const SlangNVVMValueHandle* elements,
        size_t elementCount,
        SlangNVVMValueHandle& outValue) const;

    /// Extracts one statically selected element from an array or struct value.
    SlangResult emitAggregateElementExtract(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle aggregateValue,
        uint32_t elementIndex,
        SlangNVVMValueHandle& outValue) const;

    /// Emits multiplication for same-typed scalar integer values.
    SlangResult emitIntegerMultiply(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits bitwise AND for same-typed scalar integer values.
    SlangResult emitIntegerBitAnd(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits bitwise OR for same-typed scalar integer values.
    SlangResult emitIntegerBitOr(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits bitwise XOR for same-typed scalar integer values.
    SlangResult emitIntegerBitXor(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits bitwise NOT for a scalar integer value.
    SlangResult emitIntegerBitNot(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle value,
        SlangNVVMValueHandle& outValue) const;

    /// Emits wrapping arithmetic negation for a scalar integer value.
    SlangResult emitIntegerNegate(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle value,
        SlangNVVMValueHandle& outValue) const;

    /// Emits relaxed device-scope atomic add through a naturally aligned global i32 pointer.
    SlangResult emitRelaxedGlobalI32AtomicAdd(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle pointer,
        SlangNVVMValueHandle value,
        SlangNVVMValueHandle& outOriginalValue) const;

    /// Emits scalar integer equality and returns an i1 value.
    SlangResult emitIntegerEqual(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits scalar integer inequality and returns an i1 value.
    SlangResult emitIntegerNotEqual(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits scalar signed-integer greater-than and returns an i1 value.
    SlangResult emitIntegerSignedGreaterThan(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits scalar signed-integer less-than-or-equal and returns an i1 value.
    SlangResult emitIntegerSignedLessEqual(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Emits scalar signed-integer greater-than-or-equal and returns an i1 value.
    SlangResult emitIntegerSignedGreaterEqual(
        SlangNVVMModuleHandle module,
        SlangNVVMValueHandle left,
        SlangNVVMValueHandle right,
        SlangNVVMValueHandle& outValue) const;

    /// Terminates the current void-returning insertion block.
    SlangResult emitReturnVoid(SlangNVVMModuleHandle module) const;

    /// Adds the NVVM kernel annotation for a module-owned function.
    SlangResult markFunctionAsKernel(SlangNVVMModuleHandle module, SlangNVVMValueHandle function)
        const;

    /// Serializes into a host-owned blob using the ABI's size-query/write protocol.
    SlangResult serializeModule(
        SlangNVVMModuleHandle module,
        SlangNVVMSerializationFormat format,
        ComPtr<ISlangBlob>& outBlob) const;

    /// Serializes and copies the corresponding verifier bytes into host storage.
    SlangResult serializeModule(
        SlangNVVMModuleHandle module,
        SlangNVVMSerializationFormat format,
        ComPtr<ISlangBlob>& outBlob,
        String& outDiagnostics) const;

private:
    SlangNVVMBuilderAPI m_api = {};
    SlangNVVMBuilderFoundationAPI m_foundation = {};
    SlangNVVMBuilderConstructionAPI m_construction = {};
    SlangNVVMBuilderValueOperationsAPI m_valueOperations = {};
    SlangNVVMBuilderSurfaceOperationsAPI m_surfaceOperations = {};
    SlangNVVMBuilderTextureOperationsAPI m_textureOperations = {};
    ComPtr<ISlangSharedLibrary> m_library;
};

} // namespace Slang

#endif
