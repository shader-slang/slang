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

```bash
python3 path/to/slang/tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest exhaustive.coverage-mapping.json \
    --counters exhaustive.counters.bin \
    --output exhaustive.full.lcov

python3 path/to/slang/tools/coverage-html/slang-coverage-html.py \
    exhaustive.full.lcov --output-dir exhaustive-html
```

Open `exhaustive-html/index.html` and look at `tonemap.slang.*.html`
— each `case TonemapOperator::*` line shows a coloured `(1/1)` or
`(0/1)` branch indicator. The smoke vs exhaustive diff turns three
of the four operator branches from red to green.

## Architecture

This example uses raw Vulkan rather than slang-rhi (unlike the other
slang examples). The reason is documented in detail at the top of
`vk_compute_demo.h`: slang-rhi's reflection-driven binding API has no
view of Slang's synthesized `__slang_coverage` buffer, so the demo
binds it via raw `vkUpdateDescriptorSets`.

**All raw-Vulkan code is isolated in `vk_compute_demo.h`.** When
slang-rhi gains synthetic-resource binding support (slang-rhi PR
#739), the migration path is to replace that single header and update
the corresponding calls in `main.cpp` — slang sources stay unchanged.

## Build dependencies

- Slang compiler library (linked from this repository's build).
- Vulkan SDK (the `Vulkan::Vulkan` CMake target). The example is
  silently skipped if `find_package(Vulkan)` returns not-found.
