#ifndef SLANG_NVVM_IR_BUILDER_API_H
#define SLANG_NVVM_IR_BUILDER_API_H

#include <stddef.h>
#include <stdint.h>

#define SLANG_NVVM_BUILDER_ABI_VERSION_1 1u
#define SLANG_NVVM_BUILDER_ABI_VERSION_2 2u
#define SLANG_NVVM_BUILDER_ABI_VERSION_3 3u
#define SLANG_NVVM_BUILDER_GET_API_V1_NAME "slang_getNVVMBuilderAPI_V1"
#define SLANG_NVVM_BUILDER_GET_API_V2_NAME "slang_getNVVMBuilderAPI_V2"
#define SLANG_NVVM_BUILDER_GET_API_V3_NAME "slang_getNVVMBuilderAPI_V3"

#if defined(_MSC_VER)
#define SLANG_NVVM_CALL __stdcall
#elif defined(_WIN32) && defined(__GNUC__)
#define SLANG_NVVM_CALL __attribute__((stdcall))
#else
#define SLANG_NVVM_CALL
#endif

#if defined(SLANG_NVVM_BUILDER_EXPORTS)
#if defined(_MSC_VER)
#define SLANG_NVVM_BUILDER_API __declspec(dllexport)
#elif defined(_WIN32)
#define SLANG_NVVM_BUILDER_API __attribute__((dllexport)) __attribute__((visibility("default")))
#else
#define SLANG_NVVM_BUILDER_API __attribute__((visibility("default")))
#endif
#else
#define SLANG_NVVM_BUILDER_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct SlangNVVMModule_1* SlangNVVMModuleHandle_1;
    typedef struct SlangNVVMType_1* SlangNVVMTypeHandle_1;
    typedef struct SlangNVVMValue_1* SlangNVVMValueHandle_1;
    typedef struct SlangNVVMBlock_1* SlangNVVMBlockHandle_1;

    /** Uses Slang's signed 32-bit result convention: negative values fail, other values succeed. */
    typedef int32_t SlangNVVMResult_1;

    typedef uint32_t SlangNVVMPointerModel_1;
#define SLANG_NVVM_POINTER_MODEL_TYPED ((SlangNVVMPointerModel_1)1u)

    typedef uint32_t SlangNVVMSerializationFormat_1;
#define SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY ((SlangNVVMSerializationFormat_1)0u)
#define SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE ((SlangNVVMSerializationFormat_1)1u)
/** LLVM assembly in the LLVM 7-era NVVM IR 2.0 dialect accepted by libNVVM. */
#define SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY ((SlangNVVMSerializationFormat_1)2u)

    typedef uint32_t SlangNVVMVerificationStatus_2;
#define SLANG_NVVM_VERIFICATION_NOT_RUN ((SlangNVVMVerificationStatus_2)0u)
#define SLANG_NVVM_VERIFICATION_VALID ((SlangNVVMVerificationStatus_2)1u)
#define SLANG_NVVM_VERIFICATION_INVALID ((SlangNVVMVerificationStatus_2)2u)

    typedef uint32_t SlangNVVMAddressSpace_2;
#define SLANG_NVVM_ADDRESS_SPACE_GENERIC ((SlangNVVMAddressSpace_2)0u)
#define SLANG_NVVM_ADDRESS_SPACE_GLOBAL ((SlangNVVMAddressSpace_2)1u)
#define SLANG_NVVM_ADDRESS_SPACE_SHARED ((SlangNVVMAddressSpace_2)3u)
#define SLANG_NVVM_ADDRESS_SPACE_CONSTANT ((SlangNVVMAddressSpace_2)4u)
#define SLANG_NVVM_ADDRESS_SPACE_LOCAL ((SlangNVVMAddressSpace_2)5u)

    typedef uint32_t SlangNVVMIntegerBinaryOp_2;
#define SLANG_NVVM_INTEGER_BINARY_OP_ADD ((SlangNVVMIntegerBinaryOp_2)0u)
#define SLANG_NVVM_INTEGER_BINARY_OP_SUB ((SlangNVVMIntegerBinaryOp_2)1u)

    /**
     * Version 1 of the private ABI between Slang and its optional LLVM-backed NVVM IR module.
     *
     * The caller initializes `structureSize` and `abiVersion` before passing this structure to
     * `slang_getNVVMBuilderAPI_V1`. All LLVM objects remain owned by their module and cross the ABI
     * only as opaque handles. `serializeModule` uses a two-call protocol: query the required size
     * with a null destination, then provide caller-owned storage of at least that size.
     *
     * Every non-null handle passed to a function must still be live and must have been returned for
     * the same module. Destroying a module invalidates all of its type, value, and block handles.
     * Calls that mutate or serialize one module are thread-confined and must not run concurrently.
     */
    typedef struct SlangNVVMBuilderAPI_V1
    {
        uint32_t structureSize;
        uint32_t abiVersion;

        uint32_t llvmVersionMajor;
        uint32_t llvmVersionMinor;
        uint32_t llvmVersionPatch;
        uint32_t nvvmIRVersionMajor;
        uint32_t nvvmIRVersionMinor;
        uint32_t pointerModel;

        SlangNVVMResult_1(SLANG_NVVM_CALL* createModule)(
            const char* moduleName,
            size_t moduleNameSize,
            SlangNVVMModuleHandle_1* outModule);
        void(SLANG_NVVM_CALL* destroyModule)(SlangNVVMModuleHandle_1 module);

        SlangNVVMResult_1(SLANG_NVVM_CALL* getVoidType)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMTypeHandle_1* outType);
        SlangNVVMResult_1(SLANG_NVVM_CALL* getFunctionType)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMTypeHandle_1 resultType,
            const SlangNVVMTypeHandle_1* parameterTypes,
            size_t parameterCount,
            SlangNVVMTypeHandle_1* outType);
        SlangNVVMResult_1(SLANG_NVVM_CALL* declareFunction)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMTypeHandle_1 functionType,
            const char* name,
            size_t nameSize,
            SlangNVVMValueHandle_1* outFunction);

        SlangNVVMResult_1(SLANG_NVVM_CALL* createBlock)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMValueHandle_1 function,
            const char* name,
            size_t nameSize,
            SlangNVVMBlockHandle_1* outBlock);
        SlangNVVMResult_1(SLANG_NVVM_CALL* setInsertBlock)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMBlockHandle_1 block);
        SlangNVVMResult_1(SLANG_NVVM_CALL* emitReturnVoid)(SlangNVVMModuleHandle_1 module);
        SlangNVVMResult_1(SLANG_NVVM_CALL* markFunctionAsKernel)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMValueHandle_1 function);

        SlangNVVMResult_1(SLANG_NVVM_CALL* serializeModule)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMSerializationFormat_1 format,
            void* destination,
            size_t destinationSize,
            size_t* outSerializedSize);
    } SlangNVVMBuilderAPI_V1;

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangGetNVVMBuilderAPI_V1)(
        SlangNVVMBuilderAPI_V1* outAPI);

    /**
     * Serializes a module and returns its LLVM verifier diagnostic in the same transaction.
     *
     * A query call supplies null destinations and zero capacities. Transport success is reported
     * separately from `outVerificationStatus`: an invalid module returns transport success,
     * `SLANG_NVVM_VERIFICATION_INVALID`, no serialized bytes, and a nonzero diagnostic size. Byte
     * counts exclude a diagnostic NUL terminator. If either supplied buffer is too small, neither
     * buffer is modified and both required sizes are reported. Destination ranges and the three
     * output metadata values must occupy non-overlapping storage.
     */
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMSerializeModuleWithDiagnostics_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMSerializationFormat_1 format,
        void* serializedDestination,
        size_t serializedDestinationSize,
        size_t* outSerializedSize,
        void* diagnosticDestination,
        size_t diagnosticDestinationSize,
        size_t* outDiagnosticSize,
        SlangNVVMVerificationStatus_2* outVerificationStatus);

    /// Gets a signless LLVM integer type. Valid bit widths are 1 through 8,388,608.
    /// On failure, the output type is null.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMGetIntegerType_2)(
        SlangNVVMModuleHandle_1 module,
        uint32_t bitWidth,
        SlangNVVMTypeHandle_1* outType);

    /// Gets a typed pointer to a same-module loadable type.
    /// The address space must equal one of the five declared NVVM address-space constants.
    /// On failure, the output type is null.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMGetPointerType_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 pointeeType,
        SlangNVVMAddressSpace_2 addressSpace,
        SlangNVVMTypeHandle_1* outType);

    /// Gets a same-module function parameter by its zero-based ABI position.
    /// On failure, the output value is null.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMGetFunctionParameter_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 function,
        size_t parameterIndex,
        SlangNVVMValueHandle_1* outValue);

    /// Emits a non-volatile load through a same-module typed pointer.
    /// The module must own the current unterminated insertion block.
    /// Alignment is a nonzero power-of-two byte count.
    /// On failure, the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitLoad_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 pointer,
        uint32_t alignment,
        SlangNVVMValueHandle_1* outValue);

    /// Emits a non-volatile store through a same-module typed pointer.
    /// The module must own the current unterminated insertion block.
    /// The value type must equal the pointee type, and constant address space is read-only.
    /// Alignment is a nonzero power-of-two byte count. Failure inserts nothing.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitStore_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1 pointer,
        uint32_t alignment);

    /// Emits ADD or SUB for same-module scalar integer operands of identical type.
    /// The module must own the current unterminated insertion block.
    /// On failure, the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBinary_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMIntegerBinaryOp_2 operation,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Emits a signed less-than comparison for same-module scalar integer operands of identical
    /// type. The result is i1, the module must own the current unterminated insertion block, and
    /// failure leaves the output value null without inserting an instruction.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerSignedLessThan_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Terminates the current unterminated insertion block with an unconditional branch.
    /// The target must belong to the current function. Failure inserts no instruction.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitBranch_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMBlockHandle_1 targetBlock);

    /// Terminates the current unterminated insertion block with a conditional branch.
    /// The condition must be a same-module i1 value and both targets must belong to the current
    /// function. Failure inserts no instruction.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitConditionalBranch_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 condition,
        SlangNVVMBlockHandle_1 trueBlock,
        SlangNVVMBlockHandle_1 falseBlock);

    /// Gets a same-module integer constant whose signed value is exactly representable by the
    /// requested integer type. On failure, the output value is null.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMGetIntegerConstant_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 integerType,
        int64_t value,
        SlangNVVMValueHandle_1* outValue);

    /// Emits an integer phi at the start of a same-module target block.
    /// Existing phi instructions remain before it and every non-phi instruction remains after it.
    /// On failure, the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerPhi_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMBlockHandle_1 targetBlock,
        SlangNVVMTypeHandle_1 integerType,
        SlangNVVMValueHandle_1* outValue);

    /// Adds one integer phi input from a predecessor with exactly one edge to the phi block.
    /// The function CFG must be fully terminated, the value must dominate the predecessor edge,
    /// and the phi must not already have an input from that predecessor. Failure changes nothing.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMAddIntegerPhiIncoming_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 phi,
        SlangNVVMValueHandle_1 value,
        SlangNVVMBlockHandle_1 predecessorBlock);

    /// Emits a direct call to a same-module, non-variadic integer function.
    /// The module must own the current unterminated insertion block. Every parameter and the
    /// result must be an integer type, each argument must have the exact corresponding parameter
    /// type, and every argument must be available at the insertion point. On failure, the output
    /// value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerCall_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 callee,
        const SlangNVVMValueHandle_1* arguments,
        size_t argumentCount,
        SlangNVVMValueHandle_1* outValue);

    /// Terminates the current unterminated insertion block with an integer return value.
    /// The value must be available at the insertion point and exactly match the current function's
    /// integer return type. Failure inserts no instruction.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerReturn_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 value);

    /// Emits a non-inbounds element offset from a same-module typed pointer.
    /// The base pointer must have a sized pointee, and the scalar integer offset and base pointer
    /// must both be available at the current unterminated insertion point. On failure, the output
    /// pointer is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitPointerOffset_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 basePointer,
        SlangNVVMValueHandle_1 elementOffset,
        SlangNVVMValueHandle_1* outPointer);

    /// Gets a fixed, nonempty LLVM array type with a same-module sized element type.
    /// On failure, the output type is null.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMGetArrayType_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 elementType,
        uint32_t elementCount,
        SlangNVVMTypeHandle_1* outType);

    /// Emits a non-inbounds element address from a same-module typed pointer to an LLVM array.
    /// The scalar integer index and base pointer must both be available at the current
    /// unterminated insertion point. On failure, the output pointer is null and no instruction is
    /// inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitArrayElementPointer_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 baseArrayPointer,
        SlangNVVMValueHandle_1 elementIndex,
        SlangNVVMValueHandle_1* outPointer);

    /// Emits multiplication for same-module scalar integer operands of identical type.
    /// Both operands must be available at the current unterminated insertion point. On failure,
    /// the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerMultiply_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Emits bitwise AND for same-module scalar integer operands of identical type.
    /// Both operands must be available at the current unterminated insertion point. On failure,
    /// the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBitAnd_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Emits bitwise OR for same-module scalar integer operands of identical type.
    /// Both operands must be available at the current unterminated insertion point. On failure,
    /// the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBitOr_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Emits bitwise XOR for same-module scalar integer operands of identical type.
    /// Both operands must be available at the current unterminated insertion point. On failure,
    /// the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBitXor_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Emits bitwise NOT for a same-module scalar integer operand.
    /// The operand must be available at the current unterminated insertion point. On failure,
    /// the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBitNot_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1* outValue);

    /// Emits wrapping arithmetic negation for a same-module scalar integer operand.
    /// The operand must be available at the current unterminated insertion point. On failure,
    /// the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerNegate_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1* outValue);

    /// Atomically adds an i32 value through a naturally aligned global-address-space pointer.
    /// The operation is non-volatile, relaxed, and device scoped. Both operands must be available
    /// at the current unterminated insertion point. On success, the output is the original stored
    /// value. On failure, the output is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitRelaxedGlobalI32AtomicAdd_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 pointer,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1* outOriginalValue);

    /// Emits equality for same-module scalar integer operands of identical type.
    /// The result is i1. Both operands must be available at the current unterminated insertion
    /// point. On failure, the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerEqual_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Emits inequality for same-module scalar integer operands of identical type.
    /// The result is i1. Both operands must be available at the current unterminated insertion
    /// point. On failure, the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerNotEqual_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Emits signed greater-than for same-module scalar integer operands of identical type.
    /// The result is i1. Both operands must be available at the current unterminated insertion
    /// point. On failure, the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerSignedGreaterThan_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Emits signed less-than-or-equal for same-module scalar integer operands of identical type.
    /// The result is i1. Both operands must be available at the current unterminated insertion
    /// point. On failure, the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerSignedLessEqual_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Emits signed greater-than-or-equal for same-module scalar integer operands of identical
    /// type. The result is i1. Both operands must be available at the current unterminated
    /// insertion point. On failure, the output value is null and no instruction is inserted.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerSignedGreaterEqual_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Gets the raw CUDA ABI type for `RWStructuredBuffer<int>`: a naturally aligned aggregate
    /// containing an AS1 i32 data pointer followed by a 64-bit element count. On failure, the
    /// output type is null.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMGetRawRWStructuredBufferI32Type_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1* outType);

    /// Emits a non-inbounds element address from a raw CUDA `RWStructuredBuffer<int>` value.
    /// The buffer and scalar i32 index must be available at the current unterminated insertion
    /// point. On failure, the output pointer is null and no instruction is inserted.
    typedef SlangNVVMResult_1(
        SLANG_NVVM_CALL* SlangNVVMEmitRawRWStructuredBufferI32ElementPointer_2)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 buffer,
        SlangNVVMValueHandle_1 elementIndex,
        SlangNVVMValueHandle_1* outPointer);

    /// Version 2 composes the immutable V1 API with atomic serialization diagnostics.
    ///
    /// The caller zero-initializes the structure and supplies its capacity in structureSize.
    /// The provider copies only the prefix that fits and reports its complete supported size.
    ///
    /// Functions may be appended to V2, but existing fields and semantics are immutable.
    /// Each published minimum-size capability block is all-or-none.
    /// A size inside a known block is malformed rather than a smaller capability.
    typedef struct SlangNVVMBuilderAPI_V2
    {
        uint32_t structureSize;
        uint32_t abiVersion;

        SlangNVVMBuilderAPI_V1 baseAPI;
        SlangNVVMSerializeModuleWithDiagnostics_2 serializeModuleWithDiagnostics;

        SlangNVVMGetIntegerType_2 getIntegerType;
        SlangNVVMGetPointerType_2 getPointerType;
        SlangNVVMGetFunctionParameter_2 getFunctionParameter;
        SlangNVVMEmitLoad_2 emitLoad;
        SlangNVVMEmitStore_2 emitStore;

        SlangNVVMEmitIntegerBinary_2 emitIntegerBinary;
        SlangNVVMEmitIntegerSignedLessThan_2 emitIntegerSignedLessThan;
        SlangNVVMEmitBranch_2 emitBranch;
        SlangNVVMEmitConditionalBranch_2 emitConditionalBranch;

        SlangNVVMGetIntegerConstant_2 getIntegerConstant;
        SlangNVVMEmitIntegerPhi_2 emitIntegerPhi;
        SlangNVVMAddIntegerPhiIncoming_2 addIntegerPhiIncoming;

        SlangNVVMEmitIntegerCall_2 emitIntegerCall;
        SlangNVVMEmitIntegerReturn_2 emitIntegerReturn;

        SlangNVVMEmitPointerOffset_2 emitPointerOffset;

        SlangNVVMGetArrayType_2 getArrayType;
        SlangNVVMEmitArrayElementPointer_2 emitArrayElementPointer;

        SlangNVVMEmitIntegerMultiply_2 emitIntegerMultiply;

        SlangNVVMEmitIntegerBitAnd_2 emitIntegerBitAnd;

        SlangNVVMEmitIntegerBitOr_2 emitIntegerBitOr;

        SlangNVVMEmitIntegerBitXor_2 emitIntegerBitXor;

        SlangNVVMEmitIntegerBitNot_2 emitIntegerBitNot;

        SlangNVVMEmitIntegerNegate_2 emitIntegerNegate;

        SlangNVVMEmitRelaxedGlobalI32AtomicAdd_2 emitRelaxedGlobalI32AtomicAdd;

        /// Serializes only SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY.
        SlangNVVMSerializeModuleWithDiagnostics_2 serializeNVVMIR20AssemblyWithDiagnostics;

        SlangNVVMEmitIntegerEqual_2 emitIntegerEqual;

        SlangNVVMEmitIntegerNotEqual_2 emitIntegerNotEqual;

        SlangNVVMEmitIntegerSignedGreaterThan_2 emitIntegerSignedGreaterThan;

        SlangNVVMEmitIntegerSignedLessEqual_2 emitIntegerSignedLessEqual;

        SlangNVVMEmitIntegerSignedGreaterEqual_2 emitIntegerSignedGreaterEqual;

        SlangNVVMGetRawRWStructuredBufferI32Type_2 getRawRWStructuredBufferI32Type;
        SlangNVVMEmitRawRWStructuredBufferI32ElementPointer_2
            emitRawRWStructuredBufferI32ElementPointer;
    } SlangNVVMBuilderAPI_V2;

    // This Slice 3b prefix size is frozen; appending fields must not change the minimum.
#define SLANG_NVVM_BUILDER_API_V2_MIN_SIZE                              \
    (offsetof(SlangNVVMBuilderAPI_V2, serializeModuleWithDiagnostics) + \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->serializeModuleWithDiagnostics))

    // This Slice 4 prefix is one coherent scalar-memory capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitStore) + sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitStore))

    // This Slice 7 prefix is one coherent scalar-control-flow capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitConditionalBranch) + \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitConditionalBranch))

    // This Slice 8 prefix is one coherent scalar-SSA capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE          \
    (offsetof(SlangNVVMBuilderAPI_V2, addIntegerPhiIncoming) + \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->addIntegerPhiIncoming))

    // This Slice 9 prefix is one coherent scalar-function capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerReturn) + \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerReturn))

    // This Slice 10 prefix is one coherent scalar-pointer-arithmetic capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitPointerOffset) +           \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitPointerOffset))

    // This Slice 11 prefix is one coherent scalar-array-addressing capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE          \
    (offsetof(SlangNVVMBuilderAPI_V2, emitArrayElementPointer) + \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitArrayElementPointer))

    // This Slice 12 prefix is one coherent scalar-integer-multiply capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerMultiply) +       \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerMultiply))

    // This Slice 13 prefix is one coherent scalar-integer-bit-AND capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitAnd) +        \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerBitAnd))

    // This Slice 14 prefix is one coherent scalar-integer-bit-OR capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitOr) +        \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerBitOr))

    // This Slice 15 prefix is one coherent scalar-integer-bit-XOR capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitXor) +        \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerBitXor))

    // This Slice 16 prefix is one coherent scalar-integer-bit-NOT capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitNot) +        \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerBitNot))

    // This Slice 17 prefix is one coherent scalar-integer-negate capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerNegate) +       \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerNegate))

    // This Slice 19 prefix is one coherent relaxed global-i32 atomic-add and NVVM IR 2.0
    // assembly capability. The operation requires the matching libNVVM wire dialect.
#define SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE          \
    (offsetof(SlangNVVMBuilderAPI_V2, serializeNVVMIR20AssemblyWithDiagnostics) + \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->serializeNVVMIR20AssemblyWithDiagnostics))

    // This Slice 21 prefix is one coherent scalar-integer-equality capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_EQUAL_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerEqual) +       \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerEqual))

    // This Slice 22 prefix is one coherent scalar-integer-inequality capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NOT_EQUAL_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerNotEqual) +        \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerNotEqual))

    // This Slice 23 prefix is one coherent scalar-integer-signed-greater-than capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_THAN_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerSignedGreaterThan) +         \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerSignedGreaterThan))

    // This Slice 24 prefix is one coherent scalar-integer-signed-less-equal capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_LESS_EQUAL_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerSignedLessEqual) +         \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerSignedLessEqual))

    // This Slice 25 prefix is one coherent scalar-integer-signed-greater-equal capability.
#define SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V2, emitIntegerSignedGreaterEqual) +         \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitIntegerSignedGreaterEqual))

    // This Slice 26 prefix is one coherent raw-CUDA RWStructuredBuffer<i32> capability.
#define SLANG_NVVM_BUILDER_API_V2_RAW_RW_STRUCTURED_BUFFER_I32_MIN_SIZE             \
    (offsetof(SlangNVVMBuilderAPI_V2, emitRawRWStructuredBufferI32ElementPointer) + \
     sizeof(((SlangNVVMBuilderAPI_V2*)0)->emitRawRWStructuredBufferI32ElementPointer))

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangGetNVVMBuilderAPI_V2)(
        SlangNVVMBuilderAPI_V2* outAPI);

    /** Stable semantic features advertised by the V3 builder table. */
    typedef uint32_t SlangNVVMBuilderFeature_3;
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY ((SlangNVVMBuilderFeature_3)0u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW ((SlangNVVMBuilderFeature_3)1u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_SSA ((SlangNVVMBuilderFeature_3)2u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FUNCTIONS ((SlangNVVMBuilderFeature_3)3u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_POINTER_ARITHMETIC ((SlangNVVMBuilderFeature_3)4u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_ARRAY_ADDRESSING ((SlangNVVMBuilderFeature_3)5u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY ((SlangNVVMBuilderFeature_3)6u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_AND ((SlangNVVMBuilderFeature_3)7u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_OR ((SlangNVVMBuilderFeature_3)8u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_XOR ((SlangNVVMBuilderFeature_3)9u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_NOT ((SlangNVVMBuilderFeature_3)10u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NEGATE ((SlangNVVMBuilderFeature_3)11u)
#define SLANG_NVVM_BUILDER_FEATURE_RELAXED_GLOBAL_I32_ATOMIC_ADD ((SlangNVVMBuilderFeature_3)12u)
#define SLANG_NVVM_BUILDER_FEATURE_NVVM_IR_2_0_ASSEMBLY ((SlangNVVMBuilderFeature_3)13u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_EQUAL ((SlangNVVMBuilderFeature_3)14u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NOT_EQUAL ((SlangNVVMBuilderFeature_3)15u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_THAN \
    ((SlangNVVMBuilderFeature_3)16u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_LESS_EQUAL ((SlangNVVMBuilderFeature_3)17u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_EQUAL \
    ((SlangNVVMBuilderFeature_3)18u)
#define SLANG_NVVM_BUILDER_FEATURE_RAW_RW_STRUCTURED_BUFFER_I32 ((SlangNVVMBuilderFeature_3)19u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD ((SlangNVVMBuilderFeature_3)20u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT ((SlangNVVMBuilderFeature_3)21u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY ((SlangNVVMBuilderFeature_3)22u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_DIVIDE ((SlangNVVMBuilderFeature_3)23u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NEGATE ((SlangNVVMBuilderFeature_3)24u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_EQUAL ((SlangNVVMBuilderFeature_3)25u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NOT_EQUAL ((SlangNVVMBuilderFeature_3)26u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_THAN \
    ((SlangNVVMBuilderFeature_3)27u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_EQUAL \
    ((SlangNVVMBuilderFeature_3)28u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_EQUAL \
    ((SlangNVVMBuilderFeature_3)29u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_THAN ((SlangNVVMBuilderFeature_3)30u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT ((SlangNVVMBuilderFeature_3)31u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI ((SlangNVVMBuilderFeature_3)32u)
#define SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS ((SlangNVVMBuilderFeature_3)33u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX ((SlangNVVMBuilderFeature_3)34u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT ((SlangNVVMBuilderFeature_3)35u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT ((SlangNVVMBuilderFeature_3)36u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT ((SlangNVVMBuilderFeature_3)37u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT ((SlangNVVMBuilderFeature_3)38u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT ((SlangNVVMBuilderFeature_3)39u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_UINT ((SlangNVVMBuilderFeature_3)40u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_INT ((SlangNVVMBuilderFeature_3)41u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_FLOAT ((SlangNVVMBuilderFeature_3)42u)
#define SLANG_NVVM_BUILDER_FEATURE_COUNT_3 43u
#define SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT_3 4u

    typedef struct SlangNVVMBuilderFeatureSet_3
    {
        uint64_t words[SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT_3];
    } SlangNVVMBuilderFeatureSet_3;

    typedef uint32_t SlangNVVMIntegerUnaryOp_3;
#define SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT ((SlangNVVMIntegerUnaryOp_3)0u)
#define SLANG_NVVM_INTEGER_UNARY_OP_NEGATE ((SlangNVVMIntegerUnaryOp_3)1u)

    typedef uint32_t SlangNVVMIntegerBinaryOp_3;
#define SLANG_NVVM_INTEGER_BINARY_OP_3_ADD ((SlangNVVMIntegerBinaryOp_3)0u)
#define SLANG_NVVM_INTEGER_BINARY_OP_3_SUB ((SlangNVVMIntegerBinaryOp_3)1u)
#define SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY ((SlangNVVMIntegerBinaryOp_3)2u)
#define SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_AND ((SlangNVVMIntegerBinaryOp_3)3u)
#define SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_OR ((SlangNVVMIntegerBinaryOp_3)4u)
#define SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_XOR ((SlangNVVMIntegerBinaryOp_3)5u)

    typedef uint32_t SlangNVVMIntegerCompareOp_3;
#define SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN ((SlangNVVMIntegerCompareOp_3)0u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL ((SlangNVVMIntegerCompareOp_3)1u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL ((SlangNVVMIntegerCompareOp_3)2u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN ((SlangNVVMIntegerCompareOp_3)3u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL ((SlangNVVMIntegerCompareOp_3)4u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL ((SlangNVVMIntegerCompareOp_3)5u)

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerUnary_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMIntegerUnaryOp_3 operation,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1* outValue);

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerBinary_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMIntegerBinaryOp_3 operation,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntegerCompare_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMIntegerCompareOp_3 operation,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    typedef uint32_t SlangNVVMFloatingBinaryOp_3;
#define SLANG_NVVM_FLOATING_BINARY_OP_ADD ((SlangNVVMFloatingBinaryOp_3)0u)
#define SLANG_NVVM_FLOATING_BINARY_OP_SUBTRACT ((SlangNVVMFloatingBinaryOp_3)1u)
#define SLANG_NVVM_FLOATING_BINARY_OP_MULTIPLY ((SlangNVVMFloatingBinaryOp_3)2u)
#define SLANG_NVVM_FLOATING_BINARY_OP_DIVIDE ((SlangNVVMFloatingBinaryOp_3)3u)

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMGetFloatingPointType_3)(
        SlangNVVMModuleHandle_1 module,
        uint32_t bitWidth,
        SlangNVVMTypeHandle_1* outType);

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitFloatingBinary_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMFloatingBinaryOp_3 operation,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    typedef uint32_t SlangNVVMFloatingUnaryOp_3;
#define SLANG_NVVM_FLOATING_UNARY_OP_NEGATE ((SlangNVVMFloatingUnaryOp_3)0u)

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitFloatingUnary_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMFloatingUnaryOp_3 operation,
        SlangNVVMValueHandle_1 value,
        SlangNVVMValueHandle_1* outValue);

    typedef uint32_t SlangNVVMFloatingCompareOp_3;
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_EQUAL ((SlangNVVMFloatingCompareOp_3)0u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_UNORDERED_NOT_EQUAL ((SlangNVVMFloatingCompareOp_3)1u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_THAN ((SlangNVVMFloatingCompareOp_3)2u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_EQUAL ((SlangNVVMFloatingCompareOp_3)3u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_EQUAL ((SlangNVVMFloatingCompareOp_3)4u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_THAN ((SlangNVVMFloatingCompareOp_3)5u)

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitFloatingCompare_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMFloatingCompareOp_3 operation,
        SlangNVVMValueHandle_1 left,
        SlangNVVMValueHandle_1 right,
        SlangNVVMValueHandle_1* outValue);

    /// Gets an exact floating-point constant from its width-bounded IEEE-754 bit pattern.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMGetFloatingPointConstant_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 floatingPointType,
        uint32_t bitWidth,
        uint64_t bitPattern,
        SlangNVVMValueHandle_1* outValue);

    /// Creates a typed scalar phi at the start of the explicit target block.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitPhi_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMBlockHandle_1 targetBlock,
        SlangNVVMTypeHandle_1 type,
        SlangNVVMValueHandle_1* outValue);

    /// Adds one exact-typed scalar phi input from a predecessor edge.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMAddPhiIncoming_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 phi,
        SlangNVVMValueHandle_1 value,
        SlangNVVMBlockHandle_1 predecessorBlock);

    /// Emits a direct call to a same-module, non-variadic scalar function.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitCall_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 callee,
        const SlangNVVMValueHandle_1* arguments,
        size_t argumentCount,
        SlangNVVMValueHandle_1* outValue);

    /// Terminates the current unterminated insertion block with a scalar return value.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitValueReturn_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 value);

    /** Stable target-intrinsic operations accepted by the generic V3 callback. */
    typedef uint32_t SlangNVVMIntrinsicOp_3;
#define SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_INDEX ((SlangNVVMIntrinsicOp_3)0u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_COUNT ((SlangNVVMIntrinsicOp_3)1u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_UINT ((SlangNVVMIntrinsicOp_3)2u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_INT ((SlangNVVMIntrinsicOp_3)3u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_FLOAT ((SlangNVVMIntrinsicOp_3)4u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_BALLOT ((SlangNVVMIntrinsicOp_3)5u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_UINT ((SlangNVVMIntrinsicOp_3)6u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_INT ((SlangNVVMIntrinsicOp_3)7u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_FLOAT ((SlangNVVMIntrinsicOp_3)8u)

    /// Emits one stable target intrinsic with its exact operation-defined scalar signature.
    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangNVVMEmitIntrinsic_3)(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMIntrinsicOp_3 operation,
        const SlangNVVMValueHandle_1* arguments,
        size_t argumentCount,
        SlangNVVMValueHandle_1* outValue);

    // Version 3 freezes V2 as its compatibility core. Generic callbacks and independent feature
    // bits carry forward-growing operation families and semantic availability.
    typedef struct SlangNVVMBuilderAPI_V3
    {
        uint32_t structureSize;
        uint32_t abiVersion;
        SlangNVVMBuilderAPI_V2 compatibilityAPI;
        SlangNVVMBuilderFeatureSet_3 features;
        SlangNVVMEmitIntegerUnary_3 emitIntegerUnary;
        SlangNVVMEmitIntegerBinary_3 emitIntegerBinary;
        SlangNVVMEmitIntegerCompare_3 emitIntegerCompare;
        SlangNVVMGetFloatingPointType_3 getFloatingPointType;
        SlangNVVMEmitFloatingBinary_3 emitFloatingBinary;
        SlangNVVMEmitFloatingUnary_3 emitFloatingUnary;
        SlangNVVMEmitFloatingCompare_3 emitFloatingCompare;
        SlangNVVMGetFloatingPointConstant_3 getFloatingPointConstant;
        SlangNVVMEmitPhi_3 emitPhi;
        SlangNVVMAddPhiIncoming_3 addPhiIncoming;
        SlangNVVMEmitCall_3 emitCall;
        SlangNVVMEmitValueReturn_3 emitValueReturn;
        SlangNVVMEmitIntrinsic_3 emitIntrinsic;
    } SlangNVVMBuilderAPI_V3;

#define SLANG_NVVM_BUILDER_API_V3_MIN_SIZE                  \
    (offsetof(SlangNVVMBuilderAPI_V3, emitIntegerCompare) + \
     sizeof(((SlangNVVMBuilderAPI_V3*)0)->emitIntegerCompare))

#define SLANG_NVVM_BUILDER_API_V3_SCALAR_FLOAT32_ADD_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V3, emitFloatingBinary) +   \
     sizeof(((SlangNVVMBuilderAPI_V3*)0)->emitFloatingBinary))

#define SLANG_NVVM_BUILDER_API_V3_SCALAR_FLOAT32_NEGATE_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V3, emitFloatingUnary) +       \
     sizeof(((SlangNVVMBuilderAPI_V3*)0)->emitFloatingUnary))

#define SLANG_NVVM_BUILDER_API_V3_FLOATING_COMPARE_MIN_SIZE  \
    (offsetof(SlangNVVMBuilderAPI_V3, emitFloatingCompare) + \
     sizeof(((SlangNVVMBuilderAPI_V3*)0)->emitFloatingCompare))

#define SLANG_NVVM_BUILDER_API_V3_SCALAR_FLOAT32_EQUAL_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_FLOATING_COMPARE_MIN_SIZE

#define SLANG_NVVM_BUILDER_API_V3_FLOATING_CONSTANT_MIN_SIZE      \
    (offsetof(SlangNVVMBuilderAPI_V3, getFloatingPointConstant) + \
     sizeof(((SlangNVVMBuilderAPI_V3*)0)->getFloatingPointConstant))

#define SLANG_NVVM_BUILDER_API_V3_SCALAR_FLOAT32_CONSTANT_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_FLOATING_CONSTANT_MIN_SIZE

#define SLANG_NVVM_BUILDER_API_V3_SCALAR_PHI_MIN_SIZE   \
    (offsetof(SlangNVVMBuilderAPI_V3, addPhiIncoming) + \
     sizeof(((SlangNVVMBuilderAPI_V3*)0)->addPhiIncoming))

#define SLANG_NVVM_BUILDER_API_V3_GENERIC_SCALAR_FUNCTIONS_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V3, emitValueReturn) +            \
     sizeof(((SlangNVVMBuilderAPI_V3*)0)->emitValueReturn))

#define SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE \
    (offsetof(SlangNVVMBuilderAPI_V3, emitIntrinsic) +     \
     sizeof(((SlangNVVMBuilderAPI_V3*)0)->emitIntrinsic))

#define SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_COUNT_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE

#define SLANG_NVVM_BUILDER_API_V3_WAVE_READ_LANE_AT_UINT_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE

#define SLANG_NVVM_BUILDER_API_V3_WAVE_READ_LANE_AT_INT_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE

#define SLANG_NVVM_BUILDER_API_V3_WAVE_READ_LANE_AT_FLOAT_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE

#define SLANG_NVVM_BUILDER_API_V3_WAVE_MASK_BALLOT_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE

#define SLANG_NVVM_BUILDER_API_V3_WAVE_READ_LANE_FIRST_UINT_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE

#define SLANG_NVVM_BUILDER_API_V3_WAVE_READ_LANE_FIRST_INT_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE

#define SLANG_NVVM_BUILDER_API_V3_WAVE_READ_LANE_FIRST_FLOAT_MIN_SIZE \
    SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangGetNVVMBuilderAPI_V3)(
        SlangNVVMBuilderAPI_V3* outAPI);

    SLANG_NVVM_BUILDER_API SlangNVVMResult_1 SLANG_NVVM_CALL
    slang_getNVVMBuilderAPI_V1(SlangNVVMBuilderAPI_V1* outAPI);

    SLANG_NVVM_BUILDER_API SlangNVVMResult_1 SLANG_NVVM_CALL
    slang_getNVVMBuilderAPI_V2(SlangNVVMBuilderAPI_V2* outAPI);

    SLANG_NVVM_BUILDER_API SlangNVVMResult_1 SLANG_NVVM_CALL
    slang_getNVVMBuilderAPI_V3(SlangNVVMBuilderAPI_V3* outAPI);

#ifdef __cplusplus
}
#endif

#endif
