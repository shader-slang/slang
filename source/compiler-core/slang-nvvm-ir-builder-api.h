#ifndef SLANG_NVVM_IR_BUILDER_API_H
#define SLANG_NVVM_IR_BUILDER_API_H

#include <stddef.h>
#include <stdint.h>

#define SLANG_NVVM_BUILDER_ABI_REVISION 46u
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
    typedef struct SlangNVVMDeviceLibrary* SlangNVVMDeviceLibraryHandle;

    /** Reports parse/load diagnostics synchronously. Neither callback nor userData is retained;
        text is borrowed only for this invocation. A null callback discards diagnostics. */
    typedef void(SLANG_NVVM_CALL* SlangNVVMDiagnosticCallback)(
        void* userData,
        const char* text,
        size_t textSize);
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
#define SLANG_NVVM_BUILDER_INTERFACE_MEMORY_OPERATIONS ((SlangNVVMBuilderInterfaceID)6u)
#define SLANG_NVVM_BUILDER_INTERFACE_TRACE_OPERATIONS ((SlangNVVMBuilderInterfaceID)7u)
#define SLANG_NVVM_BUILDER_INTERFACE_INSTANCE_TRANSFORM_OPERATIONS ((SlangNVVMBuilderInterfaceID)8u)
#define SLANG_NVVM_BUILDER_INTERFACE_HIT_OBJECT_OPERATIONS ((SlangNVVMBuilderInterfaceID)9u)
#define SLANG_NVVM_BUILDER_INTERFACE_CURRENT_TRANSFORM_OPERATIONS ((SlangNVVMBuilderInterfaceID)10u)
#define SLANG_NVVM_BUILDER_INTERFACE_CALLABLE_OPERATIONS ((SlangNVVMBuilderInterfaceID)11u)
#define SLANG_NVVM_BUILDER_INTERFACE_OPTIX_TARGET ((SlangNVVMBuilderInterfaceID)12u)

    /** Semantic scalar and fixed-vector categories used by operation signatures. */
    typedef uint32_t SlangNVVMValueTypeKind;
#define SLANG_NVVM_VALUE_TYPE_VOID ((SlangNVVMValueTypeKind)0u)
#define SLANG_NVVM_VALUE_TYPE_BOOL ((SlangNVVMValueTypeKind)1u)
#define SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ((SlangNVVMValueTypeKind)2u)
#define SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER ((SlangNVVMValueTypeKind)3u)
#define SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ((SlangNVVMValueTypeKind)4u)
/** Scalar BF16 semantic format, transported as i16; never IEEE half. */
#define SLANG_NVVM_VALUE_TYPE_BFLOAT16 ((SlangNVVMValueTypeKind)5u)
/** Distinct scalar FP8 formats, transported as i8 without numeric conversion semantics. */
#define SLANG_NVVM_VALUE_TYPE_FLOAT_E4M3 ((SlangNVVMValueTypeKind)6u)
#define SLANG_NVVM_VALUE_TYPE_FLOAT_E5M2 ((SlangNVVMValueTypeKind)7u)

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
// Value operation 15 is reserved after moving its final wave consumers to core.
// Value operation 16 is reserved after moving its public wave composition to core.
// Value operation 17 is reserved after moving its final wave consumers to core.
#define SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT ((SlangNVVMValueOperation)18u)
// Value operation 19 is reserved after moving its public wave composition to core.
// Value operation 20 is reserved after moving its public wave composition to core.
// Value operation 21 is reserved after moving its public wave composition to core.
// Value operation 22 is reserved after moving its public wave composition to core.
// Value operation 23 is reserved after moving its public wave composition to core.
/* Values 24-27 are retired execution-vector operations. Keep later identities stable. */
/* Value 28 is retired; synchronization uses named LLVM intrinsics. */
#define SLANG_NVVM_VALUE_OP_INTEGER_CONVERT ((SlangNVVMValueOperation)29u)
#define SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT ((SlangNVVMValueOperation)30u)
#define SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER ((SlangNVVMValueOperation)31u)
#define SLANG_NVVM_VALUE_OP_REMAINDER ((SlangNVVMValueOperation)32u)
#define SLANG_NVVM_VALUE_OP_SHIFT_LEFT ((SlangNVVMValueOperation)33u)
#define SLANG_NVVM_VALUE_OP_SHIFT_RIGHT ((SlangNVVMValueOperation)34u)
#define SLANG_NVVM_VALUE_OP_FLOAT_CONVERT ((SlangNVVMValueOperation)35u)
/* Value 36 is retired; sqrt uses the named LLVM intrinsic. */
/* Value 37 is retired; synchronization uses named LLVM intrinsics. */
#define SLANG_NVVM_VALUE_OP_BIT_REINTERPRET ((SlangNVVMValueOperation)38u)
#define SLANG_NVVM_VALUE_OP_SELECT ((SlangNVVMValueOperation)39u)
/* Value 40 is retired; core math uses named calls and ordinary composition. */
/* Value 41 is retired; core math uses named calls and ordinary composition. */
/* Value 42 is retired; trunc uses named selected-libdevice functions. */
// Value operation 43 is reserved after moving its final wave consumers to core.
// Value operation 44 is reserved after moving its final wave consumers to core.
// Value operation 45 is reserved after moving its final wave consumers to core.
/* Values 46-47 are retired; public integer-bit operations use named LLVM intrinsics. */
// Value operation 48 is reserved after moving its final wave consumers to core.
// 49 is reserved (retired abs operation).
/* Value 50 is retired; core math uses named calls and ordinary composition. */
/* Value 51 is retired; core math uses named calls and ordinary composition. */
/* Value 52 is retired; core math uses named calls and ordinary composition. */
/* Value 53 is retired; core math uses named calls and ordinary composition. */
/* Value 54 is retired; ceil uses named selected-libdevice functions. */
/* Value 55 is retired; exp uses named selected-libdevice functions. */
/* Value 56 is retired; exp2 uses named selected-libdevice functions. */
/* Value 57 is retired; floor uses named selected-libdevice functions. */
#define SLANG_NVVM_VALUE_OP_FMOD ((SlangNVVMValueOperation)58u)
/* Value 59 is retired; frac uses named floor followed by ordinary subtraction. */
/* Value 60 is retired; logarithms use named selected-libdevice functions. */
/* Value 61 is retired; logarithms use named selected-libdevice functions. */
/* Value 62 is retired; logarithms use named selected-libdevice functions. */
/* Value 63 is retired; core math uses named calls and ordinary composition. */
/* Value 64 is retired; round uses named selected-libdevice functions. */
/* Value 65 is retired; rsqrt uses named selected-libdevice functions. */
/* Value 66 is retired; core math uses named calls and ordinary composition. */
// 67 is reserved (retired scalar NaN classification).
// 68 is reserved (retired sign operation).
// 69 is reserved (retired pointer-result projection).
// 70 is reserved (retired pointer-result projection).
#define SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH ((SlangNVVMValueOperation)71u)
/* Value 72 is retired; synchronization uses named LLVM intrinsics. */
/* Value 73 is retired; core math uses named calls and ordinary composition. */
/* Value 74 is retired; core math uses named calls and ordinary composition. */
/* Value 75 is retired; core math uses named calls and ordinary composition. */
/* Value 76 is retired; core math uses named calls and ordinary composition. */
// 77 is reserved (retired pointer-result projection).
// 78 is reserved (retired pointer-result projection).
/** Snapshots currently executing lanes without synchronization; not a logical convergence mask. */
#define SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK ((SlangNVVMValueOperation)79u)
/** Reads the per-multiprocessor cycle counter; each observation remains live. */
// 80 is reserved (retired clock operation).
// 81 is reserved (retired clock64 operation).
// 82 is reserved (retired BF16 dot composition).
/** Scalar fused multiply-add with one round-to-nearest-even BF16 result. */
#define SLANG_NVVM_VALUE_OP_FMA ((SlangNVVMValueOperation)83u)
/** Approximate Float32 division, flushing input/output subnormals to signed zero. */
#define SLANG_NVVM_VALUE_OP_DIVIDE_APPROX_FTZ ((SlangNVVMValueOperation)84u)
#define SLANG_NVVM_VALUE_OPERATION_COUNT 85u

    /** Describes one complete semantic value-operation overload. */
    typedef struct SlangNVVMValueOperationDesc
    {
        SlangNVVMValueOperation operation;
        SlangNVVMValueTypeDesc resultType;
        const SlangNVVMValueTypeDesc* operandTypes;
        size_t operandCount;
    } SlangNVVMValueOperationDesc;

    /** Scalar atomic operations. Signedness is carried by `valueType`. */
    typedef uint32_t SlangNVVMAtomicOperation;
#define SLANG_NVVM_ATOMIC_OP_ADD ((SlangNVVMAtomicOperation)0u)
#define SLANG_NVVM_ATOMIC_OP_SUBTRACT ((SlangNVVMAtomicOperation)1u)
#define SLANG_NVVM_ATOMIC_OP_BIT_AND ((SlangNVVMAtomicOperation)2u)
#define SLANG_NVVM_ATOMIC_OP_BIT_OR ((SlangNVVMAtomicOperation)3u)
#define SLANG_NVVM_ATOMIC_OP_BIT_XOR ((SlangNVVMAtomicOperation)4u)
#define SLANG_NVVM_ATOMIC_OP_MIN ((SlangNVVMAtomicOperation)5u)
#define SLANG_NVVM_ATOMIC_OP_MAX ((SlangNVVMAtomicOperation)6u)
#define SLANG_NVVM_ATOMIC_OP_EXCHANGE ((SlangNVVMAtomicOperation)7u)
#define SLANG_NVVM_ATOMIC_OP_LOAD ((SlangNVVMAtomicOperation)8u)
#define SLANG_NVVM_ATOMIC_OP_STORE ((SlangNVVMAtomicOperation)9u)
#define SLANG_NVVM_ATOMIC_OP_COMPARE_EXCHANGE ((SlangNVVMAtomicOperation)10u)
#define SLANG_NVVM_ATOMIC_OPERATION_COUNT 11u

    typedef uint32_t SlangNVVMMemoryOrder;
#define SLANG_NVVM_MEMORY_ORDER_RELAXED ((SlangNVVMMemoryOrder)0u)
#define SLANG_NVVM_MEMORY_ORDER_ACQUIRE ((SlangNVVMMemoryOrder)1u)
#define SLANG_NVVM_MEMORY_ORDER_RELEASE ((SlangNVVMMemoryOrder)2u)
#define SLANG_NVVM_MEMORY_ORDER_ACQUIRE_RELEASE ((SlangNVVMMemoryOrder)3u)
#define SLANG_NVVM_MEMORY_ORDER_SEQUENTIALLY_CONSISTENT ((SlangNVVMMemoryOrder)4u)
#define SLANG_NVVM_MEMORY_ORDER_COUNT 5u

    /** Describes one complete typed atomic overload. */
    typedef struct SlangNVVMAtomicOperationDesc
    {
        SlangNVVMAtomicOperation operation;
        SlangNVVMValueTypeDesc valueType;
        SlangNVVMAddressSpace addressSpace;
        SlangNVVMMemoryOrder memoryOrder;
        /** Used only by compare-exchange; must be relaxed for every other operation. */
        SlangNVVMMemoryOrder failureMemoryOrder;
    } SlangNVVMAtomicOperationDesc;

    /** Scoped, non-atomic accesses to exactly the described scalar location. */
    typedef uint32_t SlangNVVMMemoryOperation;
#define SLANG_NVVM_MEMORY_OP_LOAD ((SlangNVVMMemoryOperation)0u)
#define SLANG_NVVM_MEMORY_OP_STORE ((SlangNVVMMemoryOperation)1u)
    typedef uint32_t SlangNVVMMemoryScope;
#define SLANG_NVVM_MEMORY_SCOPE_DEVICE ((SlangNVVMMemoryScope)0u)
#define SLANG_NVVM_MEMORY_SCOPE_WORKGROUP ((SlangNVVMMemoryScope)1u)

    typedef struct SlangNVVMMemoryOperationDesc
    {
        SlangNVVMMemoryOperation operation;
        SlangNVVMValueTypeDesc valueType;
        SlangNVVMAddressSpace addressSpace;
        SlangNVVMMemoryScope scope;
        uint32_t alignment;
    } SlangNVVMMemoryOperationDesc;

    typedef uint32_t SlangNVVMSurfaceOperation;
#define SLANG_NVVM_SURFACE_OP_LOAD ((SlangNVVMSurfaceOperation)0u)
#define SLANG_NVVM_SURFACE_OP_STORE ((SlangNVVMSurfaceOperation)1u)

    typedef uint32_t SlangNVVMSurfaceBoundaryMode;
#define SLANG_NVVM_SURFACE_BOUNDARY_ZERO ((SlangNVVMSurfaceBoundaryMode)0u)

    typedef uint32_t SlangNVVMTextureShape;
#define SLANG_NVVM_TEXTURE_SHAPE_1D ((SlangNVVMTextureShape)1u)
#define SLANG_NVVM_TEXTURE_SHAPE_2D ((SlangNVVMTextureShape)2u)
#define SLANG_NVVM_TEXTURE_SHAPE_3D ((SlangNVVMTextureShape)3u)
#define SLANG_NVVM_TEXTURE_SHAPE_CUBE ((SlangNVVMTextureShape)4u)

    /** Describes a physical surface transfer. X is bytes; remaining coordinates are texels.
        elementType is the stored representation. Conversion belongs to the producer IR. */
    typedef struct SlangNVVMSurfaceOperationDesc
    {
        SlangNVVMSurfaceOperation operation;
        SlangNVVMTextureShape shape;
        uint32_t isArray;
        SlangNVVMValueTypeDesc elementType;
        SlangNVVMSurfaceBoundaryMode boundaryMode;
    } SlangNVVMSurfaceOperationDesc;

    typedef uint32_t SlangNVVMTextureOperation;
#define SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL ((SlangNVVMTextureOperation)0u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH ((SlangNVVMTextureOperation)1u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT ((SlangNVVMTextureOperation)2u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH ((SlangNVVMTextureOperation)3u)
#define SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL ((SlangNVVMTextureOperation)4u)
#define SLANG_NVVM_TEXTURE_OP_GATHER ((SlangNVVMTextureOperation)5u)
#define SLANG_NVVM_TEXTURE_OP_SAMPLE ((SlangNVVMTextureOperation)6u)
#define SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_WIDTH ((SlangNVVMTextureOperation)7u)
#define SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_HEIGHT ((SlangNVVMTextureOperation)8u)
#define SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_DEPTH ((SlangNVVMTextureOperation)9u)
#define SLANG_NVVM_TEXTURE_OP_SURFACE_QUERY_ARRAY_SIZE ((SlangNVVMTextureOperation)10u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_LEVEL_WIDTH ((SlangNVVMTextureOperation)11u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_LEVEL_HEIGHT ((SlangNVVMTextureOperation)12u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_LEVEL_DEPTH ((SlangNVVMTextureOperation)13u)
#define SLANG_NVVM_TEXTURE_OP_QUERY_LEVELS ((SlangNVVMTextureOperation)14u)

    /** Describes one complete typed sampled-texture operation or surface dimension query. */
    typedef struct SlangNVVMTextureOperationDesc
    {
        SlangNVVMTextureOperation operation;
        SlangNVVMTextureShape shape;
        uint32_t isArray;
        SlangNVVMValueTypeDesc elementType;
        /** Selects the gathered component for GATHER; must be zero for every other operation. */
        uint32_t component;
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
        SlangNVVMResult(SLANG_NVVM_CALL* emitBitCast)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle resultType,
            SlangNVVMValueHandle value,
            SlangNVVMValueHandle* outValue);
        SlangNVVMResult(SLANG_NVVM_CALL* emitPointerAddressSpaceCast)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle resultType,
            SlangNVVMValueHandle pointer,
            SlangNVVMValueHandle* outPointer);
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

    typedef uint32_t SlangNVVMNamedIntrinsicOperandKind;
#define SLANG_NVVM_NAMED_INTRINSIC_OPERAND_VALUE ((SlangNVVMNamedIntrinsicOperandKind)0u)
#define SLANG_NVVM_NAMED_INTRINSIC_OPERAND_INTEGER_CONSTANT ((SlangNVVMNamedIntrinsicOperandKind)1u)
#define SLANG_NVVM_NAMED_INTRINSIC_OPERAND_OUT_POINTER ((SlangNVVMNamedIntrinsicOperandKind)2u)

    /** A checked operand type and its guaranteed constant classification. INTEGER_CONSTANT
        promises an integer or Boolean literal. OUT_POINTER describes generic-address-space writable
        local storage whose pointee has `type`; it is admitted only for selected device-library
        calls. Emission independently verifies the value handle. */
    typedef struct SlangNVVMNamedIntrinsicOperandDesc
    {
        SlangNVVMValueTypeDesc type;
        SlangNVVMNamedIntrinsicOperandKind kind;
    } SlangNVVMNamedIntrinsicOperandDesc;

    /** An exact LLVM registry, admitted SDK primitive or device-library name and checked signature.
       All pointers are borrowed for the synchronous call. Emission returns a null value handle for
       void calls. */
    typedef struct SlangNVVMNamedIntrinsicDesc
    {
        const char* name;
        size_t nameSize;
        SlangNVVMValueTypeDesc resultType;
        const SlangNVVMNamedIntrinsicOperandDesc* operands;
        size_t operandCount;
    } SlangNVVMNamedIntrinsicDesc;

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
        /** Pure signature query; must not create or mutate a module. */
        SlangNVVMResult(SLANG_NVVM_CALL* isNamedIntrinsicSupported)(
            const SlangNVVMNamedIntrinsicDesc* intrinsic,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitNamedIntrinsic)(
            SlangNVVMModuleHandle module,
            const SlangNVVMNamedIntrinsicDesc* intrinsic,
            const SlangNVVMValueHandle* operands,
            size_t operandCount,
            SlangNVVMValueHandle* outValue);
        /** Copies and eagerly parses immutable library bytes in a separate input context. Failure
            leaves outLibrary null and may report a diagnostic; no output module is created. */
        SlangNVVMResult(SLANG_NVVM_CALL* loadDeviceLibrary)(
            const void* bytes,
            size_t byteCount,
            SlangNVVMDeviceLibraryHandle* outLibrary,
            SlangNVVMDiagnosticCallback diagnose,
            void* userData);
        void(SLANG_NVVM_CALL* destroyDeviceLibrary)(SlangNVVMDeviceLibraryHandle library);
        /** An unsupported signature returns success with outSupported zero, without diagnostics. */
        SlangNVVMResult(SLANG_NVVM_CALL* isDeviceLibraryFunctionSupported)(
            SlangNVVMDeviceLibraryHandle library,
            const SlangNVVMNamedIntrinsicDesc* function,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitDeviceLibraryFunction)(
            SlangNVVMDeviceLibraryHandle library,
            SlangNVVMModuleHandle module,
            const SlangNVVMNamedIntrinsicDesc* function,
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
            const SlangNVVMValueHandle* operands,
            size_t operandCount,
            SlangNVVMValueHandle* outValue);
    } SlangNVVMBuilderAtomicOperationsAPI;

    /** Optional interface. Existing ABI46 tables remain unchanged.
        Load takes a pointer and returns a value; store takes pointer/value and returns null. */
    typedef struct SlangNVVMBuilderMemoryOperationsAPI
    {
#define SLANG_NVVM_MEMORY_OPERATIONS_VERSION 1u
        uint32_t structureSize;
        uint32_t version;
        SlangNVVMResult(SLANG_NVVM_CALL* isOperationSupported)(
            const SlangNVVMMemoryOperationDesc* operation,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitOperation)(
            SlangNVVMModuleHandle module,
            const SlangNVVMMemoryOperationDesc* operation,
            const SlangNVVMValueHandle* operands,
            size_t operandCount,
            SlangNVVMValueHandle* outValue);
    } SlangNVVMBuilderMemoryOperationsAPI;

    /** The finite default-payload OptiX trace ABI; motion and pointer payload fallback are absent.
     */
    typedef struct SlangNVVMTraceRayDesc
    {
        uint32_t payloadCount; /**< Number of UInt32 payload words, in [1,32]. */
    } SlangNVVMTraceRayDesc;

    /** Optional interface; all existing ABI46 table layouts remain unchanged.
        Operands: UInt64 handle; Float32 origin.xyz, direction.xyz, tmin, tmax, time;
        UInt32 mask, flags, SBT offset, SBT stride, miss index; payloadCount UInt32 words.
        Result: an ordinary array[payloadCount] of i32 values, not a vector or pointer. */
    typedef struct SlangNVVMBuilderTraceOperationsAPI
    {
#define SLANG_NVVM_TRACE_OPERATIONS_VERSION 1u
        uint32_t structureSize;
        uint32_t version;
        SlangNVVMResult(SLANG_NVVM_CALL* isTraceRaySupported)(
            const SlangNVVMTraceRayDesc* operation,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitTraceRay)(
            SlangNVVMModuleHandle module,
            const SlangNVVMTraceRayDesc* operation,
            const SlangNVVMValueHandle* operands,
            size_t operandCount,
            SlangNVVMValueHandle* outValue);
    } SlangNVVMBuilderTraceOperationsAPI;

    /** One row of a valid OptiX instance's affine transform. No SDK pointer escapes this operation.
     */
    typedef struct SlangNVVMInstanceTransformDesc
    {
        uint32_t row;     /**< Row index in [0,2]. */
        uint32_t inverse; /**< Zero for object-to-world, one for world-to-object. */
    } SlangNVVMInstanceTransformDesc;

    /** Optional interface; existing ABI46 tables are unchanged. One UInt64 instance handle
        produces Float32x4. The provider owns the SDK pointer and its effectful 16-byte read. */
    typedef struct SlangNVVMBuilderInstanceTransformOperationsAPI
    {
#define SLANG_NVVM_INSTANCE_TRANSFORM_OPERATIONS_VERSION 1u
        uint32_t structureSize;
        uint32_t version;
        SlangNVVMResult(SLANG_NVVM_CALL* isOperationSupported)(
            const SlangNVVMInstanceTransformDesc* operation,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitOperation)(
            SlangNVVMModuleHandle module,
            const SlangNVVMInstanceTransformDesc* operation,
            const SlangNVVMValueHandle* operands,
            size_t operandCount,
            SlangNVVMValueHandle* outValue);
    } SlangNVVMBuilderInstanceTransformOperationsAPI;

    /** Optional current-ray affine rows. Shares the row/direction descriptor with instance rows,
        but takes no handle: the provider composes the active transform list at the current time.
        Existing ABI46 tables and instance-handle semantics are unchanged. */
    typedef SlangNVVMBuilderInstanceTransformOperationsAPI
        SlangNVVMBuilderCurrentTransformOperationsAPI;
#define SLANG_NVVM_CURRENT_TRANSFORM_OPERATIONS_VERSION 1u

#define SLANG_NVVM_OPTIX_TARGET_VERSION 1u
    /** Configures the SDK contract before an output module acquires declarations or types. */
    typedef struct SlangNVVMBuilderOptixTargetAPI
    {
        uint32_t structureSize;
        uint32_t version;
        SlangNVVMResult(
            SLANG_NVVM_CALL* isVersionSupported)(uint32_t version, uint32_t* outSupported);
        SlangNVVMResult(
            SLANG_NVVM_CALL* setVersion)(SlangNVVMModuleHandle module, uint32_t version);
    } SlangNVVMBuilderOptixTargetAPI;

    /** Optional private callable ABI. Index is UInt32. A non-null numeric aggregate payload
        is copied into provider-local Value(T) storage, passed as a generic pointer and returned
        after the call. Null payload selects void() and returns null. No function pointer escapes.
        Both caller and callable definition must use the NVVM value representation. */
    typedef struct SlangNVVMBuilderCallableOperationsAPI
    {
#define SLANG_NVVM_CALLABLE_OPERATIONS_VERSION 1u
        uint32_t structureSize;
        uint32_t version;
        SlangNVVMResult(SLANG_NVVM_CALL* emitCall)(
            SlangNVVMModuleHandle module,
            SlangNVVMValueHandle index,
            SlangNVVMValueHandle payload,
            SlangNVVMValueHandle* outValue);
    } SlangNVVMBuilderCallableOperationsAPI;

    /** Finite operations on independently owned OptiX9 hit-object snapshots. */
    typedef uint32_t SlangNVVMHitObjectOperation;
// Zero is not a valid operation.
#define SLANG_NVVM_HIT_OBJECT_OP_MAKE_NOP ((SlangNVVMHitObjectOperation)1u)
#define SLANG_NVVM_HIT_OBJECT_OP_MAKE_MISS ((SlangNVVMHitObjectOperation)2u)
#define SLANG_NVVM_HIT_OBJECT_OP_TRAVERSE ((SlangNVVMHitObjectOperation)3u)
#define SLANG_NVVM_HIT_OBJECT_OP_INVOKE ((SlangNVVMHitObjectOperation)4u)
#define SLANG_NVVM_HIT_OBJECT_OP_QUERY ((SlangNVVMHitObjectOperation)5u)
#define SLANG_NVVM_HIT_OBJECT_OP_SET_SBT_INDEX ((SlangNVVMHitObjectOperation)6u)
#define SLANG_NVVM_HIT_OBJECT_OP_LOAD_SBT_U32 ((SlangNVVMHitObjectOperation)7u)
#define SLANG_NVVM_HIT_OBJECT_OP_REORDER ((SlangNVVMHitObjectOperation)8u)
#define SLANG_NVVM_HIT_OBJECT_OP_REORDER_HINT ((SlangNVVMHitObjectOperation)9u)
#define SLANG_NVVM_HIT_OBJECT_OP_REPORT_INTERSECTION ((SlangNVVMHitObjectOperation)10u)
// Current-hit queries have no object operand and do not restore outgoing state.
#define SLANG_NVVM_HIT_OBJECT_OP_CURRENT_QUERY ((SlangNVVMHitObjectOperation)11u)

    typedef uint32_t SlangNVVMHitObjectQuery;
#define SLANG_NVVM_HIT_OBJECT_QUERY_IS_HIT ((SlangNVVMHitObjectQuery)0u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_IS_MISS ((SlangNVVMHitObjectQuery)1u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_IS_NOP ((SlangNVVMHitObjectQuery)2u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_INSTANCE_ID ((SlangNVVMHitObjectQuery)3u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_INSTANCE_INDEX ((SlangNVVMHitObjectQuery)4u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_SBT_GAS_INDEX ((SlangNVVMHitObjectQuery)5u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_PRIMITIVE_INDEX ((SlangNVVMHitObjectQuery)6u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_HIT_KIND ((SlangNVVMHitObjectQuery)7u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_SBT_INDEX ((SlangNVVMHitObjectQuery)8u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_RAY_FLAGS ((SlangNVVMHitObjectQuery)9u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_WORLD_ORIGIN ((SlangNVVMHitObjectQuery)10u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_WORLD_DIRECTION ((SlangNVVMHitObjectQuery)11u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_TMIN ((SlangNVVMHitObjectQuery)12u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_TMAX ((SlangNVVMHitObjectQuery)13u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_TIME ((SlangNVVMHitObjectQuery)14u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_ATTRIBUTE ((SlangNVVMHitObjectQuery)15u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_CLUSTER_ID ((SlangNVVMHitObjectQuery)16u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_SPHERE ((SlangNVVMHitObjectQuery)17u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_LSS ((SlangNVVMHitObjectQuery)18u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_OBJECT_TO_WORLD ((SlangNVVMHitObjectQuery)19u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_MATRIX_WORLD_TO_OBJECT ((SlangNVVMHitObjectQuery)20u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_IS_SPHERE ((SlangNVVMHitObjectQuery)21u)
#define SLANG_NVVM_HIT_OBJECT_QUERY_IS_LSS ((SlangNVVMHitObjectQuery)22u)

    typedef struct SlangNVVMHitObjectOperationDesc
    {
        SlangNVVMHitObjectOperation operation;
        SlangNVVMHitObjectQuery query; /**< QUERY only; otherwise zero. */
        uint32_t index;        /**< Attribute0..7, LSS row0..1 or matrix row0..2; otherwise zero. */
        uint32_t payloadCount; /**< Traverse/Invoke0..32; ReportIntersection attributes0..8. */
    } SlangNVVMHitObjectOperationDesc;

    /** Optional interface; existing ABI46 tables remain unchanged.
        Storage is private and owned by each canonical HitObject local. Operand0 is its generic
        storage pointer, except REORDER_HINT and REPORT_INTERSECTION.
        Remaining operands: MAKE_MISS uses the SDK's eleven scalar arguments; TRAVERSE uses the
        trace interface's fifteen scalars followed by payload words; INVOKE uses payload words;
        SET_SBT_INDEX/LOAD_SBT_U32 use one UInt32; REORDER forms use hint/bits UInt32 values.
        REPORT_INTERSECTION uses Float32 distance, UInt32 kind and attribute words.
        Nonempty payload results are UInt32 arrays; predicates and ReportIntersection return UInt32;
       no-result calls return null. QUERY returns the exact scalar or vector prescribed by its code.
       No SDK pointer escapes. */
    typedef struct SlangNVVMBuilderHitObjectOperationsAPI
    {
#define SLANG_NVVM_HIT_OBJECT_OPERATIONS_VERSION 1u
        uint32_t structureSize;
        uint32_t version;
        SlangNVVMResult(
            SLANG_NVVM_CALL* getStorageLayout)(uint32_t* outSize, uint32_t* outAlignment);
        SlangNVVMResult(SLANG_NVVM_CALL* getHitObjectType)(
            SlangNVVMModuleHandle module,
            SlangNVVMTypeHandle* outType);
        SlangNVVMResult(SLANG_NVVM_CALL* isOperationSupported)(
            const SlangNVVMHitObjectOperationDesc* operation,
            uint32_t* outSupported);
        SlangNVVMResult(SLANG_NVVM_CALL* emitOperation)(
            SlangNVVMModuleHandle module,
            const SlangNVVMHitObjectOperationDesc* operation,
            const SlangNVVMValueHandle* operands,
            size_t operandCount,
            SlangNVVMValueHandle* outValue);
    } SlangNVVMBuilderHitObjectOperationsAPI;

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
