---
layout: user-guide
---

# Shader Execution Coverage

Slang can instrument shaders so that each executed statement increments a counter at runtime,
like gcov does for CPU code. After a dispatch, read the counters back and convert them into
an LCOV report for `genhtml`, Codecov, or VS Code Coverage Gutters.

This chapter walks through the pipeline: compile with coverage, read the generated manifest,
dispatch from a small C++ host program, and produce a report. All steps use the offline
workflow (`slangc` plus a sidecar manifest file) and run without a GPU; the dispatched kernel
is compiled for Slang's CPU target. The files, together with `run-tutorial.sh` and
`run-tutorial.ps1` scripts that execute every step, are in
[`examples/shader-coverage-tutorial`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-tutorial).

Reference material:
[`tools/shader-coverage/README.md`](https://github.com/shader-slang/slang/blob/master/tools/shader-coverage/README.md)
(workflow, support matrix) and
[`docs/design/shader-coverage-host-interface.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-host-interface.md)
(host binding contract, per-target recipes).

## Compiling with coverage

Create `hello-coverage.slang`:

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

Compile with `-trace-coverage`:

```bash
slangc hello-coverage.slang -target spirv -stage compute -entry computeMain \
    -trace-coverage -o hello-coverage.spv
```

This produces two files:

```
hello-coverage.spv
hello-coverage.spv.coverage-manifest.json
```

The `.spv` contains a hidden counter buffer and an atomic increment before each executable
statement. The `.coverage-manifest.json` sidecar records which counter slot corresponds to
which source location, and where the buffer expects to be bound.

Other targets work the same way; replace `-target spirv` with `hlsl`, `metal`, `cuda`, or
`cpp`. Exceptions: WGSL and LLVM-emitted CPU are skipped with warning E45102 (see
Troubleshooting).

## Reading the manifest

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

- `counter_count`: number of counter slots. Allocate a zero-initialized buffer of
  `counter_count * element_stride` bytes.
- `buffer`: where to bind it. On SPIR-V, a descriptor `(set, binding)`; here set 1,
  binding 0. Auto-allocation places the buffer in a new descriptor set after the shader's
  own sets, so enabling coverage can add one descriptor set to your pipeline layout (see
  Troubleshooting).
- `entries`: counter-to-source mapping. Slot 0 counts executions of line 7, and so on.

## Dispatching the precompiled kernel

A host must bind storage for the counter buffer, dispatch, and read the counters back. The
buffer is not visible to ordinary reflection; the manifest is how a host finds it.

To dispatch without a GPU, compile the same shader to a CPU shared library:

```bash
slangc hello-coverage.slang -target shader-sharedlib -stage compute -entry computeMain \
    -trace-coverage -o hello-coverage-kernel.so     # use .dll on Windows
```

`slangc` invokes the system C++ compiler and produces a library that exports `computeMain`,
plus a sidecar manifest. The only difference from the SPIR-V manifest is the `buffer` block:
the CPU target has no descriptor sets, so the manifest reports a byte offset into the
kernel's parameter payload instead:

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

The host program,
[`hello-coverage-host.cpp`](https://github.com/shader-slang/slang/blob/master/examples/shader-coverage-tutorial/hello-coverage-host.cpp),
loads the kernel, binds the three buffers (the shader's two, plus coverage), dispatches one
thread group, prints the counter slots, and writes them to a file. It is about 140 lines and
uses no Slang headers or library. The constants `kCounterCount`, `kElementStride`, and
`kUniformOffset` are the manifest values above; a production host would parse them from the
JSON.

The binding: `BufferView` is the 16-byte `{ void* data; size_t count; }` layout of a
`(RW)StructuredBuffer` parameter on the CPU target. The shader's own buffers occupy the
payload's leading fields in declaration order. The coverage buffer, zero-initialized, goes
at `uniform_offset`:

```cpp
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
```

The rest of the program loads the library (`dlopen`/`LoadLibrary`), calls `computeMain` with
a one-thread-group dispatch range, prints the slots, and writes
`hello-coverage.counters.bin`.

Copy the program from
[`examples/shader-coverage-tutorial`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-tutorial),
build, and run:

```bash
c++ -std=c++17 hello-coverage-host.cpp -o hello-coverage-host
./hello-coverage-host
```

All four inputs are positive and the gain is 2.0, so every statement runs 4 times except
`value = 0.0` and the `return value` fallthrough in `applyGain`:

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

The manifest's `entries` array maps slots to lines: slot 2 is line 9, slot 6 is line 19.
The report step below does this attribution automatically.

## Generating a report

Pass the kernel's manifest and the counters to the LCOV converter:

```bash
python3 tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest hello-coverage-kernel.so.coverage-manifest.json \
    --counters hello-coverage.counters.bin --output hello-coverage.lcov
```

The LCOV output is plain text:

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

Each `DA:line,count` record gives the execution count of one source line; the two zero
lines were never exercised. Render HTML with:

```bash
genhtml hello-coverage.lcov --output-directory coverage-html
```

`coverage-html/index.html` shows the source with executed lines in green and unexecuted
lines in red.

The counters file format is `counter_count` little-endian unsigned integers of
`element_stride` bytes each — the raw buffer contents, on any target. `--counters-text`
accepts whitespace-separated decimal values instead.

## Function and branch coverage

Two more modes can be enabled independently of line coverage:

```bash
slangc hello-coverage.slang -target spirv -stage compute -entry computeMain \
    -trace-coverage -trace-function-coverage -trace-branch-coverage \
    -o hello-coverage.spv
```

The manifest then contains 14 entries: 8 `line`, 2 `function` (`applyGain` and
`computeMain`), and 4 `branch` — one per branch arm, with `branch_site` and arm identity, so
a report can show that `if (gain > 1.0)` was true 4 times and false 0 times. Branch coverage
instruments `if`/`else` arms, loop-condition outcomes, and `switch` arms; `&&`, `||`, and
`?:` are not instrumented. The converter emits `FN:`/`FNDA:` and `BRDA:` records for these.

`-trace-coverage-boolean` replaces atomic counting with a plain store of 1. Use it when
hit/not-hit is enough and atomic contention matters.

Counter placement rules are specified in
[`docs/design/shader-coverage-counter-placement.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-counter-placement.md).

## GPU targets

On a GPU target, bind an ordinary zero-initialized storage buffer at the manifest-reported
location — on Vulkan, the `(set, binding)` from the first manifest — and read it back after
the dispatch completes. Everything else works as above.

Hosts that compile shaders at runtime through the C++ API get the same information without a
sidecar file: `castAs` the entry point's `IMetadata` to `slang::ISyntheticResourceMetadata`
(binding location) and `slang::ICoverageTracingMetadata` (counter count, source
attribution). `slang_writeCoverageManifestJson` produces the manifest JSON for the report
step.

Binding location per target:

| Target               | Where the buffer binds                                                                  |
| -------------------- | --------------------------------------------------------------------------------------- |
| Vulkan / SPIR-V, D3D | descriptor `(space, binding)` from the manifest / metadata                              |
| Metal                | a plain `[[buffer(N)]]` index (`binding`; `space` is `-1`)                              |
| CPU, CUDA            | a `(pointer, count)` pair written into the kernel parameter payload at `uniform_offset` |

Per-target recipes are in the
[host interface document](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-host-interface.md).
For complete Vulkan programs using the in-process workflow, see
[`examples/shader-coverage-image-pipeline`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-image-pipeline)
and
[`examples/shader-coverage-bvh-traversal`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-bvh-traversal).

## Options

The remaining `-trace-coverage*` options — counter width, explicit binding, reserved
descriptor sets, and the manifest output path — are documented in the
[slangc command line reference](https://github.com/shader-slang/slang/blob/master/docs/command-line-slangc-reference.md#trace-coverage)
and, with usage examples, in
[`tools/shader-coverage/README.md`](https://github.com/shader-slang/slang/blob/master/tools/shader-coverage/README.md).

## Troubleshooting

1. **64-bit atomics are not available everywhere.** The default `uint64` counters require
   64-bit shader atomics at runtime: on Vulkan the `shaderBufferInt64Atomics` feature
   (absent on MoltenVK on Apple Silicon; shader module or pipeline creation fails), on D3D
   Shader Model 6.6 (DXC rejects the generated HLSL on older profiles). Use
   `-trace-coverage-counter-width 32`. Metal is capped to 32-bit automatically; requesting
   64 produces warning W45115.
2. **`uint32` counters wrap silently at 2^32.** They do not saturate; a wrapped counter
   reads back as a small number. Use the default 64-bit width where the runtime allows it.
3. **Enabling coverage can add a descriptor set on Vulkan.** Auto-allocation places the
   buffer in the set after the highest shader-visible (or reserved) set. A pipeline layout
   built only from your own reflection data will be one set too short. Read the reported
   `(set, binding)` and include it, or use `-trace-coverage-binding`.
4. **The name `__slang_coverage` is reserved.** Declaring a global parameter with that name
   while coverage is enabled fails with error E45100.
5. **WGSL and LLVM-emitted CPU targets are skipped** with warning E45102; the shader
   compiles uninstrumented. If you expected counters and got none, check for that warning.
   For Vulkan-based WebGPU, compile for `-target spirv`.
6. **Read the counter width from the metadata, not from your compile flags.** Readback code
   that hardcodes 4- or 8-byte slots breaks when the width changes or the target is Metal.
   Use `element_stride` from the manifest or `elementByteWidth` from `CoverageBufferInfo`.
7. **Some entries have no source location.** Compiler-synthesized code (generic
   specialization, autodiff, constructor synthesis) can produce entries without file/line.
   The LCOV converter skips them. When consuming the metadata directly, handle
   `file == nullptr` / `line == 0`.
8. **Counter slots are per-compile.** Slot `K` does not mean the same source location
   across two compiles or shader variants. Aggregate by the source attribution in the
   manifest or metadata, never by slot index.
9. **In the C++ API, fetch the compiled code before the metadata.** An entry point compiles
   once and caches the artifact. If `getEntryPointMetadata` runs first, a later
   `getEntryPointHostCallable` fails with `E_INVALIDARG`. Call `getEntryPointCode` /
   `getEntryPointHostCallable` first, then query metadata.

## Further reading

- [`tools/shader-coverage/README.md`](https://github.com/shader-slang/slang/blob/master/tools/shader-coverage/README.md) —
  workflow reference: integration patterns, target support matrix, LCOV format scope,
  current limitations.
- [`docs/design/shader-coverage-host-interface.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-host-interface.md) —
  binding contract and per-target host recipes.
- [`examples/shader-coverage-image-pipeline`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-image-pipeline)
  and
  [`examples/shader-coverage-bvh-traversal`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-bvh-traversal) —
  coverage-driven Vulkan workflows on realistic kernels.
