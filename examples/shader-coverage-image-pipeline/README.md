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

## Run

```bash
./shader-coverage-image-pipeline --mode=smoke
./shader-coverage-image-pipeline --mode=full

# Compile-time disable coverage instrumentation (baseline for overhead
# measurement):
./shader-coverage-image-pipeline --mode=full --no-coverage

# Tile each config into 128-row horizontal bands to avoid GPU watchdog
# resets (Windows TDR) under count mode on the hot bilateral filter:
./shader-coverage-image-pipeline --mode=full --tile-rows=128

# Write the coverage artifacts somewhere other than the demo's source
# directory (the default). `--output-dir` creates the directory if
# needed:
./shader-coverage-image-pipeline --mode=full --output-dir=./out

# Point the demo at a different copy of the `.slang` files (useful
# when the binary has been moved away from the source tree):
./shader-coverage-image-pipeline --mode=full \
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

`--coverage-mode=hit-miss` records covered-or-not instead — each slot
is written non-atomically with `1` the first time it executes.
Concurrent same-value stores are a benign race. This removes all
atomic contention, so the full sweep runs roughly an order of
magnitude faster while still producing the same LCOV report (any
positive count is "covered", which is exactly what hit-miss
preserves). Pick `count` when you need exact execution counts; pick
`hit-miss` when you just want to know which paths fired.

```bash
# Same coverage map, much faster on the full sweep:
./shader-coverage-image-pipeline --mode=full --coverage-mode=hit-miss
```

## Tiled dispatch

`--tile-rows=N` splits each config dispatch into horizontal bands of N
rows. The shader recovers the real pixel row as `tid.y + tileOriginY`.
Default (no flag) is whole-image: each config is a single submission
with `tileOriginY = 0`.

The bilateral filter's inner loop creates heavy atomic contention on a
handful of counter slots — millions of threads all increment the same
few counters. This makes coverage count mode 20–30× slower than
uninstrumented code, and a whole-image dispatch can run long enough to
trip the GPU watchdog timeout (Windows TDR / `VK_ERROR_DEVICE_LOST`)
on the 48-config `--mode=full` sweep.

Tiling caps per-submission GPU time without affecting coverage results:
bands partition the image, every pixel is processed exactly once, and
counters accumulate across all bands and configs.

`--tile-rows=128` is a safe starting point. Alternatives:

- `--coverage-mode=hit-miss`: removes all atomic contention (non-atomic
  stores of `1`), so whole-image dispatch is safe without tiling.
- `--no-coverage`: baseline timing with no instrumentation overhead.

## HTML report

The `run_coverage.py` wrapper does all steps automatically and opens
the report. To run the steps manually:

```bash
# 1. Convert raw counters to rich LCOV (adds branch + function records):
python3 path/to/slang/tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest full.coverage-manifest.json \
    --counters full.counters.bin \
    --output full.full.lcov

# 2. Render HTML:
python3 path/to/slang/tools/coverage-html/slang-coverage-html.py \
    full.full.lcov \
    --output-dir full-html \
    --title "image-pipeline full"
```

Open `full-html/index.html` and look at `tonemap.slang.*.html`
— each `case TonemapOperator::*` line shows a coloured `(1/1)` or
`(0/1)` branch indicator. The smoke vs full diff turns three
of the four operator branches from red to green.

## End-to-end wrapper

`run_coverage.py` (in this directory) is a convenience wrapper that
compiles, dispatches, converts the raw counters to a rich LCOV, renders
an HTML report and opens it — all in one command:

```bash
# Smoke run with HTML report opened automatically:
python3 run_coverage.py --mode=smoke

# Exhaustive sweep, hit/miss mode, custom output dir:
python3 run_coverage.py --mode=full --coverage-mode=hit-miss --output-dir=./out
```

All flags accepted by the demo binary are forwarded verbatim; the
script adds `--slang-root` (default: auto-discovered from its own path)
to locate the renderer and converter.

## Architecture

### Coverage instrumentation pipeline

The five stages `main.cpp` walks through for each run:

| Stage | What happens | Key API |
|---|---|---|
| **1. Compile** | `compileShader()` creates a Slang session with `-trace-coverage`, `-trace-coverage-function`, `-trace-coverage-branch`. The compiler injects `__slang_coverage` at IR time and emits SPIR-V with `OpAtomicIAdd` (count mode) or plain stores (hit-miss mode) at every instrumented point. | `slang::ISession::loadModule`, `IComponentType::link`, `getEntryPointCode` |
| **2. Discover binding** | Query `ISyntheticResourceMetadata::getResourceInfo(0)` on the post-link metadata object to read back `(space, binding)` — the slot the compiler auto-assigned for `__slang_coverage`. | `IMetadata::castAs<ISyntheticResourceMetadata>` |
| **3. Allocate & bind** | Allocate a zeroed `counterCount × counterByteWidth` storage buffer. Build a Vulkan descriptor layout with app resources on set 0 and the coverage buffer at the discovered `(space, binding)`. | `vkCreateDescriptorSetLayout`, `vkUpdateDescriptorSets` |
| **4. Dispatch** | Submit whole-image dispatch per config (default), or horizontal-band batches if `--tile-rows=N` is set. The shader atomically increments counters as branches/lines execute. | `vkCmdDispatch` |
| **5. Readback** | Download the raw counter bytes, widen each slot to `uint64_t`, call `getEntryInfo` per counter to map slot → file/line, write manifest + LCOV + binary. | `ICoverageTracingMetadata::getEntryInfo`, `slang_writeCoverageManifestJson` |

### Raw Vulkan host

This example uses raw Vulkan rather than the standard slang-rhi helper
because `__slang_coverage` is synthesized at **IR time** — after
Slang's parameter-binding layout pass — so it is invisible to ordinary
`ProgramLayout` reflection. slang-rhi's binding paths are all
reflection-driven, so it cannot bind the buffer without extra support
(slang-rhi PR #739). Raw Vulkan lets us bind it directly once we know
its location.

All raw-Vulkan code is isolated in `vk_compute_demo.h`. When slang-rhi
PR #739 merges and the submodule is bumped, the migration replaces that
header and its callers in `main.cpp`; the Slang shader sources stay
unchanged.

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
demonstrates the **explicit / raw-binding** approach instead.

## Build dependencies

- Slang compiler library (linked from this repository's build).
- Vulkan SDK (the `Vulkan::Vulkan` CMake target). The example is
  silently skipped if `find_package(Vulkan)` returns not-found.
