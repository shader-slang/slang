#ifndef SLANG_NVVM_IR_BUILDER_API_H
#define SLANG_NVVM_IR_BUILDER_API_H

#include <stddef.h>
#include <stdint.h>

#define SLANG_NVVM_BUILDER_ABI_REVISION 28u
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

    typedef uint32_t SlangNVVMLinkage;
#define SLANG_NVVM_LINKAGE_INTERNAL ((SlangNVVMLinkage)0u)
#define SLANG_NVVM_LINKAGE_EXTERNAL ((SlangNVVMLinkage)1u)

    /** Independent semantic properties of one function definition. */
    typedef uint32_t SlangNVVMFunctionFlags;
#define SLANG_NVVM_FUNCTION_FLAG_NONE ((SlangNVVMFunctionFlags)0u)
#define SLANG_NVVM_FUNCTION_FLAG_NO_INLINE ((SlangNVVMFunctionFlags)1u << 0)

    /** Independent ABI properties of one physical function parameter. */
    typedef uint32_t SlangNVVMParameterFlags;
#define SLANG_NVVM_PARAMETER_FLAG_NONE ((SlangNVVMParameterFlags)0u)
/** The pointer parameter carries a caller-owned copy of `pointeeType`. */
#define SLANG_NVVM_PARAMETER_FLAG_BY_VALUE ((SlangNVVMParameterFlags)1u << 0)

    /** Independent semantic properties of one non-volatile load. */
    typedef uint32_t SlangNVVMLoadFlags;
#define SLANG_NVVM_LOAD_FLAG_NONE ((SlangNVVMLoadFlags)0u)
/** The referenced memory does not change for the duration of the executing program. */
#define SLANG_NVVM_LOAD_FLAG_INVARIANT ((SlangNVVMLoadFlags)1u << 0)

    typedef uint32_t SlangNVVMBuilderInterfaceID;
#define SLANG_NVVM_BUILDER_INTERFACE_FOUNDATION ((SlangNVVMBuilderInterfaceID)0u)
#define SLANG_NVVM_BUILDER_INTERFACE_CONSTRUCTION ((SlangNVVMBuilderInterfaceID)1u)
#define SLANG_NVVM_BUILDER_INTERFACE_VALUE_OPERATIONS ((SlangNVVMBuilderInterfaceID)2u)
#define SLANG_NVVM_BUILDER_INTERFACE_SURFACE_OPERATIONS ((SlangNVVMBuilderInterfaceID)3u)
#define SLANG_NVVM_BUILDER_INTERFACE_TEXTURE_OPERATIONS ((SlangNVVMBuilderInterfaceID)4u)
#define SLANG_NVVM_BUILDER_INTERFACE_ATOMIC_OPERATIONS ((SlangNVVMBuilderInterfaceID)5u)

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
#define SLANG_NVVM_VALUE_OP_REMAINDER ((SlangNVVMValueOperation)32u)
#define SLANG_NVVM_VALUE_OP_SHIFT_LEFT ((SlangNVVMValueOperation)33u)
#define SLANG_NVVM_VALUE_OP_SHIFT_RIGHT ((SlangNVVMValueOperation)34u)
#define SLANG_NVVM_VALUE_OP_FLOAT_CONVERT ((SlangNVVMValueOperation)35u)
#define SLANG_NVVM_VALUE_OP_SQRT ((SlangNVVMValueOperation)36u)
#define SLANG_NVVM_VALUE_OP_DEVICE_MEMORY_BARRIER ((SlangNVVMValueOperation)37u)
#define SLANG_NVVM_VALUE_OP_BIT_REINTERPRET ((SlangNVVMValueOperation)38u)
#define SLANG_NVVM_VALUE_OP_SELECT ((SlangNVVMValueOperation)39u)
#define SLANG_NVVM_VALUE_OP_SIN ((SlangNVVMValueOperation)40u)
#define SLANG_NVVM_VALUE_OP_COS ((SlangNVVMValueOperation)41u)
#define SLANG_NVVM_VALUE_OP_TRUNC ((SlangNVVMValueOperation)42u)
#define SLANG_NVVM_VALUE_OP_MIN ((SlangNVVMValueOperation)43u)
#define SLANG_NVVM_VALUE_OP_MAX ((SlangNVVMValueOperation)44u)
#define SLANG_NVVM_VALUE_OP_COUNT_BITS ((SlangNVVMValueOperation)45u)
#define SLANG_NVVM_VALUE_OP_REVERSE_BITS ((SlangNVVMValueOperation)46u)
#define SLANG_NVVM_VALUE_OP_FIRST_BIT_HIGH ((SlangNVVMValueOperation)47u)
#define SLANG_NVVM_VALUE_OP_FIRST_BIT_LOW ((SlangNVVMValueOperation)48u)
#define SLANG_NVVM_VALUE_OP_ABS ((SlangNVVMValueOperation)49u)
#define SLANG_NVVM_VALUE_OP_ACOS ((SlangNVVMValueOperation)50u)
#define SLANG_NVVM_VALUE_OP_ASIN ((SlangNVVMValueOperation)51u)
#define SLANG_NVVM_VALUE_OP_ATAN ((SlangNVVMValueOperation)52u)
#define SLANG_NVVM_VALUE_OP_ATAN2 ((SlangNVVMValueOperation)53u)
#define SLANG_NVVM_VALUE_OP_CEIL ((SlangNVVMValueOperation)54u)
#define SLANG_NVVM_VALUE_OP_EXP ((SlangNVVMValueOperation)55u)
#define SLANG_NVVM_VALUE_OP_EXP2 ((SlangNVVMValueOperation)56u)
#define SLANG_NVVM_VALUE_OP_FLOOR ((SlangNVVMValueOperation)57u)
#define SLANG_NVVM_VALUE_OP_FMOD ((SlangNVVMValueOperation)58u)
#define SLANG_NVVM_VALUE_OP_FRAC ((SlangNVVMValueOperation)59u)
#define SLANG_NVVM_VALUE_OP_LOG ((SlangNVVMValueOperation)60u)
#define SLANG_NVVM_VALUE_OP_LOG2 ((SlangNVVMValueOperation)61u)
#define SLANG_NVVM_VALUE_OP_LOG10 ((SlangNVVMValueOperation)62u)
#define SLANG_NVVM_VALUE_OP_POW ((SlangNVVMValueOperation)63u)
#define SLANG_NVVM_VALUE_OP_ROUND ((SlangNVVMValueOperation)64u)
#define SLANG_NVVM_VALUE_OP_RSQRT ((SlangNVVMValueOperation)65u)
#define SLANG_NVVM_VALUE_OP_TAN ((SlangNVVMValueOperation)66u)
#define SLANG_NVVM_VALUE_OP_IS_NAN ((SlangNVVMValueOperation)67u)
#define SLANG_NVVM_VALUE_OP_SIGN ((SlangNVVMValueOperation)68u)
#define SLANG_NVVM_VALUE_OP_FREXP_FRACTION ((SlangNVVMValueOperation)69u)
#define SLANG_NVVM_VALUE_OP_FREXP_EXPONENT ((SlangNVVMValueOperation)70u)
#define SLANG_NVVM_VALUE_OPERATION_COUNT 71u

    /** Describes one complete semantic value-operation overload. */
    typedef struct SlangNVVMValueOperationDesc
    {
        SlangNVVMValueOperation operation;
        SlangNVVMValueTypeDesc resultType;
        const SlangNVVMValueTypeDesc* operandTypes;
        size_t operandCount;
    } SlangNVVMValueOperationDesc;

    /** Integer read-modify-write operations. Signedness is carried by `valueType`. */
    typedef uint32_t SlangNVVMAtomicOperation;
#define SLANG_NVVM_ATOMIC_OP_ADD ((SlangNVVMAtomicOperation)0u)
#define SLANG_NVVM_ATOMIC_OP_SUBTRACT ((SlangNVVMAtomicOperation)1u)
#define SLANG_NVVM_ATOMIC_OP_BIT_AND ((SlangNVVMAtomicOperation)2u)
#define SLANG_NVVM_ATOMIC_OP_BIT_OR ((SlangNVVMAtomicOperation)3u)
#define SLANG_NVVM_ATOMIC_OP_BIT_XOR ((SlangNVVMAtomicOperation)4u)
#define SLANG_NVVM_ATOMIC_OP_MIN ((SlangNVVMAtomicOperation)5u)
#define SLANG_NVVM_ATOMIC_OP_MAX ((SlangNVVMAtomicOperation)6u)
#define SLANG_NVVM_ATOMIC_OP_EXCHANGE ((SlangNVVMAtomicOperation)7u)
#define SLANG_NVVM_ATOMIC_OPERATION_COUNT 8u

    typedef uint32_t SlangNVVMMemoryOrder;
#define SLANG_NVVM_MEMORY_ORDER_RELAXED ((SlangNVVMMemoryOrder)0u)
#define SLANG_NVVM_MEMORY_ORDER_ACQUIRE ((SlangNVVMMemoryOrder)1u)
#define SLANG_NVVM_MEMORY_ORDER_RELEASE ((SlangNVVMMemoryOrder)2u)
#define SLANG_NVVM_MEMORY_ORDER_ACQUIRE_RELEASE ((SlangNVVMMemoryOrder)3u)
#define SLANG_NVVM_MEMORY_ORDER_SEQUENTIALLY_CONSISTENT ((SlangNVVMMemoryOrder)4u)
#define SLANG_NVVM_MEMORY_ORDER_COUNT 5u

    /** Describes one complete typed atomic read-modify-write overload. */
    typedef struct SlangNVVMAtomicOperationDesc
    {
        SlangNVVMAtomicOperation operation;
        SlangNVVMValueTypeDesc valueType;
        SlangNVVMAddressSpace addressSpace;
        SlangNVVMMemoryOrder memoryOrder;
    } SlangNVVMAtomicOperationDesc;

    typedef uint32_t SlangNVVMSurfaceOperation;
#define SLANG_NVVM_SURFACE_OP_LOAD ((SlangNVVMSurfaceOperation)0u)
#define SLANG_NVVM_SURFACE_OP_STORE ((SlangNVVMSurfaceOperation)1u)

    typedef uint32_t SlangNVVMSurfaceBoundaryMode;
#define SLANG_NVVM_SURFACE_BOUNDARY_ZERO ((SlangNVVMSurfaceBoundaryMode)0u)

    typedef uint32_t SlangNVVMSurfaceStorageFormat;
#define SLANG_NVVM_SURFACE_STORAGE_NATIVE ((SlangNVVMSurfaceStorageFormat)0u)
#define SLANG_NVVM_SURFACE_STORAGE_FLOAT16 ((SlangNVVMSurfaceStorageFormat)1u)

    typedef uint32_t SlangNVVMTextureShape;
#define SLANG_NVVM_TEXTURE_SHAPE_1D ((SlangNVVMTextureShape)1u)
#define SLANG_NVVM_TEXTURE_SHAPE_2D ((SlangNVVMTextureShape)2u)
#define SLANG_NVVM_TEXTURE_SHAPE_3D ((SlangNVVMTextureShape)3u)
#define SLANG_NVVM_TEXTURE_SHAPE_CUBE ((SlangNVVMTextureShape)4u)

    /** Describes one complete typed surface-resource operation. */
    typedef struct SlangNVVMSurfaceOperationDesc
    {
        SlangNVVMSurfaceOperation operation;
        SlangNVVMTextureShape shape;
        uint32_t isArray;
        SlangNVVMValueTypeDesc elementType;
        SlangNVVMSurfaceBoundaryMode boundaryMode;
        SlangNVVMSurfaceStorageFormat storageFormat;
    } SlangNVVMSurfaceOperationDesc;

    typedef uint32_t SlangNVVMTextureOperation;
#define SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL ((SlangNVVMTextureOperation)0u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH ((SlangNVVMTextureOperation)1u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT ((SlangNVVMTextureOperation)2u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH ((SlangNVVMTextureOperation)3u)
#define SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL ((SlangNVVMTextureOperation)4u)

    /** Describes one complete typed sampled-texture operation. */
    typedef struct SlangNVVMTextureOperationDesc
    {
        SlangNVVMTextureOperation operation;
        SlangNVVMTextureShape shape;
        uint32_t isArray;
        SlangNVVMValueTypeDesc elementType;
    } SlangNVVMTextureOperationDesc;

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
        SlangNVVMResult(SLANG_NVVM_CALL* declareFunction)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle functionType,
            SlangNVVMLinkage linkage,
            SlangNVVMFunctionFlags flags,
            const char* name,
            size_t nameSize,
            SlangNVVMValueHandle* outFunction);
        SlangNVVMResult(SLANG_NVVM_CALL* getFunctionParameter)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle function,
            size_t parameterIndex,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* setFunctionParameterAttributes)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle function,
            size_t parameterIndex,
            SlangNVVMParameterFlags flags,
            SlangNVVMTypeHandle pointeeType,
            uint32_t alignment);
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
            SlangNVVMLoadFlags flags,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* emitStore)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle value,
            SlangNVVMValueHandle pointer,
            uint32_t alignment);
        SlangNVVMResult(SLANG_NVVM_CALL* emitLocalStorage)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle valueType,
            uint32_t alignment,
            const char* name,
            size_t nameSize,
            SlangNVVMValueHandle* outStorage);
        SlangNVVMResult(SLANG_NVVM_CALL* emitBranch)(
            SlangNVVMModuleHandle module,
            SlangNVVMBlockHandle targetBlock);
        SlangNVVMResult(SLANG_NVVM_CALL* emitConditionalBranch)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle condition,
            SlangNVVMBlockHandle trueBlock,
            SlangNVVMBlockHandle falseBlock);
        SlangNVVMResult(SLANG_NVVM_CALL* emitSwitch)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle condition,
            const SlangNVVMValueHandle* caseValues,
            const SlangNVVMBlockHandle* caseBlocks,
            size_t caseCount,
            SlangNVVMBlockHandle defaultBlock);
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
        SlangNVVMResult(SLANG_NVVM_CALL* emitUnreachable)(SlangNVVMModuleHandle module);
        SlangNVVMResult(SLANG_NVVM_CALL* emitPointerOffset)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle basePointer,
            SlangNVVMValueHandle elementOffset,
            SlangNVVMValueHandle* outPointer);
        SlangNVVMResult(SLANG_NVVM_CALL* emitByteOffsetPointer)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle basePointer,
            SlangNVVMValueHandle byteOffset,
            SlangNVVMTypeHandle resultPointeeType,
            SlangNVVMValueHandle* outPointer);
        SlangNVVMResult(SLANG_NVVM_CALL* emitSequentialElementPointer)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle baseSequentialPointer,
            SlangNVVMValueHandle elementIndex,
            SlangNVVMValueHandle* outPointer);
        SlangNVVMResult(SLANG_NVVM_CALL* emitStructFieldPointer)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle baseStructPointer,
            uint32_t fieldIndex,
            SlangNVVMValueHandle* outPointer);
        SlangNVVMResult(SLANG_NVVM_CALL* emitAggregateConstruct)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle aggregateType,
            const SlangNVVMValueHandle* elements,
            size_t elementCount,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* emitAggregateElementExtract)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle aggregateValue,
            uint32_t elementIndex,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* emitVectorConstruct)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle vectorType,
            const SlangNVVMValueHandle* elements,
            size_t elementCount,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* emitSequentialElementExtract)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle sequentialValue,
            SlangNVVMValueHandle elementIndex,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* declareGlobalStorage)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle valueType,
            SlangNVVMLinkage linkage,
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

    typedef struct SlangNVVMBuilderAtomicOperationsAPI
    {
        SlangNVVMResult(SLANG_NVVM_CALL* isOperationSupported)(
            const SlangNVVMAtomicOperationDesc* operation,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitOperation)(
            SlangNVVMModuleHandle module,
            const SlangNVVMAtomicOperationDesc* operation,
            SlangNVVMValueHandle pointer,
            SlangNVVMValueHandle value,
            SlangNVVMValueHandle* outOriginalValue);
    } SlangNVVMBuilderAtomicOperationsAPI;

    typedef struct SlangNVVMBuilderSurfaceOperationsAPI
    {
        SlangNVVMResult(SLANG_NVVM_CALL* isOperationSupported)(
            const SlangNVVMSurfaceOperationDesc* operation,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitOperation)(
            SlangNVVMModuleHandle module,
            const SlangNVVMSurfaceOperationDesc* operation,
            const SlangNVVMValueHandle* operands,
            size_t operandCount,
            SlangNVVMValueHandle* outValue);
    } SlangNVVMBuilderSurfaceOperationsAPI;

    typedef struct SlangNVVMBuilderTextureOperationsAPI
    {
        SlangNVVMResult(SLANG_NVVM_CALL* isOperationSupported)(
            const SlangNVVMTextureOperationDesc* operation,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitOperation)(
            SlangNVVMModuleHandle module,
            const SlangNVVMTextureOperationDesc* operation,
            const SlangNVVMValueHandle* operands,
            size_t operandCount,
            SlangNVVMValueHandle* outValue);
    } SlangNVVMBuilderTextureOperationsAPI;

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
