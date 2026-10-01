# shader-coverage-bvh-traversal

Software BVH ray traversal kernel with multiple materials and a
linear-scan fallback for traversal-stack overflow. Demonstrates how
shader coverage exposes **input-shape gaps in test data**: rare-case
code paths (degenerate triangles, unusual materials, deep traversals)
are precisely the ones not exercised by the default test scene, and
branch coverage points them out by file:line.

## Coverage scenarios

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

The host generates the same procedural mesh and BVH, but defaults to
**256x256 = 65,536 rays**. Pass **`--ray-grid-size=4096`** to select
**4096x4096 = 16,777,216 rays** for benchmarking. Scene complexity, materials,
and the smoke/full distinctions remain unchanged.

`--batch-size=N` limits rays per GPU submission. Full mode defaults to at most
262144 rays per batch, so the small default grid fits in one batch. Smoke mode
and explicit `--batch-size=0` use a single dispatch. Batching helps with GPU
watchdog limits; it does not reduce total work or guarantee that large runs
finish before a process timeout.

Set `--ray-grid-size=N` to choose an N×N grid (default: 256; range: 2–65535).
For example, `--ray-grid-size=128` uses 128×128 rays.
Halving the grid dimension quarters the ray count without changing the scene.
Very small grids may miss scene features and exercise fewer coverage paths.

A nonzero `--batch-size=N` must be a multiple of the shader's 64-ray
thread-group size, so batches do not overlap. A batch larger than the grid's
ray count uses one submission; the final batch is clipped to the remaining
rays. Grid dimensions need not be multiples of 64. Smaller batches reduce
work per submission but add overhead; reduce the grid size to reduce total
runtime. Large workloads remain subject to available GPU memory and dispatch
limits.

```bash
./shader-coverage-bvh-traversal --mode=smoke    # clean icosphere, Diffuse only
./shader-coverage-bvh-traversal --mode=full     # +materials, +degenerates, +cluster

# Compile-time disable coverage instrumentation (baseline):
./shader-coverage-bvh-traversal --mode=full --no-coverage

# Use a smaller workload split into four batches:
./shader-coverage-bvh-traversal --mode=full --ray-grid-size=128 --batch-size=4096

# Restore the original ray grid for benchmarking:
./shader-coverage-bvh-traversal --mode=full --ray-grid-size=4096 --no-coverage

# Hit/miss mode — non-atomic, no execution counts but same coverage map:
./shader-coverage-bvh-traversal --mode=full --coverage-mode=boolean

# Tune the batch size (full mode already batches by default; smaller if
# you still observe TDR, larger for fewer submissions on fast hardware),
# or force a single unbatched dispatch:
./shader-coverage-bvh-traversal --mode=full --batch-size=65536
./shader-coverage-bvh-traversal --mode=full --batch-size=0

# Write the coverage artifacts somewhere other than the demo's source
# directory (the default). `--output-dir` creates the directory if needed:
./shader-coverage-bvh-traversal --mode=full --output-dir=./out

# Point the demo at a different copy of the `.slang` files (useful
# when the binary has been moved away from the source tree):
./shader-coverage-bvh-traversal --mode=full \
    --demo-dir=/path/to/shader-coverage-bvh-traversal
```

`--coverage-mode=count` (default) records exact execution counts via
atomic add. `--coverage-mode=boolean` records covered-or-not via
non-atomic stores of `1`, removing all atomic contention; the LCOV
report is identical because the converter treats any positive count
as "covered".

Each coverage run writes:

- `<mode>.coverage-manifest.json` — counter ↔ source attribution
- `<mode>.counters.bin` — raw counter buffer; feed to
  `tools/shader-coverage/slang-coverage-to-lcov.py` to produce a full
  LCOV with line, function, and branch records

## End-to-end wrapper

`run_coverage.py` forwards the workload and batch sizing options to the runner and compiles, dispatches, converts,
renders, and opens the HTML report in one step:

```bash
python3 run_coverage.py --mode=smoke
python3 run_coverage.py --mode=full --coverage-mode=boolean
```

## HTML report

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

| Stage                  | What happens                                                                                                                                                                                                                                                                                                                         | Key API                                                                     |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------- |
| **1. Compile**         | `compileShader()` creates a Slang session with `-trace-coverage`, `-trace-coverage-function`, `-trace-coverage-branch`, and `-trace-coverage-binding 0 1`. The compiler places `__slang_coverage` at the declared slot and emits SPIR-V with `OpAtomicIAdd` (count mode) or plain stores (boolean mode) at every instrumented point. | `slang::ISession::loadModule`, `IComponentType::link`, `getEntryPointCode`  |
| **2. Fix binding**     | No runtime discovery step — the slot was dictated by `TraceCoverageBinding` at compile time. The host uses the same constants (`kCoverageBinding`, `kCoverageSet`) on the Vulkan side.                                                                                                                                               | `CompilerOptionName::TraceCoverageBinding`                                  |
| **3. Allocate & bind** | Allocate a zeroed `counterCount × counterByteWidth` storage buffer. Build a Vulkan descriptor layout with app resources (rays/tris/nodes/globals/output) on set 0 and the coverage buffer at `(kCoverageSet, kCoverageBinding)` on set 1.                                                                                            | `vkCreateDescriptorSetLayout`, `vkUpdateDescriptorSets`                     |
| **4. Dispatch**        | Submit at most 262144 rays per batch in full mode or a single dispatch in smoke mode; `--batch-size=N` overrides this. Each batch re-uploads `globals.rayBatchOffset`; the shader adds it to `tid.x` to recover the true ray index. Counters accumulate across all batches.                                                          | `vkCmdDispatch`                                                             |
| **5. Readback**        | Download the raw counter bytes, call `decodeCoverageCounters()` for the console summary, and write the manifest + raw binary for offline LCOV conversion.                                                                                                                                                                            | `ICoverageTracingMetadata::getEntryInfo`, `slang_writeCoverageManifestJson` |

### Raw Vulkan host

Same reason as `shader-coverage-image-pipeline`: Slang's
`__slang_coverage` buffer is synthesized at IR time, after the
parameter-binding layout pass, so it is invisible to ordinary
`ProgramLayout` reflection and cannot be bound via slang-rhi's
reflection-driven paths without additional support (slang-rhi PR
#739). All raw-Vulkan code is isolated in `vk_compute_demo.h`; see
the image-pipeline README for the full rationale and migration plan.

### Explicit binding

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

### Counter readback and LCOV

For the in-process console summary, [`main.cpp`](main.cpp) calls the shared
[`decodeCoverageCounters()`](../shader-coverage-common/coverage-counters.h) example
helper after `ctx.download()`. It widens the effective 32- or 64-bit counter slots
to `uint64_t`; `summarize()` then reads `hits[entry.counterIndex]` while iterating
metadata entries. The raw bytes remain unchanged for the offline converter.
See the [image-pipeline readback example](../shader-coverage-image-pipeline/README.md#counter-readback-and-lcov)
for the corresponding in-process LCOV path.

After dispatch the host downloads the raw counter buffer and writes two
artifact files: a coverage manifest JSON and the raw counters binary.
`run_coverage.py` then calls `slang-coverage-to-lcov.py` to produce the
full LCOV — the **out-of-process converter** path:

```
GPU counter buffer (uint32/uint64 × N slots)
    │  ctx.download()
    ▼
<mode>.counters.bin  (raw little-endian slots)
<mode>.coverage-manifest.json  (counter ↔ source attribution)
    │  slang-coverage-to-lcov.py
    │    --manifest  --counters  --output
    ▼
<mode>.full.lcov  (full: DA + FN/FNDA + BRDA)
    │  slang-coverage-html.py
    ▼
HTML report
```

This is the canonical workflow for hosts that want to separate data
collection (C++ app) from reporting (offline Python tooling): the app
writes the manifest and counters, and the converter can be run later,
on a different machine, or integrated into a CI pipeline.

Compare the image-pipeline demo which uses the **in-process conversion**
path instead: it calls `ICoverageTracingMetadata::getEntryInfo()` in C++
to build the full LCOV directly, skipping the manifest+converter step.

## Build dependencies

- Slang compiler library (linked from this repository's build).
- Vulkan SDK (the `Vulkan::Vulkan` CMake target). The example is
  silently skipped if `find_package(Vulkan)` returns not-found.
