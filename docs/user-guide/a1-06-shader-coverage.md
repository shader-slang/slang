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
generated metadata, producing a report, and wiring up a real dispatch — plus the pitfalls you
are most likely to hit along the way. Every step up to the real dispatch runs on any machine
with a Slang release and Python 3; no GPU is required until the end.

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

## From counters to a report — no GPU required

After a real dispatch you would read the counter buffer back from the GPU. To see the
reporting pipeline work right now, we can feed the converter hand-written counter values with
`--counters-text` instead. Imagine a dispatch of one thread group (4 threads) where every
input is positive: every statement runs 4 times, except `value = 0.0` (inputs were never
negative) and the `return value` fallthrough in `applyGain` (the gain is always 2.0).

```bash
echo "4 4 0 4 4 4 0 4" | python3 tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest hello-coverage.spv.coverage-manifest.json \
    --counters-text - --output hello-coverage.lcov
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

With a real dispatch, the only difference is where the numbers come from: you pass the raw
readback bytes with `--counters` instead of `--counters-text` (each slot is a little-endian
unsigned integer of `element_stride` bytes).

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

## Running for real: binding the counter buffer

To get real numbers, your application must bind storage for the hidden buffer, dispatch, and
read the counters back. The buffer is deliberately _invisible to ordinary reflection_ — your
reflection-driven binding code will not see it. Instead, hosts discover it through one of two
equivalent channels:

- **In-process (C++ API)**: query the compiled artifact's `IMetadata` and `castAs` two
  interfaces — `slang::ISyntheticResourceMetadata` for the binding location and
  `slang::ICoverageTracingMetadata` for counter counts and source attribution. No sidecar
  file is involved.
- **Offline (slangc + sidecar)**: the `.coverage-manifest.json` you have already seen carries
  the same information for hosts that dispatch precompiled shaders, possibly on machines
  without Slang installed.

The binding step itself is ordinary resource binding in your API of choice; what varies per
target is the location's shape:

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

## Where to go next

- [`tools/shader-coverage/README.md`](https://github.com/shader-slang/slang/blob/master/tools/shader-coverage/README.md) —
  the workflow reference: integration patterns, the target support matrix, LCOV format
  scope, and current limitations.
- [`docs/design/shader-coverage-host-interface.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-host-interface.md) —
  the binding contract and per-target host recipes.
- The two runnable examples named above, which demonstrate coverage-driven workflows on
  realistic kernels: finding unexercised code paths in an image pipeline, and exposing
  input-shape gaps in BVH traversal test data.
