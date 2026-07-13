// hello-coverage-host.cpp: load the kernel slangc precompiled into
// hello-coverage-kernel.so, bind the hidden counter buffer where the
// sidecar manifest says, dispatch one thread group, print the computed
// outputs, and write the raw coverage counters for the LCOV converter.
// Uses no Slang headers or library — the manifest is the whole contract.
//
// Pass --no-coverage to run as a plain CPU shared-library dispatch,
// without binding or reporting the coverage buffer. The `coverageEnabled`
// blocks below are exactly what coverage adds to a host; without them
// the program also runs a kernel compiled without -trace-coverage.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
static void* loadKernel(const char* path)
{
    return (void*)LoadLibraryA(path);
}
static void* findFunc(void* lib, const char* name)
{
    return (void*)GetProcAddress((HMODULE)lib, name);
}
#else
#include <dlfcn.h>
static void* loadKernel(const char* path)
{
    return dlopen(path, RTLD_NOW);
}
static void* findFunc(void* lib, const char* name)
{
    return dlsym(lib, name);
}
#endif

// The CPU compute-kernel ABI (see prelude/slang-cpp-types.h): group-ID
// range, entry-point uniforms, and the global-parameter payload.
struct UInt3
{
    uint32_t x, y, z;
};
struct ComputeVaryingInput
{
    UInt3 startGroupID;
    UInt3 endGroupID;
};
typedef void (*ComputeFunc)(ComputeVaryingInput*, void*, void*);

// How a (RW)StructuredBuffer<T> parameter is laid out in the payload:
// 16 bytes on a 64-bit host. The manifest offsets used below assume
// this; a different host word size reports different offsets.
struct BufferView
{
    void* data;
    size_t count;
};

// Values from hello-coverage-kernel.so.coverage-manifest.json. A real
// integration parses them out of the JSON; they are inlined here to
// keep the listing dependency-free.
constexpr uint32_t kCounterCount = 9;   // "counter_count"
constexpr uint32_t kElementStride = 8;  // "buffer": "element_stride"
constexpr uint32_t kUniformOffset = 32; // "buffer": "uniform_offset"

#ifdef _WIN32
constexpr const char* kKernelPath = "hello-coverage-kernel.dll";
#else
constexpr const char* kKernelPath = "./hello-coverage-kernel.so";
#endif

int main(int argc, char** argv)
{
    const bool coverageEnabled = !(argc > 1 && std::strcmp(argv[1], "--no-coverage") == 0);

    void* library = loadKernel(kKernelPath);
    auto computeMain = library ? (ComputeFunc)findFunc(library, "computeMain") : nullptr;
    if (!computeMain)
    {
        std::fprintf(stderr, "cannot load %s\n", kKernelPath);
        return 1;
    }

    // Bind. On CPU, "binding" means writing a (pointer, count) pair
    // into the parameter payload. The shader's own buffers occupy the
    // leading fields in declaration order.
    float inputs[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float outputs[4] = {};
    BufferView inputView = {inputs, 4};
    BufferView outputView = {outputs, 4};

    std::vector<uint8_t> payload(
        coverageEnabled ? kUniformOffset + sizeof(BufferView) : 2 * sizeof(BufferView),
        0);
    std::memcpy(payload.data(), &inputView, sizeof(inputView));
    std::memcpy(payload.data() + sizeof(BufferView), &outputView, sizeof(outputView));

    // Coverage addition: counter storage sized from the manifest, bound
    // at the manifest-reported uniform_offset. Counters must start
    // zeroed.
    static_assert(kElementStride == 8, "manifest says uint64 counters");
    std::vector<uint64_t> coverageCounters(kCounterCount, 0);
    if (coverageEnabled)
    {
        BufferView coverageView = {coverageCounters.data(), kCounterCount};
        std::memcpy(payload.data() + kUniformOffset, &coverageView, sizeof(coverageView));
    }

    // Dispatch one thread group and show the computed outputs.
    ComputeVaryingInput varying = {{0, 0, 0}, {1, 1, 1}};
    computeMain(&varying, nullptr, payload.data());

    for (int i = 0; i < 4; ++i)
        std::printf("output[%d] = %g\n", i, outputs[i]);

    if (!coverageEnabled)
        return 0;

    // Coverage addition: dump the counter slots and save them for the
    // LCOV converter. Which source line each slot counts is the
    // manifest's "entries" job — the converter does that attribution.
    for (uint32_t i = 0; i < kCounterCount; ++i)
        std::printf("counter[%u] = %llu\n", i, (unsigned long long)coverageCounters[i]);

    std::ofstream("hello-coverage.counters.bin", std::ios::binary)
        .write((const char*)coverageCounters.data(), kCounterCount * kElementStride);
    return 0;
}
