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
  LCOV pipeline via slang-rhi. On the CPU backend this produces a
  complete, accurate coverage report (see *Backend status matrix*
  below). On Metal the dispatch completes and entry-block counters
  are correct, but some counter slots inside nested function calls
  currently return deterministic-but-wrong values; this is an
  active investigation.

## Usage

### Everything in one shot (CPU)

```bash
./build/Debug/bin/shader-coverage-demo --mode=dispatch --backend=cpu
genhtml coverage.lcov -o coverage-html/
open coverage-html/index.html
```

### Separate compile + report (any backend)

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

### Sample CPU coverage report

After `--mode=dispatch --backend=cpu` followed by `genhtml`, you'll
see a per-line report like this (reformatted from the actual HTML):

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
| `cpu` | ✅ Works | ✅ **Fully working** — produces correct counter values and a complete LCOV report |
| `metal` | ✅ Works | ⚠️ Pipeline builds; dispatch runs; only counters *before the `if (i >= particleCount) return;` guard* are written. Diagnosed by seeding the counter buffer with `0xDEADBEEF`: counter 16 (inside the guard body) shows 512 writes while counters for the rest of `computeMain` remain at the sentinel. That means every Metal thread takes the early-return branch — i.e. `particleCount` is being read as 0. Root cause is that `cursor["Params"]["particleCount"].setData(...)` on Metal doesn't deliver the constant-buffer value to the shader. **Not a coverage-feature issue** — belongs against slang-rhi's Metal backend. Force-inlining the helpers was tried and didn't help (verifying the bug isn't helper-specific). |
| `vulkan` | Untested on this branch | Untested |
| `d3d12` | Untested | Untested |

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

## Scenarios (future expansion)

A follow-up will add `--scenario=fluid-only|mixed|edge-cases` so the
demo generates distinct particle inputs and shows a gcov-style story
of *coverage percentages rising as the test suite expands*. Straight-
forward extension now that dispatch works on CPU.
