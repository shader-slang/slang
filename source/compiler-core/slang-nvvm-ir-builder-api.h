#ifndef SLANG_NVVM_IR_BUILDER_API_H
#define SLANG_NVVM_IR_BUILDER_API_H

#include <stddef.h>
#include <stdint.h>

#define SLANG_NVVM_BUILDER_ABI_VERSION_1 1u
#define SLANG_NVVM_BUILDER_ABI_VERSION_2 2u
#define SLANG_NVVM_BUILDER_GET_API_V1_NAME "slang_getNVVMBuilderAPI_V1"
#define SLANG_NVVM_BUILDER_GET_API_V2_NAME "slang_getNVVMBuilderAPI_V2"

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

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangGetNVVMBuilderAPI_V2)(
        SlangNVVMBuilderAPI_V2* outAPI);

    SLANG_NVVM_BUILDER_API SlangNVVMResult_1 SLANG_NVVM_CALL
    slang_getNVVMBuilderAPI_V1(SlangNVVMBuilderAPI_V1* outAPI);

    SLANG_NVVM_BUILDER_API SlangNVVMResult_1 SLANG_NVVM_CALL
    slang_getNVVMBuilderAPI_V2(SlangNVVMBuilderAPI_V2* outAPI);

#ifdef __cplusplus
}
#endif

#endif
