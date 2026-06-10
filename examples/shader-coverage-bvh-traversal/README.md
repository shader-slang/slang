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
  material; the full scene uses all four.
- **Degenerate-triangle skip** (`isDegenerate`): only fires on meshes
  with zero-area triangles, which production meshes occasionally
  contain but typical test meshes don't.
- **Stack-overflow fallback** (`linearScanRemaining`): only fires
  when the BVH traversal stack exceeds 24 entries. The full scene
  brings the stack closer but doesn't exceed it — a known gap in the
  current scene generator that branch coverage surfaces clearly.
- **Ray-AABB / ray-triangle edge cases**: parallel-ray rejection,
  bounds-rejection branches that need specifically constructed input
  rays.

## Run

The host driver generates a procedural mesh, builds a BVH on CPU,
uploads, and dispatches 4096×4096 = 16.7M rays from a synthetic camera
in batches of 512×512 (64 batches). Batching keeps each GPU submission
short to avoid OS watchdog resets (Windows TDR) under coverage
instrumentation; see `kBatchRays` in `main.cpp` to tune the batch size.

```bash
./shader-coverage-bvh-traversal --mode=smoke    # clean icosphere, Diffuse only
./shader-coverage-bvh-traversal --mode=full   # +materials, +degenerates, +cluster

# Compile-time disable coverage instrumentation (baseline):
./shader-coverage-bvh-traversal --mode=full --no-coverage

# Hit/miss mode — non-atomic, no execution counts but same coverage map:
./shader-coverage-bvh-traversal --mode=full --coverage-mode=hit-miss

# Write the coverage artifacts somewhere other than the demo's source
# directory (the default). `--output-dir` creates the directory if
# needed:
./shader-coverage-bvh-traversal --mode=full --output-dir=./out

# Point the demo at a different copy of the `.slang` files (useful
# when the binary has been moved away from the source tree):
./shader-coverage-bvh-traversal --mode=full \
    --demo-dir=/path/to/shader-coverage-bvh-traversal
```

`--coverage-mode=count` (default) records exact execution counts via
atomic add. `--coverage-mode=hit-miss` records covered-or-not via
non-atomic stores of `1`, removing all atomic contention; the LCOV
report is identical because the converter treats any positive count
as "covered".

Each coverage run writes:

- `<mode>.coverage-manifest.json` — counter ↔ source attribution
- `<mode>.lcov` — line-only LCOV (quick view)
- `<mode>.counters.bin` — raw counter buffer; feed to
  `tools/shader-coverage/slang-coverage-to-lcov.py` for the rich LCOV
  with branch+function records

## End-to-end wrapper

`run_coverage.py` (in this directory) compiles, dispatches, converts,
renders, and opens the HTML report in one step:

```bash
python3 run_coverage.py --mode=smoke
python3 run_coverage.py --mode=full --coverage-mode=hit-miss
```

## Generate an HTML report (manual)

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
    --title "bvh-traversal full"
```

## Architecture

### Coverage instrumentation pipeline

The five stages `main.cpp` walks through for each run:

| Stage | What happens | Key API |
|---|---|---|
| **1. Compile** | `compileShader()` creates a Slang session with `-trace-coverage`, `-trace-coverage-function`, `-trace-coverage-branch`, and `-trace-coverage-binding 0 1`. The compiler places `__slang_coverage` at the declared slot and emits SPIR-V with `OpAtomicIAdd` (count mode) or plain stores (hit-miss mode) at every instrumented point. | `slang::ISession::loadModule`, `IComponentType::link`, `getEntryPointCode` |
| **2. Fix binding** | No runtime discovery step — the slot was dictated by `TraceCoverageBinding` at compile time. The host uses the same constants (`kCoverageBinding`, `kCoverageSet`) on the Vulkan side. | `CompilerOptionName::TraceCoverageBinding` |
| **3. Allocate & bind** | Allocate a zeroed `counterCount × counterByteWidth` storage buffer. Build a Vulkan descriptor layout with app resources (rays/tris/nodes/globals/output) on set 0 and the coverage buffer at `(kCoverageSet, kCoverageBinding)` on set 1. | `vkCreateDescriptorSetLayout`, `vkUpdateDescriptorSets` |
| **4. Dispatch** | Submit 64 batches of 512×512 rays. Each batch re-uploads `globals.rayBatchOffset`; the shader adds it to `tid.x` to recover the true ray index. Counters accumulate across all batches. Batching caps per-submission GPU time to avoid TDR. | `vkCmdDispatch` (×64) |
| **5. Readback** | Download the raw counter bytes, widen each slot to `uint64_t`, call `getEntryInfo` per counter to map slot → file/line, write manifest + LCOV + binary. | `ICoverageTracingMetadata::getEntryInfo`, `slang_writeCoverageManifestJson` |

### Why raw Vulkan instead of slang-rhi

Same reason as `shader-coverage-image-pipeline`: Slang's
`__slang_coverage` buffer is synthesized at IR time, after the
parameter-binding layout pass, so it is invisible to ordinary
`ProgramLayout` reflection and cannot be bound via slang-rhi's
reflection-driven paths without additional support (slang-rhi PR
#739). All raw-Vulkan code is isolated in `vk_compute_demo.h`; see
the image-pipeline README for the full rationale and migration plan.

### Explicit / raw binding (this demo)

This demo uses the **explicit / raw-binding** approach: the host
dictates where `__slang_coverage` lives before compilation using the
`-trace-coverage-binding <binding> <space>` compiler option (or its
API equivalent `CompilerOptionName::TraceCoverageBinding`), then
writes the buffer to that same hardcoded slot at runtime:

```cpp
// Compile time: tell the compiler to place __slang_coverage at
// descriptor set kCoverageSet, binding kCoverageBinding.
pin.name  = slang::CompilerOptionName::TraceCoverageBinding;
pin.value.intValue0 = kCoverageBinding; // binding index
pin.value.intValue1 = kCoverageSet;     // descriptor set / space

// Runtime: bind the counter buffer at exactly that slot.
ctx.writeStorageBuffer(set1, kCoverageBinding, coverageBuf);
```

The advantage is simplicity: no post-compile metadata query; the slot
is a compile-time constant. The trade-off is that the host must ensure
the slot does not collide with any of the shader's own resources. This
demo isolates the coverage buffer on a dedicated descriptor set
(`kCoverageSet = 1`) so adding or removing application bindings on
set 0 can never cause a collision.

Compare the image-pipeline demo (`shader-coverage-image-pipeline`)
which demonstrates the **metadata-derived** binding approach instead,
where the compiler picks the slot and the host discovers it after
compilation via `ISyntheticResourceMetadata`.

## Build dependencies

- Slang compiler library (linked from this repository's build).
- Vulkan SDK (the `Vulkan::Vulkan` CMake target). The example is
  silently skipped if `find_package(Vulkan)` returns not-found.
