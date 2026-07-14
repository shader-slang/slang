// shader-coverage-backends: one shader, four coverage dispatch paths.
//
// This example dispatches the user-guide coverage tutorial's kernel
// (hello-coverage.slang) with `--backend=cpu`, `--backend=cuda`,
// `--backend=vulkan`, or `--backend=metal`, and shows that the
// coverage workflow is the same everywhere:
//
//   1. compile with coverage enabled (Slang C++ API),
//   2. discover the hidden `__slang_coverage` buffer through
//      `slang::ISyntheticResourceMetadata`,
//   3. bind host-allocated counter storage the way that backend's
//      binding model requires,
//   4. dispatch, read the counters back, and attribute them to source
//      lines through `slang::ICoverageTracingMetadata`.
//
// Only step 3 differs per backend, and each path here is a compact,
// runnable implementation of the corresponding recipe in
// docs/design/shader-coverage-host-interface.md:
//
//   cpu    — the kernel is compiled to a host-callable shared library;
//            the counter buffer is a (pointer, count) pair written into
//            the kernel's global-parameter payload at the metadata's
//            `uniformOffset`.
//   cuda   — the same uniform-marshaling contract as cpu, with the
//            differences inherent to the driver model: the pair holds a
//            device pointer, the payload is copied into the module's
//            `SLANG_globalParams` constant symbol, and readback is a
//            device-to-host copy. Slang emits PTX through NVRTC.
//   vulkan — the counter buffer is an ordinary storage buffer bound at
//            the metadata's descriptor `(space, binding)`; uses the
//            same raw-Vulkan helper as the other coverage examples.
//   metal  — the counter buffer is set on the compute encoder at the
//            metadata's `binding` index (`[[buffer(N)]]`); the emitted
//            MSL is compiled at runtime by the Metal framework, so no
//            external Metal toolchain is needed. (Apple platforms only.)
//
// Every run ends the same way: a per-line hit table on stdout, plus
// `<backend>.coverage-manifest.json` and `<backend>.counters.bin`
// ready for tools/shader-coverage/slang-coverage-to-lcov.py.

// metal-cpp must be included before any Slang headers: its own headers
// resolve types like `NS::String` by unqualified name internally, and a
// preceding `using namespace` / Slang core include makes those lookups
// ambiguous. This is the only translation unit in the example, so the
// *_PRIVATE_IMPLEMENTATION definitions live here too.
#if defined(SLANG_EXAMPLE_HAS_METAL)

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#endif

#include "slang-com-ptr.h"
#include "slang.h"

#if defined(SLANG_EXAMPLE_HAS_VULKAN)
#include "vk_compute_demo.h"
#endif

#if defined(SLANG_EXAMPLE_HAS_CUDA)
#include <cuda.h>
#endif

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using Slang::ComPtr;

namespace
{

// ## Small utilities

[[noreturn]] void fail(const std::string& message)
{
    std::cerr << "error: " << message << "\n";
    std::exit(1);
}

void checkSlang(SlangResult result, const char* what)
{
    if (SLANG_FAILED(result))
        fail(std::string(what) + " failed with SlangResult " + std::to_string(result));
}

void diagnoseIfNeeded(slang::IBlob* diagnostics)
{
    if (diagnostics && diagnostics->getBufferSize() > 0)
    {
        std::cerr.write(
            reinterpret_cast<const char*>(diagnostics->getBufferPointer()),
            diagnostics->getBufferSize());
        std::cerr << "\n";
    }
}

// Resolves the directory containing `hello-coverage.slang`, in the same
// way the sibling coverage examples do: an explicit `--demo-dir=<path>`
// wins; otherwise the source directory of this file (the normal
// run-from-source-tree case); otherwise the current working directory.
std::filesystem::path g_demoDirOverride;

std::filesystem::path getDemoDirectory()
{
    if (!g_demoDirOverride.empty())
        return g_demoDirOverride;
    std::filesystem::path sourceDir = std::filesystem::path(__FILE__).parent_path();
    if (std::filesystem::exists(sourceDir / "hello-coverage.slang"))
        return sourceDir;
    return std::filesystem::current_path();
}

// ## Compilation
//
// One compile function serves all four backends; only the target
// format and profile differ. Coverage is requested the same way
// everywhere, and the counter width is passed through so the Vulkan
// path can demonstrate both widths (`--counter-width 64` requires
// 64-bit shader atomics on the device; see the pitfalls section of the
// tutorial). Metal ignores a 64-bit request with warning E45115 and
// caps to 32-bit — which is why everything downstream reads the
// *effective* width from the metadata instead of trusting the request.

struct CompiledProgram
{
    ComPtr<slang::IComponentType> linked;
    ComPtr<slang::IBlob> code;               // target code (vulkan / metal)
    ComPtr<ISlangSharedLibrary> hostLibrary; // host-callable library (cpu)
    ComPtr<slang::IMetadata> metadata;
    slang::ICoverageTracingMetadata* coverage = nullptr;
    slang::ISyntheticResourceMetadata* synthetic = nullptr;
    slang::SyntheticResourceInfo resourceInfo = {};
    uint32_t counterCount = 0;
    uint32_t elementByteWidth = 0;
};

CompiledProgram compileWithCoverage(
    SlangCompileTarget target,
    const char* profileName,
    int counterByteWidth)
{
    ComPtr<slang::IGlobalSession> globalSession;
    checkSlang(slang::createGlobalSession(globalSession.writeRef()), "createGlobalSession");

    // For the CPU path, pick a real C++ toolchain for the
    // host-callable compilation, matching the coverage runtime unit
    // tests. This is a requirement, not a preference: coverage
    // instrumentation is skipped on the slang-llvm JIT path (warning
    // E45102), so without a system C++ compiler the dispatch below
    // would run uninstrumented and fail the non-zero counter check.
    if (target == SLANG_SHADER_HOST_CALLABLE)
    {
        const SlangPassThrough cppCompilers[] = {
            SLANG_PASS_THROUGH_VISUAL_STUDIO,
            SLANG_PASS_THROUGH_GCC,
            SLANG_PASS_THROUGH_CLANG,
        };
        for (auto candidate : cppCompilers)
        {
            if (SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(candidate)))
            {
                globalSession->setDefaultDownstreamCompiler(SLANG_SOURCE_LANGUAGE_CPP, candidate);
                globalSession->setDownstreamCompilerForTransition(
                    SLANG_CPP_SOURCE,
                    SLANG_SHADER_HOST_CALLABLE,
                    candidate);
                break;
            }
        }
    }

    // For the CUDA path, PTX emission runs through NVRTC (which ships
    // with the CUDA toolkit and is loaded at runtime). Precheck it so a
    // missing toolkit surfaces as one clear message instead of a
    // downstream-compiler diagnostic later in the compile.
    if (target == SLANG_PTX)
    {
        if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
            fail("the cuda backend needs NVRTC to emit PTX; install the CUDA toolkit");
    }

    slang::TargetDesc targetDesc = {};
    targetDesc.format = target;
    targetDesc.profile = globalSession->findProfile(profileName);

    slang::CompilerOptionEntry options[2] = {};
    options[0].name = slang::CompilerOptionName::TraceCoverage;
    options[0].value.kind = slang::CompilerOptionValueKind::Int;
    options[0].value.intValue0 = 1;
    options[1].name = slang::CompilerOptionName::TraceCoverageCounterByteWidth;
    options[1].value.kind = slang::CompilerOptionValueKind::Int;
    options[1].value.intValue0 = counterByteWidth;

    const std::string searchPath = getDemoDirectory().string();
    const char* searchPaths[] = {searchPath.c_str()};

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;
    sessionDesc.compilerOptionEntries = options;
    sessionDesc.compilerOptionEntryCount = 2;

    ComPtr<slang::ISession> session;
    checkSlang(globalSession->createSession(sessionDesc, session.writeRef()), "createSession");

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModule("hello-coverage", diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    if (!module)
        fail("loadModule(hello-coverage) — run from the example directory or pass --demo-dir");

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    if (!entryPoint)
        fail("findEntryPointByName(computeMain)");

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composed;
    checkSlang(
        session->createCompositeComponentType(components, 2, composed.writeRef(), nullptr),
        "createCompositeComponentType");

    CompiledProgram out;
    diagnostics.setNull();
    checkSlang(composed->link(out.linked.writeRef(), diagnostics.writeRef()), "link");
    diagnoseIfNeeded(diagnostics);

    // Fetch the compiled result first, then the metadata. Either order
    // works; the code fetch is where compile diagnostics surface (e.g.
    // E45115 for a capped Metal width request), so doing it first keeps
    // error reporting next to the compile. The CPU path gets a
    // host-callable shared library; the code-emitting targets get a
    // code blob.
    diagnostics.setNull();
    if (target == SLANG_SHADER_HOST_CALLABLE)
    {
        checkSlang(
            out.linked->getEntryPointHostCallable(
                0,
                0,
                out.hostLibrary.writeRef(),
                diagnostics.writeRef()),
            "getEntryPointHostCallable");
    }
    else
    {
        checkSlang(
            out.linked->getEntryPointCode(0, 0, out.code.writeRef(), diagnostics.writeRef()),
            "getEntryPointCode");
    }
    diagnoseIfNeeded(diagnostics);

    diagnostics.setNull();
    checkSlang(
        out.linked->getEntryPointMetadata(0, 0, out.metadata.writeRef(), diagnostics.writeRef()),
        "getEntryPointMetadata");
    diagnoseIfNeeded(diagnostics);

    out.coverage = (slang::ICoverageTracingMetadata*)out.metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    out.synthetic = (slang::ISyntheticResourceMetadata*)out.metadata->castAs(
        slang::ISyntheticResourceMetadata::getTypeGuid());
    if (!out.coverage || !out.synthetic)
        fail("expected coverage + synthetic-resource metadata on the artifact");

    out.counterCount = out.coverage->getCounterCount();
    if (out.counterCount == 0)
        fail("expected a non-zero coverage counter count");

    // Coverage currently emits exactly one synthetic resource: the
    // hidden counter buffer.
    if (out.synthetic->getResourceCount() != 1)
        fail("expected exactly one synthetic resource");
    checkSlang(out.synthetic->getResourceInfo(0, &out.resourceInfo), "getResourceInfo");

    // The *effective* counter width. Always read it from here: it can
    // differ from the requested width (Metal caps counting-mode
    // counters to 32-bit), and readback code that assumes a width
    // breaks silently when it changes.
    slang::CoverageBufferInfo bufferInfo;
    checkSlang(out.coverage->getBufferInfo(&bufferInfo), "getBufferInfo");
    out.elementByteWidth = bufferInfo.elementByteWidth;
    if (out.elementByteWidth != 4 && out.elementByteWidth != 8)
        fail("unexpected coverage counter element width");

    return out;
}

// ## Shared dispatch inputs and result checking
//
// Four threads, one negative input. With gain fixed at 2.0 the kernel
// computes max(value, 0) * 2, so the expected outputs are fully
// determined — and so are the expected counter values: the
// negative-clamp line runs exactly once, and applyGain's
// `return value;` fallthrough never runs.

constexpr uint32_t kThreadCount = 4;
const float kInputs[kThreadCount] = {1.0f, -2.0f, 3.0f, 4.0f};
const float kExpectedOutputs[kThreadCount] = {2.0f, 0.0f, 6.0f, 8.0f};

void checkOutputs(const float* outputs, const char* backendName)
{
    for (uint32_t i = 0; i < kThreadCount; ++i)
    {
        if (outputs[i] != kExpectedOutputs[i])
        {
            fail(
                std::string(backendName) + " output[" + std::to_string(i) + "] = " +
                std::to_string(outputs[i]) + ", expected " + std::to_string(kExpectedOutputs[i]));
        }
    }
}

// Reads counter slot `index` at the effective element width.
uint64_t readCounter(const void* counters, uint32_t elementByteWidth, uint32_t index)
{
    if (elementByteWidth == 8)
        return reinterpret_cast<const uint64_t*>(counters)[index];
    return reinterpret_cast<const uint32_t*>(counters)[index];
}

// ## Reporting
//
// The same ending for every backend: attribute the raw counters to
// source lines via ICoverageTracingMetadata, print the hit table, and
// write the manifest + counter snapshot that the LCOV converter
// consumes.

void report(const CompiledProgram& program, const void* counters, const std::string& backendName)
{
    // Aggregate line entries by source line, the same way LCOV export
    // does. (Multiple statements on one line get distinct counters.)
    std::map<uint32_t, uint64_t> hitsByLine;
    for (uint32_t i = 0; i < program.coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        checkSlang(program.coverage->getEntryInfo(i, &entry), "getEntryInfo");
        if (entry.kind != slang::CoverageEntryKind::Line)
            continue;
        // Entries without a real source location (compiler-synthesized
        // code) are skipped, matching the LCOV converter's behavior.
        if (!entry.file || entry.line == 0)
            continue;
        hitsByLine[entry.line] +=
            readCounter(counters, program.elementByteWidth, entry.counterIndex);
    }

    std::cout << "\n[" << backendName << "] line coverage of hello-coverage.slang:\n";
    for (const auto& [line, hits] : hitsByLine)
    {
        std::cout << "  line " << line << ": " << hits << (hits == 0 ? "   <-- never executed" : "")
                  << "\n";
    }

    // Assert the two counts the shader arranges deliberately (see the
    // header comment in hello-coverage.slang; line numbers are that
    // file's). checkOutputs alone cannot catch a coverage regression
    // that miscounts while the kernel results stay right — a wrong hit
    // table must fail the run, not just print.
    const auto expectHits = [&](uint32_t line, uint64_t expected)
    {
        const auto it = hitsByLine.find(line);
        const uint64_t actual = (it == hitsByLine.end()) ? 0 : it->second;
        if (actual != expected)
            fail(
                "[" + backendName + "] line " + std::to_string(line) + " has " +
                std::to_string(actual) + " hits, expected " + std::to_string(expected));
    };
    expectHits(16, 0); // applyGain's `return value;` fallthrough: gain is fixed at 2.0
    expectHits(26, 1); // the negative-input clamp: exactly one negative input

    // Write the manifest + counter snapshot for the offline pipeline.
    ComPtr<ISlangBlob> manifest;
    checkSlang(
        slang_writeCoverageManifestJson(program.coverage, manifest.writeRef()),
        "slang_writeCoverageManifestJson");
    const std::string manifestPath = backendName + ".coverage-manifest.json";
    const std::string countersPath = backendName + ".counters.bin";
    {
        std::ofstream out(manifestPath, std::ios::binary);
        out.write((const char*)manifest->getBufferPointer(), manifest->getBufferSize());
    }
    {
        std::ofstream out(countersPath, std::ios::binary);
        out.write((const char*)counters, size_t(program.counterCount) * program.elementByteWidth);
    }
    std::cout << "\nWrote " << manifestPath << " and " << countersPath << ". Convert with:\n"
              << "  python3 tools/shader-coverage/slang-coverage-to-lcov.py \\\n"
              << "      --manifest " << manifestPath << " --counters " << countersPath
              << " --output " << backendName << ".lcov\n";
}

// ## Backend: CPU
//
// The kernel becomes a host-callable shared library with the standard
// three-argument ABI from prelude/slang-cpp-types.h. Structured-buffer
// parameters are (pointer, count) pairs in the global-parameter
// payload; the coverage buffer is one more such pair, at the byte
// offset the metadata reports in `uniformOffset`. There is no device
// readback — the counters are host memory throughout.

struct CpuUInt3
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
};

struct CpuComputeVaryingInput
{
    CpuUInt3 startGroupID;
    CpuUInt3 endGroupID;
};

typedef void (*CpuComputeFunc)(
    CpuComputeVaryingInput* varyingInput,
    void* entryPointParams,
    void* globalParams);

struct CpuBufferView
{
    void* data = nullptr;
    size_t count = 0;
};

void runCpu(int counterByteWidth)
{
    auto program = compileWithCoverage(SLANG_SHADER_HOST_CALLABLE, "sm_5_0", counterByteWidth);

    auto computeFunc = (CpuComputeFunc)program.hostLibrary->findFuncByName("computeMain");
    if (!computeFunc)
        fail("findFuncByName(computeMain)");

    // On CPU the marshaling fields are the binding contract; the
    // descriptor fields don't apply. The fixed-offset writes below
    // assume the two user buffers pack first in declaration order with
    // the coverage view appended after them, so check that contract
    // (like the coverage runtime unit tests do) instead of letting a
    // regressed offset turn into an out-of-bounds write.
    if (program.resourceInfo.uniformOffset < 0)
        fail("expected a CPU uniform marshaling offset for the coverage buffer");
    if (program.resourceInfo.uniformStride != int32_t(sizeof(CpuBufferView)))
        fail("coverage buffer uniform stride is not the (pointer, count) pair size");
    if (size_t(program.resourceInfo.uniformOffset) < 2 * sizeof(CpuBufferView))
        fail("coverage buffer uniform offset overlaps the user buffers' payload");

    float inputs[kThreadCount];
    std::memcpy(inputs, kInputs, sizeof(kInputs));
    float outputs[kThreadCount] = {};
    CpuBufferView inputView = {inputs, kThreadCount};
    CpuBufferView outputView = {outputs, kThreadCount};

    std::vector<uint8_t> counters(size_t(program.counterCount) * program.elementByteWidth, 0);
    CpuBufferView coverageView = {counters.data(), program.counterCount};

    // Build the global-parameter payload: the two user buffers occupy
    // the leading fields in declaration order; the synthesized coverage
    // buffer is appended after them at `uniformOffset`.
    std::vector<uint8_t> globalParams(
        size_t(program.resourceInfo.uniformOffset) + sizeof(CpuBufferView),
        0);
    std::memcpy(globalParams.data(), &inputView, sizeof(inputView));
    std::memcpy(globalParams.data() + sizeof(CpuBufferView), &outputView, sizeof(outputView));
    std::memcpy(
        globalParams.data() + program.resourceInfo.uniformOffset,
        &coverageView,
        sizeof(coverageView));

    // Dispatch one thread group.
    CpuComputeVaryingInput varying = {};
    varying.endGroupID = {1, 1, 1};
    computeFunc(&varying, nullptr, globalParams.data());

    checkOutputs(outputs, "cpu");
    report(program, counters.data(), "cpu");
}

// ## Backend: CUDA
//
// CUDA uses the same uniform-marshaling contract as CPU: the coverage
// buffer is one more (pointer, count) pair in the global-parameter
// payload, at the byte offset the metadata reports in `uniformOffset`.
// The differences are the ones inherent to the driver model: the
// pointer in the pair is a device pointer from `cuMemAlloc`, the
// payload is copied into the module's `SLANG_globalParams` constant
// symbol rather than passed as a function argument (the emitted kernel
// takes no launch-time parameters — this entry point has no uniform
// parameters), and reading the counters back is a device-to-host copy.
// Both counter widths work without any device opt-in: the backend
// selects the matching CUDA atomic-add form directly.

#if defined(SLANG_EXAMPLE_HAS_CUDA)
void checkCuda(CUresult result, const char* what)
{
    if (result != CUDA_SUCCESS)
    {
        const char* name = nullptr;
        cuGetErrorName(result, &name);
        fail(std::string(what) + " failed with " + (name ? name : std::to_string(int(result))));
    }
}

void runCuda(int counterByteWidth)
{
    auto program = compileWithCoverage(SLANG_PTX, "cuda_sm_7_0", counterByteWidth);

    // On CUDA, as on CPU, the marshaling fields are the binding
    // contract; the descriptor fields don't apply.
    if (program.resourceInfo.uniformOffset < 0)
        fail("expected a uniform marshaling offset for the coverage buffer on CUDA");

    checkCuda(cuInit(0), "cuInit");
    CUdevice device = 0;
    checkCuda(cuDeviceGet(&device, 0), "cuDeviceGet");
    // Use the device's primary context rather than cuCtxCreate: the
    // primary-context API is signature-stable across CUDA toolkits
    // (cuCtxCreate grew a CUctxCreateParams parameter in CUDA 13) and
    // is the same pattern slang-rhi's CUDA device uses. Unlike
    // cuCtxCreate, retaining does not make the context current, so set
    // it explicitly.
    CUcontext context = nullptr;
    checkCuda(cuDevicePrimaryCtxRetain(&context, device), "cuDevicePrimaryCtxRetain");
    checkCuda(cuCtxSetCurrent(context), "cuCtxSetCurrent");

    // cuModuleLoadData parses PTX as a NUL-terminated string, and the
    // code blob is not guaranteed to carry the terminator; copying into
    // a std::string appends one.
    std::string ptx((const char*)program.code->getBufferPointer(), program.code->getBufferSize());
    CUmodule module = nullptr;
    checkCuda(cuModuleLoadData(&module, ptx.c_str()), "cuModuleLoadData");
    CUfunction kernel = nullptr;
    checkCuda(cuModuleGetFunction(&kernel, module, "computeMain"), "cuModuleGetFunction");

    // Device storage for the shader's two buffers and the counters.
    // The counters must start zeroed, exactly like every other backend.
    CUdeviceptr inputBuffer = 0;
    CUdeviceptr outputBuffer = 0;
    CUdeviceptr counterBuffer = 0;
    checkCuda(cuMemAlloc(&inputBuffer, sizeof(kInputs)), "cuMemAlloc(input)");
    checkCuda(cuMemcpyHtoD(inputBuffer, kInputs, sizeof(kInputs)), "cuMemcpyHtoD(input)");
    checkCuda(cuMemAlloc(&outputBuffer, kThreadCount * sizeof(float)), "cuMemAlloc(output)");
    checkCuda(cuMemsetD8(outputBuffer, 0, kThreadCount * sizeof(float)), "cuMemsetD8(output)");
    const size_t counterBytes = size_t(program.counterCount) * program.elementByteWidth;
    checkCuda(cuMemAlloc(&counterBuffer, counterBytes), "cuMemAlloc(counters)");
    checkCuda(cuMemsetD8(counterBuffer, 0, counterBytes), "cuMemsetD8(counters)");

    // Build the global-parameter payload exactly like the CPU path —
    // (pointer, count) pairs in declaration order, the coverage buffer
    // appended at `uniformOffset` — and copy it into the module's
    // SLANG_globalParams symbol, which is where the emitted kernel
    // reads its global parameters from.
    struct CudaBufferView
    {
        CUdeviceptr data = 0;
        size_t count = 0;
    };
    static_assert(
        sizeof(CudaBufferView) == 16,
        "the CUDA prelude's (RW)StructuredBuffer is a 16-byte (pointer, count) pair");
    // The fixed-offset writes below assume the two user buffers pack
    // first in declaration order with the coverage view appended after
    // them, so check that contract (like the coverage runtime unit
    // tests do) instead of letting a regressed offset turn into an
    // out-of-bounds write.
    if (program.resourceInfo.uniformStride != int32_t(sizeof(CudaBufferView)))
        fail("coverage buffer uniform stride is not the (pointer, count) pair size");
    if (size_t(program.resourceInfo.uniformOffset) < 2 * sizeof(CudaBufferView))
        fail("coverage buffer uniform offset overlaps the user buffers' payload");
    CudaBufferView inputView = {inputBuffer, kThreadCount};
    CudaBufferView outputView = {outputBuffer, kThreadCount};
    CudaBufferView coverageView = {counterBuffer, program.counterCount};

    std::vector<uint8_t> globalParams(
        size_t(program.resourceInfo.uniformOffset) + sizeof(CudaBufferView),
        0);
    std::memcpy(globalParams.data(), &inputView, sizeof(inputView));
    std::memcpy(globalParams.data() + sizeof(CudaBufferView), &outputView, sizeof(outputView));
    std::memcpy(
        globalParams.data() + program.resourceInfo.uniformOffset,
        &coverageView,
        sizeof(coverageView));

    CUdeviceptr paramsSymbol = 0;
    size_t paramsSymbolSize = 0;
    checkCuda(
        cuModuleGetGlobal(&paramsSymbol, &paramsSymbolSize, module, "SLANG_globalParams"),
        "cuModuleGetGlobal(SLANG_globalParams)");
    if (paramsSymbolSize < globalParams.size())
        fail("SLANG_globalParams is smaller than the marshaled parameter payload");
    checkCuda(
        cuMemcpyHtoD(paramsSymbol, globalParams.data(), globalParams.size()),
        "cuMemcpyHtoD(SLANG_globalParams)");

    // Dispatch one thread group of kThreadCount threads.
    checkCuda(
        cuLaunchKernel(kernel, 1, 1, 1, kThreadCount, 1, 1, 0, nullptr, nullptr, nullptr),
        "cuLaunchKernel");
    checkCuda(cuCtxSynchronize(), "cuCtxSynchronize");

    float outputs[kThreadCount] = {};
    checkCuda(cuMemcpyDtoH(outputs, outputBuffer, sizeof(outputs)), "cuMemcpyDtoH(outputs)");
    std::vector<uint8_t> counters(counterBytes);
    checkCuda(cuMemcpyDtoH(counters.data(), counterBuffer, counterBytes), "cuMemcpyDtoH(counters)");

    checkOutputs(outputs, "cuda");
    report(program, counters.data(), "cuda");

    cuMemFree(counterBuffer);
    cuMemFree(outputBuffer);
    cuMemFree(inputBuffer);
    cuModuleUnload(module);
    cuDevicePrimaryCtxRelease(device);
}
#endif

// ## Backend: Vulkan
//
// The counter buffer is an ordinary storage buffer at the descriptor
// `(space, binding)` reported by the metadata. Auto-allocation places
// it in a fresh descriptor set after the shader's own sets, so the
// pipeline layout below has one more set than the shader's reflection
// alone would suggest. Uses the same raw-Vulkan helper as the other
// coverage examples (see vk_compute_demo.h for why raw Vulkan rather
// than slang-rhi, for now).

#if defined(SLANG_EXAMPLE_HAS_VULKAN)
void runVulkan(int counterByteWidth)
{
    auto program = compileWithCoverage(SLANG_SPIRV, "spirv_1_5", counterByteWidth);

    if (program.resourceInfo.space < 0 || program.resourceInfo.binding < 0)
        fail("expected a descriptor (space, binding) for the coverage buffer on SPIR-V");
    const auto coverageSet = (uint32_t)program.resourceInfo.space;
    const auto coverageBinding = (uint32_t)program.resourceInfo.binding;
    // The user's buffers occupy set 0; auto-allocation must have
    // steered the coverage buffer into its own set (the Metal path
    // makes the analogous buffer-index check).
    if (coverageSet == 0)
        fail("coverage descriptor set collides with the user buffers' set");

    vkdemo::Context ctx;
    // 64-bit counters need `shaderBufferInt64Atomics` at pipeline
    // creation; filter the device pick accordingly. (This is the
    // device-feature pitfall from the tutorial: on runtimes without the
    // feature — notably MoltenVK on Apple Silicon — the 64-bit path has
    // no eligible device, and 32-bit is the answer.)
    ctx.init(/* requireInt64Atomics: */ program.elementByteWidth == 8);

    std::vector<std::vector<VkDescriptorSetLayoutBinding>> setBindings;
    setBindings.resize(std::max<uint32_t>(1, coverageSet + 1));
    auto pushBinding = [](std::vector<VkDescriptorSetLayoutBinding>& v, uint32_t b)
    {
        VkDescriptorSetLayoutBinding lb = {};
        lb.binding = b;
        lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        lb.descriptorCount = 1;
        lb.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        v.push_back(lb);
    };
    pushBinding(setBindings[0], 0); // inputBuffer
    pushBinding(setBindings[0], 1); // outputBuffer
    pushBinding(setBindings[coverageSet], coverageBinding);

    // Slang's SPIR-V emit renames the entry point to "main".
    auto pipeline = ctx.createComputePipeline(
        program.code->getBufferPointer(),
        program.code->getBufferSize(),
        setBindings,
        "main");

    auto inputBuffer = ctx.createBuffer(sizeof(kInputs), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    ctx.upload(inputBuffer, kInputs, sizeof(kInputs));
    auto outputBuffer =
        ctx.createBuffer(kThreadCount * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    const VkDeviceSize counterBytes = VkDeviceSize(program.counterCount) * program.elementByteWidth;
    auto counterBuffer = ctx.createBuffer(counterBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    std::vector<uint8_t> zeros(counterBytes, 0);
    ctx.upload(counterBuffer, zeros.data(), counterBytes);

    std::vector<VkDescriptorSet> sets(setBindings.size());
    for (size_t i = 0; i < sets.size(); ++i)
        sets[i] = ctx.allocateDescriptorSet(pipeline.setLayouts[i]);
    ctx.writeStorageBuffer(sets[0], 0, inputBuffer);
    ctx.writeStorageBuffer(sets[0], 1, outputBuffer);
    ctx.writeStorageBuffer(sets[coverageSet], coverageBinding, counterBuffer);

    ctx.dispatch(pipeline, sets, 1, 1, 1);

    float outputs[kThreadCount] = {};
    ctx.download(outputBuffer, outputs, sizeof(outputs));
    std::vector<uint8_t> counters(counterBytes);
    ctx.download(counterBuffer, counters.data(), counterBytes);

    checkOutputs(outputs, "vulkan");
    report(program, counters.data(), "vulkan");

    ctx.destroyBuffer(inputBuffer);
    ctx.destroyBuffer(outputBuffer);
    ctx.destroyBuffer(counterBuffer);
    ctx.destroyPipeline(pipeline);
}
#endif

// ## Backend: Metal
//
// The counter buffer is set on the compute encoder at the plain
// `[[buffer(N)]]` index the metadata reports in `binding`; Metal has
// no descriptor-space dimension (`space` is -1). The emitted MSL is
// compiled at runtime with the Metal framework's own compiler, so the
// example needs a Metal device but no external toolchain. Counting-mode
// counters are always 32-bit on Metal — MSL has no 64-bit atomic
// fetch-add, and the compiler caps the width automatically (with
// warning E45115 if 64 was requested explicitly).

#if defined(SLANG_EXAMPLE_HAS_METAL)
void runMetal(int counterByteWidth)
{
    auto program = compileWithCoverage(SLANG_METAL, "metal", counterByteWidth);

    if (program.resourceInfo.binding < 0)
        fail("expected a [[buffer(N)]] binding for the coverage buffer on Metal");
    // The user's buffers occupy indices 0 and 1 in declaration order;
    // auto-allocation must have steered the coverage buffer elsewhere.
    const auto coverageIndex = (NS::UInteger)program.resourceInfo.binding;
    if (coverageIndex == 0 || coverageIndex == 1)
        fail("coverage buffer index collides with a user buffer");

    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    if (!device)
        fail("no Metal device available");
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

    NS::Error* error = nullptr;
    NS::String* source =
        NS::String::string((const char*)program.code->getBufferPointer(), NS::UTF8StringEncoding);
    MTL::Library* library = device->newLibrary(source, nullptr, &error);
    if (!library)
    {
        fail(
            std::string("Metal compile failed: ") +
            (error ? error->localizedDescription()->utf8String() : "unknown error"));
    }
    MTL::Function* function =
        library->newFunction(NS::String::string("computeMain", NS::UTF8StringEncoding));
    if (!function)
        fail("Metal function computeMain not found");
    MTL::ComputePipelineState* pipeline = device->newComputePipelineState(function, &error);
    if (!pipeline)
    {
        fail(
            std::string("Metal pipeline failed: ") +
            (error ? error->localizedDescription()->utf8String() : "unknown error"));
    }

    MTL::Buffer* inputBuffer =
        device->newBuffer(kInputs, sizeof(kInputs), MTL::ResourceStorageModeShared);
    MTL::Buffer* outputBuffer =
        device->newBuffer(kThreadCount * sizeof(float), MTL::ResourceStorageModeShared);
    const size_t counterBytes = size_t(program.counterCount) * program.elementByteWidth;
    MTL::Buffer* counterBuffer = device->newBuffer(counterBytes, MTL::ResourceStorageModeShared);
    std::memset(outputBuffer->contents(), 0, kThreadCount * sizeof(float));
    std::memset(counterBuffer->contents(), 0, counterBytes);

    MTL::CommandQueue* queue = device->newCommandQueue();
    MTL::CommandBuffer* commandBuffer = queue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();
    encoder->setComputePipelineState(pipeline);
    encoder->setBuffer(inputBuffer, 0, 0);
    encoder->setBuffer(outputBuffer, 0, 1);
    encoder->setBuffer(counterBuffer, 0, coverageIndex);
    encoder->dispatchThreadgroups(MTL::Size(1, 1, 1), MTL::Size(kThreadCount, 1, 1));
    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
    if (commandBuffer->status() != MTL::CommandBufferStatusCompleted)
        fail("Metal command buffer did not complete");

    checkOutputs((const float*)outputBuffer->contents(), "metal");
    report(program, counterBuffer->contents(), "metal");

    queue->release();
    counterBuffer->release();
    outputBuffer->release();
    inputBuffer->release();
    pipeline->release();
    function->release();
    library->release();
    pool->release();
    device->release();
}
#endif

void printUsage()
{
    std::cout << "usage: shader-coverage-backends --backend=<cpu|cuda|vulkan|metal> [options]\n"
              << "  --backend=<name>     dispatch path to run (required)\n"
              << "  --counter-width=<n>  32 (default) or 64. 64 needs 64-bit shader atomics on\n"
              << "                       the device (Vulkan); Metal always caps to 32; CPU and\n"
              << "                       CUDA support both widths with no device opt-in.\n"
              << "  --demo-dir=<path>    directory containing hello-coverage.slang\n";
}

} // namespace

int main(int argc, char** argv)
{
    std::string backend;
    int counterWidthBits = 32;
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg.rfind("--backend=", 0) == 0)
            backend = arg.substr(10);
        else if (arg.rfind("--counter-width=", 0) == 0)
        {
            try
            {
                counterWidthBits = std::stoi(arg.substr(16));
            }
            catch (const std::exception&)
            {
                printUsage();
                return 1;
            }
        }
        else if (arg.rfind("--demo-dir=", 0) == 0)
            g_demoDirOverride = arg.substr(11);
        else
        {
            printUsage();
            return arg == "--help" || arg == "-h" ? 0 : 1;
        }
    }
    if (counterWidthBits != 32 && counterWidthBits != 64)
        fail("--counter-width must be 32 or 64");
    const int counterByteWidth = counterWidthBits / 8;

    // Failures below are terminal: the Vulkan/Metal helpers report
    // errors by throwing, and the demo's answer to any of them is the
    // same — print the message and exit nonzero.
    try
    {
        if (backend == "cpu")
        {
            runCpu(counterByteWidth);
        }
        else if (backend == "cuda")
        {
#if defined(SLANG_EXAMPLE_HAS_CUDA)
            runCuda(counterByteWidth);
#else
            fail("this build has no CUDA support (CUDA toolkit not found at configure time)");
#endif
        }
        else if (backend == "vulkan")
        {
#if defined(SLANG_EXAMPLE_HAS_VULKAN)
            runVulkan(counterByteWidth);
#else
            fail("this build has no Vulkan support (Vulkan loader not found at configure time)");
#endif
        }
        else if (backend == "metal")
        {
#if defined(SLANG_EXAMPLE_HAS_METAL)
            runMetal(counterByteWidth);
#else
            fail("the metal backend is only available on Apple platforms");
#endif
        }
        else
        {
            printUsage();
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        fail(e.what());
    }
    return 0;
}
