# shader-coverage-image-pipeline

Multi-stage GPU image processing pipeline (bilateral denoise → tone
mapping → gamma encoding) that exercises Slang's shader coverage
instrumentation across recognizable kernel shapes. Demonstrates how
**branch and function coverage surface unexercised code paths that
line coverage alone marks "covered."**

## Coverage scenarios

The kernel chain has several many-armed switches whose default test
inputs only hit one arm:

- `applyTonemap(op)` — 4-way switch over Reinhard / ACES / Hable /
  Uncharted2 operators
- `sampleWithBoundary(mode)` — 4-way switch over Clamp / Wrap /
  Reflect / Black boundary handling
- `applyGamma(mode)` — 3-way switch over sRGB / Linear / Rec.709
- Bilateral filter fast-path vs general path (radius-dependent)

A **smoke** run dispatches one operator/boundary/gamma combination;
a **full** run sweeps the full 4×4×3 = 48 configuration matrix.
The coverage delta between the two runs is the demo's headline.

Both modes default to **320x180 pixels** to keep execution counting practical.
`--width=1920 --height=1080` selects **1920x1080** for performance experiments
without changing the 48-configuration matrix. The large counting sweep can
exceed process timeouts even when individual GPU submissions are tiled.

## Workload and dispatch sizing

Set `--width=N` and `--height=N` to choose the image dimensions independently
(defaults: 320 and 180; each must be 1–65535, with at most INT32_MAX pixels
total). For example, `--width=640 --height=360` selects a 640x360 image.

`--tile-rows=N` controls rows per submission, not image size. It must be 0
(whole image) or a multiple of the shader's 8-row thread-group height, so
tiles do not overlap. Full mode defaults to 128 rows; smoke mode defaults
to 0. A tile taller than the image uses one submission, and the final tile
is clipped to the remaining rows. Arbitrary image dimensions are supported.
Increasing width also increases work per tile. Smaller tiles help with GPU
watchdog limits but add submission overhead; reduce image dimensions to
reduce total runtime. Very small images may exercise fewer coverage paths.

## Run

Run these commands from the repository root. When launching elsewhere, pass
`--demo-dir` with the shader directory.

```bash
./build/Release/bin/shader-coverage-image-pipeline --mode=smoke
./build/Release/bin/shader-coverage-image-pipeline --mode=full

# Compile-time disable coverage instrumentation (baseline for overhead
# measurement):
./build/Release/bin/shader-coverage-image-pipeline --mode=full --no-coverage

# Use a smaller workload with 32-row tiles:
./build/Release/bin/shader-coverage-image-pipeline --mode=full --width=160 --height=90 --tile-rows=32

# Restore the original benchmark size:
./build/Release/bin/shader-coverage-image-pipeline --mode=full --width=1920 --height=1080 --no-coverage

# Tune the tile height (full mode already tiles into 128-row bands by
# default to avoid GPU watchdog resets under count mode on the hot
# bilateral filter), or force whole-image dispatch:
./build/Release/bin/shader-coverage-image-pipeline --mode=full --tile-rows=64
./build/Release/bin/shader-coverage-image-pipeline --mode=full --tile-rows=0

# Write the coverage artifacts somewhere other than the demo's source
# directory (the default). `--output-dir` creates the directory if
# needed:
./build/Release/bin/shader-coverage-image-pipeline --mode=full --output-dir=./out

# Point the demo at a different copy of the `.slang` files (useful
# when the binary has been moved away from the source tree):
./build/Release/bin/shader-coverage-image-pipeline --mode=full \
    --demo-dir=/path/to/shader-coverage-image-pipeline
```

Each coverage run writes alongside the executable:

- `<mode>.coverage-mapping.json` — counter ↔ source attribution
- `<mode>.lcov` — line-only LCOV (quick view)
- `<mode>.counters.bin` — raw counter buffer; feed to
  `tools/shader-coverage/slang-coverage-to-lcov.py` for a rich LCOV
  with branch+function records

The wall-clock time is printed for the dispatch loop so you can
measure the coverage instrumentation overhead by comparing
`--coverage` vs `--no-coverage` runs at the same `--mode=`.

## Counter modes

`--coverage-mode=count` (default) records exact execution counts via
atomic add. The bilateral filter's inner loop contends heavily on a
few counter slots, so on this workload count mode is the dominant
cost of instrumentation. Use `--tile-rows=N` to cap per-submission
GPU time if TDR is a concern (see **Tiled dispatch** below).

`--coverage-mode=boolean` records covered-or-not instead — each slot
is written non-atomically with `1` the first time it executes.
Concurrent same-value stores are a benign race. This removes all
atomic contention, so the full sweep runs roughly an order of
magnitude faster while still producing the same LCOV report (any
positive count is "covered", which is exactly what boolean
preserves). Pick `count` when you need exact execution counts; pick
`boolean` when you just want to know which paths fired.

```bash
# Same coverage map, much faster on the full sweep:
./build/Release/bin/shader-coverage-image-pipeline --mode=full --coverage-mode=boolean
```

## Tiled dispatch

`--tile-rows=N` splits each config dispatch into horizontal bands of N
rows. The shader recovers the real pixel row as `tid.y + tileOriginY`.
Full mode defaults to 128-row bands; smoke mode and `--tile-rows=0`
dispatch the whole image per config,
a single submission with `tileOriginY = 0`.

The bilateral filter's inner loop creates heavy atomic contention on a
handful of counter slots — millions of threads all increment the same
few counters. Coverage count mode can be substantially slower than
uninstrumented code, and a whole-image dispatch can run long enough to
trip the GPU watchdog timeout (Windows TDR / `VK_ERROR_DEVICE_LOST`)
on the 48-config `--mode=full` sweep.

Tiling caps per-submission GPU time without affecting coverage results:
bands partition the image, every pixel is processed exactly once, and
counters accumulate across all bands and configs.

The default `--tile-rows=128` limits per-submission work; reduce it if
you still observe TDR. Tiling does not reduce total work or guarantee that a
large run finishes within a process timeout. Alternatives:

- `--coverage-mode=boolean`: removes all atomic contention (non-atomic
  stores of `1`), but does not guarantee a timeout-free large run.
- `--no-coverage`: baseline timing with no instrumentation overhead.

## End-to-end wrapper

`run_coverage.py` forwards the workload and batch sizing options to the runner. It is a convenience wrapper that
compiles, dispatches, converts the raw counters to a rich LCOV, renders
an HTML report and opens it — all in one command:

```bash
# Smoke run with HTML report opened automatically:
python3 run_coverage.py --mode=smoke

# Exhaustive sweep, boolean mode, custom output dir:
python3 run_coverage.py --mode=full --coverage-mode=boolean --output-dir=./out
```

All flags accepted by the demo binary are forwarded verbatim; the
script adds `--slang-root` (default: auto-discovered from its own path)
to locate the renderer and converter.

## HTML report

The demo writes a full LCOV directly, so only the renderer is needed:

```bash
python3 path/to/slang/tools/coverage-html/slang-coverage-html.py \
    full.lcov \
    --output-dir full-html \
    --title "image-pipeline full"
```

Open `full-html/index.html` and look at `tonemap.slang.*.html`
— each `case TonemapOperator::*` line shows a coloured `(1/1)` or
`(0/1)` branch indicator. The smoke vs full diff turns three
of the four operator branches from red to green.

## Architecture

### Coverage instrumentation pipeline

The host in [`main.cpp`](main.cpp) runs these steps:

| Stage     | Description                                                                                                                   | Key API                                                          |
| --------- | ----------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------- |
| Compile   | Enable line, function and branch coverage. Let Slang select a free coverage binding.                                          | `IComponentType::link`, `getEntryPointCode`                      |
| Describe  | Read synthetic-resource metadata and register it before RHI creates the program.                                              | `getCoverageResourceDesc`, `ShaderProgramSyntheticResourcesDesc` |
| Bind      | Bind application buffers by reflected name; allocate zeroed coverage storage and bind its opaque resource ID.                 | `ShaderCursor`, `bindSyntheticResource`                          |
| Dispatch  | Upload parameters and dispatch each tile or batch, waiting before reusing parameters. Counters accumulate across submissions. | `ICommandEncoder`, `IComputePassEncoder`, `ICommandQueue`        |
| Read back | Read the effective-width counter bytes and attribute slots through `entry.counterIndex`.                                      | `IDevice::readBuffer`, `ICoverageTracingMetadata`                |

### slang-rhi host

This example uses slang-rhi on Vulkan. `compileShader()` retains the linked
Slang component and its metadata. In `exampleMain()`, the host chains a
`ShaderProgramSyntheticResourcesDesc` through `ShaderProgramDesc.next` before
`createShaderProgram()`, then calls `bindSyntheticResource()` on the root object.
Ordinary reflection still does not contain `__slang_coverage`; the extension
supplies its compiler-generated binding information.

[`coverage-rhi.h`](../shader-coverage-common/coverage-rhi.h) only translates the
single coverage resource and creates storage buffers. Program creation, binding,
dispatch and readback stay visible in `main.cpp`. With `--no-coverage`, the host
omits the extension and coverage buffer entirely.

For integration **without slang-rhi**, see
[`shader-coverage-backends`](../shader-coverage-backends/), which retains direct
CPU, CUDA, Vulkan and Metal binding. RHI synthetic bindings currently support
Vulkan and CUDA; these two larger workflow examples select Vulkan explicitly.

### Metadata-derived binding

This demo uses the **metadata-derived** binding approach: it does not
tell the compiler where to place `__slang_coverage`; the compiler
auto-assigns a free descriptor slot and records the choice. The host
discovers that slot after compilation by querying
`ISyntheticResourceMetadata`:

```cpp
auto* synth = (slang::ISyntheticResourceMetadata*)
    metadata->castAs(slang::ISyntheticResourceMetadata::getTypeGuid());
slang::SyntheticResourceInfo info = {};
synth->getResourceInfo(0, &info);
// info.space, info.binding now hold the assigned (set, binding)
```

The advantage over hardcoding is that the host never needs to predict
or reserve a slot — if the shader's own resource count changes, the
compiler picks a different free slot automatically and the host follows
without a source change.

Compare the BVH-traversal demo (`shader-coverage-bvh-traversal`) which
demonstrates the **explicit placement** approach instead.

### Counter readback and LCOV

The readback section in [`main.cpp`](main.cpp) separates three host-side steps:

```cpp
// 1. Read back after queue->waitOnHost() has completed.
std::vector<uint8_t> rawBytes(size_t(counterCount) * counterByteWidth);
ComPtr<slang::IBlob> readback;
checkSlang(device->readBuffer(coverageBuf, 0, rawBytes.size(), readback.writeRef()),
           "read coverage buffer");
std::memcpy(rawBytes.data(), readback->getBufferPointer(), rawBytes.size());

// 2. Decode the effective 32- or 64-bit slots into uint64_t values.
auto hits = decodeCoverageCounters(rawBytes.data(), rawBytes.size(), counterByteWidth);

// 3. Attribute those slots to source locations and write the LCOV report.
writeLcov(shader.coverageMetadata, hits, outDir / (mode + ".lcov"),
          ("image-pipeline-" + mode).c_str());
```

[`decodeCoverageCounters()`](../shader-coverage-common/coverage-counters.h) is an
example helper, not a Slang API. It decodes little-endian bytes using the effective
`CoverageBufferInfo::elementByteWidth`, without requiring an aligned pointer.
It does not perform GPU synchronization, readback, or source attribution.
`writeLcov()` iterates `getEntryCount()` metadata entries and looks up each entry's
`hits[entry.counterIndex]`; multiple entries can share one counter slot, so an
entry index is not a counter index. Keep `rawBytes` for `writeCountersBinary()`:
writing the widened `hits` instead would change the binary layout for 32-bit counters.

After dispatch the host downloads the raw counter buffer and uses
`ICoverageTracingMetadata::getEntryInfo()` to convert it directly to a
**full LCOV** file — with line (`DA`), function (`FN`/`FNDA`), and branch
(`BRDA`) records — without any external tool:

```
GPU counter buffer (uint32/uint64 × N slots)
    │  `IDevice::readBuffer()`
    ▼
host: std::vector<uint8_t> rawBytes   ← retained for counters.bin
    │  decodeCoverageCounters()
    ▼
host: std::vector<uint64_t> hits   ← one entry per counter slot
    │  ICoverageTracingMetadata::getEntryInfo(i, &entry)
    │    entry.kind  → DA / FN+FNDA / BRDA record
    │    entry.file, entry.line, entry.functionName, entry.branchSiteID …
    │    hits[entry.counterIndex] → count
    ▼
<mode>.lcov  (full: DA + FN/FNDA + BRDA)
    │  slang-coverage-html.py
    ▼
HTML report
```

This is the **in-process conversion path**: the counters and the source
attribution are both available in the same process, so the full LCOV can
be produced with one loop over the metadata entries. No intermediate
manifest file or external converter is needed for the HTML step.

Compare the BVH-traversal demo which uses the **out-of-process converter**
path instead: it writes the manifest JSON and raw counter binary separately,
then calls `slang-coverage-to-lcov.py` to produce the full LCOV.

## Build dependencies

- Slang compiler library (linked from this repository's build).
- Build with `SLANG_ENABLE_EXAMPLES=ON` and `SLANG_ENABLE_SLANG_RHI=ON`.
- A Vulkan loader and compatible GPU driver at runtime (MoltenVK on macOS).
  RHI handles loading Vulkan; the example no longer links directly to its loader.
- The standard example target writes the executable to `build/Release/bin/`
  with the `release` preset. `run_coverage.py` discovers that location.
- Counters default to 32 bits for MoltenVK. `--counter-width=64` requires
  `AtomicInt64` support; device creation fails if no suitable device is available.

### Application output and diagnostics

Pass `--output-file=results.bin` to save the final application output, including
with `--no-coverage`. The parent directory must exist. The file contains four
native-endian float32 components per pixel/ray in row-major order. Image full mode
saves the result of the final configuration. Readback occurs after the timed loop.
This option initializes the output to NaNs so the regression detects missing writes.

RHI messages and failing SlangResult values are printed to stderr. If the backend
reports device loss during a count-mode dispatch, reduce the tile/batch size or
use `--coverage-mode=boolean` to reduce atomic contention.

### Checking the RHI integration

After building both workflow examples, run this from the repository root:

```bash
python3 examples/shader-coverage-common/check-rhi-examples.py --bin-dir build/Release/bin
```

The check runs small smoke/full workloads, compares tiled/batched counters with
single-dispatch counters, checks count/boolean hit locations and LCOV export,
and compares finite application output across recording modes, batching and
coverage-disabled baselines (with a small floating-point tolerance). On a GPU with 64-bit buffer atomics,
repeat with `--counter-width=64`. Windows can use `python` in place of `python3`;
pass the actual executable directory with `--bin-dir` for a custom build tree.
