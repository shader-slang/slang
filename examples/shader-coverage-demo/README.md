# shader-coverage-demo

End-to-end demonstration of Slang's `-trace-coverage` feature and the
host-side helper library `slang-coverage-rt`. Runs in three modes:

- **`--mode=compile`** — Compiles `simulate.slang` via slang-rhi with
  `-trace-coverage` pinned on the session. The coverage pass runs,
  writes a `.slangcov` manifest next to the invocation, and reports
  the counter count. Exercises the compile half of the pipeline.

- **`--mode=report`** — Takes an existing manifest plus a binary
  counter buffer (little-endian `uint32_t` per counter), accumulates
  the hits via `slang-coverage-rt`, and writes an LCOV `.info` file
  consumable by `genhtml`, Codecov, VS Code Coverage Gutters, etc.
  Exercises the library/report half of the pipeline with no GPU
  required — counters can be captured from `slang-test` output, a
  previous dispatch run, or any compatible host.

- **`--mode=dispatch`** — Full compile → bind → dispatch → readback →
  LCOV pipeline via slang-rhi. Works end-to-end on both the CPU
  target and Vulkan/SPIR-V (tested via MoltenVK on Apple M4);
  Metal is blocked on an unrelated slang-rhi bug (see *Backend
  status matrix*).

## Usage

### Quick start (any validated backend)

```bash
# CPU
./build/Debug/bin/shader-coverage-demo --mode=dispatch --backend=cpu

# Vulkan (requires Vulkan SDK + MoltenVK on macOS, or native driver elsewhere)
./build/Debug/bin/shader-coverage-demo --mode=dispatch --backend=vulkan

genhtml coverage.lcov -o coverage-html/
open coverage-html/index.html
```

Both backends produce byte-identical LCOV output: `84.6% (22 of 26
lines)` covered, with line 91 correctly flagged as dead code.

### Separate compile + report (no GPU required)

```bash
# 1. Compile.
./build/Debug/bin/shader-coverage-demo --mode=compile --backend=cpu

# 2. Obtain a counter-buffer snapshot by any means. For a one-off
#    experiment, the buffer contents can come from slang-test output
#    or any compute harness that runs the instrumented shader. Write
#    them as a packed `uint32_t` binary file (N counters → N*4 bytes).

# 3. Convert to LCOV.
./build/Debug/bin/shader-coverage-demo --mode=report \
    --manifest=simulate.slangcov \
    --counters=counters.bin \
    --output=coverage.lcov

# 4. Render.
genhtml coverage.lcov -o coverage-html/
```

## What the shader exercises

`simulate.slang` is a tiny particle-physics compute kernel that
branches on particle type — `FLUID`, `GAS`, `SOLID`, and an
intentionally-unreachable "unknown" error path on line 91. Different
input mixes exercise different branches, which makes the coverage
numbers meaningful: running a scenario with only FLUID particles
leaves the GAS and SOLID branches uncovered. The unreachable branch
stays uncovered regardless of the scenario, demonstrating that the
tool does spot dead code the way gcov does for CPU programs.

### Sample coverage report (CPU and Vulkan produce identical output)

After `--mode=dispatch` followed by `genhtml`, you'll see a per-line
report like this (reformatted from the actual HTML):

```
 67 | 1536 |     uint i = tid.x;
 68 |  512 |     if (i >= particleCount)
 71 |  512 |     Particle p = particles[i];
 73 |  512 |     if (p.type == PARTICLE_TYPE_FLUID) { ... }
 75 |  352 |         stepFluid(p, dt);
 79 |  336 |         stepGas(p, dt);
 83 |  336 |         stepSolid(p, dt);
 91 |    0 |         p.flags |= 0x1u;    ← uncovered dead-code branch
```

`lcov --summary` reports `lines: 84.6% (22 of 26 lines)`. The four
uncovered lines all live in the intentionally-unreachable "unknown
type" branch and are exactly what a regression-watch would flag.

## Backend status matrix

| Backend | `--mode=compile` | `--mode=dispatch` |
|---|---|---|
| `cpu` | ✅ Works | ✅ **Fully working** — correct counter values, complete LCOV report, dead-code detection verified |
| `vulkan` (incl. SPIR-V) | ✅ Works | ✅ **Fully working** — validated via MoltenVK on Apple M4; produces byte-identical output to CPU |
| `metal` | ✅ Works | ⚠️ Pipeline builds; dispatch runs; only counters *before the `if (i >= particleCount) return;` guard* are written — every Metal thread takes the early-return branch because `cursor["Params"]["particleCount"].setData(...)` on Metal doesn't deliver the constant-buffer value to the shader. **Not a coverage-feature issue** — a slang-rhi Metal constant-buffer-binding bug. Should be filed against slang-rhi. |
| `d3d12` | Untested | Untested |

## SPIR-V integration (Vulkan, custom engines)

Because the `vulkan` dispatch path goes through SPIR-V, the SPIR-V
codegen is validated end-to-end by the `--mode=dispatch --backend=vulkan`
run. The compiled shader has all the properties a Vulkan host needs:

| Property | Verified in the generated SPIR-V |
|---|---|
| `spirv-val` spec compliance | ✅ Passes cleanly |
| Native atomic instructions for counters | ✅ 43 × `OpAtomicIAdd` in the demo's output |
| Coverage buffer exposed in entry-point interface | ✅ `OpEntryPoint GLCompute %computeMain "main" %Params %particles %...InvocationID %__slang_coverage` |
| Source-level debug info | ✅ `OpSource Slang 1` preserved (for debug tooling) |

### For engines that don't use slang-rhi

Any Vulkan host can integrate shader coverage without depending on
slang-rhi. The integration surface is small:

1. **Compile the shader separately** via `slangc`:
   ```bash
   SLANG_COVERAGE_MANIFEST_PATH=shader.slangcov \
   slangc shader.slang \
       -target spirv \
       -stage compute -entry main \
       -trace-coverage \
       -o shader.spv
   ```
   This produces both `shader.spv` and the `shader.slangcov` sidecar
   manifest. The manifest reports the counter count, the assigned
   descriptor-set/binding, and the counter-index → `(file, line)`
   mapping.

2. **Parse the manifest via `slang-coverage-rt`** to learn the binding
   slot and allocate the right-sized SSBO:
   ```c
   SlangCoverageContext* ctx;
   slang_coverage_create("shader.slangcov", &ctx);
   uint32_t N = slang_coverage_counter_count(ctx);
   const SlangCoverageBindingInfo* binding = slang_coverage_binding(ctx);
   // binding->space, binding->binding → where to bind the SSBO
   ```

3. **Allocate + bind the SSBO** using standard Vulkan calls:
   ```c
   VkBufferCreateInfo bufInfo = { .size = N * 4, .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, ... };
   VkBuffer coverageBuffer;
   vkCreateBuffer(device, &bufInfo, nullptr, &coverageBuffer);
   // bind at (binding->space, binding->binding) via VkWriteDescriptorSet
   ```

4. **Dispatch normally**, then barrier + copy-to-staging + map to read
   counters back:
   ```c
   vkCmdDispatch(cmd, groupsX, groupsY, groupsZ);
   vkCmdPipelineBarrier(/* COMPUTE_SHADER → TRANSFER */);
   vkCmdCopyBuffer(cmd, coverageBuffer, stagingBuffer, ...);
   vkMapMemory(device, stagingMem, 0, N * 4, 0, &mapped);
   ```

5. **Feed the counters to `slang-coverage-rt`** to accumulate and emit
   LCOV:
   ```c
   slang_coverage_accumulate(ctx, (uint32_t*)mapped, N);
   slang_coverage_save_lcov(ctx, "coverage.lcov", "test_run");
   ```

Total integration cost for a typical Vulkan engine: ~30 lines of
additional host code, no new runtime dependency beyond
`libslang-coverage-rt`.

## History / design notes

Earlier revisions of this feature synthesized the `__slang_coverage`
buffer at IR-pass time — after Slang's AST-derived reflection tree
had already been frozen. That meant `ShaderCursor["__slang_coverage"]`
returned an invalid cursor on all backends, the buffer was never
actually bound at dispatch, and the demo's dispatch mode produced
zeroed counters. The architectural fix was to synthesize the buffer
as an AST-level `VarDecl` during semantic checking, before parameter
binding runs, so that every downstream layer (reflection, layout,
target codegen) treats it identically to a user-declared global.
That change lives in `source/slang/slang-check-synthesize-coverage.{h,cpp}`.

The practical consequence: any backend that correctly handles a
user-declared `RWStructuredBuffer<uint>` + `kIROp_AtomicAdd` also
correctly handles coverage. Validated empirically on CPU and Vulkan.

## Scenarios (future expansion)

A follow-up will add `--scenario=fluid-only|mixed|edge-cases` so the
demo generates distinct particle inputs and shows a gcov-style story
of *coverage percentages rising as the test suite expands*. Straight-
forward extension now that dispatch works on CPU and Vulkan.
