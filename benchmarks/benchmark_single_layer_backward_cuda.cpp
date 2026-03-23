// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// C++ benchmark for single-layer backward pass using Slang -> PTX -> CUDA JIT.
// This mirrors benchmark_single_layer_backward.py for fair comparison.
//
// Build:
//   nvcc -o benchmark_cuda benchmark_single_layer_backward_cuda.cpp \
//        -I<slang_root>/include -L<slang_root>/build/Release/lib -lslang \
//        -lcuda -std=c++17
//
// Run:
//   LD_LIBRARY_PATH=<slang_root>/build/Release/lib ./benchmark_cuda [--size tiny|small|medium|large] [--all]

#include <cuda.h>
#include <cuda_fp16.h>

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

#include "slang-com-ptr.h"
#include "slang.h"

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

#define CUDA_CHECK(call)                                                          \
    do                                                                            \
    {                                                                             \
        CUresult err = (call);                                                    \
        if (err != CUDA_SUCCESS)                                                  \
        {                                                                         \
            const char* errStr = nullptr;                                         \
            cuGetErrorString(err, &errStr);                                       \
            fprintf(stderr, "CUDA error at %s:%d: %s\n", __FILE__, __LINE__,     \
                    errStr ? errStr : "unknown");                                 \
            exit(1);                                                              \
        }                                                                         \
    } while (0)

#define SLANG_CHECK(call)                                                         \
    do                                                                            \
    {                                                                             \
        SlangResult res = (call);                                                 \
        if (SLANG_FAILED(res))                                                    \
        {                                                                         \
            fprintf(stderr, "Slang error at %s:%d: 0x%x\n", __FILE__, __LINE__,  \
                    (unsigned)res);                                                \
            exit(1);                                                              \
        }                                                                         \
    } while (0)

static void printDiagnostics(slang::IBlob* blob)
{
    if (blob && blob->getBufferSize() > 0)
    {
        fprintf(stderr, "Slang diagnostics:\n%s\n", (const char*)blob->getBufferPointer());
    }
}

// ---------------------------------------------------------------------------
// Network configurations (matching the Python version)
// ---------------------------------------------------------------------------

struct NetworkConfig
{
    const char* name;
    int inputSize;
    int outputSize;
};

static const NetworkConfig kConfigs[] = {
    {"tiny",   32,  16},
    {"small",  64,  16},
    {"medium", 128, 32},
    {"large",  256, 64},
};

static constexpr int BATCH_SIZE  = 256;
static constexpr int WARMUP      = 50;
static constexpr int ITERATIONS  = 200;

// ---------------------------------------------------------------------------
// Find the neural module directory (mirrors Python find_neural_module_dir)
// ---------------------------------------------------------------------------

static std::string findNeuralModuleDir(const std::string& slangRoot)
{
    std::vector<std::string> candidates = {
        slangRoot + "/build/Release/lib/slang-standard-module-2026.1.2",
        slangRoot + "/build/Debug/lib/slang-standard-module-2026.1.2",
    };
    for (auto& c : candidates)
    {
        auto path = fs::path(c) / "neural.slang-module";
        if (fs::exists(path))
            return c;
    }
    return "";
}

static std::string findSlangLibDir(const std::string& slangRoot)
{
    std::vector<std::string> candidates = {
        slangRoot + "/build/Release/lib",
        slangRoot + "/build/Debug/lib",
    };
    for (auto& c : candidates)
    {
        auto path = fs::path(c) / "libslang.so";
        if (fs::exists(path))
            return c;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Compile Slang shader to PTX
// ---------------------------------------------------------------------------

static std::string compileToPTX(
    const std::string& slangRoot,
    const std::string& shaderDir,
    const std::string& neuralModuleDir,
    int inputSize,
    int outputSize)
{
    using namespace Slang;

    // Create global session
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()));

    // Set up target: PTX
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;

    // Set up preprocessor defines
    slang::PreprocessorMacroDesc macros[2];
    macros[0].name = "INPUT_SIZE";
    std::string inputSizeStr = std::to_string(inputSize);
    macros[0].value = inputSizeStr.c_str();
    macros[1].name = "OUTPUT_SIZE";
    std::string outputSizeStr = std::to_string(outputSize);
    macros[1].value = outputSizeStr.c_str();

    // Set up search paths
    std::string slangLibDir = findSlangLibDir(slangRoot);
    const char* searchPaths[] = {
        shaderDir.c_str(),
        neuralModuleDir.c_str(),
        slangLibDir.c_str(),
    };

    // Create session
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 3;
    sessionDesc.preprocessorMacros = macros;
    sessionDesc.preprocessorMacroCount = 2;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()));

    // Load the module
    ComPtr<slang::IBlob> diagnosticBlob;
    slang::IModule* module = session->loadModule(
        "benchmark_single_layer_backward",
        diagnosticBlob.writeRef());
    if (!module)
    {
        printDiagnostics(diagnosticBlob);
        fprintf(stderr, "Failed to load Slang module\n");
        exit(1);
    }
    printDiagnostics(diagnosticBlob);

    // Find entry point
    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK(module->findEntryPointByName(
        "compute_single_layer_backward", entryPoint.writeRef()));

    // Compose and link
    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> composedProgram;
    SLANG_CHECK(session->createCompositeComponentType(
        components, 2, composedProgram.writeRef(), diagnosticBlob.writeRef()));
    printDiagnostics(diagnosticBlob);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK(composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef()));
    printDiagnostics(diagnosticBlob);

    // Get PTX code
    ComPtr<slang::IBlob> ptxBlob;
    SLANG_CHECK(linkedProgram->getEntryPointCode(
        0, 0, ptxBlob.writeRef(), diagnosticBlob.writeRef()));
    printDiagnostics(diagnosticBlob);

    if (!ptxBlob || ptxBlob->getBufferSize() == 0)
    {
        fprintf(stderr, "Failed to generate PTX code\n");
        exit(1);
    }

    std::string ptx((const char*)ptxBlob->getBufferPointer(), ptxBlob->getBufferSize());
    printf("  PTX generated: %zu bytes\n", ptx.size());
    return ptx;
}

// ---------------------------------------------------------------------------
// Run benchmark for a single configuration
// ---------------------------------------------------------------------------

static double runBenchmark(const NetworkConfig& config, const std::string& slangRoot, bool fullBatch = false)
{
    int inputSize  = config.inputSize;
    int outputSize = config.outputSize;
    int totalParams = inputSize * outputSize + outputSize;

    printf("\n======================================================================\n");
    printf("Single Layer Backward: %d -> %d\n", inputSize, outputSize);
    printf("Parameters: %d\n", totalParams);
    printf("======================================================================\n");

    // Find neural module
    std::string neuralModuleDir = findNeuralModuleDir(slangRoot);
    if (neuralModuleDir.empty())
    {
        fprintf(stderr, "ERROR: neural.slang-module not found\n");
        return -1.0;
    }

    std::string shaderDir =
        (fs::path(__FILE__).parent_path()).string();

    // --- Step 1: Compile Slang to PTX ---
    printf("  Compiling Slang -> PTX...\n");
    std::string ptx = compileToPTX(slangRoot, shaderDir, neuralModuleDir, inputSize, outputSize);

    // --- Step 2: Initialize CUDA driver API ---
    CUDA_CHECK(cuInit(0));
    CUdevice device;
    CUDA_CHECK(cuDeviceGet(&device, 0));

    char deviceName[256];
    CUDA_CHECK(cuDeviceGetName(deviceName, sizeof(deviceName), device));
    printf("  CUDA Device: %s\n", deviceName);

    CUcontext ctx;
    CUDA_CHECK(cuDevicePrimaryCtxRetain(&ctx, device));
    CUDA_CHECK(cuCtxSetCurrent(ctx));

    // --- Step 3: Load PTX via JIT ---
    printf("  Loading PTX via CUDA JIT...\n");
    CUmodule cuModule;
    CUresult loadResult = cuModuleLoadDataEx(&cuModule, ptx.c_str(), 0, nullptr, nullptr);
    if (loadResult != CUDA_SUCCESS)
    {
        const char* errStr = nullptr;
        cuGetErrorString(loadResult, &errStr);
        fprintf(stderr, "Failed to JIT-load PTX: %s\n", errStr ? errStr : "unknown");

        // Dump PTX for debugging
        FILE* f = fopen("/tmp/debug_kernel.ptx", "w");
        if (f) { fwrite(ptx.c_str(), 1, ptx.size(), f); fclose(f); }
        fprintf(stderr, "PTX dumped to /tmp/debug_kernel.ptx\n");
        cuCtxDestroy(ctx);
        return -1.0;
    }

    CUfunction kernel;
    CUDA_CHECK(cuModuleGetFunction(&kernel, cuModule, "compute_single_layer_backward"));
    printf("  Kernel loaded successfully.\n");

    // --- Step 4: Allocate GPU buffers ---
    size_t paramsBytes     = totalParams * sizeof(__half);
    size_t inputsBytes     = BATCH_SIZE * inputSize * sizeof(__half);
    size_t gradOutputBytes = BATCH_SIZE * outputSize * sizeof(__half);
    size_t gradParamsBytes = totalParams * sizeof(__half);
    size_t gradInputsBytes = BATCH_SIZE * inputSize * sizeof(__half);

    // Generate random data on host
    std::mt19937 rng(42);
    std::normal_distribution<float> dist(0.0f, 0.1f);

    auto makeHalfBuffer = [&](size_t count) -> std::vector<__half> {
        std::vector<__half> buf(count);
        for (size_t i = 0; i < count; i++)
            buf[i] = __float2half(dist(rng));
        return buf;
    };

    auto hParams     = makeHalfBuffer(totalParams);
    auto hInputs     = makeHalfBuffer(BATCH_SIZE * inputSize);
    auto hGradOutput = makeHalfBuffer(BATCH_SIZE * outputSize);
    std::vector<__half> hZeros;

    // Allocate device memory
    CUdeviceptr dParams, dInputs, dGradOutputs, dGradParams, dGradInputs;
    CUDA_CHECK(cuMemAlloc(&dParams,      paramsBytes));
    CUDA_CHECK(cuMemAlloc(&dInputs,      inputsBytes));
    CUDA_CHECK(cuMemAlloc(&dGradOutputs, gradOutputBytes));
    CUDA_CHECK(cuMemAlloc(&dGradParams,  gradParamsBytes));
    CUDA_CHECK(cuMemAlloc(&dGradInputs,  gradInputsBytes));

    // Upload data
    CUDA_CHECK(cuMemcpyHtoD(dParams,      hParams.data(),     paramsBytes));
    CUDA_CHECK(cuMemcpyHtoD(dInputs,      hInputs.data(),     inputsBytes));
    CUDA_CHECK(cuMemcpyHtoD(dGradOutputs, hGradOutput.data(), gradOutputBytes));
    CUDA_CHECK(cuMemsetD8(dGradParams,  0, gradParamsBytes));
    CUDA_CHECK(cuMemsetD8(dGradInputs, 0, gradInputsBytes));

    // --- Step 5: Prepare kernel launch parameters ---
    //
    // Slang compiles StructuredBuffer<T> / RWStructuredBuffer<T> to a struct:
    //   struct { T* data; size_t count; }   (16 bytes total)
    // See prelude/slang-cuda-prelude.h for the definition.
    //
    // The PTX signature confirms this:
    //   .param .align 8 .b8 param_N[16]   for each buffer
    //   .param .u32 param_5               for batch_size (uniform int)
    //
    struct StructuredBufferArg
    {
        CUdeviceptr data;
        size_t count;
    };

    StructuredBufferArg argParams     = { dParams,      (size_t)totalParams };
    StructuredBufferArg argInputs     = { dInputs,      (size_t)(BATCH_SIZE * inputSize) };
    StructuredBufferArg argGradOut    = { dGradOutputs, (size_t)(BATCH_SIZE * outputSize) };
    StructuredBufferArg argGradParams = { dGradParams,  (size_t)totalParams };
    StructuredBufferArg argGradInputs = { dGradInputs,  (size_t)(BATCH_SIZE * inputSize) };
    int batchSize = BATCH_SIZE;

    void* kernelArgs[] = {
        &argParams,
        &argInputs,
        &argGradOut,
        &argGradParams,
        &argGradInputs,
        &batchSize,
    };

    // Thread configuration:
    // slangpy's dispatch(thread_count=[N,1,1]) divides by numthreads to get groups:
    //   groups = ceil(BATCH_SIZE / 32)
    // The shader uses SV_GroupID.x as sample index, one group per sample.
    // To process ALL BATCH_SIZE samples, we need BATCH_SIZE groups.
    unsigned int blockX = 32;  // must match [numthreads(32, 1, 1)]
    unsigned int blockY = 1;
    unsigned int blockZ = 1;
    // slangpy dispatch(thread_count=[N,1,1]) means N *total threads*, internally
    // computing ceil(N/numthreads) groups. With numthreads=32:
    //   thread_count=[256,1,1] → 8 groups → only 8 samples processed!
    //
    // The shader uses SV_GroupID.x as sample index, so to process ALL BATCH_SIZE
    // samples we need BATCH_SIZE groups, i.e. thread_count=[BATCH_SIZE*32, 1, 1].
    //
    // Default: match slangpy's Python benchmark (8 groups) for apples-to-apples.
    // Use --full-batch to dispatch BATCH_SIZE groups (correct full-batch behavior).
    unsigned int gridX  = fullBatch ? BATCH_SIZE : (BATCH_SIZE + blockX - 1) / blockX;
    int samplesProcessed = fullBatch ? BATCH_SIZE : (int)gridX;
    unsigned int gridY  = 1;
    unsigned int gridZ  = 1;

    printf("  Launch config: grid=(%u,%u,%u) block=(%u,%u,%u)\n",
           gridX, gridY, gridZ, blockX, blockY, blockZ);
    printf("  Samples processed per dispatch: %d%s\n",
           samplesProcessed,
           fullBatch ? " (full batch)" : " (matching slangpy thread_count)");

    // --- Step 6: Warmup ---
    printf("  Warming up (%d iterations)...\n", WARMUP);
    for (int i = 0; i < WARMUP; i++)
    {
        CUDA_CHECK(cuLaunchKernel(
            kernel,
            gridX, gridY, gridZ,
            blockX, blockY, blockZ,
            0, nullptr,
            kernelArgs, nullptr));
    }
    CUDA_CHECK(cuCtxSynchronize());

    // --- Step 7: Benchmark with CUDA events ---
    printf("  Benchmarking (%d iterations)...\n", ITERATIONS);

    CUevent startEvent, stopEvent;
    CUDA_CHECK(cuEventCreate(&startEvent, CU_EVENT_DEFAULT));
    CUDA_CHECK(cuEventCreate(&stopEvent, CU_EVENT_DEFAULT));

    CUDA_CHECK(cuEventRecord(startEvent, nullptr));

    for (int i = 0; i < ITERATIONS; i++)
    {
        CUDA_CHECK(cuLaunchKernel(
            kernel,
            gridX, gridY, gridZ,
            blockX, blockY, blockZ,
            0, nullptr,
            kernelArgs, nullptr));
    }

    CUDA_CHECK(cuEventRecord(stopEvent, nullptr));
    CUDA_CHECK(cuEventSynchronize(stopEvent));

    float totalTimeMs = 0.0f;
    CUDA_CHECK(cuEventElapsedTime(&totalTimeMs, startEvent, stopEvent));

    double avgTimeMs = (double)totalTimeMs / ITERATIONS;
    double throughput = samplesProcessed / (avgTimeMs / 1000.0);

    printf("\nResults:\n");
    printf("  Backend: CUDA (PTX JIT)%s\n", fullBatch ? " [full batch]" : " [slangpy-compat]");
    printf("  Network: %d -> %d\n", inputSize, outputSize);
    printf("  Total time: %.4f ms (%d iterations)\n", totalTimeMs, ITERATIONS);
    printf("  Avg time: %.4f ms\n", avgTimeMs);
    printf("  Samples/dispatch: %d\n", samplesProcessed);
    printf("  Throughput: %.0f samples/s\n", throughput);

    // Cleanup
    CUDA_CHECK(cuEventDestroy(startEvent));
    CUDA_CHECK(cuEventDestroy(stopEvent));
    CUDA_CHECK(cuMemFree(dParams));
    CUDA_CHECK(cuMemFree(dInputs));
    CUDA_CHECK(cuMemFree(dGradOutputs));
    CUDA_CHECK(cuMemFree(dGradParams));
    CUDA_CHECK(cuMemFree(dGradInputs));
    CUDA_CHECK(cuModuleUnload(cuModule));
    CUDA_CHECK(cuCtxSetCurrent(nullptr));
    CUDA_CHECK(cuDevicePrimaryCtxRelease(device));

    return avgTimeMs;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // Parse arguments
    std::string size = "small";
    bool runAll = false;
    bool fullBatch = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--size") == 0 && i + 1 < argc)
        {
            size = argv[++i];
        }
        else if (strcmp(argv[i], "--all") == 0)
        {
            runAll = true;
        }
        else if (strcmp(argv[i], "--full-batch") == 0)
        {
            fullBatch = true;
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            printf("Usage: %s [--size tiny|small|medium|large] [--all] [--full-batch]\n", argv[0]);
            printf("\n");
            printf("Options:\n");
            printf("  --size S       Network size (default: small)\n");
            printf("  --all          Run all sizes\n");
            printf("  --full-batch   Dispatch BATCH_SIZE groups (correct full-batch)\n");
            printf("                 Default: match slangpy's thread_count dispatch\n");
            return 0;
        }
    }

    // Determine slang root: 5 levels up from this file's directory
    fs::path thisFile = fs::path(__FILE__).parent_path();
    fs::path slangRoot = thisFile / ".." / ".." / ".." / ".." / "..";
    slangRoot = fs::canonical(slangRoot);
    std::string slangRootStr = slangRoot.string();
    printf("Slang root: %s\n", slangRootStr.c_str());

    printf("======================================================================\n");
    printf("Single Layer Backward Pass Benchmark (C++ / CUDA PTX JIT)\n");
    printf("======================================================================\n");
    printf("Batch size: %d\n", BATCH_SIZE);
    printf("Warmup: %d, Iterations: %d\n", WARMUP, ITERATIONS);
    printf("Mode: %s\n", fullBatch ? "full-batch (BATCH_SIZE groups)"
                                   : "slangpy-compat (ceil(BATCH_SIZE/32) groups)");

    if (runAll)
    {
        struct Result { std::string name; int input; int output; double timeMs; };
        std::vector<Result> results;

        for (auto& cfg : kConfigs)
        {
            double t = runBenchmark(cfg, slangRootStr, fullBatch);
            results.push_back({cfg.name, cfg.inputSize, cfg.outputSize, t});
        }

        printf("\n======================================================================\n");
        printf("Summary: Single Layer Backward (C++ CUDA PTX JIT)\n");
        printf("======================================================================\n");
        printf("%-10s %-15s %-12s %s\n", "Size", "Network", "Time (ms)", "Status");
        printf("--------------------------------------------------\n");
        for (auto& r : results)
        {
            char net[32];
            snprintf(net, sizeof(net), "%d->%d", r.input, r.output);
            if (r.timeMs >= 0)
                printf("%-10s %-15s %-12.4f OK\n", r.name.c_str(), net, r.timeMs);
            else
                printf("%-10s %-15s %-12s FAILED\n", r.name.c_str(), net, "N/A");
        }
    }
    else
    {
        const NetworkConfig* found = nullptr;
        for (auto& cfg : kConfigs)
        {
            if (size == cfg.name) { found = &cfg; break; }
        }
        if (!found)
        {
            fprintf(stderr, "Unknown size: %s (use tiny, small, medium, large)\n", size.c_str());
            return 1;
        }
        runBenchmark(*found, slangRootStr, fullBatch);
    }

    return 0;
}
