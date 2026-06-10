# shader-coverage-image-pipeline

Multi-stage GPU image processing pipeline (bilateral denoise → tone
mapping → gamma encoding) that exercises Slang's shader coverage
instrumentation across recognizable kernel shapes. Demonstrates how
**branch and function coverage surface unexercised code paths that
line coverage alone marks "covered."**

## What it shows

The kernel chain has several many-armed switches whose default test
inputs only hit one arm:

- `applyTonemap(op)` — 4-way switch over Reinhard / ACES / Hable /
  Uncharted2 operators
- `sampleWithBoundary(mode)` — 4-way switch over Clamp / Wrap /
  Reflect / Black boundary handling
- `applyGamma(mode)` — 3-way switch over sRGB / Linear / Rec.709
- Bilateral filter fast-path vs general path (radius-dependent)

A **smoke** run dispatches one operator/boundary/gamma combination;
an **exhaustive** run sweeps the full 4×4×3 = 48 configuration matrix.
The coverage delta between the two runs is the demo's headline.

## Run

```bash
./shader-coverage-image-pipeline --mode=smoke
./shader-coverage-image-pipeline --mode=exhaustive

# Compile-time disable coverage instrumentation (baseline for overhead
# measurement):
./shader-coverage-image-pipeline --mode=exhaustive --no-coverage

# Single whole-image dispatch per config instead of horizontal tiles
# (see "Why the dispatch is tiled" below for what tiling buys you):
./shader-coverage-image-pipeline --mode=exhaustive --dispatch=whole

# Write the coverage artifacts somewhere other than the demo's source
# directory (the default). `--output-dir` creates the directory if
# needed:
./shader-coverage-image-pipeline --mode=exhaustive --output-dir=./out

# Point the demo at a different copy of the `.slang` files (useful
# when the binary has been moved away from the source tree):
./shader-coverage-image-pipeline --mode=exhaustive \
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

## Picking a coverage mode

`--coverage-mode=count` (default) records exact execution counts via
atomic add. The bilateral filter's inner loop contends heavily on a
few counter slots, so on this workload count mode is the dominant
cost of instrumentation (the section below on tiling describes the
mitigation for the resulting wall-time blow-up).

`--coverage-mode=hit-miss` records covered-or-not instead — each slot
is written non-atomically with `1` the first time it executes.
Concurrent same-value stores are a benign race. This removes all
atomic contention, so the exhaustive sweep runs roughly an order of
magnitude faster while still producing the same LCOV report (any
positive count is "covered", which is exactly what hit-miss
preserves). Pick `count` when you need exact execution counts; pick
`hit-miss` when you just want to know which paths fired.

```bash
# Same coverage map, much faster on the exhaustive sweep:
./shader-coverage-image-pipeline --mode=exhaustive --coverage-mode=hit-miss
```

## Why the dispatch is tiled (and how to turn it off)

`--dispatch=tiled` (default) splits each config into horizontal bands
(`kTileRows` rows at a time) and passes the band's row offset to the
shader as `PipelineParams.tileOriginY` so each band recovers its real
pixel row (`tid.y + tileOriginY`). `--dispatch=whole` submits each
config as a single whole-image dispatch with `tileOriginY = 0`.

The reason tiling is the default is the **cost of coverage
instrumentation under count mode**. Each covered line / branch /
function increments a counter with an atomic add, and on a hot path
— the bilateral filter's inner loop runs for every pixel — a handful
of counter slots take millions of contended atomic adds. That makes
the instrumented shader far slower than the uninstrumented one
(20–30× on the GPUs we measured). A single whole-image dispatch of
that instrumented kernel can run long enough to trip the GPU's
watchdog timeout (Windows TDR / `VK_ERROR_DEVICE_LOST`) and abort
the run — especially the 48-config `--mode=exhaustive` sweep.

Tiling caps how long any single GPU submission runs, keeping each
one well under the watchdog limit. It does **not** change the
coverage results: the bands partition the image, every pixel is
processed exactly once, and the counter buffer accumulates across
all bands and configs. (Total GPU work is unchanged — tiling spreads
it across more, shorter submissions; it does not reduce the per-
execution atomic cost itself.)

`--dispatch=whole` is provided as an opt-in for cases where the TDR
risk doesn't apply:

- `--no-coverage`: no instrumentation cost, so whole-image is fine
  (and a useful baseline for measuring tiled overhead).
- `--coverage-mode=hit-miss`: no atomic contention, so whole-image
  is fine on the workloads here even with coverage on.
- demonstrating the TDR symptom directly under count mode (don't
  use this on workloads you actually need to finish).

## Generate an HTML report

The `run_coverage.py` wrapper does all steps automatically and opens
the report. To run the steps manually:

```bash
# 1. Convert raw counters to rich LCOV (adds branch + function records):
python3 path/to/slang/tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest exhaustive.coverage-manifest.json \
    --counters exhaustive.counters.bin \
    --output exhaustive.full.lcov

# 2. Render HTML:
python3 path/to/slang/tools/coverage-html/slang-coverage-html.py \
    exhaustive.full.lcov \
    --output-dir exhaustive-html \
    --title "image-pipeline exhaustive"
```

Open `exhaustive-html/index.html` and look at `tonemap.slang.*.html`
— each `case TonemapOperator::*` line shows a coloured `(1/1)` or
`(0/1)` branch indicator. The smoke vs exhaustive diff turns three
of the four operator branches from red to green.

## End-to-end wrapper

`run_coverage.py` (in this directory) is a convenience wrapper that
compiles, dispatches, converts the raw counters to a rich LCOV, renders
an HTML report and opens it — all in one command:

```bash
# Smoke run with HTML report opened automatically:
python3 run_coverage.py --mode=smoke

# Exhaustive sweep, hit/miss mode, custom output dir:
python3 run_coverage.py --mode=exhaustive --coverage-mode=hit-miss --output-dir=./out
```

All flags accepted by the demo binary are forwarded verbatim; the
script adds `--slang-root` (default: auto-discovered from its own path)
to locate the renderer and converter.

## Architecture

### Why raw Vulkan instead of slang-rhi

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

### Metadata-derived binding (this demo)

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
