# shader-coverage-bvh-traversal

Software BVH ray traversal kernel with multiple materials and a
linear-scan fallback for traversal-stack overflow. Demonstrates how
shader coverage exposes **input-shape gaps in test data**: rare-case
code paths (degenerate triangles, unusual materials, deep traversals)
are precisely the ones not exercised by the default test scene, and
branch coverage points them out by file:line.

## What it shows

The traversal kernel has several rarely-fired branches:

- **Material dispatch** (`evaluateMaterial`): 4-way switch over
  Diffuse / Emissive / Metallic / Debug. The default mesh uses one
  material; the stress scene uses all four.
- **Degenerate-triangle skip** (`isDegenerate`): only fires on meshes
  with zero-area triangles, which production meshes occasionally
  contain but typical test meshes don't.
- **Stack-overflow fallback** (`linearScanRemaining`): only fires
  when the BVH traversal stack exceeds 24 entries. The stress scene
  brings the stack closer but doesn't exceed it — a known gap in the
  current scene generator that branch coverage surfaces clearly.
- **Ray-AABB / ray-triangle edge cases**: parallel-ray rejection,
  bounds-rejection branches that need specifically constructed input
  rays.

## Run

The host driver generates a procedural mesh, builds a BVH on CPU,
uploads, and dispatches 512×512 = 262144 rays from a synthetic camera.

```bash
./shader-coverage-bvh-traversal --mode=smoke    # clean icosphere, Diffuse only
./shader-coverage-bvh-traversal --mode=stress   # +materials, +degenerates, +cluster

# Compile-time disable coverage instrumentation (baseline):
./shader-coverage-bvh-traversal --mode=stress --no-coverage

# Hit/miss mode — non-atomic, no execution counts but same coverage map:
./shader-coverage-bvh-traversal --mode=stress --coverage-mode=hit-miss

# Write the coverage artifacts somewhere other than the demo's source
# directory (the default). `--output-dir` creates the directory if
# needed:
./shader-coverage-bvh-traversal --mode=stress --output-dir=./out
```

`--coverage-mode=count` (default) records exact execution counts via
atomic add. `--coverage-mode=hit-miss` records covered-or-not via
non-atomic stores of `1`, removing all atomic contention; the LCOV
report is identical because the converter treats any positive count
as "covered".

Each coverage run writes:

- `<mode>.coverage-mapping.json`
- `<mode>.lcov`
- `<mode>.counters.bin` — feed to
  `tools/shader-coverage/slang-coverage-to-lcov.py` for the rich LCOV
  with branch+function records

## Architecture

Same raw-Vulkan-based architecture as `shader-coverage-image-pipeline`
— see that example's README for rationale + the `vk_compute_demo.h`
swap-out boundary when slang-rhi gains synthetic-resource binding.

## Build dependencies

- Slang compiler library (linked from this repository's build).
- Vulkan SDK (the `Vulkan::Vulkan` CMake target). The example is
  silently skipped if `find_package(Vulkan)` returns not-found.
