# shader-coverage-demo

End-to-end demonstration of Slang's `-trace-coverage` feature and the
host-side helper library `slang-coverage-rt`. Runs in three modes:

- **`--mode=compile`** — Compiles `simulate.slang` via slang-rhi with
  `-trace-coverage` pinned on the session. Queries
  `slang::ICoverageTracingMetadata` through the standard compile API
  and serializes it to a `.coverage-mapping.json` manifest next to
  the invocation. Exercises the compile half of the pipeline.

- **`--mode=report`** — Takes an existing manifest plus a binary
  counter buffer (little-endian `uint32_t` per counter), accumulates
  the hits via `slang-coverage-rt`, and writes an LCOV `.info` file
  consumable by `genhtml`, Codecov, VS Code Coverage Gutters, etc.
  Exercises the library/report half of the pipeline with no GPU
  required — counters can be captured from `slang-test` output, a
  previous dispatch run, or any compatible host.

- **`--mode=dispatch`** — Full compile → bind → dispatch → readback →
  LCOV pipeline via slang-rhi. Works end-to-end on CPU, Vulkan, D3D12
  and CUDA, producing byte-identical LCOV across all four backends.
  Metal runs but has pre-existing slang-rhi quirks (see *Backend
  status matrix*).

## Usage

### Quick start (any validated backend)

Two steps: run the demo to produce `coverage.lcov`, then render it
to HTML. Platform-specific rendering commands are in
[Rendering the report](#rendering-the-report) below.

```bash
# Step 1: run the demo. Pick any of: cpu, vulkan, d3d12, cuda.
./build/Debug/bin/shader-coverage-demo --mode=dispatch --backend=cpu
# → produces coverage.lcov and simulate.coverage-mapping.json

# Step 2: render. See "Rendering the report" below for your platform.
```

CPU, Vulkan, D3D12 and CUDA produce **byte-identical LCOV output** —
same hit counts per source line — which validates that instrumentation
semantics, slot assignment, and binding work consistently across
backends. D3D12 and Vulkan have been validated on desktop Windows
with NVIDIA drivers; CPU runs via the slang-rhi CPU backend; CUDA
requires an NVIDIA CUDA runtime.

### Separate compile + report (no GPU required)

```bash
# 1. Compile. Produces simulate.coverage-mapping.json alongside.
./build/Debug/bin/shader-coverage-demo --mode=compile --backend=cpu

# 2. Obtain a counter-buffer snapshot by any means. For a one-off
#    experiment, the buffer contents can come from slang-test output
#    or any compute harness that runs the instrumented shader. Write
#    them as a packed `uint32_t` binary file (N counters → N*4 bytes).

# 3. Convert to LCOV.
./build/Debug/bin/shader-coverage-demo --mode=report \
    --manifest=simulate.coverage-mapping.json \
    --counters=counters.bin \
    --output=coverage.lcov

# 4. Render to HTML. See "Rendering the report" below.
```

### Rendering the report

`coverage.lcov` is the industry-standard LCOV format, consumable by
several tools. Pick the instructions for your platform; the result is
a `coverage-html/` directory with `index.html` you can open in any
browser.

#### Linux

```bash
sudo apt install lcov         # Debian/Ubuntu; or: dnf install lcov / pacman -S lcov
genhtml coverage.lcov -o coverage-html/
xdg-open coverage-html/index.html
```

#### macOS

```bash
brew install lcov
genhtml coverage.lcov -o coverage-html/
open coverage-html/index.html
```

#### Windows

`genhtml` requires Perl; easier to use `reportgenerator` (.NET tool):

```powershell
dotnet tool install --global dotnet-reportgenerator-globaltool
reportgenerator -reports:coverage.lcov -targetdir:coverage-html -reporttypes:Html
start coverage-html\index.html
```

#### Other renderer options

| Tool | Platforms | When to use |
|---|---|---|
| `reportgenerator` (.NET tool) | Linux, macOS, Windows | Cross-platform alternative to `genhtml`; no Perl required |
| VS Code Coverage Gutters | Any (in-editor) | Per-developer view; inline line-level coverage in the editor, no HTML needed |
| Codecov / Coveralls | SaaS | Team-wide dashboards and PR annotations |

#### What the report shows

`index.html` lists every source file with its per-file coverage
percentage (sortable). Click through to a file to see each line
color-coded: green for covered, red for uncovered, gray for
non-executable. The demo's `simulate.slang` + `physics.slang`
should show ~84.6% coverage with the unreachable "unknown type"
branch flagged red — that's the dead-code-detection signal.

## What the shader exercises

`simulate.slang` is a tiny particle-physics compute kernel that
branches on particle type — `FLUID`, `GAS`, `SOLID`, and an
intentionally-unreachable "unknown" error path. Different input
mixes exercise different branches, which makes the coverage numbers
meaningful: running a scenario with only FLUID particles leaves the
GAS and SOLID branches uncovered. The unreachable branch stays
uncovered regardless of the scenario, demonstrating that the tool
spots dead code the way gcov does for CPU programs.

### Sample per-line hits (CPU or Vulkan)

```
 17 |  688 |     __slang_coverage[applyGravity entry]      ← physics helper, called per particle
 22 |  528 |     __slang_coverage[stepFluid entry]         ← FLUID branch, hit on most particles
 33 |  504 |     __slang_coverage[simulate.slang main loop]
 34 |  168 |     __slang_coverage[GAS branch body]
 41 |  504 |     __slang_coverage[common path after branch]
 ...
 55 |    0 |     __slang_coverage[unreachable "unknown type" branch]
 58 |    0 |     __slang_coverage[unreachable write-back]
```

Aggregated across the 64-particle × 8-step dispatch the demo runs.
The uncovered lines all live in the intentionally-unreachable
"unknown type" branch — exactly what a regression-watch would flag.

## Backend status matrix

| Backend | `--mode=compile` | `--mode=dispatch` |
|---|---|---|
| `cpu` | ✅ Works | ✅ **Fully working** — clean non-zero counter values, complete LCOV report, dead-code detection verified |
| `vulkan` (incl. SPIR-V) | ✅ Works | ✅ **Fully working** — validated on macOS (MoltenVK) and desktop Windows with NVIDIA drivers; byte-identical LCOV to CPU |
| `d3d12` | ✅ Works | ✅ **Fully working** — validated on desktop Windows; byte-identical LCOV to CPU/Vulkan |
| `cuda` | ✅ Works | ✅ **Fully working** — validated on desktop Windows with NVIDIA CUDA runtime; byte-identical LCOV to CPU/Vulkan/D3D12 |
| `metal` | ✅ Works (with benign unused-variable warnings from Metal's compiler) | ⚠️ Pipeline builds; dispatch runs; but counter values are unreliable — most slots are zero while others show overflow-like values. **Not a coverage-feature issue** — a pre-existing slang-rhi Metal binding / initialization quirk. To be filed against slang-rhi. |

## SPIR-V integration (Vulkan, custom engines)

Because the `vulkan` dispatch path goes through SPIR-V, the SPIR-V
codegen is validated end-to-end by
`--mode=dispatch --backend=vulkan`. The compiled shader has all the
properties a Vulkan host needs:

| Property | Verified in the generated SPIR-V |
|---|---|
| `spirv-val` spec compliance | ✅ Passes cleanly |
| Native atomic instructions for counters | ✅ One `OpAtomicIAdd` per counter op |
| Coverage buffer exposed in entry-point interface | ✅ `OpEntryPoint GLCompute %computeMain "main" %Params %particles %...InvocationID %__slang_coverage` |
| Source-level debug info | ✅ `OpSource Slang 1` preserved (for debug tooling) |
| Reflection visibility | ✅ `__slang_coverage` appears in `slangc -reflection-json` and resolves via standard `ShaderCursor["__slang_coverage"]` |

### For engines that don't use slang-rhi

Any Vulkan host can integrate shader coverage without depending on
slang-rhi. The integration surface is small:

1. **Compile the shader** via `slangc`:
   ```bash
   slangc shader.slang \
       -target spirv \
       -stage compute -entry main \
       -trace-coverage \
       -o shader.spv
   ```
   slangc writes `shader.spv` plus `shader.spv.coverage-mapping.json`
   next to it. The manifest reports the counter count, the assigned
   descriptor-set/binding, and the `slot → (file, line)` mapping.

2. **Parse the manifest via `slang-coverage-rt`** to size the
   counter buffer and learn where to bind it:
   ```c
   SlangCoverageContext* ctx;
   slang_coverage_create("shader.spv.coverage-mapping.json", &ctx);
   uint32_t N = slang_coverage_counter_count(ctx);
   const SlangCoverageBindingInfo* binding = slang_coverage_binding(ctx);
   // binding->space, binding->binding → where to bind the SSBO
   ```

3. **Allocate + bind the SSBO** using standard Vulkan calls:
   ```c
   VkBufferCreateInfo bufInfo = { .size = N * 4,
       .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, ... };
   VkBuffer coverageBuffer;
   vkCreateBuffer(device, &bufInfo, nullptr, &coverageBuffer);
   // bind at (binding->space, binding->binding) via VkWriteDescriptorSet
   ```

4. **Dispatch normally**, then barrier + copy-to-staging + map to
   read counters back:
   ```c
   vkCmdDispatch(cmd, groupsX, groupsY, groupsZ);
   vkCmdPipelineBarrier(/* COMPUTE_SHADER → TRANSFER */);
   vkCmdCopyBuffer(cmd, coverageBuffer, stagingBuffer, ...);
   vkMapMemory(device, stagingMem, 0, N * 4, 0, &mapped);
   ```

5. **Feed the counters to `slang-coverage-rt`** to accumulate and
   emit LCOV:
   ```c
   slang_coverage_accumulate(ctx, (uint32_t*)mapped, N);
   slang_coverage_save_lcov(ctx, "coverage.lcov", "test_run");
   ```

Total integration cost for a typical Vulkan engine: ~30 lines of
additional host code, no new runtime dependency beyond
`libslang-coverage-rt`.

### Alternative: compile API (no slangc sidecar needed)

Hosts that go through the Slang compile API directly can query the
same data via `slang::ICoverageTracingMetadata` on the artifact's
`IMetadata`, without needing the sidecar file. See the demo's
`writeManifestFromMetadata` helper in `main.cpp` for a concrete
example.

## History / design notes

Earlier revisions of this feature synthesized `__slang_coverage` at
IR-pass time — after Slang's AST-derived reflection tree had already
been frozen. That meant `ShaderCursor["__slang_coverage"]` returned
an invalid cursor on all backends, the buffer was never actually
bound at dispatch, and the demo's dispatch mode produced zeroed
counters. The architectural fix was to synthesize the buffer as an
AST-level `VarDecl` during semantic checking, before parameter
binding runs, so every downstream layer (reflection, layout, target
codegen) treats it identically to a user-declared global. That
change lives in `source/slang/slang-check-synthesize-coverage.{h,cpp}`.

The practical consequence: any backend that correctly handles a
user-declared `RWStructuredBuffer<uint>` + `kIROp_AtomicAdd` also
correctly handles coverage. Validated empirically on cpu, vulkan,
d3d12 and cuda — all four produce byte-identical counter output.

One host-side gotcha worth flagging: `slang::ISession::loadModule()`
returns a module pointer that the session keeps its own ref on.
Callers must `AddRef` (e.g. `ComPtr<IModule> m = session->loadModule(...)`)
rather than steal the pointer via `ComPtr::attach()`. An early
revision of this demo used `.attach()`, which left the caller and
session sharing a single ref; the resulting double-release surfaced
as heap corruption on Windows D3D12/CUDA/CPU device teardown while
going silently unnoticed on macOS. Every other Slang example uses
the `=` form; reuse that pattern.

## Scenarios (future expansion)

A follow-up will add `--scenario=fluid-only|mixed|edge-cases` so the
demo generates distinct particle inputs and shows a gcov-style story
of *coverage percentages rising as the test suite expands*. Straight-
forward extension now that dispatch works on CPU and Vulkan.
