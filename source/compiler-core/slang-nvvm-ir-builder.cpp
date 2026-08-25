#include "slang-nvvm-ir-builder.h"

#include "core/slang-blob.h"
#include "slang-downstream-compiler-util.h"

namespace Slang
{

static bool _hasRequiredFunctions(const SlangNVVMBuilderAPI_V1& api)
{
    return api.createModule && api.destroyModule && api.getVoidType && api.getFunctionType &&
           api.declareFunction && api.createBlock && api.setInsertBlock && api.emitReturnVoid &&
           api.markFunctionAsKernel && api.serializeModule;
}

// Rejects an ABI implementation that reports success without producing its required handle.
static SlangResult _validateHandleResult(SlangNVVMResult_1 result, const void* handle)
{
    if (result < 0)
        return result;
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
    if (api.structureSize != sizeof(api) || api.abiVersion != SLANG_NVVM_BUILDER_ABI_VERSION_1 ||
        api.llvmVersionMajor != 14 || api.llvmVersionMinor != 0 || api.llvmVersionPatch != 6 ||
        api.nvvmIRVersionMajor != 2 || api.nvvmIRVersionMinor != 0 ||
        api.pointerModel != SLANG_NVVM_POINTER_MODEL_TYPED || !_hasRequiredFunctions(api))
    {
        return SLANG_E_NO_INTERFACE;
    }
    if (!library)
        return SLANG_E_INVALID_ARG;

    outBuilder.m_api = api;
    outBuilder.m_library = library;
    return SLANG_OK;
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

} // namespace Slang
