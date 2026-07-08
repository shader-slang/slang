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
and producing a report — plus the pitfalls you are most likely to hit along the way. Every
step, including the real dispatch, runs on any machine with a Slang release, a C++ compiler,
and Python 3: the host program uses Slang's CPU target, and a later section shows what changes
on GPU targets.

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
> with warning E45102 (see Pitfalls below).

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
   pipeline layout (see Pitfalls).
3. **`entries` map counters back to source**: counter slot 0 counts executions of line 7
   (the `if (gain > 1.0)` statement), and so on. This attribution is what turns raw numbers
   into a report.

## Running for real: a minimal host program

To get real counter values, an application must do three things the manifest describes: bind
storage for the hidden buffer, dispatch, and read the counters back. The buffer is
deliberately _invisible to ordinary reflection_ — your reflection-driven binding code will not
see it. In-process hosts discover it by querying the compiled artifact's `IMetadata` and
`castAs`-ing two interfaces: `slang::ISyntheticResourceMetadata` answers _where the buffer
binds_, and `slang::ICoverageTracingMetadata` answers _what the counters mean_.

The program below does the whole round trip using Slang's CPU target, so it runs on any
machine with no GPU or graphics API involved. The CPU target compiles the kernel into a
directly callable function, and "binding" a buffer means writing a `(pointer, count)` pair
into a parameter payload at a byte offset the metadata reports — the same discovery contract
as on GPU targets, with `memcpy` standing in for descriptor sets.

Save this as `hello-coverage-host.cpp` next to `hello-coverage.slang`:

```cpp
// hello-coverage-host.cpp: compile hello-coverage.slang for the CPU
// with coverage enabled, bind the hidden counter buffer, dispatch one
// thread group, and print per-line execution counts.

#include "slang-com-ptr.h"
#include "slang.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

using Slang::ComPtr;

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

static void check(SlangResult result, const char* what)
{
    if (SLANG_FAILED(result))
    {
        std::cerr << what << " failed\n";
        std::exit(1);
    }
}

int main()
{
    // 1. Compile hello-coverage.slang for the CPU with line coverage.
    ComPtr<slang::IGlobalSession> globalSession;
    check(slang::createGlobalSession(globalSession.writeRef()), "createGlobalSession");

    // Coverage skips the LLVM-emitted CPU path (warning E45102), so pin
    // a real C++ compiler for the host-callable route.
    const SlangPassThrough cppCompilers[] = {
        SLANG_PASS_THROUGH_VISUAL_STUDIO,
        SLANG_PASS_THROUGH_GCC,
        SLANG_PASS_THROUGH_CLANG,
    };
    SlangPassThrough cppCompiler = SLANG_PASS_THROUGH_NONE;
    for (auto candidate : cppCompilers)
        if (SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(candidate)))
        {
            cppCompiler = candidate;
            break;
        }
    if (cppCompiler == SLANG_PASS_THROUGH_NONE)
    {
        std::cerr << "no C++ compiler found for the CPU target\n";
        return 1;
    }
    globalSession->setDefaultDownstreamCompiler(SLANG_SOURCE_LANGUAGE_CPP, cppCompiler);
    globalSession->setDownstreamCompilerForTransition(
        SLANG_CPP_SOURCE,
        SLANG_SHADER_HOST_CALLABLE,
        cppCompiler);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SHADER_HOST_CALLABLE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry coverageOption = {};
    coverageOption.name = slang::CompilerOptionName::TraceCoverage;
    coverageOption.value.kind = slang::CompilerOptionValueKind::Int;
    coverageOption.value.intValue0 = 1;

    const char* searchPaths[] = {"."};
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;
    sessionDesc.compilerOptionEntries = &coverageOption;
    sessionDesc.compilerOptionEntryCount = 1;

    ComPtr<slang::ISession> session;
    check(globalSession->createSession(sessionDesc, session.writeRef()), "createSession");

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModule("hello-coverage", diagnostics.writeRef());
    if (!module)
    {
        std::cerr << "run this from the directory containing hello-coverage.slang\n";
        return 1;
    }

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    check(
        session->createCompositeComponentType(components, 2, program.writeRef(), nullptr),
        "composite");
    ComPtr<slang::IComponentType> linked;
    check(program->link(linked.writeRef(), nullptr), "link");

    // 2. Fetch the compiled kernel. Do this BEFORE the metadata query:
    // the entry point compiles once and caches its result, and the
    // cache must hold the executable form.
    ComPtr<ISlangSharedLibrary> library;
    check(
        linked->getEntryPointHostCallable(0, 0, library.writeRef(), diagnostics.writeRef()),
        "getEntryPointHostCallable");
    auto computeMain = (ComputeFunc)library->findFuncByName("computeMain");

    // 3. Discover the hidden coverage buffer. ISyntheticResourceMetadata
    // reports where it lives; ICoverageTracingMetadata reports what the
    // counters mean.
    ComPtr<slang::IMetadata> metadata;
    check(linked->getEntryPointMetadata(0, 0, metadata.writeRef(), nullptr), "metadata");
    auto* coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    auto* synthetic = (slang::ISyntheticResourceMetadata*)metadata->castAs(
        slang::ISyntheticResourceMetadata::getTypeGuid());

    slang::SyntheticResourceInfo bufferLocation = {};
    check(synthetic->getResourceInfo(0, &bufferLocation), "getResourceInfo");

    slang::CoverageBufferInfo bufferInfo = {};
    check(coverage->getBufferInfo(&bufferInfo), "getBufferInfo");
    const uint32_t counterCount = coverage->getCounterCount();

    // 4. Allocate and bind. On CPU, "binding" means writing a
    // (pointer, count) pair into the parameter payload at the byte
    // offset the metadata reports. The user's two buffers occupy the
    // leading fields in declaration order; the coverage buffer goes at
    // `uniformOffset`. Counter storage is sized from the *reported*
    // element width — never assume it.
    float inputs[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float outputs[4] = {};
    std::vector<uint8_t> counters(size_t(counterCount) * bufferInfo.elementByteWidth, 0);

    BufferView inputView = {inputs, 4};
    BufferView outputView = {outputs, 4};
    BufferView coverageView = {counters.data(), counterCount};

    std::vector<uint8_t> payload(bufferLocation.uniformOffset + sizeof(BufferView), 0);
    std::memcpy(payload.data(), &inputView, sizeof(inputView));
    std::memcpy(payload.data() + sizeof(BufferView), &outputView, sizeof(outputView));
    std::memcpy(payload.data() + bufferLocation.uniformOffset, &coverageView, sizeof(coverageView));

    // 5. Dispatch one thread group and attribute the counters.
    ComputeVaryingInput varying = {{0, 0, 0}, {1, 1, 1}};
    computeMain(&varying, nullptr, payload.data());

    std::map<uint32_t, uint64_t> hitsByLine;
    for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        check(coverage->getEntryInfo(i, &entry), "getEntryInfo");
        if (entry.kind != slang::CoverageEntryKind::Line || !entry.file)
            continue;
        uint64_t hits = bufferInfo.elementByteWidth == 8
                            ? ((const uint64_t*)counters.data())[entry.counterIndex]
                            : ((const uint32_t*)counters.data())[entry.counterIndex];
        hitsByLine[entry.line] += hits;
    }
    for (auto& [line, hits] : hitsByLine)
        std::cout << "line " << line << ": " << hits << "\n";

    // 6. Write the manifest + counters for the LCOV converter.
    ComPtr<ISlangBlob> manifest;
    check(slang_writeCoverageManifestJson(coverage, manifest.writeRef()), "manifest");
    std::ofstream("hello-coverage.coverage-manifest.json", std::ios::binary)
        .write((const char*)manifest->getBufferPointer(), manifest->getBufferSize());
    std::ofstream("hello-coverage.counters.bin", std::ios::binary)
        .write((const char*)counters.data(), counters.size());
    return 0;
}
```

Build it against the `include/` headers and `slang` library of a Slang release (or your own
build):

```bash
# Linux / macOS
c++ -std=c++17 hello-coverage-host.cpp -I$SLANG/include \
    -L$SLANG/lib -lslang -Wl,-rpath,$SLANG/lib -o hello-coverage-host

# Windows (from a Visual Studio developer prompt)
cl /std:c++17 /EHsc /I%SLANG%\include hello-coverage-host.cpp \
    /link /LIBPATH:%SLANG%\lib slang.lib
```

Run it. All four inputs are positive and the gain is always `2.0`, so every statement runs
4 times except the two lines those inputs never reach — `value = 0.0` and the `return value`
fallthrough in `applyGain`:

```
$ ./hello-coverage-host
line 7: 4
line 8: 4
line 9: 0
line 16: 4
line 17: 4
line 18: 4
line 19: 0
line 20: 4
```

It also wrote two files — `hello-coverage.coverage-manifest.json` (a manifest like the one
you read earlier, obtained in-process via `slang_writeCoverageManifestJson`; on the CPU
target its `buffer` block reports the payload's `uniform_offset` instead of a meaningful
descriptor location) and `hello-coverage.counters.bin` (the raw counter memory) — which is
exactly what the reporting step consumes.

Two details in the program deserve a second look, because both bite in real integrations:

- **Step 2 runs before step 3 on purpose.** An entry point compiles once and caches the
  resulting artifact; querying metadata first would cache a form that cannot later be
  turned into a callable library, and `getEntryPointHostCallable` would fail.
- **The counter allocation reads `elementByteWidth` from the metadata** instead of assuming
  8 bytes — the same rule as Pitfall 6 below.

## From counters to a report

Feed the manifest and the raw counters to the LCOV converter:

```bash
python3 tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest hello-coverage.coverage-manifest.json \
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
the CPU program `memcpy`-ed a `(pointer, count)` pair into a payload, a GPU host binds an
ordinary zero-initialized storage buffer through its graphics API, and reads it back after
the dispatch completes. Discovery also comes through the same two equivalent channels:
the in-process metadata interfaces the program used, or — for hosts that dispatch precompiled
shaders, possibly on machines without Slang installed — the `.coverage-manifest.json` sidecar
you read earlier.

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

## Options you will eventually want

- **`-trace-coverage-counter-width 32`** — use `uint32` counters instead of the default
  `uint64`. Required when the runtime cannot do 64-bit shader atomics (see Pitfalls); the
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

## Pitfalls

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
9. **In the C++ API, fetch the compiled code before the metadata.** An entry point compiles
   once and caches the artifact. If `getEntryPointMetadata` runs first, the cache holds a
   form that a later `getEntryPointHostCallable` cannot use, and it fails with
   `E_INVALIDARG`. Call `getEntryPointCode` / `getEntryPointHostCallable` first, then query
   metadata — as the host program above does.

## Where to go next

- [`tools/shader-coverage/README.md`](https://github.com/shader-slang/slang/blob/master/tools/shader-coverage/README.md) —
  the workflow reference: integration patterns, the target support matrix, LCOV format
  scope, and current limitations.
- [`docs/design/shader-coverage-host-interface.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-host-interface.md) —
  the binding contract and per-target host recipes.
- The two runnable examples named above, which demonstrate coverage-driven workflows on
  realistic kernels: finding unexercised code paths in an image pipeline, and exposing
  input-shape gaps in BVH traversal test data.
