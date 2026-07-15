// unit-test-coverage-cuda-runtime.cpp

#include "core/slang-list.h"
#include "core/slang-platform.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <type_traits>

using namespace Slang;

// End-to-end runtime test for the CUDA shader-coverage path.
//
// The compile-time contract (metadata shape, atomicAdd form selection,
// GlobalParams packing) is covered by `unit-test-coverage-tracing-metadata.cpp`
// and the filecheck tests under `tests/language-feature/coverage/`, and the
// in-process CPU dispatch is covered by `unit-test-coverage-cpu-runtime.cpp`.
// What none of those verify is that a host following the documented CUDA
// binding recipe (docs/design/shader-coverage-host-interface.md, "CUDA
// hosts") actually observes correct execution counts on a GPU. This test
// closes that gap:
//
//   1. compile a compute shader with `-trace-coverage` to PTX,
//   2. discover the hidden `__slang_coverage` buffer through
//      `ISyntheticResourceMetadata`'s uniform-marshaling contract — the same
//      `uniformOffset` / `uniformStride` fields the CPU path uses, with the
//      difference that the (pointer, count) pair holds a device pointer,
//   3. build the global-params payload on the host and copy it into the
//      module's `SLANG_globalParams` constant symbol (the CUDA analogue of
//      passing the payload pointer to the kernel function),
//   4. launch one thread group through the CUDA driver API, copy outputs and
//      counters back to the host,
//   5. validate the counter values against the exact execution counts the
//      dispatch must produce (including a zero count for an unreached
//      branch), for both the default uint64 and opt-down uint32 counter
//      widths.
//
// The CUDA driver API is loaded dynamically (nvcuda.dll / libcuda.so.1), so
// this file builds without the CUDA toolkit; the test reports `Ignored` on
// machines without an NVIDIA driver, a CUDA device, or NVRTC (which Slang
// loads at runtime to emit PTX).

namespace
{

// ## Minimal dynamic CUDA driver API
//
// Only the handful of driver entry points this test dispatches through,
// resolved at runtime with the local type declarations below, so no CUDA
// toolkit headers or import libraries are needed at build time. The `_v2`
// symbol names match what `cuda.h` resolves these functions to since CUDA
// 3.2 (the same names slang-rhi's dynamic loader uses).

typedef int CudaResult;                        // CUresult; 0 is CUDA_SUCCESS
typedef int CudaDevice;                        // CUdevice
typedef unsigned long long CudaDevicePtr;      // CUdeviceptr on 64-bit hosts
typedef struct CudaContextImpl* CudaContext;   // CUcontext
typedef struct CudaModuleImpl* CudaModule;     // CUmodule
typedef struct CudaFunctionImpl* CudaFunction; // CUfunction

struct CudaDriverApi
{
    CudaResult (*cuInit)(unsigned int flags) = nullptr;
    CudaResult (*cuDeviceGetCount)(int* count) = nullptr;
    CudaResult (*cuDeviceGet)(CudaDevice* device, int ordinal) = nullptr;
    CudaResult (*cuDevicePrimaryCtxRetain)(CudaContext* context, CudaDevice device) = nullptr;
    CudaResult (*cuDevicePrimaryCtxRelease)(CudaDevice device) = nullptr;
    CudaResult (*cuCtxSetCurrent)(CudaContext context) = nullptr;
    CudaResult (*cuCtxSynchronize)() = nullptr;
    CudaResult (*cuModuleLoadData)(CudaModule* module, const void* image) = nullptr;
    CudaResult (*cuModuleUnload)(CudaModule module) = nullptr;
    CudaResult (*cuModuleGetFunction)(CudaFunction* function, CudaModule module, const char* name) =
        nullptr;
    CudaResult (*cuModuleGetGlobal)(
        CudaDevicePtr* devicePtr,
        size_t* bytes,
        CudaModule module,
        const char* name) = nullptr;
    CudaResult (*cuMemAlloc)(CudaDevicePtr* devicePtr, size_t bytes) = nullptr;
    CudaResult (*cuMemFree)(CudaDevicePtr devicePtr) = nullptr;
    CudaResult (*cuMemcpyHtoD)(CudaDevicePtr dst, const void* src, size_t bytes) = nullptr;
    CudaResult (*cuMemcpyDtoH)(void* dst, CudaDevicePtr src, size_t bytes) = nullptr;
    CudaResult (*cuMemsetD8)(CudaDevicePtr dst, unsigned char value, size_t bytes) = nullptr;
    CudaResult (*cuLaunchKernel)(
        CudaFunction function,
        unsigned int gridDimX,
        unsigned int gridDimY,
        unsigned int gridDimZ,
        unsigned int blockDimX,
        unsigned int blockDimY,
        unsigned int blockDimZ,
        unsigned int sharedMemBytes,
        void* stream,
        void** kernelParams,
        void** extra) = nullptr;

    SharedLibrary::Handle library = nullptr;

    // Load the driver shared library and resolve every entry point above.
    // Returns false — leaving the struct unusable — when the library or any
    // symbol is missing, which the test treats as "no CUDA here" and ignores.
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
        auto resolve = [&](auto& funcPtr, const char* symbolName)
        {
            funcPtr = reinterpret_cast<std::remove_reference_t<decltype(funcPtr)>>(
                SharedLibrary::findSymbolAddressByName(library, symbolName));
            if (!funcPtr)
                allFound = false;
        };
        resolve(cuInit, "cuInit");
        resolve(cuDeviceGetCount, "cuDeviceGetCount");
        resolve(cuDeviceGet, "cuDeviceGet");
        resolve(cuDevicePrimaryCtxRetain, "cuDevicePrimaryCtxRetain");
        resolve(cuDevicePrimaryCtxRelease, "cuDevicePrimaryCtxRelease_v2");
        resolve(cuCtxSetCurrent, "cuCtxSetCurrent");
        resolve(cuCtxSynchronize, "cuCtxSynchronize");
        resolve(cuModuleLoadData, "cuModuleLoadData");
        resolve(cuModuleUnload, "cuModuleUnload");
        resolve(cuModuleGetFunction, "cuModuleGetFunction");
        resolve(cuModuleGetGlobal, "cuModuleGetGlobal_v2");
        resolve(cuMemAlloc, "cuMemAlloc_v2");
        resolve(cuMemFree, "cuMemFree_v2");
        resolve(cuMemcpyHtoD, "cuMemcpyHtoD_v2");
        resolve(cuMemcpyDtoH, "cuMemcpyDtoH_v2");
        resolve(cuMemsetD8, "cuMemsetD8_v2");
        resolve(cuLaunchKernel, "cuLaunchKernel");
        return allFound;
#endif
    }

    ~CudaDriverApi()
    {
        if (library)
            SharedLibrary::unload(library);
    }
};

// The device-side representation of a (RW)StructuredBuffer<T> parameter, as
// declared in `prelude/slang-cuda-prelude.h`: a data pointer followed by an
// element count — the same 16-byte shape as the CPU path, except the pointer
// is a device pointer. This is the value a host must store at
// `uniformOffset` inside the global-params payload to bind the coverage
// counter buffer.
// RAII releases for driver-owned resources. SLANG_CHECK_ABORT throws
// through the test body, and the test server runs several tests in one
// process, so a failing assertion must not leak device memory, the
// loaded module, or the retained primary context — a leaked context in
// particular can undermine the diagnosis of whichever test runs next.
struct CudaBufferGuard
{
    const CudaDriverApi& api;
    CudaDevicePtr ptr;
    ~CudaBufferGuard() { api.cuMemFree(ptr); }
};

struct CudaModuleGuard
{
    const CudaDriverApi& api;
    CudaModule module;
    ~CudaModuleGuard() { api.cuModuleUnload(module); }
};

struct CudaPrimaryContextGuard
{
    const CudaDriverApi& api;
    CudaDevice device;
    ~CudaPrimaryContextGuard() { api.cuDevicePrimaryCtxRelease(device); }
};

struct CudaStructuredBufferView
{
    CudaDevicePtr data = 0;
    size_t count = 0;
};

// Every line we assert on holds exactly one statement, so one coverage
// counter maps to one source line and the expected counts below are exact.
static const char* const kShaderSource = R"(
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = tid.x;
    for (uint i = 0; i < 3; ++i)
    {
        value += i;
    }
    if (tid.x > 100u)
    {
        value += 1000u;
    }
    outputBuffer[tid.x] = value;
}
)";

static const uint32_t kThreadCount = 4;
static const uint32_t kLoopTripCount = 3;

// Return the 1-based line number of the first source line containing
// `needle`, or 0 if not found. Locating asserted lines by content keeps the
// expected-count table valid when the shader source is edited.
static uint32_t findLineContaining(const char* source, const char* needle)
{
    uint32_t line = 1;
    for (const char* cursor = source; *cursor;)
    {
        const char* lineEnd = strchr(cursor, '\n');
        size_t lineLength = lineEnd ? size_t(lineEnd - cursor) : strlen(cursor);
        if (String(UnownedStringSlice(cursor, lineLength)).indexOf(needle) >= 0)
            return line;
        if (!lineEnd)
            break;
        cursor = lineEnd + 1;
        ++line;
    }
    return 0;
}

// Read counter slot `index` from a counter buffer of the given element
// width. The instrumented kernel writes native uint64 or uint32 elements
// depending on `-trace-coverage-counter-width`.
static uint64_t readCounter(const void* counters, int counterByteWidth, uint32_t index)
{
    if (counterByteWidth == 8)
        return ((const uint64_t*)counters)[index];
    return ((const uint32_t*)counters)[index];
}

static void diagnoseIfNeeded(slang::IBlob* diagnostics, const char* label)
{
    if (diagnostics && diagnostics->getBufferSize() > 0)
    {
        fprintf(
            stderr,
            "coverageCudaRuntimeDispatch %s diagnostics:\n%s\n",
            label,
            (const char*)diagnostics->getBufferPointer());
    }
}

// Compile the test shader with coverage tracing at the requested counter
// width, launch one 4-thread group through the CUDA driver API with a
// device-allocated counter buffer, and validate both the kernel's output
// values and the exact per-line coverage counts. The caller's CUDA context
// is current on this thread; every driver call below runs against it.
static void runCoverageCudaRuntimeTest(
    slang::IGlobalSession* globalSession,
    CudaDriverApi& cuda,
    int counterByteWidth)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    targetDesc.profile = globalSession->findProfile("cuda_sm_7_0");

    slang::CompilerOptionEntry coverageOptions[2] = {};
    coverageOptions[0].name = slang::CompilerOptionName::TraceCoverage;
    coverageOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[0].value.intValue0 = 1;
    coverageOptions[1].name = slang::CompilerOptionName::TraceCoverageCounterByteWidth;
    coverageOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[1].value.intValue0 = counterByteWidth;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = coverageOptions;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(coverageOptions);

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageCudaRuntime",
        "coverageCudaRuntime.slang",
        kShaderSource,
        diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics, "loadModule");
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(components, 2, program.writeRef(), nullptr) ==
        SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    diagnostics.setNull();
    SLANG_CHECK_ABORT(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);
    diagnoseIfNeeded(diagnostics, "link");

    diagnostics.setNull();
    ComPtr<slang::IBlob> ptxBlob;
    SlangResult codeResult =
        linked->getEntryPointCode(0, 0, ptxBlob.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics, "getEntryPointCode");
    SLANG_CHECK_ABORT(codeResult == SLANG_OK);

    // Discover the hidden coverage buffer through the synthetic-resource
    // metadata contract, exactly the way a direct CUDA host would.
    diagnostics.setNull();
    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK_ABORT(
        linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    auto coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK_ABORT(coverage != nullptr);
    auto syntheticResources = (slang::ISyntheticResourceMetadata*)metadata->castAs(
        slang::ISyntheticResourceMetadata::getTypeGuid());
    SLANG_CHECK_ABORT(syntheticResources != nullptr);

    const uint32_t counterCount = coverage->getCounterCount();
    SLANG_CHECK_ABORT(counterCount > 0);

    SLANG_CHECK_ABORT(syntheticResources->getResourceCount() == 1);
    slang::SyntheticResourceInfo resourceInfo;
    SLANG_CHECK_ABORT(syntheticResources->getResourceInfo(0, &resourceInfo) == SLANG_OK);
    SLANG_CHECK(resourceInfo.uniformOffset >= 0);
    // The CUDA representation of the coverage buffer is a
    // (device pointer, element count) pair; the reported stride is the size
    // of that representation in the global-params payload. Abort on
    // mismatch: the payload below is sized from these fields, so continuing
    // with an out-of-contract stride (e.g. the `0` "unavailable" sentinel)
    // would turn a metadata regression into an out-of-bounds write instead
    // of a clean test failure.
    SLANG_CHECK_ABORT(resourceInfo.uniformStride == int32_t(sizeof(CudaStructuredBufferView)));

    // A real host discovers the counter element width from the metadata
    // rather than trusting what it asked for, so exercise that path: read
    // `CoverageBufferInfo::elementByteWidth`, check it matches the requested
    // width, and size the counter storage from the reported value.
    slang::CoverageBufferInfo bufferInfo;
    SLANG_CHECK_ABORT(coverage->getBufferInfo(&bufferInfo) == SLANG_OK);
    SLANG_CHECK_ABORT(bufferInfo.elementByteWidth == uint32_t(counterByteWidth));
    const int reportedCounterByteWidth = int(bufferInfo.elementByteWidth);

    // cuModuleLoadData parses PTX as a NUL-terminated string, and the code
    // blob is not guaranteed to carry the terminator; copying into a String
    // appends one.
    const String ptx(
        UnownedStringSlice((const char*)ptxBlob->getBufferPointer(), ptxBlob->getBufferSize()));
    CudaModule cudaModule = nullptr;
    SLANG_CHECK_ABORT(cuda.cuModuleLoadData(&cudaModule, ptx.getBuffer()) == 0);
    CudaModuleGuard moduleGuard{cuda, cudaModule};
    CudaFunction kernel = nullptr;
    SLANG_CHECK_ABORT(cuda.cuModuleGetFunction(&kernel, cudaModule, "computeMain") == 0);

    // Device storage for the shader's output buffer and the counters, both
    // zero-initialized (counters must start zeroed).
    CudaDevicePtr outputBuffer = 0;
    SLANG_CHECK_ABORT(cuda.cuMemAlloc(&outputBuffer, kThreadCount * sizeof(uint32_t)) == 0);
    CudaBufferGuard outputGuard{cuda, outputBuffer};
    SLANG_CHECK_ABORT(cuda.cuMemsetD8(outputBuffer, 0, kThreadCount * sizeof(uint32_t)) == 0);
    const size_t counterBytes = size_t(counterCount) * reportedCounterByteWidth;
    CudaDevicePtr counterBuffer = 0;
    SLANG_CHECK_ABORT(cuda.cuMemAlloc(&counterBuffer, counterBytes) == 0);
    CudaBufferGuard counterGuard{cuda, counterBuffer};
    SLANG_CHECK_ABORT(cuda.cuMemsetD8(counterBuffer, 0, counterBytes) == 0);

    // Build the global-params payload. `outputBuffer` is the only
    // user-declared global, so it sits at offset 0; the synthesized coverage
    // buffer is reported at `uniformOffset`. The two views must not overlap.
    SLANG_CHECK_ABORT(resourceInfo.uniformOffset >= int32_t(sizeof(CudaStructuredBufferView)));
    CudaStructuredBufferView outputView;
    outputView.data = outputBuffer;
    outputView.count = kThreadCount;
    CudaStructuredBufferView coverageView;
    coverageView.data = counterBuffer;
    coverageView.count = counterCount;

    List<uint8_t> globalParams;
    globalParams.setCount(Index(resourceInfo.uniformOffset) + Index(resourceInfo.uniformStride));
    memset(globalParams.getBuffer(), 0, globalParams.getCount());
    memcpy(globalParams.getBuffer(), &outputView, sizeof(outputView));
    memcpy(
        globalParams.getBuffer() + resourceInfo.uniformOffset,
        &coverageView,
        sizeof(coverageView));

    // Unlike the CPU path, the payload is not a kernel argument: the emitted
    // kernel reads its globals from the module-level `SLANG_globalParams`
    // constant symbol, so copy the payload there before launching.
    CudaDevicePtr paramsSymbol = 0;
    size_t paramsSymbolSize = 0;
    SLANG_CHECK_ABORT(
        cuda.cuModuleGetGlobal(
            &paramsSymbol,
            &paramsSymbolSize,
            cudaModule,
            "SLANG_globalParams") == 0);
    SLANG_CHECK_ABORT(paramsSymbolSize >= size_t(globalParams.getCount()));
    SLANG_CHECK_ABORT(
        cuda.cuMemcpyHtoD(paramsSymbol, globalParams.getBuffer(), globalParams.getCount()) == 0);

    // Launch a single thread group. The entry point has no uniform
    // parameters, so the emitted kernel takes no launch-time arguments.
    SLANG_CHECK_ABORT(
        cuda.cuLaunchKernel(kernel, 1, 1, 1, kThreadCount, 1, 1, 0, nullptr, nullptr, nullptr) ==
        0);
    SLANG_CHECK_ABORT(cuda.cuCtxSynchronize() == 0);

    // The instrumented kernel must still compute correct results:
    // outputBuffer[t] = t + (0 + 1 + 2).
    uint32_t outputValues[kThreadCount] = {};
    SLANG_CHECK_ABORT(cuda.cuMemcpyDtoH(outputValues, outputBuffer, sizeof(outputValues)) == 0);
    for (uint32_t t = 0; t < kThreadCount; ++t)
        SLANG_CHECK(outputValues[t] == t + 3);

    List<uint8_t> counterBytesHost;
    counterBytesHost.setCount(Index(counterBytes));
    SLANG_CHECK_ABORT(
        cuda.cuMemcpyDtoH(counterBytesHost.getBuffer(), counterBuffer, counterBytes) == 0);

    // Expected exact execution counts per single-statement source line.
    struct ExpectedLine
    {
        const char* statement;
        uint64_t expectedCount;
    };
    const ExpectedLine expectedLines[] = {
        {"uint value = tid.x;", kThreadCount},
        {"value += i;", kThreadCount * kLoopTripCount},
        {"value += 1000u;", 0},
        {"outputBuffer[tid.x] = value;", kThreadCount},
    };

    for (const auto& expected : expectedLines)
    {
        const uint32_t line = findLineContaining(kShaderSource, expected.statement);
        SLANG_CHECK_ABORT(line != 0);

        // Sum every line-entry counter attributed to this source line. The
        // unreached branch must still be instrumented (an entry exists) and
        // read back as zero — that distinguishes "executed zero times" from
        // "not instrumented at all".
        uint32_t entriesOnLine = 0;
        uint64_t totalCount = 0;
        for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
        {
            slang::CoverageEntryInfo entry;
            SLANG_CHECK_ABORT(coverage->getEntryInfo(i, &entry) == SLANG_OK);
            if (entry.kind != slang::CoverageEntryKind::Line)
                continue;
            if (entry.line != line)
                continue;
            SLANG_CHECK_ABORT(entry.counterIndex < counterCount);
            ++entriesOnLine;
            totalCount += readCounter(
                counterBytesHost.getBuffer(),
                reportedCounterByteWidth,
                entry.counterIndex);
        }
        SLANG_CHECK(entriesOnLine == 1);
        SLANG_CHECK(totalCount == expected.expectedCount);
    }
}

} // anonymous namespace

SLANG_UNIT_TEST(coverageCudaRuntimeDispatch)
{
    // The test needs three runtime facilities that vary per machine: the
    // CUDA driver (loaded dynamically above), a CUDA device, and NVRTC
    // (loaded by Slang to emit PTX). Missing any of them means "this
    // machine cannot run CUDA coverage", not a product failure, so the
    // test reports Ignored rather than failing.
    CudaDriverApi cuda;
    if (!cuda.load())
    {
        SLANG_IGNORE_TEST;
    }
    if (cuda.cuInit(0) != 0)
    {
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        SLANG_IGNORE_TEST;
    }

    // Use the device's primary context, the same pattern as the
    // shader-coverage-backends example and slang-rhi's CUDA device.
    // Unlike cuCtxCreate, retaining does not make the context current,
    // so set it explicitly.
    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    // Default 64-bit counters exercise the 64-bit atomicAdd form; the
    // opt-down width exercises the 32-bit form. Neither needs a device
    // opt-in on CUDA.
    runCoverageCudaRuntimeTest(globalSession, cuda, 8);
    runCoverageCudaRuntimeTest(globalSession, cuda, 4);
}
