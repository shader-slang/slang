#ifndef SLANG_NVVM_IR_BUILDER_API_H
#define SLANG_NVVM_IR_BUILDER_API_H

#include <stddef.h>
#include <stdint.h>

#define SLANG_NVVM_BUILDER_ABI_REVISION 2u
#define SLANG_NVVM_BUILDER_GET_API_NAME "slang_getNVVMBuilderAPI"

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

    typedef struct SlangNVVMModule* SlangNVVMModuleHandle;
    typedef struct SlangNVVMType* SlangNVVMTypeHandle;
    typedef struct SlangNVVMValue* SlangNVVMValueHandle;
    typedef struct SlangNVVMBlock* SlangNVVMBlockHandle;

    /** Uses Slang's signed 32-bit result convention: negative values fail, other values succeed. */
    typedef int32_t SlangNVVMResult;

    typedef uint32_t SlangNVVMPointerModel;
#define SLANG_NVVM_POINTER_MODEL_TYPED ((SlangNVVMPointerModel)1u)

    typedef uint32_t SlangNVVMSerializationFormat;
#define SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY ((SlangNVVMSerializationFormat)0u)
#define SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE ((SlangNVVMSerializationFormat)1u)
/** LLVM assembly in the LLVM 7-era NVVM IR 2.0 dialect accepted by libNVVM. */
#define SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY ((SlangNVVMSerializationFormat)2u)

    typedef uint32_t SlangNVVMVerificationStatus;
#define SLANG_NVVM_VERIFICATION_NOT_RUN ((SlangNVVMVerificationStatus)0u)
#define SLANG_NVVM_VERIFICATION_VALID ((SlangNVVMVerificationStatus)1u)
#define SLANG_NVVM_VERIFICATION_INVALID ((SlangNVVMVerificationStatus)2u)

    typedef uint32_t SlangNVVMAddressSpace;
#define SLANG_NVVM_ADDRESS_SPACE_GENERIC ((SlangNVVMAddressSpace)0u)
#define SLANG_NVVM_ADDRESS_SPACE_GLOBAL ((SlangNVVMAddressSpace)1u)
#define SLANG_NVVM_ADDRESS_SPACE_SHARED ((SlangNVVMAddressSpace)3u)
#define SLANG_NVVM_ADDRESS_SPACE_CONSTANT ((SlangNVVMAddressSpace)4u)
#define SLANG_NVVM_ADDRESS_SPACE_LOCAL ((SlangNVVMAddressSpace)5u)

    typedef uint32_t SlangNVVMGlobalLinkage;
#define SLANG_NVVM_GLOBAL_LINKAGE_INTERNAL ((SlangNVVMGlobalLinkage)0u)
#define SLANG_NVVM_GLOBAL_LINKAGE_EXTERNAL ((SlangNVVMGlobalLinkage)1u)

    typedef uint32_t SlangNVVMBuilderInterfaceID;
#define SLANG_NVVM_BUILDER_INTERFACE_FOUNDATION ((SlangNVVMBuilderInterfaceID)0u)
#define SLANG_NVVM_BUILDER_INTERFACE_CONSTRUCTION ((SlangNVVMBuilderInterfaceID)1u)
#define SLANG_NVVM_BUILDER_INTERFACE_VALUE_OPERATIONS ((SlangNVVMBuilderInterfaceID)2u)

    /** Semantic scalar and fixed-vector categories used by operation signatures. */
    typedef uint32_t SlangNVVMValueTypeKind;
#define SLANG_NVVM_VALUE_TYPE_VOID ((SlangNVVMValueTypeKind)0u)
#define SLANG_NVVM_VALUE_TYPE_BOOL ((SlangNVVMValueTypeKind)1u)
#define SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ((SlangNVVMValueTypeKind)2u)
#define SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER ((SlangNVVMValueTypeKind)3u)
#define SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ((SlangNVVMValueTypeKind)4u)

    typedef struct SlangNVVMValueTypeDesc
    {
        SlangNVVMValueTypeKind kind;
        uint32_t bitWidth;
        uint32_t laneCount;
    } SlangNVVMValueTypeDesc;

    typedef uint32_t SlangNVVMValueOperation;
#define SLANG_NVVM_VALUE_OP_ADD ((SlangNVVMValueOperation)0u)
#define SLANG_NVVM_VALUE_OP_SUBTRACT ((SlangNVVMValueOperation)1u)
#define SLANG_NVVM_VALUE_OP_MULTIPLY ((SlangNVVMValueOperation)2u)
#define SLANG_NVVM_VALUE_OP_DIVIDE ((SlangNVVMValueOperation)3u)
#define SLANG_NVVM_VALUE_OP_BIT_AND ((SlangNVVMValueOperation)4u)
#define SLANG_NVVM_VALUE_OP_BIT_OR ((SlangNVVMValueOperation)5u)
#define SLANG_NVVM_VALUE_OP_BIT_XOR ((SlangNVVMValueOperation)6u)
#define SLANG_NVVM_VALUE_OP_BIT_NOT ((SlangNVVMValueOperation)7u)
#define SLANG_NVVM_VALUE_OP_NEGATE ((SlangNVVMValueOperation)8u)
#define SLANG_NVVM_VALUE_OP_EQUAL ((SlangNVVMValueOperation)9u)
#define SLANG_NVVM_VALUE_OP_NOT_EQUAL ((SlangNVVMValueOperation)10u)
#define SLANG_NVVM_VALUE_OP_LESS_THAN ((SlangNVVMValueOperation)11u)
#define SLANG_NVVM_VALUE_OP_GREATER_THAN ((SlangNVVMValueOperation)12u)
#define SLANG_NVVM_VALUE_OP_LESS_EQUAL ((SlangNVVMValueOperation)13u)
#define SLANG_NVVM_VALUE_OP_GREATER_EQUAL ((SlangNVVMValueOperation)14u)
#define SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX ((SlangNVVMValueOperation)15u)
#define SLANG_NVVM_VALUE_OP_WAVE_LANE_COUNT ((SlangNVVMValueOperation)16u)
#define SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT ((SlangNVVMValueOperation)17u)
#define SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT ((SlangNVVMValueOperation)18u)
#define SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST ((SlangNVVMValueOperation)19u)
#define SLANG_NVVM_VALUE_OP_WAVE_MASK_IS_FIRST_LANE ((SlangNVVMValueOperation)20u)
#define SLANG_NVVM_VALUE_OP_WAVE_MASK_ANY_TRUE ((SlangNVVMValueOperation)21u)
#define SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_TRUE ((SlangNVVMValueOperation)22u)
#define SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL ((SlangNVVMValueOperation)23u)
#define SLANG_NVVM_VALUE_OP_THREAD_INDEX ((SlangNVVMValueOperation)24u)
#define SLANG_NVVM_VALUE_OP_BLOCK_INDEX ((SlangNVVMValueOperation)25u)
#define SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS ((SlangNVVMValueOperation)26u)
#define SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS ((SlangNVVMValueOperation)27u)
#define SLANG_NVVM_VALUE_OP_WORKGROUP_BARRIER ((SlangNVVMValueOperation)28u)
#define SLANG_NVVM_VALUE_OP_INTEGER_CONVERT ((SlangNVVMValueOperation)29u)
#define SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT ((SlangNVVMValueOperation)30u)
#define SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER ((SlangNVVMValueOperation)31u)
#define SLANG_NVVM_VALUE_OPERATION_COUNT 32u

    /** Describes one complete semantic value-operation overload. */
    typedef struct SlangNVVMValueOperationDesc
    {
        SlangNVVMValueOperation operation;
        SlangNVVMValueTypeDesc resultType;
        const SlangNVVMValueTypeDesc* operandTypes;
        size_t operandCount;
    } SlangNVVMValueOperationDesc;

    /** Owns module lifetime and verified serialization. */
    typedef struct SlangNVVMBuilderFoundationAPI
    {
        SlangNVVMResult(SLANG_NVVM_CALL* createModule)(
            const char* moduleName,
            size_t moduleNameSize,
            SlangNVVMModuleHandle* outModule);
        void(SLANG_NVVM_CALL* destroyModule)(SlangNVVMModuleHandle module);
        SlangNVVMResult(SLANG_NVVM_CALL* serializeModuleWithDiagnostics)(
            SlangNVVMModuleHandle module,
            SlangNVVMSerializationFormat format,
            void* serializedDestination,
            size_t serializedDestinationSize,
            size_t* outSerializedSize,
            void* diagnosticDestination,
            size_t diagnosticDestinationSize,
            size_t* outDiagnosticSize,
            SlangNVVMVerificationStatus* outVerificationStatus);
        SlangNVVMResult(SLANG_NVVM_CALL* serializeNVVMIR20AssemblyWithDiagnostics)(
            SlangNVVMModuleHandle module,
            SlangNVVMSerializationFormat format,
            void* serializedDestination,
            size_t serializedDestinationSize,
            size_t* outSerializedSize,
            void* diagnosticDestination,
            size_t diagnosticDestinationSize,
            size_t* outDiagnosticSize,
            SlangNVVMVerificationStatus* outVerificationStatus);
    } SlangNVVMBuilderFoundationAPI;

    /** Owns structural IR construction. Every callback is required by the current ABI. */
    typedef struct SlangNVVMBuilderConstructionAPI
    {
        SlangNVVMResult(SLANG_NVVM_CALL* getVoidType)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* getIntegerType)(
            SlangNVVMModuleHandle module,
            uint32_t bitWidth,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* getFloatingPointType)(
            SlangNVVMModuleHandle module,
            uint32_t bitWidth,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* getPointerType)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle pointeeType,
            SlangNVVMAddressSpace addressSpace,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* getFunctionType)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle resultType,
            const SlangNVVMTypeHandle* parameterTypes,
            size_t parameterCount,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* getArrayType)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle elementType,
            uint32_t elementCount,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* getVectorType)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle elementType,
            uint32_t elementCount,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* getStructType)(
            SlangNVVMModuleHandle module,
            const SlangNVVMTypeHandle* fieldTypes,
            size_t fieldCount,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* getRawRWStructuredBufferI32Type)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* declareFunction)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle functionType,
            const char* name,
            size_t nameSize,
            SlangNVVMValueHandle* outFunction);
        SlangNVVMResult(SLANG_NVVM_CALL* getFunctionParameter)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle function,
            size_t parameterIndex,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* createBlock)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle function,
            const char* name,
            size_t nameSize,
            SlangNVVMBlockHandle* outBlock);
        SlangNVVMResult(SLANG_NVVM_CALL* setInsertBlock)(
            SlangNVVMModuleHandle module,
            SlangNVVMBlockHandle block);
        SlangNVVMResult(SLANG_NVVM_CALL* emitLoad)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle pointer,
            uint32_t alignment,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* emitStore)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle value,
            SlangNVVMValueHandle pointer,
            uint32_t alignment);
        SlangNVVMResult(SLANG_NVVM_CALL* emitBranch)(
            SlangNVVMModuleHandle module,
            SlangNVVMBlockHandle targetBlock);
        SlangNVVMResult(SLANG_NVVM_CALL* emitConditionalBranch)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle condition,
            SlangNVVMBlockHandle trueBlock,
            SlangNVVMBlockHandle falseBlock);
        SlangNVVMResult(SLANG_NVVM_CALL* getIntegerConstant)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle integerType,
            int64_t value,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* getFloatingPointConstant)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle floatingPointType,
            uint32_t bitWidth,
            uint64_t bitPattern,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* emitPhi)(
            SlangNVVMModuleHandle module,
            SlangNVVMBlockHandle targetBlock,
            SlangNVVMTypeHandle type,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* addPhiIncoming)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle phi,
            SlangNVVMValueHandle value,
            SlangNVVMBlockHandle predecessorBlock);
        SlangNVVMResult(SLANG_NVVM_CALL* emitCall)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle callee,
            const SlangNVVMValueHandle* arguments,
            size_t argumentCount,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* emitValueReturn)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle value);
        SlangNVVMResult(SLANG_NVVM_CALL* emitReturnVoid)(SlangNVVMModuleHandle module);
        SlangNVVMResult(SLANG_NVVM_CALL* emitPointerOffset)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle basePointer,
            SlangNVVMValueHandle elementOffset,
            SlangNVVMValueHandle* outPointer);
        SlangNVVMResult(SLANG_NVVM_CALL* emitArrayElementPointer)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle baseArrayPointer,
            SlangNVVMValueHandle elementIndex,
            SlangNVVMValueHandle* outPointer);
        SlangNVVMResult(SLANG_NVVM_CALL* emitStructFieldPointer)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle baseStructPointer,
            uint32_t fieldIndex,
            SlangNVVMValueHandle* outPointer);
        SlangNVVMResult(SLANG_NVVM_CALL* emitVectorElementExtract)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle vector,
            uint32_t elementIndex,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* emitRawRWStructuredBufferI32ElementPointer)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle buffer,
            SlangNVVMValueHandle elementIndex,
            SlangNVVMValueHandle* outPointer);
        SlangNVVMResult(SLANG_NVVM_CALL* emitRelaxedGlobalI32AtomicAdd)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle pointer,
            SlangNVVMValueHandle value,
            SlangNVVMValueHandle* outOriginalValue);
        SlangNVVMResult(SLANG_NVVM_CALL* declareGlobalStorage)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle valueType,
            SlangNVVMGlobalLinkage linkage,
            SlangNVVMAddressSpace addressSpace,
            uint32_t alignment,
            const char* name,
            size_t nameSize,
            SlangNVVMValueHandle* outStorage);
        SlangNVVMResult(SLANG_NVVM_CALL* markFunctionAsKernel)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle function);
    } SlangNVVMBuilderConstructionAPI;

    typedef struct SlangNVVMBuilderValueOperationsAPI
    {
        SlangNVVMResult(SLANG_NVVM_CALL* isOperationSupported)(
            const SlangNVVMValueOperationDesc* operation,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitOperation)(
            SlangNVVMModuleHandle module,
            const SlangNVVMValueOperationDesc* operation,
            const SlangNVVMValueHandle* operands,
            size_t operandCount,
            SlangNVVMValueHandle* outValue);
    } SlangNVVMBuilderValueOperationsAPI;

    typedef SlangNVVMResult(SLANG_NVVM_CALL* SlangNVVMQueryBuilderInterface)(
        SlangNVVMBuilderInterfaceID interfaceID,
        const void** outInterface);

    /** Exact current root table. Its metadata is part of cache identity and compatibility checks.
     */
    typedef struct SlangNVVMBuilderAPI
    {
        uint32_t llvmVersionMajor;
        uint32_t llvmVersionMinor;
        uint32_t llvmVersionPatch;
        uint32_t nvvmIRVersionMajor;
        uint32_t nvvmIRVersionMinor;
        uint32_t pointerModel;
        SlangNVVMQueryBuilderInterface queryInterface;
    } SlangNVVMBuilderAPI;

    typedef SlangNVVMResult(
        SLANG_NVVM_CALL* SlangGetNVVMBuilderAPI)(uint32_t abiRevision, SlangNVVMBuilderAPI* outAPI);

    SLANG_NVVM_BUILDER_API SlangNVVMResult SLANG_NVVM_CALL
    slang_getNVVMBuilderAPI(uint32_t abiRevision, SlangNVVMBuilderAPI* outAPI);

#ifdef __cplusplus
}
#endif

#endif
