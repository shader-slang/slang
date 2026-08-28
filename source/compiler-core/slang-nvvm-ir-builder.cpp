#include "slang-nvvm-ir-builder.h"

#include "core/slang-blob.h"
#include "slang-downstream-compiler-util.h"

#include <cstring>

namespace Slang
{

static bool _hasRequiredFunctions(const SlangNVVMBuilderAPI_V1& api)
{
    return api.createModule && api.destroyModule && api.getVoidType && api.getFunctionType &&
           api.declareFunction && api.createBlock && api.setInsertBlock && api.emitReturnVoid &&
           api.markFunctionAsKernel && api.serializeModule;
}

// Checks the complete frozen V1 contract used by both standalone and nested API tables.
static bool _isCompatibleV1(const SlangNVVMBuilderAPI_V1& api)
{
    return api.structureSize == sizeof(api) && api.abiVersion == SLANG_NVVM_BUILDER_ABI_VERSION_1 &&
           api.llvmVersionMajor == 14 && api.llvmVersionMinor == 0 && api.llvmVersionPatch == 6 &&
           api.nvvmIRVersionMajor == 2 && api.nvvmIRVersionMinor == 0 &&
           api.pointerModel == SLANG_NVVM_POINTER_MODEL_TYPED && _hasRequiredFunctions(api);
}

// Checks whether a successful V2 transport returned a usable verification classification.
static bool _isVerificationStatus(SlangNVVMVerificationStatus_2 status)
{
    return status == SLANG_NVVM_VERIFICATION_VALID || status == SLANG_NVVM_VERIFICATION_INVALID;
}

// Treats the appended Slice 4 fields as one coherent scalar-memory capability.
static bool _supportsScalarOperations(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE && api.getIntegerType &&
           api.getPointerType && api.getFunctionParameter && api.emitLoad && api.emitStore;
}

// Treats the appended Slice 7 fields as one coherent scalar-control-flow capability.
static bool _supportsScalarControlFlow(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE &&
           api.emitIntegerBinary && api.emitIntegerSignedLessThan && api.emitBranch &&
           api.emitConditionalBranch;
}

// Treats the appended Slice 8 fields as one coherent scalar-SSA capability.
static bool _supportsScalarSSA(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE &&
           api.getIntegerConstant && api.emitIntegerPhi && api.addIntegerPhiIncoming;
}

// Treats the appended Slice 9 fields as one coherent scalar-function capability.
static bool _supportsScalarFunctions(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE &&
           api.emitIntegerCall && api.emitIntegerReturn;
}

// Treats the appended Slice 10 field as one coherent scalar-pointer-arithmetic capability.
static bool _supportsScalarPointerArithmetic(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE &&
           api.emitPointerOffset;
}

// Treats the appended Slice 11 fields as one coherent scalar-array-addressing capability.
static bool _supportsScalarArrayAddressing(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE &&
           api.getArrayType && api.emitArrayElementPointer;
}

// Treats the appended Slice 12 field as one coherent scalar-integer-multiply capability.
static bool _supportsScalarIntegerMultiply(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE &&
           api.emitIntegerMultiply;
}

// Treats the appended Slice 13 field as one coherent scalar-integer-bit-AND capability.
static bool _supportsScalarIntegerBitAnd(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE &&
           api.emitIntegerBitAnd;
}

// Treats the appended Slice 14 field as one coherent scalar-integer-bit-OR capability.
static bool _supportsScalarIntegerBitOr(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE &&
           api.emitIntegerBitOr;
}

// Treats the appended Slice 15 field as one coherent scalar-integer-bit-XOR capability.
static bool _supportsScalarIntegerBitXor(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE &&
           api.emitIntegerBitXor;
}

// Treats the appended Slice 16 field as one coherent scalar-integer-bit-NOT capability.
static bool _supportsScalarIntegerBitNot(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE &&
           api.emitIntegerBitNot;
}

// Treats the appended Slice 17 field as one coherent scalar-integer-negate capability.
static bool _supportsScalarIntegerNegate(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE &&
           api.emitIntegerNegate;
}

// Treats the second Slice 19 field as the matching libNVVM text-wire capability.
static bool _supportsNVVMIR20Assembly(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE &&
           api.serializeNVVMIR20AssemblyWithDiagnostics;
}

// Treats the appended Slice 19 fields as one coherent relaxed global-i32 atomic-add capability.
static bool _supportsRelaxedGlobalI32AtomicAdd(const SlangNVVMBuilderAPI_V2& api)
{
    return _supportsNVVMIR20Assembly(api) && api.emitRelaxedGlobalI32AtomicAdd;
}

// Treats the appended Slice 21 field as one coherent scalar-integer-equality capability.
static bool _supportsScalarIntegerEqual(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_EQUAL_MIN_SIZE &&
           api.emitIntegerEqual;
}

// Treats the appended Slice 22 field as one coherent scalar-integer-inequality capability.
static bool _supportsScalarIntegerNotEqual(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NOT_EQUAL_MIN_SIZE &&
           api.emitIntegerNotEqual;
}

// Treats the appended Slice 23 field as one coherent signed-integer-greater-than capability.
static bool _supportsScalarIntegerSignedGreaterThan(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >=
               SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_THAN_MIN_SIZE &&
           api.emitIntegerSignedGreaterThan;
}

// Treats the appended Slice 24 field as one coherent signed-integer-less-equal capability.
static bool _supportsScalarIntegerSignedLessEqual(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >=
               SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_LESS_EQUAL_MIN_SIZE &&
           api.emitIntegerSignedLessEqual;
}

// Treats the appended Slice 25 field as one coherent signed-integer-greater-equal capability.
static bool _supportsScalarIntegerSignedGreaterEqual(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >=
               SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE &&
           api.emitIntegerSignedGreaterEqual;
}

// Treats the two appended Slice 26 fields as one coherent raw-resource capability.
static bool _supportsRawRWStructuredBufferI32(const SlangNVVMBuilderAPI_V2& api)
{
    return api.structureSize >= SLANG_NVVM_BUILDER_API_V2_RAW_RW_STRUCTURED_BUFFER_I32_MIN_SIZE &&
           api.getRawRWStructuredBufferI32Type && api.emitRawRWStructuredBufferI32ElementPointer;
}

static void _addFeature(SlangNVVMBuilderFeatureSet_3& features, SlangNVVMBuilderFeature_3 feature)
{
    features.words[feature / 64u] |= uint64_t(1) << (feature % 64u);
}

static bool _hasFeature(
    const SlangNVVMBuilderFeatureSet_3& features,
    SlangNVVMBuilderFeature_3 feature)
{
    if (feature >= SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT_3 * 64u)
        return false;
    return (features.words[feature / 64u] & (uint64_t(1) << (feature % 64u))) != 0;
}

// Converts each coherent frozen V2 prefix into the independent V3 semantic vocabulary.
static SlangNVVMBuilderFeatureSet_3 _getV2Features(const SlangNVVMBuilderAPI_V2& api)
{
    SlangNVVMBuilderFeatureSet_3 features = {};
#define SLANG_ADD_V2_FEATURE(test, feature) \
    if (test(api))                          \
    _addFeature(features, feature)
    SLANG_ADD_V2_FEATURE(_supportsScalarOperations, SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarControlFlow,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW);
    SLANG_ADD_V2_FEATURE(_supportsScalarSSA, SLANG_NVVM_BUILDER_FEATURE_SCALAR_SSA);
    SLANG_ADD_V2_FEATURE(_supportsScalarFunctions, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FUNCTIONS);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarPointerArithmetic,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_POINTER_ARITHMETIC);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarArrayAddressing,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_ARRAY_ADDRESSING);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerMultiply,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerBitAnd,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_AND);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerBitOr,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_OR);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerBitXor,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_XOR);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerBitNot,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_NOT);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerNegate,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NEGATE);
    SLANG_ADD_V2_FEATURE(
        _supportsRelaxedGlobalI32AtomicAdd,
        SLANG_NVVM_BUILDER_FEATURE_RELAXED_GLOBAL_I32_ATOMIC_ADD);
    SLANG_ADD_V2_FEATURE(
        _supportsNVVMIR20Assembly,
        SLANG_NVVM_BUILDER_FEATURE_NVVM_IR_2_0_ASSEMBLY);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerEqual,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_EQUAL);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerNotEqual,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NOT_EQUAL);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerSignedGreaterThan,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_THAN);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerSignedLessEqual,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_LESS_EQUAL);
    SLANG_ADD_V2_FEATURE(
        _supportsScalarIntegerSignedGreaterEqual,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_EQUAL);
    SLANG_ADD_V2_FEATURE(
        _supportsRawRWStructuredBufferI32,
        SLANG_NVVM_BUILDER_FEATURE_RAW_RW_STRUCTURED_BUFFER_I32);
#undef SLANG_ADD_V2_FEATURE
    return features;
}

// Rejects success without a required handle and never exposes a handle from a failed provider call.
template<typename T>
static SlangResult _validateHandleResult(SlangNVVMResult_1 result, T& handle)
{
    if (result < 0)
    {
        handle = nullptr;
        return result;
    }
    return handle ? result : SLANG_FAIL;
}

/* static */ SlangResult NVVMIRBuilder::load(
    const String& path,
    ISlangSharedLibraryLoader* loader,
    NVVMIRBuilder& outBuilder)
{
    outBuilder = NVVMIRBuilder();
    if (!loader)
        loader = DefaultSharedLibraryLoader::getSingleton();

    ComPtr<ISlangSharedLibrary> library;
    SLANG_RETURN_ON_FAIL(DownstreamCompilerUtil::loadSharedLibrary(
        path,
        loader,
        nullptr,
        "slang-llvm-nvvm",
        library));
    if (!library)
        return SLANG_FAIL;

    SlangGetNVVMBuilderAPI_V3 getAPIV3 = reinterpret_cast<SlangGetNVVMBuilderAPI_V3>(
        library->findFuncByName(SLANG_NVVM_BUILDER_GET_API_V3_NAME));
    if (getAPIV3)
    {
        SlangNVVMBuilderAPI_V3 api = {};
        api.structureSize = uint32_t(sizeof(api));
        api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_3;
        SLANG_RETURN_ON_FAIL(getAPIV3(&api));
        return initialize(api, library, outBuilder);
    }

    SlangGetNVVMBuilderAPI_V2 getAPIV2 = reinterpret_cast<SlangGetNVVMBuilderAPI_V2>(
        library->findFuncByName(SLANG_NVVM_BUILDER_GET_API_V2_NAME));
    if (getAPIV2)
    {
        SlangNVVMBuilderAPI_V2 api = {};
        api.structureSize = uint32_t(sizeof(api));
        api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_2;
        SLANG_RETURN_ON_FAIL(getAPIV2(&api));
        return initialize(api, library, outBuilder);
    }

    SlangGetNVVMBuilderAPI_V1 getAPI = reinterpret_cast<SlangGetNVVMBuilderAPI_V1>(
        library->findFuncByName(SLANG_NVVM_BUILDER_GET_API_V1_NAME));
    if (!getAPI)
        return SLANG_E_NO_INTERFACE;

    SlangNVVMBuilderAPI_V1 api = {};
    api.structureSize = uint32_t(sizeof(api));
    api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_1;
    SLANG_RETURN_ON_FAIL(getAPI(&api));
    return initialize(api, library, outBuilder);
}

/* static */ SlangResult NVVMIRBuilder::initialize(
    const SlangNVVMBuilderAPI_V1& api,
    ISlangSharedLibrary* library,
    NVVMIRBuilder& outBuilder)
{
    outBuilder = NVVMIRBuilder();
    if (!_isCompatibleV1(api))
        return SLANG_E_NO_INTERFACE;
    if (!library)
        return SLANG_E_INVALID_ARG;

    outBuilder.m_api = api;
    outBuilder.m_library = library;
    return SLANG_OK;
}

/* static */ SlangResult NVVMIRBuilder::initialize(
    const SlangNVVMBuilderAPI_V2& api,
    ISlangSharedLibrary* library,
    NVVMIRBuilder& outBuilder)
{
    outBuilder = NVVMIRBuilder();
    const bool hasPartialScalarPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE;
    const bool hasPartialScalarControlFlowPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE;
    const bool hasPartialScalarSSAPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE;
    const bool hasPartialScalarFunctionPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE;
    const bool hasPartialScalarPointerArithmeticPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE;
    const bool hasPartialScalarArrayPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE;
    const bool hasPartialScalarIntegerMultiplyPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE;
    const bool hasPartialScalarIntegerBitAndPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE;
    const bool hasPartialScalarIntegerBitOrPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE;
    const bool hasPartialScalarIntegerBitXorPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE;
    const bool hasPartialScalarIntegerBitNotPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE;
    const bool hasPartialScalarIntegerNegatePrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE;
    const bool hasPartialRelaxedGlobalI32AtomicAddPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE;
    const bool hasPartialScalarIntegerEqualPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_EQUAL_MIN_SIZE;
    const bool hasPartialScalarIntegerNotEqualPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_EQUAL_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NOT_EQUAL_MIN_SIZE;
    const bool hasPartialScalarIntegerSignedGreaterThanPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NOT_EQUAL_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_THAN_MIN_SIZE;
    const bool hasPartialScalarIntegerSignedLessEqualPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_THAN_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_LESS_EQUAL_MIN_SIZE;
    const bool hasPartialScalarIntegerSignedGreaterEqualPrefix =
        api.structureSize > SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_LESS_EQUAL_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE;
    const bool hasPartialRawRWStructuredBufferI32Prefix =
        api.structureSize >
            SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE &&
        api.structureSize < SLANG_NVVM_BUILDER_API_V2_RAW_RW_STRUCTURED_BUFFER_I32_MIN_SIZE;
    if (api.structureSize < SLANG_NVVM_BUILDER_API_V2_MIN_SIZE || hasPartialScalarPrefix ||
        hasPartialScalarControlFlowPrefix || hasPartialScalarSSAPrefix ||
        hasPartialScalarFunctionPrefix || hasPartialScalarPointerArithmeticPrefix ||
        hasPartialScalarArrayPrefix || hasPartialScalarIntegerMultiplyPrefix ||
        hasPartialScalarIntegerBitAndPrefix || hasPartialScalarIntegerBitOrPrefix ||
        hasPartialScalarIntegerBitXorPrefix || hasPartialScalarIntegerBitNotPrefix ||
        hasPartialScalarIntegerNegatePrefix || hasPartialRelaxedGlobalI32AtomicAddPrefix ||
        hasPartialScalarIntegerEqualPrefix || hasPartialScalarIntegerNotEqualPrefix ||
        hasPartialScalarIntegerSignedGreaterThanPrefix ||
        hasPartialScalarIntegerSignedLessEqualPrefix ||
        hasPartialScalarIntegerSignedGreaterEqualPrefix ||
        hasPartialRawRWStructuredBufferI32Prefix ||
        api.abiVersion != SLANG_NVVM_BUILDER_ABI_VERSION_2 || !_isCompatibleV1(api.baseAPI) ||
        !api.serializeModuleWithDiagnostics ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE &&
         !_supportsScalarOperations(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE &&
         !_supportsScalarControlFlow(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE &&
         !_supportsScalarSSA(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE &&
         !_supportsScalarFunctions(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE &&
         !_supportsScalarPointerArithmetic(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE &&
         !_supportsScalarArrayAddressing(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE &&
         !_supportsScalarIntegerMultiply(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE &&
         !_supportsScalarIntegerBitAnd(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE &&
         !_supportsScalarIntegerBitOr(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE &&
         !_supportsScalarIntegerBitXor(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE &&
         !_supportsScalarIntegerBitNot(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE &&
         !_supportsScalarIntegerNegate(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE &&
         !_supportsRelaxedGlobalI32AtomicAdd(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_EQUAL_MIN_SIZE &&
         !_supportsScalarIntegerEqual(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NOT_EQUAL_MIN_SIZE &&
         !_supportsScalarIntegerNotEqual(api)) ||
        (api.structureSize >=
             SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_THAN_MIN_SIZE &&
         !_supportsScalarIntegerSignedGreaterThan(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_LESS_EQUAL_MIN_SIZE &&
         !_supportsScalarIntegerSignedLessEqual(api)) ||
        (api.structureSize >=
             SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE &&
         !_supportsScalarIntegerSignedGreaterEqual(api)) ||
        (api.structureSize >= SLANG_NVVM_BUILDER_API_V2_RAW_RW_STRUCTURED_BUFFER_I32_MIN_SIZE &&
         !_supportsRawRWStructuredBufferI32(api)))
    {
        return SLANG_E_NO_INTERFACE;
    }
    if (!library)
        return SLANG_E_INVALID_ARG;

    const size_t retainedSize = api.structureSize < sizeof(outBuilder.m_apiV2)
                                    ? api.structureSize
                                    : sizeof(outBuilder.m_apiV2);
    SlangNVVMBuilderAPI_V2 retainedAPI = {};
    std::memcpy(&retainedAPI, &api, retainedSize);
    retainedAPI.structureSize = uint32_t(retainedSize);

    outBuilder.m_api = retainedAPI.baseAPI;
    outBuilder.m_apiV2 = retainedAPI;
    outBuilder.m_features = _getV2Features(retainedAPI);
    outBuilder.m_library = library;
    return SLANG_OK;
}

/* static */ SlangResult NVVMIRBuilder::initialize(
    const SlangNVVMBuilderAPI_V3& api,
    ISlangSharedLibrary* library,
    NVVMIRBuilder& outBuilder)
{
    outBuilder = NVVMIRBuilder();
    const bool advertisesFloat32Binary =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD) ||
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT) ||
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY) ||
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_DIVIDE);
    const bool advertisesFloat32Negate =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NEGATE);
    const bool advertisesFloat32Compare =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_EQUAL) ||
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NOT_EQUAL) ||
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_THAN) ||
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_EQUAL) ||
        _hasFeature(
            api.features,
            SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_EQUAL) ||
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_THAN);
    const bool advertisesFloat32Constant =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT);
    const bool advertisesScalarPhi =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI);
    const bool advertisesGenericScalarFunctions =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS);
    const bool advertisesWaveLaneIndex =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX);
    const bool advertisesWaveLaneCount =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT);
    const bool advertisesWaveReadLaneAtUInt =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT);
    const bool advertisesWaveReadLaneAtInt =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT);
    const bool advertisesWaveReadLaneAtFloat =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT);
    const bool advertisesWaveMaskBallot =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT);
    const bool advertisesWaveReadLaneFirstUInt =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_UINT);
    const bool advertisesWaveReadLaneFirstInt =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_INT);
    const bool advertisesWaveReadLaneFirstFloat =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_FLOAT);
    const bool advertisesWaveMaskIsFirstLane =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_IS_FIRST_LANE);
    const bool advertisesWaveMaskAnyTrue =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ANY_TRUE);
    const bool advertisesWaveMaskAllTrue =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_TRUE);
    const bool advertisesWaveMaskAllEqualInt =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_INT);
    const bool advertisesWaveMaskAllEqualUInt =
        _hasFeature(api.features, SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_UINT);
    if (api.structureSize < SLANG_NVVM_BUILDER_API_V3_MIN_SIZE ||
        api.abiVersion != SLANG_NVVM_BUILDER_ABI_VERSION_3 ||
        api.compatibilityAPI.structureSize != sizeof(SlangNVVMBuilderAPI_V2) ||
        !api.emitIntegerUnary || !api.emitIntegerBinary || !api.emitIntegerCompare ||
        (advertisesFloat32Binary &&
         (api.structureSize < SLANG_NVVM_BUILDER_API_V3_SCALAR_FLOAT32_ADD_MIN_SIZE ||
          !api.getFloatingPointType || !api.emitFloatingBinary)) ||
        (advertisesFloat32Negate &&
         (api.structureSize < SLANG_NVVM_BUILDER_API_V3_SCALAR_FLOAT32_NEGATE_MIN_SIZE ||
          !api.getFloatingPointType || !api.emitFloatingUnary)) ||
        (advertisesFloat32Compare &&
         (api.structureSize < SLANG_NVVM_BUILDER_API_V3_FLOATING_COMPARE_MIN_SIZE ||
          !api.getFloatingPointType || !api.emitFloatingCompare)) ||
        (advertisesFloat32Constant &&
         (api.structureSize < SLANG_NVVM_BUILDER_API_V3_FLOATING_CONSTANT_MIN_SIZE ||
          !api.getFloatingPointType || !api.getFloatingPointConstant)) ||
        (advertisesScalarPhi &&
         (api.structureSize < SLANG_NVVM_BUILDER_API_V3_SCALAR_PHI_MIN_SIZE || !api.emitPhi ||
          !api.addPhiIncoming)) ||
        (advertisesGenericScalarFunctions &&
         (api.structureSize < SLANG_NVVM_BUILDER_API_V3_GENERIC_SCALAR_FUNCTIONS_MIN_SIZE ||
          !api.getFloatingPointType || !api.emitCall || !api.emitValueReturn)) ||
        ((advertisesWaveLaneIndex || advertisesWaveLaneCount || advertisesWaveReadLaneAtUInt ||
          advertisesWaveReadLaneAtInt || advertisesWaveReadLaneAtFloat ||
          advertisesWaveMaskBallot || advertisesWaveReadLaneFirstUInt ||
          advertisesWaveReadLaneFirstInt || advertisesWaveReadLaneFirstFloat ||
          advertisesWaveMaskIsFirstLane || advertisesWaveMaskAnyTrue || advertisesWaveMaskAllTrue ||
          advertisesWaveMaskAllEqualInt || advertisesWaveMaskAllEqualUInt) &&
         (api.structureSize < SLANG_NVVM_BUILDER_API_V3_WAVE_LANE_INDEX_MIN_SIZE ||
          !api.emitIntrinsic)))
    {
        return SLANG_E_NO_INTERFACE;
    }

    NVVMIRBuilder compatibilityBuilder;
    SLANG_RETURN_ON_FAIL(initialize(api.compatibilityAPI, library, compatibilityBuilder));

    const size_t retainedSize = api.structureSize < sizeof(outBuilder.m_apiV3)
                                    ? api.structureSize
                                    : sizeof(outBuilder.m_apiV3);
    SlangNVVMBuilderAPI_V3 retainedAPI = {};
    std::memcpy(&retainedAPI, &api, retainedSize);
    retainedAPI.structureSize = uint32_t(retainedSize);

    outBuilder = compatibilityBuilder;
    outBuilder.m_apiV3 = retainedAPI;
    outBuilder.m_features = retainedAPI.features;
    return SLANG_OK;
}

bool NVVMIRBuilder::supportsScalarOperations() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY);
}

bool NVVMIRBuilder::supportsScalarControlFlow() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW);
}

bool NVVMIRBuilder::supportsScalarSSA() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_SSA);
}

bool NVVMIRBuilder::supportsScalarFunctions() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FUNCTIONS);
}

bool NVVMIRBuilder::supportsScalarPointerArithmetic() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_POINTER_ARITHMETIC);
}

bool NVVMIRBuilder::supportsScalarArrayAddressing() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_ARRAY_ADDRESSING);
}

bool NVVMIRBuilder::supportsScalarIntegerMultiply() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY);
}

bool NVVMIRBuilder::supportsScalarIntegerBitAnd() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_AND);
}

bool NVVMIRBuilder::supportsScalarIntegerBitOr() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_OR);
}

bool NVVMIRBuilder::supportsScalarIntegerBitXor() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_XOR);
}

bool NVVMIRBuilder::supportsScalarIntegerBitNot() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_NOT);
}

bool NVVMIRBuilder::supportsScalarIntegerNegate() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NEGATE);
}

bool NVVMIRBuilder::supportsNVVMIR20Assembly() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_NVVM_IR_2_0_ASSEMBLY);
}

bool NVVMIRBuilder::supportsRelaxedGlobalI32AtomicAdd() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_RELAXED_GLOBAL_I32_ATOMIC_ADD);
}

bool NVVMIRBuilder::supportsScalarIntegerEqual() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_EQUAL);
}

bool NVVMIRBuilder::supportsScalarIntegerNotEqual() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NOT_EQUAL);
}

bool NVVMIRBuilder::supportsScalarIntegerSignedGreaterThan() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_THAN);
}

bool NVVMIRBuilder::supportsScalarIntegerSignedLessEqual() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_LESS_EQUAL);
}

bool NVVMIRBuilder::supportsScalarIntegerSignedGreaterEqual() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_EQUAL);
}

bool NVVMIRBuilder::supportsRawRWStructuredBufferI32() const
{
    return supportsFeature(SLANG_NVVM_BUILDER_FEATURE_RAW_RW_STRUCTURED_BUFFER_I32);
}

bool NVVMIRBuilder::supportsFeature(SlangNVVMBuilderFeature_3 feature) const
{
    return feature < SLANG_NVVM_BUILDER_FEATURE_COUNT_3 && _hasFeature(m_features, feature);
}

bool NVVMIRBuilder::supportsFeatures(const SlangNVVMBuilderFeatureSet_3& requiredFeatures) const
{
    for (uint32_t i = 0; i < SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT_3; ++i)
    {
        const uint32_t firstFeature = i * 64u;
        const uint32_t remainingFeatures = SLANG_NVVM_BUILDER_FEATURE_COUNT_3 > firstFeature
                                               ? SLANG_NVVM_BUILDER_FEATURE_COUNT_3 - firstFeature
                                               : 0;
        const uint64_t knownFeatureMask = remainingFeatures >= 64u ? ~uint64_t(0)
                                          : remainingFeatures
                                              ? (uint64_t(1) << remainingFeatures) - 1u
                                              : 0;
        if ((requiredFeatures.words[i] & ~knownFeatureMask) != 0 ||
            (requiredFeatures.words[i] & ~m_features.words[i]) != 0)
        {
            return false;
        }
    }
    return true;
}

String NVVMIRBuilder::getVersionString() const
{
    if (!isInitialized())
        return String();

    StringBuilder builder;
    if (m_apiV3.structureSize)
    {
        builder << "slang-llvm-nvvm;builder-abi=" << SLANG_NVVM_BUILDER_ABI_VERSION_3
                << ";builder-api-size=" << m_apiV3.structureSize
                << ";llvm=" << m_api.llvmVersionMajor << "." << m_api.llvmVersionMinor << "."
                << m_api.llvmVersionPatch << ";nvvm-ir=" << m_api.nvvmIRVersionMajor << "."
                << m_api.nvvmIRVersionMinor << ";pointer-model=" << uint32_t(m_api.pointerModel)
                << ";feature-words=";
        for (uint32_t i = 0; i < SLANG_NVVM_BUILDER_FEATURE_WORD_COUNT_3; ++i)
        {
            if (i)
                builder << ",";
            builder << m_features.words[i];
        }
        builder << ";timestamp="
                << SharedLibraryUtils::getSharedLibraryTimestamp(
                       reinterpret_cast<void*>(m_api.createModule));
        return builder.produceString();
    }

    builder << "slang-llvm-nvvm;builder-abi="
            << (supportsSerializationDiagnostics() ? SLANG_NVVM_BUILDER_ABI_VERSION_2
                                                   : SLANG_NVVM_BUILDER_ABI_VERSION_1)
            << ";builder-api-size="
            << (supportsSerializationDiagnostics() ? m_apiV2.structureSize : m_api.structureSize)
            << ";llvm=" << m_api.llvmVersionMajor << "." << m_api.llvmVersionMinor << "."
            << m_api.llvmVersionPatch << ";nvvm-ir=" << m_api.nvvmIRVersionMajor << "."
            << m_api.nvvmIRVersionMinor << ";pointer-model=" << uint32_t(m_api.pointerModel)
            << ";scalar-operations=" << (supportsScalarOperations() ? 1 : 0)
            << ";scalar-control-flow=" << (supportsScalarControlFlow() ? 1 : 0)
            << ";scalar-ssa=" << (supportsScalarSSA() ? 1 : 0)
            << ";scalar-functions=" << (supportsScalarFunctions() ? 1 : 0)
            << ";scalar-pointer-arithmetic=" << (supportsScalarPointerArithmetic() ? 1 : 0)
            << ";scalar-array-addressing=" << (supportsScalarArrayAddressing() ? 1 : 0)
            << ";scalar-integer-multiply=" << (supportsScalarIntegerMultiply() ? 1 : 0)
            << ";scalar-integer-bit-and=" << (supportsScalarIntegerBitAnd() ? 1 : 0)
            << ";scalar-integer-bit-or=" << (supportsScalarIntegerBitOr() ? 1 : 0)
            << ";scalar-integer-bit-xor=" << (supportsScalarIntegerBitXor() ? 1 : 0)
            << ";scalar-integer-bit-not=" << (supportsScalarIntegerBitNot() ? 1 : 0)
            << ";scalar-integer-negate=" << (supportsScalarIntegerNegate() ? 1 : 0)
            << ";nvvm-ir-2.0-assembly=" << (supportsNVVMIR20Assembly() ? 1 : 0)
            << ";relaxed-global-i32-atomic-add=" << (supportsRelaxedGlobalI32AtomicAdd() ? 1 : 0)
            << ";scalar-integer-equal=" << (supportsScalarIntegerEqual() ? 1 : 0)
            << ";scalar-integer-not-equal=" << (supportsScalarIntegerNotEqual() ? 1 : 0)
            << ";scalar-integer-signed-greater-than="
            << (supportsScalarIntegerSignedGreaterThan() ? 1 : 0)
            << ";scalar-integer-signed-less-equal="
            << (supportsScalarIntegerSignedLessEqual() ? 1 : 0)
            << ";scalar-integer-signed-greater-equal="
            << (supportsScalarIntegerSignedGreaterEqual() ? 1 : 0)
            << ";raw-rw-structured-buffer-i32=" << (supportsRawRWStructuredBufferI32() ? 1 : 0)
            << ";timestamp="
            << SharedLibraryUtils::getSharedLibraryTimestamp(
                   reinterpret_cast<void*>(m_api.createModule));
    return builder.produceString();
}

SlangResult NVVMIRBuilder::createModule(
    const UnownedStringSlice& moduleName,
    SlangNVVMModuleHandle_1& outModule) const
{
    outModule = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult_1 result =
        m_api.createModule(moduleName.begin(), moduleName.getLength(), &outModule);
    return _validateHandleResult(result, outModule);
}

void NVVMIRBuilder::destroyModule(SlangNVVMModuleHandle_1 module) const
{
    if (isInitialized())
        m_api.destroyModule(module);
}

SlangResult NVVMIRBuilder::getVoidType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult_1 result = m_api.getVoidType(module, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getIntegerType(
    SlangNVVMModuleHandle_1 module,
    uint32_t bitWidth,
    SlangNVVMTypeHandle_1& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.getIntegerType(module, bitWidth, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getFloatingPointType(
    SlangNVVMModuleHandle_1 module,
    uint32_t bitWidth,
    SlangNVVMTypeHandle_1& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_DIVIDE) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NEGATE) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_EQUAL) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NOT_EQUAL) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_THAN) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_EQUAL) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_EQUAL) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_THAN) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI) &&
        !supportsFeature(SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV3.getFloatingPointType(module, bitWidth, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getPointerType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 pointeeType,
    SlangNVVMAddressSpace_2 addressSpace,
    SlangNVVMTypeHandle_1& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.getPointerType(module, pointeeType, addressSpace, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::getFunctionType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 resultType,
    const SlangNVVMTypeHandle_1* parameterTypes,
    size_t parameterCount,
    SlangNVVMTypeHandle_1& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult_1 result =
        m_api.getFunctionType(module, resultType, parameterTypes, parameterCount, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::declareFunction(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 functionType,
    const UnownedStringSlice& name,
    SlangNVVMValueHandle_1& outFunction) const
{
    outFunction = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult_1 result =
        m_api.declareFunction(module, functionType, name.begin(), name.getLength(), &outFunction);
    return _validateHandleResult(result, outFunction);
}

SlangResult NVVMIRBuilder::getFunctionParameter(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function,
    size_t parameterIndex,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.getFunctionParameter(module, function, parameterIndex, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::createBlock(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function,
    const UnownedStringSlice& name,
    SlangNVVMBlockHandle_1& outBlock) const
{
    outBlock = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    const SlangNVVMResult_1 result =
        m_api.createBlock(module, function, name.begin(), name.getLength(), &outBlock);
    return _validateHandleResult(result, outBlock);
}

SlangResult NVVMIRBuilder::setInsertBlock(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMBlockHandle_1 block) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    return m_api.setInsertBlock(module, block);
}

SlangResult NVVMIRBuilder::emitLoad(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 pointer,
    uint32_t alignment,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitLoad(module, pointer, alignment, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitStore(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1 pointer,
    uint32_t alignment) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarOperations())
        return SLANG_E_NOT_AVAILABLE;
    return m_apiV2.emitStore(module, value, pointer, alignment);
}

SlangResult NVVMIRBuilder::emitIntegerBinary(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerBinaryOp_2 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarControlFlow())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitIntegerBinary(module, operation, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerUnary(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerUnaryOp_3 operation,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    SlangNVVMBuilderFeature_3 feature = SLANG_NVVM_BUILDER_FEATURE_COUNT_3;
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_NOT;
        break;
    case SLANG_NVVM_INTEGER_UNARY_OP_NEGATE:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NEGATE;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }
    if (!supportsFeature(feature))
        return SLANG_E_NOT_AVAILABLE;

    if (m_apiV3.structureSize)
    {
        const SlangNVVMResult_1 result =
            m_apiV3.emitIntegerUnary(module, operation, value, &outValue);
        return _validateHandleResult(result, outValue);
    }
    return operation == SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT
               ? emitIntegerBitNot(module, value, outValue)
               : emitIntegerNegate(module, value, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBinaryOperation(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerBinaryOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    SlangNVVMBuilderFeature_3 feature = SLANG_NVVM_BUILDER_FEATURE_COUNT_3;
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_BINARY_OP_3_ADD:
    case SLANG_NVVM_INTEGER_BINARY_OP_3_SUB:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW;
        break;
    case SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY;
        break;
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_AND:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_AND;
        break;
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_OR:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_OR;
        break;
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_XOR:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_XOR;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }
    if (!supportsFeature(feature))
        return SLANG_E_NOT_AVAILABLE;

    if (m_apiV3.structureSize)
    {
        const SlangNVVMResult_1 result =
            m_apiV3.emitIntegerBinary(module, operation, left, right, &outValue);
        return _validateHandleResult(result, outValue);
    }

    switch (operation)
    {
    case SLANG_NVVM_INTEGER_BINARY_OP_3_ADD:
        return emitIntegerBinary(module, SLANG_NVVM_INTEGER_BINARY_OP_ADD, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_SUB:
        return emitIntegerBinary(module, SLANG_NVVM_INTEGER_BINARY_OP_SUB, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY:
        return emitIntegerMultiply(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_AND:
        return emitIntegerBitAnd(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_OR:
        return emitIntegerBitOr(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_XOR:
        return emitIntegerBitXor(module, left, right, outValue);
    default:
        return SLANG_E_INVALID_ARG;
    }
}

SlangResult NVVMIRBuilder::emitIntegerCompare(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerCompareOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    SlangNVVMBuilderFeature_3 feature = SLANG_NVVM_BUILDER_FEATURE_COUNT_3;
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW;
        break;
    case SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_EQUAL;
        break;
    case SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NOT_EQUAL;
        break;
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_THAN;
        break;
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_LESS_EQUAL;
        break;
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_EQUAL;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }
    if (!supportsFeature(feature))
        return SLANG_E_NOT_AVAILABLE;

    if (m_apiV3.structureSize)
    {
        const SlangNVVMResult_1 result =
            m_apiV3.emitIntegerCompare(module, operation, left, right, &outValue);
        return _validateHandleResult(result, outValue);
    }

    switch (operation)
    {
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN:
        return emitIntegerSignedLessThan(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL:
        return emitIntegerEqual(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL:
        return emitIntegerNotEqual(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN:
        return emitIntegerSignedGreaterThan(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL:
        return emitIntegerSignedLessEqual(module, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL:
        return emitIntegerSignedGreaterEqual(module, left, right, outValue);
    default:
        return SLANG_E_INVALID_ARG;
    }
}

SlangResult NVVMIRBuilder::emitFloatingBinary(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMFloatingBinaryOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    SlangNVVMBuilderFeature_3 requiredFeature = 0;
    switch (operation)
    {
    case SLANG_NVVM_FLOATING_BINARY_OP_ADD:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD;
        break;
    case SLANG_NVVM_FLOATING_BINARY_OP_SUBTRACT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT;
        break;
    case SLANG_NVVM_FLOATING_BINARY_OP_MULTIPLY:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY;
        break;
    case SLANG_NVVM_FLOATING_BINARY_OP_DIVIDE:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_DIVIDE;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }
    if (!supportsFeature(requiredFeature))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV3.emitFloatingBinary(module, operation, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitFloatingUnary(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMFloatingUnaryOp_3 operation,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (operation != SLANG_NVVM_FLOATING_UNARY_OP_NEGATE)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NEGATE))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV3.emitFloatingUnary(module, operation, value, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitFloatingCompare(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMFloatingCompareOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    SlangNVVMBuilderFeature_3 feature = 0;
    switch (operation)
    {
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_EQUAL:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_EQUAL;
        break;
    case SLANG_NVVM_FLOATING_COMPARE_OP_UNORDERED_NOT_EQUAL:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NOT_EQUAL;
        break;
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_THAN:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_THAN;
        break;
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_EQUAL:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_EQUAL;
        break;
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_EQUAL:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_EQUAL;
        break;
    case SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_THAN:
        feature = SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_THAN;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }
    if (!supportsFeature(feature))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV3.emitFloatingCompare(module, operation, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::getFloatingPointConstant(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 floatingPointType,
    uint32_t bitWidth,
    uint64_t bitPattern,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (bitWidth != 32 || (bitPattern >> 32) != 0)
        return SLANG_E_INVALID_ARG;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_CONSTANT))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV3
            .getFloatingPointConstant(module, floatingPointType, bitWidth, bitPattern, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitPhi(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMBlockHandle_1 targetBlock,
    SlangNVVMTypeHandle_1 type,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV3.emitPhi(module, targetBlock, type, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::addPhiIncoming(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 phi,
    SlangNVVMValueHandle_1 value,
    SlangNVVMBlockHandle_1 predecessorBlock) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_PHI))
        return SLANG_E_NOT_AVAILABLE;
    return m_apiV3.addPhiIncoming(module, phi, value, predecessorBlock);
}

SlangResult NVVMIRBuilder::emitCall(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 callee,
    const SlangNVVMValueHandle_1* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV3.emitCall(module, callee, arguments, argumentCount, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitValueReturn(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsFeature(SLANG_NVVM_BUILDER_FEATURE_GENERIC_SCALAR_FUNCTIONS))
        return SLANG_E_NOT_AVAILABLE;
    return m_apiV3.emitValueReturn(module, value);
}

SlangResult NVVMIRBuilder::emitIntrinsic(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntrinsicOp_3 operation,
    const SlangNVVMValueHandle_1* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    SlangNVVMBuilderFeature_3 requiredFeature = 0;
    switch (operation)
    {
    case SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_INDEX:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_COUNT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_UINT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_INT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_FLOAT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_BALLOT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_UINT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_UINT;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_INT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_INT;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_FLOAT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_FLOAT;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_IS_FIRST_LANE:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_IS_FIRST_LANE;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ANY_TRUE:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ANY_TRUE;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_TRUE:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_TRUE;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_INT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_INT;
        break;
    case SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_UINT:
        requiredFeature = SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_UINT;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }
    if (!supportsFeature(requiredFeature))
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV3.emitIntrinsic(module, operation, arguments, argumentCount, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerSignedLessThan(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarControlFlow())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitIntegerSignedLessThan(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitBranch(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMBlockHandle_1 targetBlock) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarControlFlow())
        return SLANG_E_NOT_AVAILABLE;
    return m_apiV2.emitBranch(module, targetBlock);
}

SlangResult NVVMIRBuilder::emitConditionalBranch(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 condition,
    SlangNVVMBlockHandle_1 trueBlock,
    SlangNVVMBlockHandle_1 falseBlock) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarControlFlow())
        return SLANG_E_NOT_AVAILABLE;
    return m_apiV2.emitConditionalBranch(module, condition, trueBlock, falseBlock);
}

SlangResult NVVMIRBuilder::getIntegerConstant(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 integerType,
    int64_t value,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarSSA())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.getIntegerConstant(module, integerType, value, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerPhi(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMBlockHandle_1 targetBlock,
    SlangNVVMTypeHandle_1 integerType,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarSSA())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitIntegerPhi(module, targetBlock, integerType, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::addIntegerPhiIncoming(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 phi,
    SlangNVVMValueHandle_1 value,
    SlangNVVMBlockHandle_1 predecessorBlock) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarSSA())
        return SLANG_E_NOT_AVAILABLE;
    return m_apiV2.addIntegerPhiIncoming(module, phi, value, predecessorBlock);
}

SlangResult NVVMIRBuilder::emitIntegerCall(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 callee,
    const SlangNVVMValueHandle_1* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarFunctions())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitIntegerCall(module, callee, arguments, argumentCount, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerReturn(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarFunctions())
        return SLANG_E_NOT_AVAILABLE;
    return m_apiV2.emitIntegerReturn(module, value);
}

SlangResult NVVMIRBuilder::emitPointerOffset(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 basePointer,
    SlangNVVMValueHandle_1 elementOffset,
    SlangNVVMValueHandle_1& outPointer) const
{
    outPointer = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarPointerArithmetic())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitPointerOffset(module, basePointer, elementOffset, &outPointer);
    return _validateHandleResult(result, outPointer);
}

SlangResult NVVMIRBuilder::getArrayType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle_1& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarArrayAddressing())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.getArrayType(module, elementType, elementCount, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::emitArrayElementPointer(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 baseArrayPointer,
    SlangNVVMValueHandle_1 elementIndex,
    SlangNVVMValueHandle_1& outPointer) const
{
    outPointer = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarArrayAddressing())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitArrayElementPointer(module, baseArrayPointer, elementIndex, &outPointer);
    return _validateHandleResult(result, outPointer);
}

SlangResult NVVMIRBuilder::emitIntegerMultiply(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerMultiply())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitIntegerMultiply(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBitAnd(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerBitAnd())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitIntegerBitAnd(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBitOr(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerBitOr())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitIntegerBitOr(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBitXor(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerBitXor())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitIntegerBitXor(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerBitNot(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerBitNot())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitIntegerBitNot(module, value, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerNegate(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerNegate())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitIntegerNegate(module, value, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitRelaxedGlobalI32AtomicAdd(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 pointer,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1& outOriginalValue) const
{
    outOriginalValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsRelaxedGlobalI32AtomicAdd())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitRelaxedGlobalI32AtomicAdd(module, pointer, value, &outOriginalValue);
    return _validateHandleResult(result, outOriginalValue);
}

SlangResult NVVMIRBuilder::emitIntegerEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerEqual())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitIntegerEqual(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerNotEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerNotEqual())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitIntegerNotEqual(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerSignedGreaterThan(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerSignedGreaterThan())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitIntegerSignedGreaterThan(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerSignedLessEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerSignedLessEqual())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitIntegerSignedLessEqual(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::emitIntegerSignedGreaterEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue) const
{
    outValue = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsScalarIntegerSignedGreaterEqual())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result =
        m_apiV2.emitIntegerSignedGreaterEqual(module, left, right, &outValue);
    return _validateHandleResult(result, outValue);
}

SlangResult NVVMIRBuilder::getRawRWStructuredBufferI32Type(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1& outType) const
{
    outType = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsRawRWStructuredBufferI32())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.getRawRWStructuredBufferI32Type(module, &outType);
    return _validateHandleResult(result, outType);
}

SlangResult NVVMIRBuilder::emitRawRWStructuredBufferI32ElementPointer(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 buffer,
    SlangNVVMValueHandle_1 elementIndex,
    SlangNVVMValueHandle_1& outPointer) const
{
    outPointer = nullptr;
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsRawRWStructuredBufferI32())
        return SLANG_E_NOT_AVAILABLE;
    const SlangNVVMResult_1 result = m_apiV2.emitRawRWStructuredBufferI32ElementPointer(
        module,
        buffer,
        elementIndex,
        &outPointer);
    return _validateHandleResult(result, outPointer);
}

SlangResult NVVMIRBuilder::emitReturnVoid(SlangNVVMModuleHandle_1 module) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    return m_api.emitReturnVoid(module);
}

SlangResult NVVMIRBuilder::markFunctionAsKernel(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function) const
{
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    return m_api.markFunctionAsKernel(module, function);
}

SlangResult NVVMIRBuilder::serializeModule(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMSerializationFormat_1 format,
    ComPtr<ISlangBlob>& outBlob) const
{
    if (supportsSerializationDiagnostics())
    {
        String diagnostics;
        return serializeModule(module, format, outBlob, diagnostics);
    }

    outBlob.setNull();
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;

    size_t requiredSize = 0;
    SLANG_RETURN_ON_FAIL(m_api.serializeModule(module, format, nullptr, 0, &requiredSize));
    if (!requiredSize || UInt64(requiredSize) > UInt64(kMaxIndex))
        return SLANG_FAIL;

    List<uint8_t> storage;
    storage.setCount(Index(requiredSize));
    size_t actualSize = 0;
    SLANG_RETURN_ON_FAIL(
        m_api.serializeModule(module, format, storage.getBuffer(), requiredSize, &actualSize));
    if (actualSize != requiredSize)
        return SLANG_FAIL;

    outBlob = ListBlob::moveCreate(storage);
    return SLANG_OK;
}

SlangResult NVVMIRBuilder::serializeModule(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMSerializationFormat_1 format,
    ComPtr<ISlangBlob>& outBlob,
    String& outDiagnostics) const
{
    outBlob.setNull();
    outDiagnostics = String();
    if (!isInitialized())
        return SLANG_E_UNINITIALIZED;
    if (!supportsSerializationDiagnostics())
        return SLANG_E_NOT_AVAILABLE;

    SlangNVVMSerializeModuleWithDiagnostics_2 serializeWithDiagnostics =
        m_apiV2.serializeModuleWithDiagnostics;
    if (format == SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY)
    {
        if (!supportsNVVMIR20Assembly())
            return SLANG_E_NOT_AVAILABLE;
        serializeWithDiagnostics = m_apiV2.serializeNVVMIR20AssemblyWithDiagnostics;
    }

    size_t requiredSerializedSize = 0;
    size_t requiredDiagnosticSize = 0;
    SlangNVVMVerificationStatus_2 queryStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_RETURN_ON_FAIL(serializeWithDiagnostics(
        module,
        format,
        nullptr,
        0,
        &requiredSerializedSize,
        nullptr,
        0,
        &requiredDiagnosticSize,
        &queryStatus));

    if (!_isVerificationStatus(queryStatus) || UInt64(requiredSerializedSize) > UInt64(kMaxIndex) ||
        UInt64(requiredDiagnosticSize) > UInt64(kMaxIndex))
    {
        return SLANG_FAIL;
    }
    if (queryStatus == SLANG_NVVM_VERIFICATION_VALID)
    {
        if (!requiredSerializedSize)
            return SLANG_FAIL;
    }
    else if (requiredSerializedSize || !requiredDiagnosticSize)
    {
        return SLANG_FAIL;
    }

    List<uint8_t> serializedStorage;
    serializedStorage.setCount(Index(requiredSerializedSize));
    List<char> diagnosticStorage;
    diagnosticStorage.setCount(Index(requiredDiagnosticSize));

    size_t actualSerializedSize = 0;
    size_t actualDiagnosticSize = 0;
    SlangNVVMVerificationStatus_2 writeStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_RETURN_ON_FAIL(serializeWithDiagnostics(
        module,
        format,
        serializedStorage.getCount() ? serializedStorage.getBuffer() : nullptr,
        requiredSerializedSize,
        &actualSerializedSize,
        diagnosticStorage.getCount() ? diagnosticStorage.getBuffer() : nullptr,
        requiredDiagnosticSize,
        &actualDiagnosticSize,
        &writeStatus));

    if (actualSerializedSize != requiredSerializedSize ||
        actualDiagnosticSize != requiredDiagnosticSize || writeStatus != queryStatus)
    {
        return SLANG_FAIL;
    }

    if (requiredDiagnosticSize)
    {
        outDiagnostics = String(
            UnownedStringSlice(diagnosticStorage.getBuffer(), Index(requiredDiagnosticSize)));
    }
    if (queryStatus == SLANG_NVVM_VERIFICATION_INVALID)
        return SLANG_FAIL;

    outBlob = ListBlob::moveCreate(serializedStorage);
    return SLANG_OK;
}

} // namespace Slang
