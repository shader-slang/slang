---
layout: user-guide
---

# Shader Execution Coverage

Slang supports gcov-style code coverage for shaders to report execution counts for GPU
and CPU kernels. The compiler instruments each executable statement to increment a
counter at runtime; after a dispatch, read the counters back and convert them into an LCOV
report for LCOV compatible reporting tools such as `genhtml`, Codecov, or VS Code Coverage Gutters.

This chapter walks through the pipeline: compile with coverage, read the generated manifest,
dispatch from a small C++ host program, and produce a report. All steps use the offline
workflow (`slangc` plus a sidecar manifest file) and run without a GPU; the dispatched kernel
is compiled for Slang's CPU target. The closing section summarizes GPU targets and the
in-process C++ API. The files to execute this tutorial are in
[`examples/shader-coverage-tutorial`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-tutorial).

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

Other targets work the same way. Replace `-target spirv` with `hlsl`, `metal`, `cuda`, or
`cpp` to switch to a different target. Exceptions: WGSL and the LLVM JIT CPU path are skipped
with warning E45102 (the `cpp` and `shader-sharedlib` targets compile through a system C++
compiler and are supported).

## Manifest structure

The LCOV converter and the host application take care of the manifest. User does not need to
read or modify. This is what it contains, trimmed to the relevant fields:

```json
{
    "counter_count": 8,
    "buffer": {
        "element_type": "uint64",
        "element_stride": 8,
        "space": 1,
        "binding": 0
    },
    "entries": [
        {
            "kind": "line",
            "counter": 0,
            "file": "hello-coverage.slang",
            "line": 7
        },
        ...
    ]
}
```

- `counter_count`: number of counter slots. Allocate a zero-initialized buffer of
  `counter_count * element_stride` bytes.
- `buffer`: where to bind it. On SPIR-V, a descriptor `(set, binding)`; here set 1,
  binding 0. Auto-allocation places the buffer in a new descriptor set after the shader's
  own sets, so enabling coverage can add one descriptor set to the pipeline layout (see
  [Current limitations](https://github.com/shader-slang/slang/blob/master/tools/shader-coverage/README.md#current-limitations)).
- `entries`: counter-to-source mapping. Slot 0 counts executions of line 7
  (`if (gain > 1.0)`), and so on.

## Dispatching the precompiled kernel

A host must bind storage for the counter buffer, dispatch, and read the counters back. The
buffer is not visible to ordinary reflection and the manifest is how a host finds it.

To dispatch without a GPU, compile the same shader to a CPU shared library:

```bash
slangc hello-coverage.slang -target shader-sharedlib -stage compute -entry computeMain \
    -trace-coverage -o hello-coverage-kernel.so     # use .dll on Windows
```

`slangc` invokes the system C++ compiler and produces a library that exports `computeMain`,
plus a sidecar manifest. The only difference from the SPIR-V manifest is the `buffer` block:
the CPU target has no descriptor sets, so the manifest reports a byte offset into the
kernel's parameter payload instead (`space` and `binding` remain only as placeholders):

```json
    "buffer": {
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
thread group, prints the computed outputs and the counter slots, and writes the coverage counters to
a file. It is about 150 lines and uses no Slang headers or library. The manifest is its
only connection to Slang. Three hardcoded constants in the host code carry the manifest values
shown above — `kCounterCount` from `counter_count`, `kElementStride` from `buffer.element_stride`, and
`kUniformOffset` from `buffer.uniform_offset`. Hardcoding keeps the example simple and free of a
JSON-parser dependency. A real host reads these from the manifest at run time.

The binding: `BufferView` is the 16-byte `{ void* data; size_t count; }` layout of a
`(RW)StructuredBuffer` parameter on the CPU target. The shader's own buffers occupy the
payload's leading fields in declaration order. The `coverageEnabled` block is everything
coverage adds: zero-initialized counter storage, bound at `uniform_offset`:

```cpp
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
```

The rest of the program loads the library (`dlopen`/`LoadLibrary`), calls `computeMain` with
a one-thread-group dispatch range, prints the outputs and the counter slots, and writes
`hello-coverage.counters.bin`. Run it with `--no-coverage` to skip the coverage additions —
it then works as a plain CPU shared-library dispatch, including against a kernel compiled
without `-trace-coverage`.

Copy the program from
[`examples/shader-coverage-tutorial`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-tutorial),
build, and run:

```bash
c++ -std=c++17 hello-coverage-host.cpp -o hello-coverage-host -ldl
./hello-coverage-host
```

Alternatively, use `run-tutorial.sh` / `run-tutorial.ps1` from the tutorial directory
to execute every tutorial step.

All four inputs are positive and the gain is 2.0, so every statement runs 4 times except
`value = 0.0` and the `return value` fallthrough in `applyGain`:

```
output[0] = 2
output[1] = 4
output[2] = 6
output[3] = 8
counter[0] = 4
counter[1] = 4
counter[2] = 0
counter[3] = 4
counter[4] = 4
counter[5] = 4
counter[6] = 0
counter[7] = 4
```

The manifest's `entries` array maps slots to lines: slot 2 is line 9 (the `return value`
fallthrough), slot 6 is line 19 (`value = 0.0`). The report step below does this
attribution automatically.

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
lines in red. `genhtml` ships with the lcov package on Linux and macOS but has no common
Windows distribution. Where it is unavailable, the repository's own renderer (the same tool
Slang's CI uses) produces an equivalent report anywhere Python runs:

```bash
python3 tools/coverage-html/slang-coverage-html.py hello-coverage.lcov --output-dir coverage-html
```

Any other LCOV consumer works as well — for example, the VS Code Coverage Gutters extension
reads `hello-coverage.lcov` directly.

The counters file format is `counter_count` little-endian unsigned integers of
`element_stride` bytes each — the raw buffer contents, on any target. `--counters-text`
accepts whitespace-separated decimal values instead.

## Further reading

Line coverage is one of three modes. `-trace-function-coverage` adds one entry per
function. `-trace-branch-coverage` adds one entry per branch arm — `if`/`else` arms,
loop-condition outcomes, and `switch` arms; `&&`, `||`, and `?:` are not instrumented — so a
report can give true/false counts per condition. The modes are independent. The converter
emits `FN:`/`FNDA:` and `BRDA:` records for them. `-trace-coverage-boolean` replaces atomic
counting with a plain store of 1 in whichever modes are enabled; it enables no mode by
itself. Use it when hit/not-hit is enough.

On GPU targets the workflow is the same; only the binding location changes: a descriptor
`(set, binding)` on Vulkan/D3D, a `[[buffer(N)]]` index on Metal, a payload offset on
CPU/CUDA. Bind a zero-initialized storage buffer there and read it back after the dispatch.
Hosts that compile shaders at runtime through the C++ API read the same information from the
entry point metadata (`slang::ISyntheticResourceMetadata`: binding location;
`slang::ICoverageTracingMetadata`: counters and source attribution) instead of a sidecar
file.

For details:

- [slangc command line reference](https://github.com/shader-slang/slang/blob/master/docs/command-line-slangc-reference.md#trace-coverage) —
  the `-trace-coverage*` options: coverage modes, counter width, explicit binding, reserved
  descriptor sets, manifest output path.
- [`tools/shader-coverage/README.md`](https://github.com/shader-slang/slang/blob/master/tools/shader-coverage/README.md) —
  workflow reference: integration patterns, option usage examples, target support matrix,
  LCOV format scope, current limitations.
- [`docs/design/shader-coverage-host-interface.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-host-interface.md) —
  binding contract and per-target host recipes.
- [`docs/design/shader-coverage-counter-placement.md`](https://github.com/shader-slang/slang/blob/master/docs/design/shader-coverage-counter-placement.md) —
  where each mode places its counters, with worked examples.
- [`examples/shader-coverage-image-pipeline`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-image-pipeline) —
  Vulkan, in-process API, auto-allocated binding: the compiler picks the coverage slot and
  the host reads it back from the metadata after compilation. Multi-stage image kernels
  (denoise, tone map, gamma) with many-armed switches; a smoke-vs-full run shows branch and
  function coverage catching switch arms that line coverage alone marks covered. Also
  demonstrates count vs boolean recording modes and counter-width selection.
- [`examples/shader-coverage-bvh-traversal`](https://github.com/shader-slang/slang/tree/master/examples/shader-coverage-bvh-traversal) —
  Vulkan, in-process API, explicit binding: the host pins the coverage slot up front with
  `TraceCoverageBinding` so the pipeline layout is fixed before compilation. BVH ray
  traversal where branch coverage surfaces input-shape gaps in the test scene: degenerate
  triangles, the traversal-stack-overflow fallback, and material-dispatch arms the default
  mesh never fires.
