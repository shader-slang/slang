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
  LCOV pipeline via slang-rhi. End-to-end verified on Vulkan via the
  new `ExtraDescriptorBinding` slang-rhi API; other backends await
  the matching per-backend slang-rhi work. See *Backend status
  matrix* for the per-backend details.

## Usage

### Quick start (any validated backend)

Two steps: run the demo to produce `coverage.lcov`, then render it
to HTML. Platform-specific rendering commands are in
[Rendering the report](#rendering-the-report) below.

```bash
# Step 1: run the demo on Vulkan (the verified end-to-end backend).
./build/Debug/bin/shader-coverage-demo --mode=dispatch --backend=vulkan
# → produces coverage.lcov and simulate.coverage-mapping.json

# Step 2: render. See "Rendering the report" below for your platform.
```

Other backends (cpu/d3d12/cuda/metal) currently work in
`--mode=compile` but await per-backend slang-rhi `ExtraDescriptorBinding`
support for `--mode=dispatch`. See *Backend status matrix* below.

### Pinning the coverage buffer at a specific (index, space)

By default the demo lets parameter binding auto-allocate the
`__slang_coverage` buffer's slot. Pass `--coverage-binding=<index>:<space>`
to pin it via the new `-trace-coverage-binding` compile option —
useful when the host needs the slot fixed before reflection runs
(e.g. a pre-built D3D12 root signature):

```bash
# End-to-end verified on Vulkan via the new ExtraDescriptorBinding API.
./build/Debug/bin/shader-coverage-demo \
    --mode=dispatch --backend=vulkan \
    --coverage-binding=7:0
# → __slang_coverage lands at DescriptorSet 0, Binding 7
# → "[coverage] binding pinned at (index=7, space=0) — round-trip verified"
```

After the dispatch, the demo asserts via the
`ICoverageTracingMetadata` API that the metadata-reported binding
matches what was requested. The CPU and CUDA backends pack globals
into a uniform-offset struct rather than exposing them at
(set, register) slots, so on `--backend=cpu`/`cuda` the option
silently has no effect (the demo prints a `[coverage] note: …`
explaining this).

> **Non-zero descriptor space:** D3D12 handles
> `--coverage-binding=N:M` with `M != 0` end-to-end. Vulkan and
> WebGPU require slang-rhi's binding-data builder to support more
> than one descriptor set per shader object; the demo refuses to
> dispatch on those backends with non-zero space and prints a
> clear `[coverage] skip: …` message rather than letting the
> `SLANG_RHI_ASSERT` fire mid-dispatch. CPU and CUDA are
> unaffected because they don't use (set, register). Tracked in
> shader-slang/slang#10959.

Vulkan dispatch produces real per-line LCOV (e.g. `DA:31,512` =
64 particles × 8 dispatches = 512 hits per always-executed line),
verified on macOS via MoltenVK. Other backends produce correct
compile output but their dispatch path is gated on per-backend
slang-rhi `ExtraDescriptorBinding` rollout — see the matrix
below.

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
non-executable. The unreachable "unknown type" branch in
`simulate.slang` will be flagged red — that's the
dead-code-detection signal the feature exists to surface; see
[What the report shows](#what-the-report-shows-1) further down for
how to read the per-line counts the demo produces.

## What the shader exercises

`simulate.slang` is a tiny particle-physics compute kernel that
branches on particle type — `FLUID`, `GAS`, `SOLID`, and an
intentionally-unreachable "unknown" error path. Different input
mixes exercise different branches, which makes the coverage numbers
meaningful: running a scenario with only FLUID particles leaves the
GAS and SOLID branches uncovered. The unreachable branch stays
uncovered regardless of the scenario, demonstrating that the tool
spots dead code the way gcov does for CPU programs.

### What the report shows

Aggregated across the 64-particle × 8-step dispatch the demo runs,
the rendered LCOV groups source lines into three buckets:

- **Hot lines, large hit counts** — `applyGravity`, the
  per-iteration entries of `stepFluid`/`stepGas`/`stepSolid`, and
  the kernel's main flow. Roughly proportional to how many
  particles take that path × the number of steps.
- **Conditional lines, hit counts that vary with input** — the
  inelastic floor-bounce inside `stepSolid`, only entered when a
  particle dips below `y = 0`. With the demo's default mixed
  scenario the count is non-zero but smaller than the entry-point
  hits.
- **Zero-hit lines, flagged as uncovered** — the
  intentionally-unreachable `else` branch in `simulate.slang`'s
  particle-type dispatch. This is the dead-code detection signal:
  a regression-watch over the LCOV report would flag any change
  that left it un-zero, or any *previously*-non-zero line that
  fell to zero.

Exact per-line numbers are not pinned in this README on purpose —
they shift across compiler versions as the instrumentation density
evolves (e.g. when branch coverage lands or counter packing
changes), and the report itself is the canonical answer. Run the
demo and open `coverage-html/index.html` to see the current values.

## Backend status matrix

The compiler-side instrumentation (counter ops, IR-time buffer
synthesis, `ICoverageTracingMetadata` generation) works on every
backend. Dispatch via slang-rhi additionally requires the
`ExtraDescriptorBinding` machinery to be implemented per-backend.
Vulkan is the first backend with that support; others are scoped as
follow-ups.

Customers integrating directly against raw graphics APIs (Tier 1 —
production engines, custom shader runtimes, content-creation
applications) are not affected by the per-backend slang-rhi
rollout: they declare the slot through their native pipeline-
layout / root-signature API based on `ICoverageTracingMetadata`.

| Backend | `--mode=compile` | `--mode=dispatch` (via slang-rhi) |
|---|---|---|
| `vulkan` (incl. SPIR-V) | Supported | **Verified end-to-end** via `ExtraDescriptorBinding` + `setExtraBinding`. Real per-line LCOV produced on macOS/MoltenVK. Default and `--coverage-binding=N:0` work; `--coverage-binding=N:M` with `M != 0` blocked by a separate slang-rhi multi-set limitation (shader-slang/slang#10959). |
| `cpu` | Supported | Pending — slang-rhi CPU backend needs the same `ExtraDescriptorBinding` merge applied to its global-uniform packing. |
| `d3d12` | Supported | Pending — same `ExtraDescriptorBinding` design extends to root-signature merging. |
| `cuda` | Supported | Pending — slang-rhi CUDA path packs globals as kernel parameters; needs equivalent extras handling. |
| `metal` | Supported (with benign unused-variable warnings from Metal's compiler) | Pending — same per-backend rollout. Note: a separate pre-existing slang-rhi Metal binding quirk (shader-slang/slang-rhi#724) causes counter values to be unreliable; that's independent of the `ExtraDescriptorBinding` work. |

## SPIR-V integration (Vulkan, custom engines)

Because the `vulkan` dispatch path goes through SPIR-V, the SPIR-V
codegen is validated end-to-end by
`--mode=dispatch --backend=vulkan`. The compiled shader has all the
properties a Vulkan host needs:

| Property | Verified in the generated SPIR-V |
|---|---|
| `spirv-val` spec compliance | Passes cleanly |
| Native atomic instructions for counters | One `OpAtomicIAdd` per counter op |
| Coverage buffer exposed in entry-point interface | `OpEntryPoint GLCompute %computeMain "main" %Params %particles %...InvocationID %__slang_coverage` |
| Source-level debug info | `OpSource Slang 1` preserved (for debug tooling) |
| Descriptor decoration on the buffer | `OpDecorate %__slang_coverage DescriptorSet 0 Binding N` (where N is the metadata-reported binding) |

The buffer is **not** in Slang's public reflection (no
`ShaderCursor["__slang_coverage"]` resolution) — it's synthesized at
IR time and surfaced via `slang::ICoverageTracingMetadata`. Hosts
declare the slot in their own pipeline-layout API based on the
metadata-reported `(set, binding)`.

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
`IMetadata`, without needing the sidecar file. To produce the
canonical `.coverage-mapping.json` bytes from in-process metadata,
call `slang_writeCoverageManifestJson(metadata, &blob)` — the bytes
are byte-identical to slangc's sidecar output. The demo's
`writeManifestFromMetadata` in `main.cpp` is a working example of
this pattern.

## Notes for host integrators

- **Use `=` for `loadModule`, not `ComPtr::attach()`.**
  `slang::ISession::loadModule()` returns a module pointer that the
  session keeps its own ref on, so callers must `AddRef`. The `=`
  form (`ComPtr<IModule> m = session->loadModule(...)`) is correct;
  `.attach()` causes a double-release that surfaces as heap
  corruption on Windows D3D12/CUDA/CPU device teardown.
- **Non-zero descriptor space (Vulkan / WebGPU).** Vulkan and WebGPU
  through slang-rhi currently require `space == 0`. The demo refuses
  to dispatch on those backends with non-zero space and prints a
  clear `[coverage] skip` message. D3D12 handles non-zero spaces
  end-to-end. Tracked in shader-slang/slang#10959.

For the design rationale and pipeline architecture, see
[`docs/design/shader-coverage.md`](../../docs/design/shader-coverage.md).

## Scenarios (future expansion)

A follow-up will add `--scenario=fluid-only|mixed|edge-cases` so the
demo generates distinct particle inputs and shows a gcov-style story
of *coverage percentages rising as the test suite expands*. Straight-
forward extension now that dispatch works on CPU and Vulkan.
