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

#include <stddef.h>
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
#include <optix_stack_size.h>
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
    CudaResult (*cuMemAlloc)(CUdeviceptr* pointer, size_t size) = nullptr;
    CudaResult (*cuMemFree)(CUdeviceptr pointer) = nullptr;
    CudaResult (*cuMemcpyHtoD)(CUdeviceptr destination, const void* source, size_t size) = nullptr;
    CudaResult (*cuMemcpyDtoH)(void* destination, CUdeviceptr source, size_t size) = nullptr;
    CudaResult (*cuCtxSynchronize)() = nullptr;

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

    // Keep execution symbols optional for the existing compile/link-only test.
    bool loadExecutionFunctions()
    {
        if (!library)
            return false;
        bool allFound = true;
        auto resolve = [&](auto& function, const char* name)
        {
            function = reinterpret_cast<std::remove_reference_t<decltype(function)>>(
                SharedLibrary::findSymbolAddressByName(library, name));
            allFound = allFound && function;
        };
        resolve(cuMemAlloc, "cuMemAlloc_v2");
        resolve(cuMemFree, "cuMemFree_v2");
        resolve(cuMemcpyHtoD, "cuMemcpyHtoD_v2");
        resolve(cuMemcpyDtoH, "cuMemcpyDtoH_v2");
        resolve(cuCtxSynchronize, "cuCtxSynchronize");
        return allFound;
    }
};

// Device allocations must be released before the surrounding primary-context guard restores
// the caller's context, including when a runtime assertion aborts this test.
struct CudaAllocationGuard
{
    CudaDriverApi* api;
    CUdeviceptr pointer = 0;

    explicit CudaAllocationGuard(CudaDriverApi* driver)
        : api(driver)
    {
    }

    ~CudaAllocationGuard()
    {
        if (pointer)
            api->cuMemFree(pointer);
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
    const char* entryPointName,
    slang::IComponentType** outProgram = nullptr)
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
    if (outProgram)
        *outProgram = program.detach();
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

// Exercise the direct NVVM entry ABI and SBT lifetime without tracing rays. Reusing the pipeline
// and SBT allocation with changed data catches accidentally invariant SBT loads across launches.
SLANG_UNIT_TEST(nvvmOptixRaygenBindings)
{
    auto globalSession = unitTestContext->slangGlobalSession;
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)))
    {
        SLANG_IGNORE_TEST;
    }

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != kCudaSuccess)
    {
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != kCudaSuccess || !deviceCount)
    {
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(cuda.loadExecutionFunctions());

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

    OptixDeviceContextOptions contextOptions = {};
    contextOptions.validationMode = OPTIX_DEVICE_CONTEXT_VALIDATION_MODE_ALL;
    OptixDeviceContextGuard optixContext;
    SLANG_CHECK_ABORT(_checkOptixResult(
        optixDeviceContextCreate(primaryContext, &contextOptions, &optixContext.context),
        "optixDeviceContextCreate"));

    const char* source = R"(
        cbuffer Globals
        {
            RWStructuredBuffer<uint> output;
            uint base;
        };
        [shader("raygeneration")]
        void raygenMain(uniform uint bias)
        {
            uint3 index = DispatchRaysIndex();
            uint3 size = DispatchRaysDimensions();
            uint linear = (index.z * size.y + index.y) * size.x + index.x;
            uint offset = 7 * linear;
            output[offset + 0] = index.x;
            output[offset + 1] = index.y;
            output[offset + 2] = index.z;
            output[offset + 3] = size.x;
            output[offset + 4] = size.y;
            output[offset + 5] = size.z;
            output[offset + 6] = base + bias + linear;
        }
    )";

    // These are the independently specified CUDA buffer and SBT data layouts. Reflection below
    // verifies the compiler agrees; it does not supply the expected offsets or output values.
    struct Parameters
    {
        CUdeviceptr output;
        uint64_t count;
        uint32_t base;
        uint32_t padding;
    };
    struct alignas(OPTIX_SBT_RECORD_ALIGNMENT) RaygenRecord
    {
        char header[OPTIX_SBT_RECORD_HEADER_SIZE];
        uint32_t bias;
    };
    static_assert(sizeof(Parameters) == 24 && offsetof(Parameters, base) == 16);
    static_assert(offsetof(RaygenRecord, bias) == OPTIX_SBT_RECORD_HEADER_SIZE);

    auto capability = globalSession->findCapability("cuda_sm_8_0");
    SLANG_CHECK_ABORT(capability != SLANG_CAPABILITY_UNKNOWN);
    const SlangOptimizationLevel optimizations[] = {
        SLANG_OPTIMIZATION_LEVEL_NONE,
        SLANG_OPTIMIZATION_LEVEL_MAXIMAL};
    for (auto optimization : optimizations)
    {
        slang::CompilerOptionEntry options[3] = {};
        options[0].name = slang::CompilerOptionName::EmitCUDAMethod;
        options[0].value.kind = slang::CompilerOptionValueKind::Int;
        options[0].value.intValue0 = SLANG_EMIT_CUDA_VIA_NVVM;
        options[1].name = slang::CompilerOptionName::Capability;
        options[1].value.kind = slang::CompilerOptionValueKind::Int;
        options[1].value.intValue0 = int32_t(capability);
        options[2].name = slang::CompilerOptionName::Optimization;
        options[2].value.kind = slang::CompilerOptionValueKind::Int;
        options[2].value.intValue0 = optimization;
        slang::TargetDesc target = {};
        target.format = SLANG_PTX;
        target.compilerOptionEntries = options;
        target.compilerOptionEntryCount = SLANG_COUNT_OF(options);
        slang::SessionDesc description = {};
        description.targets = &target;
        description.targetCount = 1;
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(globalSession->createSession(description, session.writeRef())));
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> sourceModule(session->loadModuleFromSourceString(
            "nvvmOptixRaygenBindings",
            "nvvm-optix-raygen-bindings.slang",
            source,
            diagnostics.writeRef()));
        SLANG_CHECK_ABORT(_checkSlangResult(
            sourceModule ? SLANG_OK : SLANG_FAIL,
            diagnostics,
            "loadModuleFromSourceString"));
        ComPtr<slang::IComponentType> program;
        auto ptx = _compileEntryPoint(session, sourceModule, "raygenMain", program.writeRef());
        auto layout = program->getLayout();
        SLANG_CHECK_ABORT(layout && layout->getParameterCount() == 1);
        auto globals = layout->getParameterByIndex(0);
        SLANG_CHECK_ABORT(globals && globals->getOffset() == 0);
        auto globalsType = globals->getTypeLayout();
        SLANG_CHECK_ABORT(globalsType && globalsType->getSize() == sizeof(CUdeviceptr));
        auto fields = globalsType->getElementTypeLayout();
        SLANG_CHECK_ABORT(fields && fields->getFieldCount() == 2);
        SLANG_CHECK_ABORT(fields->getStride() == sizeof(Parameters));
        auto outputIndex = fields->findFieldIndexByName("output");
        auto baseIndex = fields->findFieldIndexByName("base");
        SLANG_CHECK_ABORT(outputIndex >= 0 && baseIndex >= 0);
        auto outputField = fields->getFieldByIndex(unsigned(outputIndex));
        auto baseField = fields->getFieldByIndex(unsigned(baseIndex));
        SLANG_CHECK_ABORT(outputField->getOffset() == 0);
        SLANG_CHECK_ABORT(outputField->getTypeLayout()->getSize() == 16);
        SLANG_CHECK_ABORT(baseField->getOffset() == offsetof(Parameters, base));
        SLANG_CHECK_ABORT(baseField->getTypeLayout()->getSize() == sizeof(uint32_t));
        SLANG_CHECK_ABORT(layout->getEntryPointCount() == 1);
        auto entryLayout = layout->getEntryPointByIndex(0);
        SLANG_CHECK_ABORT(entryLayout && entryLayout->getParameterCount() == 1);
        auto bias = entryLayout->getParameterByIndex(0);
        SLANG_CHECK_ABORT(bias && bias->getOffset() == 0);
        SLANG_CHECK_ABORT(bias->getTypeLayout()->getSize() == sizeof(uint32_t));

        OptixModuleCompileOptions moduleOptions = {};
        moduleOptions.optLevel = OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
        moduleOptions.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_NONE;
        OptixPipelineCompileOptions pipelineOptions = {};
        pipelineOptions.pipelineLaunchParamsVariableName = "SLANG_globalParams";
        OptixModuleGuard module;
        char log[8192] = {};
        size_t logSize = sizeof(log);
        SLANG_CHECK_ABORT(_checkOptixResult(
            optixModuleCreate(
                optixContext.context,
                &moduleOptions,
                &pipelineOptions,
                static_cast<const char*>(ptx->getBufferPointer()),
                ptx->getBufferSize(),
                log,
                &logSize,
                &module.module),
            "optixModuleCreate(raygen)",
            log));
        OptixProgramGroupDesc groupDescription = {};
        groupDescription.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
        groupDescription.raygen.module = module.module;
        groupDescription.raygen.entryFunctionName = "__raygen__raygenMain";
        OptixProgramGroupOptions groupOptions = {};
        OptixProgramGroupsGuard groups;
        log[0] = 0;
        logSize = sizeof(log);
        SLANG_CHECK_ABORT(_checkOptixResult(
            optixProgramGroupCreate(
                optixContext.context,
                &groupDescription,
                1,
                &groupOptions,
                log,
                &logSize,
                groups.groups),
            "optixProgramGroupCreate(raygen)",
            log));
        OptixPipelineLinkOptions linkOptions = {};
        linkOptions.maxTraceDepth = 0;
        OptixPipelineGuard pipeline;
        log[0] = 0;
        logSize = sizeof(log);
        SLANG_CHECK_ABORT(_checkOptixResult(
            optixPipelineCreate(
                optixContext.context,
                &pipelineOptions,
                &linkOptions,
                groups.groups,
                1,
                log,
                &logSize,
                &pipeline.pipeline),
            "optixPipelineCreate(raygen)",
            log));
        OptixStackSizes stackSizes = {};
        SLANG_CHECK_ABORT(_checkOptixResult(
            optixUtilAccumulateStackSizes(groups.groups[0], &stackSizes, pipeline.pipeline),
            "optixUtilAccumulateStackSizes"));
        unsigned directTraversal = 0, directState = 0, continuation = 0;
        SLANG_CHECK_ABORT(_checkOptixResult(
            optixUtilComputeStackSizes(
                &stackSizes,
                0,
                0,
                0,
                &directTraversal,
                &directState,
                &continuation),
            "optixUtilComputeStackSizes"));
        SLANG_CHECK_ABORT(_checkOptixResult(
            optixPipelineSetStackSize(
                pipeline.pipeline,
                directTraversal,
                directState,
                continuation,
                1),
            "optixPipelineSetStackSize"));

        CudaAllocationGuard output(&cuda), parameters(&cuda), launch(&cuda), sbt(&cuda),
            miss(&cuda);
        SLANG_CHECK_ABORT(cuda.cuMemAlloc(&output.pointer, 170 * sizeof(uint32_t)) == kCudaSuccess);
        SLANG_CHECK_ABORT(cuda.cuMemAlloc(&parameters.pointer, sizeof(Parameters)) == kCudaSuccess);
        SLANG_CHECK_ABORT(cuda.cuMemAlloc(&launch.pointer, sizeof(CUdeviceptr)) == kCudaSuccess);
        SLANG_CHECK_ABORT(cuda.cuMemAlloc(&sbt.pointer, sizeof(RaygenRecord)) == kCudaSuccess);
        char emptyMiss[OPTIX_SBT_RECORD_HEADER_SIZE] = {};
        SLANG_CHECK_ABORT(cuda.cuMemAlloc(&miss.pointer, sizeof(emptyMiss)) == kCudaSuccess);
        SLANG_CHECK_ABORT(
            cuda.cuMemcpyHtoD(miss.pointer, emptyMiss, sizeof(emptyMiss)) == kCudaSuccess);
        RaygenRecord record = {};
        SLANG_CHECK_ABORT(_checkOptixResult(
            optixSbtRecordPackHeader(groups.groups[0], &record),
            "optixSbtRecordPackHeader"));
        OptixShaderBindingTable table = {};
        table.raygenRecord = sbt.pointer;
        // Match slang-rhi's mandatory zeroed miss record, even though this program never traces.
        table.missRecordBase = miss.pointer;
        table.missRecordStrideInBytes = sizeof(emptyMiss);
        table.missRecordCount = 1;

        for (unsigned iteration = 0; iteration < 2; ++iteration)
        {
            unsigned dimensions[3] = {
                iteration ? 3u : 4u,
                iteration ? 2u : 3u,
                iteration ? 1u : 2u};
            uint32_t actual[170], expected[170];
            for (unsigned i = 0; i < SLANG_COUNT_OF(actual); ++i)
                actual[i] = expected[i] = 0xa5a5a5a5u;
            actual[0] = expected[0] = 0x13579bdfu;
            actual[169] = expected[169] = 0xdeadbeefu;
            Parameters hostParameters =
                {output.pointer + sizeof(uint32_t), 168, iteration ? 700u : 100u, 0xabcdef01u};
            record.bias = iteration ? 31u : 13u;
            SLANG_CHECK_ABORT(
                cuda.cuMemcpyHtoD(output.pointer, actual, sizeof(actual)) == kCudaSuccess);
            SLANG_CHECK_ABORT(
                cuda.cuMemcpyHtoD(parameters.pointer, &hostParameters, sizeof(hostParameters)) ==
                kCudaSuccess);
            SLANG_CHECK_ABORT(
                cuda.cuMemcpyHtoD(
                    launch.pointer,
                    &parameters.pointer,
                    sizeof(parameters.pointer)) == kCudaSuccess);
            SLANG_CHECK_ABORT(
                cuda.cuMemcpyHtoD(sbt.pointer, &record, sizeof(record)) == kCudaSuccess);
            SLANG_CHECK_ABORT(_checkOptixResult(
                optixLaunch(
                    pipeline.pipeline,
                    nullptr,
                    launch.pointer,
                    sizeof(CUdeviceptr),
                    &table,
                    dimensions[0],
                    dimensions[1],
                    dimensions[2]),
                "optixLaunch"));
            SLANG_CHECK_ABORT(cuda.cuCtxSynchronize() == kCudaSuccess);
            SLANG_CHECK_ABORT(
                cuda.cuMemcpyDtoH(actual, output.pointer, sizeof(actual)) == kCudaSuccess);
            for (unsigned z = 0; z < dimensions[2]; ++z)
                for (unsigned y = 0; y < dimensions[1]; ++y)
                    for (unsigned x = 0; x < dimensions[0]; ++x)
                    {
                        unsigned index = (z * dimensions[1] + y) * dimensions[0] + x;
                        uint32_t values[] = {
                            x,
                            y,
                            z,
                            dimensions[0],
                            dimensions[1],
                            dimensions[2],
                            hostParameters.base + record.bias + index};
                        for (unsigned channel = 0; channel < SLANG_COUNT_OF(values); ++channel)
                            expected[1 + 7 * index + channel] = values[channel];
                    }
            for (unsigned i = 0; i < SLANG_COUNT_OF(actual); ++i)
            {
                if (actual[i] != expected[i])
                    fprintf(
                        stderr,
                        "OptiX O%d launch %u word %u: expected %u, got %u\n",
                        int(optimization),
                        iteration,
                        i,
                        expected[i],
                        actual[i]);
                SLANG_CHECK(actual[i] == expected[i]);
            }
        }
    }
}

#endif // SLANG_UNIT_TEST_ENABLE_OPTIX
