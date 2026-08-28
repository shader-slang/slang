#ifndef SLANG_NVVM_IR_BUILDER_API_H
#define SLANG_NVVM_IR_BUILDER_API_H

#include <stddef.h>
#include <stdint.h>

#define SLANG_NVVM_BUILDER_ABI_REVISION 1u
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

    // These feature IDs are retained only until the exact requirement conversion in Slice 70.
    typedef uint32_t SlangNVVMBuilderFeature;
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY ((SlangNVVMBuilderFeature)0u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW ((SlangNVVMBuilderFeature)1u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_SSA ((SlangNVVMBuilderFeature)2u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FUNCTIONS ((SlangNVVMBuilderFeature)3u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_POINTER_ARITHMETIC ((SlangNVVMBuilderFeature)4u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_ARRAY_ADDRESSING ((SlangNVVMBuilderFeature)5u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY ((SlangNVVMBuilderFeature)6u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_AND ((SlangNVVMBuilderFeature)7u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_OR ((SlangNVVMBuilderFeature)8u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_XOR ((SlangNVVMBuilderFeature)9u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_NOT ((SlangNVVMBuilderFeature)10u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NEGATE ((SlangNVVMBuilderFeature)11u)
#define SLANG_NVVM_BUILDER_FEATURE_RELAXED_GLOBAL_I32_ATOMIC_ADD ((SlangNVVMBuilderFeature)12u)
#define SLANG_NVVM_BUILDER_FEATURE_NVVM_IR_2_0_ASSEMBLY ((SlangNVVMBuilderFeature)13u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_EQUAL ((SlangNVVMBuilderFeature)14u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NOT_EQUAL ((SlangNVVMBuilderFeature)15u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_THAN ((SlangNVVMBuilderFeature)16u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_LESS_EQUAL ((SlangNVVMBuilderFeature)17u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_EQUAL \
    ((SlangNVVMBuilderFeature)18u)
#define SLANG_NVVM_BUILDER_FEATURE_RAW_RW_STRUCTURED_BUFFER_I32 ((SlangNVVMBuilderFeature)19u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD ((SlangNVVMBuilderFeature)20u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT ((SlangNVVMBuilderFeature)21u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY ((SlangNVVMBuilderFeature)22u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_DIVIDE ((SlangNVVMBuilderFeature)23u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NEGATE ((SlangNVVMBuilderFeature)24u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_EQUAL ((SlangNVVMBuilderFeature)25u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NOT_EQUAL ((SlangNVVMBuilderFeature)26u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_THAN \
    ((SlangNVVMBuilderFeature)27u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_EQUAL ((SlangNVVMBuilderFeature)28u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_EQUAL \
    ((SlangNVVMBuilderFeature)29u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_THAN ((SlangNVVMBuilderFeature)30u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT ((SlangNVVMBuilderFeature)31u)
#define SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI ((SlangNVVMBuilderFeature)32u)
#define SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS ((SlangNVVMBuilderFeature)33u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX ((SlangNVVMBuilderFeature)34u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT ((SlangNVVMBuilderFeature)35u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT ((SlangNVVMBuilderFeature)36u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT ((SlangNVVMBuilderFeature)37u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT ((SlangNVVMBuilderFeature)38u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT ((SlangNVVMBuilderFeature)39u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_UINT ((SlangNVVMBuilderFeature)40u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_INT ((SlangNVVMBuilderFeature)41u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_FLOAT ((SlangNVVMBuilderFeature)42u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_IS_FIRST_LANE ((SlangNVVMBuilderFeature)43u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ANY_TRUE ((SlangNVVMBuilderFeature)44u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_TRUE ((SlangNVVMBuilderFeature)45u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_INT ((SlangNVVMBuilderFeature)46u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_UINT ((SlangNVVMBuilderFeature)47u)
#define SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_FLOAT ((SlangNVVMBuilderFeature)48u)
#define SLANG_NVVM_BUILDER_FEATURE_COUNT 49u
#define SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT 4u

    typedef struct SlangNVVMBuilderFeatureSet
    {
        uint64_t words[SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT];
    } SlangNVVMBuilderFeatureSet;

    // These operation codes are temporary facade vocabulary removed by Slice 70.
    typedef uint32_t SlangNVVMIntegerUnaryOp;
#define SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT ((SlangNVVMIntegerUnaryOp)0u)
#define SLANG_NVVM_INTEGER_UNARY_OP_NEGATE ((SlangNVVMIntegerUnaryOp)1u)

    typedef uint32_t SlangNVVMIntegerBinaryOp;
#define SLANG_NVVM_INTEGER_BINARY_OP_ADD ((SlangNVVMIntegerBinaryOp)0u)
#define SLANG_NVVM_INTEGER_BINARY_OP_SUBTRACT ((SlangNVVMIntegerBinaryOp)1u)
#define SLANG_NVVM_INTEGER_BINARY_OP_MULTIPLY ((SlangNVVMIntegerBinaryOp)2u)
#define SLANG_NVVM_INTEGER_BINARY_OP_BIT_AND ((SlangNVVMIntegerBinaryOp)3u)
#define SLANG_NVVM_INTEGER_BINARY_OP_BIT_OR ((SlangNVVMIntegerBinaryOp)4u)
#define SLANG_NVVM_INTEGER_BINARY_OP_BIT_XOR ((SlangNVVMIntegerBinaryOp)5u)

    typedef uint32_t SlangNVVMIntegerCompareOp;
#define SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN ((SlangNVVMIntegerCompareOp)0u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL ((SlangNVVMIntegerCompareOp)1u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL ((SlangNVVMIntegerCompareOp)2u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN ((SlangNVVMIntegerCompareOp)3u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL ((SlangNVVMIntegerCompareOp)4u)
#define SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL ((SlangNVVMIntegerCompareOp)5u)

    typedef uint32_t SlangNVVMFloatingBinaryOp;
#define SLANG_NVVM_FLOATING_BINARY_OP_ADD ((SlangNVVMFloatingBinaryOp)0u)
#define SLANG_NVVM_FLOATING_BINARY_OP_SUBTRACT ((SlangNVVMFloatingBinaryOp)1u)
#define SLANG_NVVM_FLOATING_BINARY_OP_MULTIPLY ((SlangNVVMFloatingBinaryOp)2u)
#define SLANG_NVVM_FLOATING_BINARY_OP_DIVIDE ((SlangNVVMFloatingBinaryOp)3u)

    typedef uint32_t SlangNVVMFloatingUnaryOp;
#define SLANG_NVVM_FLOATING_UNARY_OP_NEGATE ((SlangNVVMFloatingUnaryOp)0u)

    typedef uint32_t SlangNVVMFloatingCompareOp;
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_EQUAL ((SlangNVVMFloatingCompareOp)0u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_UNORDERED_NOT_EQUAL ((SlangNVVMFloatingCompareOp)1u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_THAN ((SlangNVVMFloatingCompareOp)2u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_EQUAL ((SlangNVVMFloatingCompareOp)3u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_EQUAL ((SlangNVVMFloatingCompareOp)4u)
#define SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_THAN ((SlangNVVMFloatingCompareOp)5u)

    typedef uint32_t SlangNVVMIntrinsicOp;
#define SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_INDEX ((SlangNVVMIntrinsicOp)0u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_COUNT ((SlangNVVMIntrinsicOp)1u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_UINT ((SlangNVVMIntrinsicOp)2u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_INT ((SlangNVVMIntrinsicOp)3u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_FLOAT ((SlangNVVMIntrinsicOp)4u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_BALLOT ((SlangNVVMIntrinsicOp)5u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_UINT ((SlangNVVMIntrinsicOp)6u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_INT ((SlangNVVMIntrinsicOp)7u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_FLOAT ((SlangNVVMIntrinsicOp)8u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_IS_FIRST_LANE ((SlangNVVMIntrinsicOp)9u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ANY_TRUE ((SlangNVVMIntrinsicOp)10u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_TRUE ((SlangNVVMIntrinsicOp)11u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_INT ((SlangNVVMIntrinsicOp)12u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_UINT ((SlangNVVMIntrinsicOp)13u)
#define SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_FLOAT ((SlangNVVMIntrinsicOp)14u)

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
