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
    if (api.structureSize < SLANG_NVVM_BUILDER_API_V2_MIN_SIZE || hasPartialScalarPrefix ||
        hasPartialScalarControlFlowPrefix || hasPartialScalarSSAPrefix ||
        hasPartialScalarFunctionPrefix || hasPartialScalarPointerArithmeticPrefix ||
        hasPartialScalarArrayPrefix || hasPartialScalarIntegerMultiplyPrefix ||
        hasPartialScalarIntegerBitAndPrefix || hasPartialScalarIntegerBitOrPrefix ||
        hasPartialScalarIntegerBitXorPrefix || hasPartialScalarIntegerBitNotPrefix ||
        hasPartialScalarIntegerNegatePrefix || hasPartialRelaxedGlobalI32AtomicAddPrefix ||
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
         !_supportsRelaxedGlobalI32AtomicAdd(api)))
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
    outBuilder.m_library = library;
    return SLANG_OK;
}

bool NVVMIRBuilder::supportsScalarOperations() const
{
    return _supportsScalarOperations(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarControlFlow() const
{
    return _supportsScalarControlFlow(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarSSA() const
{
    return _supportsScalarSSA(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarFunctions() const
{
    return _supportsScalarFunctions(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarPointerArithmetic() const
{
    return _supportsScalarPointerArithmetic(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarArrayAddressing() const
{
    return _supportsScalarArrayAddressing(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarIntegerMultiply() const
{
    return _supportsScalarIntegerMultiply(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarIntegerBitAnd() const
{
    return _supportsScalarIntegerBitAnd(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarIntegerBitOr() const
{
    return _supportsScalarIntegerBitOr(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarIntegerBitXor() const
{
    return _supportsScalarIntegerBitXor(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarIntegerBitNot() const
{
    return _supportsScalarIntegerBitNot(m_apiV2);
}

bool NVVMIRBuilder::supportsScalarIntegerNegate() const
{
    return _supportsScalarIntegerNegate(m_apiV2);
}

bool NVVMIRBuilder::supportsNVVMIR20Assembly() const
{
    return _supportsNVVMIR20Assembly(m_apiV2);
}

bool NVVMIRBuilder::supportsRelaxedGlobalI32AtomicAdd() const
{
    return _supportsRelaxedGlobalI32AtomicAdd(m_apiV2);
}

String NVVMIRBuilder::getVersionString() const
{
    if (!isInitialized())
        return String();

    StringBuilder builder;
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
