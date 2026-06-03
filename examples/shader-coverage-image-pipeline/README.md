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

# Default counter width is 32-bit so the demo runs on MoltenVK out of
# the box (Apple Silicon lacks `VK_KHR_shader_atomic_int64`). On a
# desktop Vulkan driver that supports the extension, pass
# `--counter-width=64` to exercise the wider counters (cannot wrap
# within any practical run; default in production use):
./shader-coverage-image-pipeline --mode=exhaustive --counter-width=64
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
