// unit-test-optix-multiple-definition.cpp
//
// TODO: This is a temporary end-to-end reproducer. After the duplicate-definition issue is fixed,
// refactor this coverage and/or move it to slang-rhi, where OptiX integration tests normally live.

#if defined(SLANG_UNIT_TEST_ENABLE_OPTIX)

#include "core/slang-platform.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <type_traits>

// slang-unit-test-tool is also loaded on machines without an NVIDIA driver. Avoid a link-time
// dependency on the CUDA driver so those machines can load the module and ignore this test. The
// test still requires both CUDA and OptiX when it executes.
typedef struct CUctx_st* CUcontext;
typedef struct CUstream_st* CUstream;

#define OPTIX_DONT_INCLUDE_CUDA
#define OPTIX_ENABLE_SDK_MIXING
#include <optix.h>
#include <optix_function_table_definition.h>
#include <optix_stubs.h>

using namespace Slang;

namespace
{

typedef int CudaResult;
typedef int CudaDevice;

static const CudaResult kCudaSuccess = 0;

// The test needs a CUDA context for OptiX, but must not make the entire Slang unit-test module
// depend on the CUDA driver. Resolve just the context-management entry points dynamically so an
// OptiX-enabled build can still report this test as ignored on a machine without an NVIDIA driver.
struct CudaDriverApi
{
    CudaResult (*cuInit)(unsigned int flags) = nullptr;
    CudaResult (*cuDeviceGetCount)(int* count) = nullptr;
    CudaResult (*cuDeviceGet)(CudaDevice* device, int ordinal) = nullptr;
    CudaResult (*cuDevicePrimaryCtxRetain)(CUcontext* context, CudaDevice device) = nullptr;
    CudaResult (*cuDevicePrimaryCtxRelease)(CudaDevice device) = nullptr;
    CudaResult (*cuCtxGetCurrent)(CUcontext* context) = nullptr;
    CudaResult (*cuCtxSetCurrent)(CUcontext context) = nullptr;

    SharedLibrary::Handle library = nullptr;

    bool load()
    {
#if SLANG_WINDOWS_FAMILY
        const char* const libraryNames[] = {"nvcuda.dll"};
#elif SLANG_LINUX_FAMILY
        const char* const libraryNames[] = {"libcuda.so.1", "libcuda.so"};
#else
        return false;
#endif

#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
        for (const char* name : libraryNames)
        {
            if (SLANG_SUCCEEDED(SharedLibrary::loadWithPlatformPath(name, library)))
                break;
        }
        if (!library)
            return false;

        bool allFound = true;
        auto resolve = [&](auto& function, const char* name)
        {
            function = reinterpret_cast<std::remove_reference_t<decltype(function)>>(
                SharedLibrary::findSymbolAddressByName(library, name));
            if (!function)
                allFound = false;
        };

        resolve(cuInit, "cuInit");
        resolve(cuDeviceGetCount, "cuDeviceGetCount");
        resolve(cuDeviceGet, "cuDeviceGet");
        resolve(cuDevicePrimaryCtxRetain, "cuDevicePrimaryCtxRetain");
        resolve(cuDevicePrimaryCtxRelease, "cuDevicePrimaryCtxRelease_v2");
        resolve(cuCtxGetCurrent, "cuCtxGetCurrent");
        resolve(cuCtxSetCurrent, "cuCtxSetCurrent");
        return allFound;
#endif
    }

    ~CudaDriverApi()
    {
        if (library)
            SharedLibrary::unload(library);
    }
};

// Restore the caller's CUDA context and release the retained primary context on every exit path,
// including assertion failures thrown by SLANG_CHECK_ABORT.
struct CudaPrimaryContextGuard
{
    CudaDriverApi* api = nullptr;
    CudaDevice device = 0;
    CUcontext previousContext = nullptr;
    bool retained = false;

    ~CudaPrimaryContextGuard()
    {
        if (!api)
            return;
        api->cuCtxSetCurrent(previousContext);
        if (retained)
            api->cuDevicePrimaryCtxRelease(device);
    }
};

struct OptixDeviceContextGuard
{
    OptixDeviceContext context = nullptr;
    ~OptixDeviceContextGuard()
    {
        if (context)
            optixDeviceContextDestroy(context);
    }
};

struct OptixModuleGuard
{
    OptixModule module = nullptr;
    ~OptixModuleGuard()
    {
        if (module)
            optixModuleDestroy(module);
    }
};

struct OptixProgramGroupsGuard
{
    OptixProgramGroup groups[2] = {};
    ~OptixProgramGroupsGuard()
    {
        for (OptixProgramGroup group : groups)
        {
            if (group)
                optixProgramGroupDestroy(group);
        }
    }
};

struct OptixPipelineGuard
{
    OptixPipeline pipeline = nullptr;
    ~OptixPipelineGuard()
    {
        if (pipeline)
            optixPipelineDestroy(pipeline);
    }
};

static bool _checkSlangResult(SlangResult result, slang::IBlob* diagnostics, const char* operation)
{
    if (SLANG_SUCCEEDED(result))
        return true;

    fprintf(stderr, "%s failed", operation);
    if (diagnostics && diagnostics->getBufferSize())
    {
        fprintf(
            stderr,
            ":\n%.*s",
            int(diagnostics->getBufferSize()),
            static_cast<const char*>(diagnostics->getBufferPointer()));
    }
    fprintf(stderr, "\n");
    return false;
}

static bool _checkOptixResult(OptixResult result, const char* operation, const char* log = nullptr)
{
    if (result == OPTIX_SUCCESS)
        return true;

    StringBuilder message;
    message << operation << " failed: " << optixGetErrorName(result) << " ("
            << optixGetErrorString(result) << ")";
    if (log && log[0])
        message << "\n" << log;
    getTestReporter()->message(TestMessageType::TestFailure, message.toString().getBuffer());
    return false;
}

// Compile one entry point at a time from the shared source module. Each returned PTX blob therefore
// contains its own copy of every reachable synthesized helper, matching the way applications build
// separate OptiX modules for ray-generation and callable entry points.
static ComPtr<slang::IBlob> _compileEntryPoint(
    slang::ISession* session,
    slang::IModule* module,
    const char* entryPointName)
{
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IEntryPoint> entryPoint;
    SlangResult result = module->findEntryPointByName(entryPointName, entryPoint.writeRef());
    SLANG_CHECK_ABORT(_checkSlangResult(result, diagnostics, "findEntryPointByName"));

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> program;
    result = session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        program.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(_checkSlangResult(result, diagnostics, "createCompositeComponentType"));

    ComPtr<slang::IBlob> code;
    diagnostics.setNull();
    result = program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    SLANG_CHECK_ABORT(_checkSlangResult(result, diagnostics, "getEntryPointCode"));
    SLANG_CHECK_ABORT(code && code->getBufferSize() != 0);
    return code;
}

} // namespace

// Reproduce the multiple-definition failure that occurs when OptiX links separately compiled Slang
// entry points. Both entry points construct CallablePayload, so both PTX modules define the same
// synthesized CallablePayload initializer. Relocatable device code makes that helper externally
// visible unless Slang or the downstream toolchain gives it module-local linkage.
SLANG_UNIT_TEST(optixMultipleDefinition)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        SLANG_IGNORE_TEST;
    }

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != kCudaSuccess)
    {
        SLANG_IGNORE_TEST;
    }

    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != kCudaSuccess || deviceCount == 0)
    {
        SLANG_IGNORE_TEST;
    }

    CudaPrimaryContextGuard cudaContext;
    cudaContext.api = &cuda;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&cudaContext.device, 0) == kCudaSuccess);
    SLANG_CHECK_ABORT(cuda.cuCtxGetCurrent(&cudaContext.previousContext) == kCudaSuccess);

    CUcontext primaryContext = nullptr;
    SLANG_CHECK_ABORT(
        cuda.cuDevicePrimaryCtxRetain(&primaryContext, cudaContext.device) == kCudaSuccess);
    cudaContext.retained = true;
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(primaryContext) == kCudaSuccess);

    if (optixInit() != OPTIX_SUCCESS)
    {
        SLANG_IGNORE_TEST;
    }

    const char* source = R"(
        struct CallablePayload
        {
            uint x;
            uint y;
        };

        [shader("raygeneration")]
        void rayGen()
        {
            CallablePayload payload = {1, 2};
            CallShader(0, payload);
        }

        [shader("callable")]
        void callableMain(inout CallablePayload payload)
        {
            CallablePayload replacement = {payload.x + 1, payload.y + 1};
            payload = replacement;
        }
    )";

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module = session->loadModuleFromSourceString(
        "optixMultipleDefinition",
        "optix-multiple-definition.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(_checkSlangResult(
        module ? SLANG_OK : SLANG_FAIL,
        diagnostics,
        "loadModuleFromSourceString"));

    ComPtr<slang::IBlob> rayGenPtx = _compileEntryPoint(session, module, "rayGen");
    ComPtr<slang::IBlob> callablePtx = _compileEntryPoint(session, module, "callableMain");

    OptixDeviceContextOptions contextOptions = {};
    OptixDeviceContextGuard optixContext;
    SLANG_CHECK_ABORT(_checkOptixResult(
        optixDeviceContextCreate(primaryContext, &contextOptions, &optixContext.context),
        "optixDeviceContextCreate"));

    OptixModuleCompileOptions moduleOptions = {};
    moduleOptions.optLevel = OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
    moduleOptions.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_DEFAULT;

    OptixPipelineCompileOptions pipelineOptions = {};
    pipelineOptions.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_ANY;
    pipelineOptions.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;

    OptixModuleGuard rayGenModule;
    char rayGenLog[8192] = {};
    size_t rayGenLogSize = sizeof(rayGenLog);
    SLANG_CHECK_ABORT(_checkOptixResult(
        optixModuleCreate(
            optixContext.context,
            &moduleOptions,
            &pipelineOptions,
            static_cast<const char*>(rayGenPtx->getBufferPointer()),
            rayGenPtx->getBufferSize(),
            rayGenLog,
            &rayGenLogSize,
            &rayGenModule.module),
        "optixModuleCreate(rayGen)",
        rayGenLog));

    OptixModuleGuard callableModule;
    char callableLog[8192] = {};
    size_t callableLogSize = sizeof(callableLog);
    SLANG_CHECK_ABORT(_checkOptixResult(
        optixModuleCreate(
            optixContext.context,
            &moduleOptions,
            &pipelineOptions,
            static_cast<const char*>(callablePtx->getBufferPointer()),
            callablePtx->getBufferSize(),
            callableLog,
            &callableLogSize,
            &callableModule.module),
        "optixModuleCreate(callableMain)",
        callableLog));

    OptixProgramGroupDesc programGroupDescs[2] = {};
    programGroupDescs[0].kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
    programGroupDescs[0].raygen.module = rayGenModule.module;
    programGroupDescs[0].raygen.entryFunctionName = "__raygen__rayGen";
    programGroupDescs[1].kind = OPTIX_PROGRAM_GROUP_KIND_CALLABLES;
    programGroupDescs[1].callables.moduleDC = callableModule.module;
    programGroupDescs[1].callables.entryFunctionNameDC = "__direct_callable__callableMain";

    OptixProgramGroupOptions programGroupOptions = {};
    OptixProgramGroupsGuard programGroups;
    char programGroupLog[8192] = {};
    size_t programGroupLogSize = sizeof(programGroupLog);
    SLANG_CHECK_ABORT(_checkOptixResult(
        optixProgramGroupCreate(
            optixContext.context,
            programGroupDescs,
            SLANG_COUNT_OF(programGroupDescs),
            &programGroupOptions,
            programGroupLog,
            &programGroupLogSize,
            programGroups.groups),
        "optixProgramGroupCreate",
        programGroupLog));

    OptixPipelineLinkOptions linkOptions = {};
    linkOptions.maxTraceDepth = 1;

    OptixPipelineGuard pipeline;
    char pipelineLog[8192] = {};
    size_t pipelineLogSize = sizeof(pipelineLog);
    SLANG_CHECK_ABORT(_checkOptixResult(
        optixPipelineCreate(
            optixContext.context,
            &pipelineOptions,
            &linkOptions,
            programGroups.groups,
            SLANG_COUNT_OF(programGroups.groups),
            pipelineLog,
            &pipelineLogSize,
            &pipeline.pipeline),
        "optixPipelineCreate",
        pipelineLog));
}

#endif // SLANG_UNIT_TEST_ENABLE_OPTIX
