---
layout: user-guide
---

# Shader Execution Coverage

Slang can instrument your shaders so that each executed statement increments a counter at
runtime — the same idea as gcov-style coverage for CPU code, applied to GPU (and CPU) kernels.
After a dispatch, you read the counters back and convert them into an LCOV report that tools
like `genhtml`, Codecov, or VS Code Coverage Gutters can render, showing exactly which lines,
functions, and branch outcomes your test content actually exercised.

This tutorial walks through the whole pipeline hands-on: compiling with coverage, reading the
generated metadata, binding the counter buffer and dispatching from a small C++ host program,
and producing a report — plus the pitfalls you are most likely to hit along the way. The
whole tour uses the _offline_ workflow: `slangc` precompiles the shader, a sidecar manifest
file describes the counters, and the host program consumes both without linking Slang at all.
Every step, including the real dispatch, runs on any machine with a Slang release, a C++
compiler, and Python 3: the dispatched kernel is compiled for Slang's CPU target, and later
sections show what changes on GPU targets and in hosts that compile shaders at runtime
through the C++ API. The tutorial's files — together with `run-tutorial.sh` and
`run-tutorial.ps1` scripts that execute every step in order — are available under
[`examples/shader-coverage-tutorial`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-tutorial).

This chapter is a guided tour, not the reference. The reference material lives in
[`tools/shader-coverage/README.md`](https://github.com/shader-slang/slang/blob/master/tools/shader-coverage/README.md)
(workflow and support matrix) and
[`docs/design/shader-coverage-host-interface.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-host-interface.md)
(the host binding contract, with per-target recipes).

## Your first coverage build

Create a file named `hello-coverage.slang`:

```hlsl
// hello-coverage.slang
StructuredBuffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;

float applyGain(float value, float gain)
{
    if (gain > 1.0)
        return value * gain;
    return value;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    uint index = threadId.x;
    float value = inputBuffer[index];
    if (value < 0.0)
        value = 0.0;
    outputBuffer[index] = applyGain(value, 2.0);
}
```

Compile it with line coverage enabled by adding one flag, `-trace-coverage`:

```bash
slangc hello-coverage.slang -target spirv -stage compute -entry computeMain \
    -trace-coverage -o hello-coverage.spv
```

Two files appear:

```
hello-coverage.spv
hello-coverage.spv.coverage-manifest.json
```

The `.spv` is your shader, now containing a hidden counter buffer and an atomic increment
before each executable statement. The `.coverage-manifest.json` _sidecar_ is the map that
makes the counters meaningful: it records which counter slot corresponds to which source
location, and where the hidden buffer expects to be bound.

> #### Note
>
> Coverage works the same way on other targets — swap `-target spirv` for `hlsl`, `metal`,
> `cuda`, or `cpp`. Two targets are the exception: WGSL and LLVM-emitted CPU are skipped
> with warning E45102 (see Troubleshooting below).

## Reading the manifest

Open the sidecar (or pretty-print it with `python3 -m json.tool`). The important parts:

```json
{
    "format": "slang-coverage",
    "version": 2,
    "counter_count": 8,
    "buffer": {
        "name": "__slang_coverage",
        "element_type": "uint64",
        "element_stride": 8,
        "space": 1,
        "binding": 0
    },
    "entries": [
        {
            "kind": "line",
            "counter": 0,
            "mode": "count",
            "file": "hello-coverage.slang",
            "line": 7,
            "start_column": 5
        },
        ...
    ]
}
```

Three things to take away:

1. **`counter_count` tells you how much storage to allocate**: 8 slots here, each
   `element_stride` bytes wide (8 bytes — `uint64` is the default counter width). Your host
   allocates a zero-initialized buffer of `counter_count * element_stride` bytes.
2. **`buffer` tells you where to bind it**. On SPIR-V that is a descriptor `(set, binding)` —
   here set 1, binding 0. Note that auto-allocation placed it in a _fresh_ descriptor set
   after your shader's own sets, so enabling coverage can add one descriptor set to your
   pipeline layout (see Troubleshooting).
3. **`entries` map counters back to source**: counter slot 0 counts executions of line 7
   (the `if (gain > 1.0)` statement), and so on. This attribution is what turns raw numbers
   into a report.

## Running for real: dispatching the precompiled kernel

To get real counter values, a host must do the three things the manifest describes: bind
storage for the hidden buffer, dispatch, and read the counters back. The buffer is
deliberately _invisible to ordinary reflection_ — reflection-driven binding code will not see
it; the manifest is how a host discovers it.

A dispatch you can run on any machine — no GPU, no graphics API — comes from compiling the
same shader once more, this time to a directly callable CPU shared library:

```bash
slangc hello-coverage.slang -target shader-sharedlib -stage compute -entry computeMain \
    -trace-coverage -o hello-coverage-kernel.so     # use .dll on Windows
```

Same flag, different target: `slangc` drives your system C++ compiler and produces a library
that exports `computeMain`, plus its own sidecar,
`hello-coverage-kernel.so.coverage-manifest.json`. That manifest differs from the SPIR-V one
in exactly one place — the `buffer` block. The CPU target has no descriptor sets; the kernel
receives its global parameters as one in-memory payload, and the manifest instead reports the
byte offset where the coverage buffer's `(pointer, count)` pair belongs:

```json
    "buffer": {
        "name": "__slang_coverage",
        "element_type": "uint64",
        "element_stride": 8,
        "space": 0,
        "binding": 0,
        "uniform_offset": 32,
        "uniform_stride": 16
    },
```

The host program below is the whole integration: it loads the precompiled kernel, binds the
three buffers (the shader's two, plus one for coverage) by writing `(pointer, count)` pairs
into the payload, dispatches one thread group, prints the raw counter slots, and saves them
for the report step. It needs no Slang headers or library at all — the kernel is already
compiled, and the three constants near the top are the numbers you just read in the manifest
(a production host would parse them out of the JSON).

Save this as `hello-coverage-host.cpp` next to the shader:

```cpp
// hello-coverage-host.cpp: load the kernel slangc precompiled into
// hello-coverage-kernel.so, bind the hidden counter buffer where the
// sidecar manifest says, dispatch one thread group, and write the raw
// counters for the LCOV converter. Uses no Slang headers or library —
// the manifest is the whole contract.

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

// How a (RW)StructuredBuffer<T> parameter is laid out in the payload.
struct BufferView
{
    void* data;
    size_t count;
};

// Values from hello-coverage-kernel.so.coverage-manifest.json. A real
// integration parses them out of the JSON; they are inlined here to
// keep the listing dependency-free.
constexpr uint32_t kCounterCount = 8;   // "counter_count"
constexpr uint32_t kElementStride = 8;  // "buffer": "element_stride"
constexpr uint32_t kUniformOffset = 32; // "buffer": "uniform_offset"

#ifdef _WIN32
constexpr const char* kKernelPath = "hello-coverage-kernel.dll";
#else
constexpr const char* kKernelPath = "./hello-coverage-kernel.so";
#endif

int main()
{
    void* library = loadKernel(kKernelPath);
    auto computeMain = library ? (ComputeFunc)findFunc(library, "computeMain") : nullptr;
    if (!computeMain)
    {
        std::fprintf(stderr, "cannot load %s\n", kKernelPath);
        return 1;
    }

    // Bind. On CPU, "binding" means writing a (pointer, count) pair
    // into the parameter payload. The shader's own buffers occupy the
    // leading fields in declaration order; the coverage buffer goes at
    // the manifest-reported uniform_offset. Counters must start zeroed.
    float inputs[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float outputs[4] = {};
    static_assert(kElementStride == 8, "manifest says uint64 counters");
    std::vector<uint64_t> counters(kCounterCount, 0);

    BufferView inputView = {inputs, 4};
    BufferView outputView = {outputs, 4};
    BufferView coverageView = {counters.data(), kCounterCount};

    std::vector<uint8_t> payload(kUniformOffset + sizeof(BufferView), 0);
    std::memcpy(payload.data(), &inputView, sizeof(inputView));
    std::memcpy(payload.data() + sizeof(BufferView), &outputView, sizeof(outputView));
    std::memcpy(payload.data() + kUniformOffset, &coverageView, sizeof(coverageView));

    // Dispatch one thread group, then dump the counter slots. Which
    // source line each slot counts is the manifest's "entries" job —
    // the LCOV converter does that attribution for us.
    ComputeVaryingInput varying = {{0, 0, 0}, {1, 1, 1}};
    computeMain(&varying, nullptr, payload.data());

    for (uint32_t i = 0; i < kCounterCount; ++i)
        std::printf("counter[%u] = %llu\n", i, (unsigned long long)counters[i]);

    std::ofstream("hello-coverage.counters.bin", std::ios::binary)
        .write((const char*)counters.data(), kCounterCount * kElementStride);
    return 0;
}
```

Build and run it — an ordinary C++ compile, with no SDK include or library paths:

```bash
c++ -std=c++17 hello-coverage-host.cpp -o hello-coverage-host
./hello-coverage-host
```

All four inputs are positive and the gain is always `2.0`, so every statement runs 4 times
except the two lines those inputs never reach — `value = 0.0` and the `return value`
fallthrough in `applyGain`:

```
counter[0] = 4
counter[1] = 4
counter[2] = 0
counter[3] = 4
counter[4] = 4
counter[5] = 4
counter[6] = 0
counter[7] = 4
```

Which source line each slot counts is the manifest's `entries` job — slot 2 is line 9 and
slot 6 is line 19 — and the report step next attributes every slot automatically.

## From counters to a report

Feed the kernel's manifest and the raw counters to the LCOV converter:

```bash
python3 tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest hello-coverage-kernel.so.coverage-manifest.json \
    --counters hello-coverage.counters.bin --output hello-coverage.lcov
```

The resulting LCOV file is plain text:

```
TN:slang_coverage
SF:hello-coverage.slang
DA:7,4
DA:8,4
DA:9,0
DA:16,4
DA:17,4
DA:18,4
DA:19,0
DA:20,4
end_of_record
```

Each `DA:line,count` record says how often a source line executed — and the two zeros point
straight at the code your inputs never exercised. Render it as HTML with the standard LCOV
tool:

```bash
genhtml hello-coverage.lcov --output-directory coverage-html
```

Open `coverage-html/index.html` and you will see `hello-coverage.slang` with executed lines
in green and the two unexercised lines in red.

The converter treats the counters file as `counter_count` little-endian unsigned integers of
`element_stride` bytes each — which is precisely the memory a dispatch fills, on any target.
(For experiments without a counters file, `--counters-text` accepts whitespace-separated
decimal values instead.)

## Function and branch coverage

Line coverage answers "did this statement run". Two more modes sharpen the picture:

```bash
slangc hello-coverage.slang -target spirv -stage compute -entry computeMain \
    -trace-coverage -trace-function-coverage -trace-branch-coverage \
    -o hello-coverage.spv
```

The manifest now contains 14 entries of three kinds: 8 `line` entries as before, 2 `function`
entries (one per user-authored function — `applyGain` and `computeMain`), and 4 `branch`
entries — one per _branch arm_, carrying `branch_site` and arm identity so a report can say
"the `if (gain > 1.0)` condition was true 4 times and false 0 times". Branch coverage covers
`if`/`else` arms, loop-condition outcomes, and `switch` dispatch arms; expression-level
branches (`&&`, `||`, `?:`) are not instrumented. The LCOV converter turns these into
`FN:`/`FNDA:` and `BRDA:` records automatically.

The modes are independent — you can enable any subset. There is also a cheaper fourth option:
`-trace-coverage-boolean` replaces the atomic counting with a plain store of `1`, giving
hit/not-hit data with no atomic contention when you don't need exact counts.

For a precise statement of where each mode places its counters (with worked examples), see
[`docs/design/shader-coverage-counter-placement.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-counter-placement.md).

## The same on GPU targets

Everything the host program did carries over to GPU targets unchanged except one step: where
the CPU program wrote a `(pointer, count)` pair into a payload, a GPU host binds an ordinary
zero-initialized storage buffer through its graphics API — on Vulkan at the `(set, binding)`
you saw in the first manifest — and reads it back after the dispatch completes.

GPU engines also often compile shaders at runtime through the Slang C++ API rather than
shipping `slangc` output. That _in-process_ workflow gets the same information without any
sidecar file: `castAs` the entry point's `IMetadata` to `slang::ISyntheticResourceMetadata`
for the binding location and to `slang::ICoverageTracingMetadata` for counter counts and
source attribution, and call `slang_writeCoverageManifestJson` to produce the manifest JSON
for the report step.

What varies per target is only the shape of the binding location:

| Target               | Where the buffer binds                                                                  |
| -------------------- | --------------------------------------------------------------------------------------- |
| Vulkan / SPIR-V, D3D | descriptor `(space, binding)` from the manifest / metadata                              |
| Metal                | a plain `[[buffer(N)]]` index (`binding`; `space` is `-1`)                              |
| CPU, CUDA            | a `(pointer, count)` pair written into the kernel parameter payload at `uniform_offset` |

Step-by-step recipes for Vulkan, Metal, CPU, and CUDA hosts — each with an executable
in-tree reference — live in the
[host interface document](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-host-interface.md).
For complete runnable programs, see
[`examples/shader-coverage-image-pipeline`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-image-pipeline)
and
[`examples/shader-coverage-bvh-traversal`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-bvh-traversal),
which compile with coverage, dispatch on Vulkan, read counters back, and render LCOV reports.
Both use the in-process workflow described above — compile through the C++ API, discover the
buffer via the metadata interfaces — with Vulkan descriptor binding in place of the payload
`memcpy`. The offline workflow's example is this chapter itself; its files live in
[`examples/shader-coverage-tutorial`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-tutorial).

## Options you will eventually want

- **`-trace-coverage-counter-width 32`** — use `uint32` counters instead of the default
  `uint64`. Required when the runtime cannot do 64-bit shader atomics (see Troubleshooting); the
  cost is silent wraparound past 2^32 hits per slot. The chosen width is recorded in the
  manifest (`element_type` / `element_stride`) and on
  `CoverageBufferInfo::elementByteWidth` — always read it from there rather than assuming.
- **`-trace-coverage-binding <index> <space>`** — pin the buffer at an explicit location
  instead of auto-allocation, for hosts that need the slot fixed at compile time.
- **`-trace-coverage-reserved-space <space>`** — tell auto-allocation about descriptor sets
  your runtime pipeline layout owns but the shader never references, so the coverage buffer
  stays out of them. Khronos descriptor-set targets only; repeatable.
- **`-coverage-manifest-output <path>`** — write the sidecar to an explicit path instead of
  `<output>.coverage-manifest.json`, e.g. when the compiled output goes to stdout.

## Troubleshooting

Each of these is a real failure mode with a specific symptom — worth scanning before your
first integration.

1. **64-bit atomics are not available everywhere.** The default `uint64` counters require
   64-bit shader atomic support at runtime: on Vulkan, the `shaderBufferInt64Atomics`
   feature (notably absent on MoltenVK on Apple Silicon — shader module or pipeline creation
   fails); on D3D, Shader Model 6.6 (DXC rejects the generated HLSL on older profiles). The
   fix is `-trace-coverage-counter-width 32`. Metal is handled for you: MSL has no 64-bit
   atomic fetch-add at all, so the compiler automatically caps Metal counters to 32-bit
   (requesting 64 explicitly produces warning W45115).
2. **`uint32` counters wrap silently at 2^32.** They do not saturate — a counter that wraps
   reads back as a _small_ number, which can badly mislead hot-loop analysis. Use the default
   64-bit width where the runtime allows it.
3. **Enabling coverage can add a descriptor set on Vulkan.** Auto-allocation places the
   buffer in the set after your highest shader-visible (or reserved) set. If your pipeline
   layout is built purely from your own reflection data, it will be one set too short — read
   the reported `(set, binding)` and include it, or pin the location with
   `-trace-coverage-binding`.
4. **The name `__slang_coverage` is reserved.** Declaring your own global parameter with
   that name while any coverage mode is enabled fails with error E45100.
5. **WGSL and LLVM-emitted CPU targets are skipped**, with warning E45102 — the shader
   compiles uninstrumented rather than failing. If you expected counters and got none,
   check for that warning. (For Vulkan-based WebGPU workflows, compile for `-target spirv`.)
6. **Read the counter width from the metadata, not from your compile flags.** Readback code
   that hardcodes 4- or 8-byte slots breaks the moment someone changes the width (or targets
   Metal, where the cap is automatic). `element_stride` in the manifest and
   `elementByteWidth` in `CoverageBufferInfo` always tell the truth.
7. **Some entries have no source location.** Code synthesized by the compiler (generic
   specialization, autodiff, constructor synthesis) can produce counters whose entries carry
   no file/line. The LCOV converter skips them; if you consume the metadata directly, expect
   `file == nullptr` / `line == 0` and handle it.
8. **Counter slots are per-compile.** Slot `K` does not mean the same source location across
   two compiles or shader variants. Always aggregate by the source attribution in the
   manifest or metadata, never by slot index.
9. **In the C++ API, fetch the compiled code before the metadata.** When using the
   in-process workflow, an entry point compiles once and caches the artifact. If
   `getEntryPointMetadata` runs first, the cache holds a form that a later
   `getEntryPointHostCallable` cannot use, and it fails with `E_INVALIDARG`. Call
   `getEntryPointCode` / `getEntryPointHostCallable` first, then query metadata.

## Further reading

- [`tools/shader-coverage/README.md`](https://github.com/shader-slang/slang/blob/master/tools/shader-coverage/README.md) —
  the workflow reference: integration patterns, the target support matrix, LCOV format
  scope, and current limitations.
- [`docs/design/shader-coverage-host-interface.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-host-interface.md) —
  the binding contract and per-target host recipes.
- The two runnable examples named above, which demonstrate coverage-driven workflows on
  realistic kernels: finding unexercised code paths in an image pipeline, and exposing
  input-shape gaps in BVH traversal test data.
