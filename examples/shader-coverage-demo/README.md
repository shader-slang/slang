# shader-coverage-demo

End-to-end demonstration of Slang's `-trace-coverage` feature and the
host-side helper library `slang-coverage-rt`. Runs in two modes:

- **`--mode=compile`** — Compiles `simulate.slang` via slang-rhi with
  `-trace-coverage` pinned on the session. The coverage pass runs,
  writes a `.slangcov` manifest next to the invocation, and reports
  the counter count. Exercises the compile half of the pipeline.
- **`--mode=report`** — Takes an existing manifest plus a binary
  counter buffer (little-endian `uint32_t` per counter), accumulates
  the hits via `slang-coverage-rt`, and writes an LCOV `.info` file
  consumable by `genhtml`, Codecov, VS Code Coverage Gutters, etc.
  Exercises the library/report half of the pipeline.

- **`--mode=dispatch`** — Full compile → bind → dispatch → readback →
  LCOV pipeline via slang-rhi. Works end-to-end on Metal
  (architecture empirically validated). The shader executes with
  its instrumentation, but because the synthesized coverage buffer
  is not yet reflection-visible, the `__slang_coverage` `ShaderCursor`
  comes back invalid and the counter buffer is never bound to the
  shader — so the resulting LCOV shows unreliable/zero hit counts.
  The dispatch mode is kept in the demo precisely to **expose that
  reflection gap concretely** — see *Known limitations*.

## Usage

```bash
# 1. Compile the shader. Writes `simulate.slangcov` in the current
#    working directory.
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
open coverage-html/index.html
```

## What the shader exercises

`simulate.slang` is a tiny particle-physics compute kernel that
branches on particle type — `FLUID`, `GAS`, `SOLID`, and an unreachable
"unknown" error path. Different input mixes exercise different
branches, which makes the coverage numbers meaningful: running a
scenario with only FLUID particles leaves the GAS and SOLID branches
uncovered. The unreachable branch stays uncovered regardless of the
scenario, demonstrating that the tool does spot dead code the way
gcov does for CPU programs.

## Known limitations (follow-ups)

**Dispatch-through-slang-rhi binds no counters.** The coverage IR
pass synthesizes an implicit `RWStructuredBuffer<uint> __slang_coverage`
*after* slang-rhi has built its reflection view of the module, which
means neither `ShaderCursor` nor the pipeline-layout builder is aware
of the extra parameter.

The demo detects this at runtime — `cursor["__slang_coverage"]`
returns an invalid cursor under current slang-rhi — and prints:
```
[dispatch] NOTE: `__slang_coverage` not found in reflection;
[dispatch] the shader will still run but counters will stay 0.
```

The fix is to make the coverage synthesis *reflection-visible* — the
pass should register the buffer in the same layout structures that
`collectGlobalUniformParameters` builds, so slang-rhi's automatic
bindings pick it up. That's a small patch to the main compiler; it
lives in the follow-up work log on shader-slang/slang#10794.

### Backend-specific status matrix

| Backend | `--mode=compile` | `--mode=dispatch` |
|---|---|---|
| `cpu` | Works (manifest produced). LLVM codegen then reports a 3-vs-4 function-argument mismatch for the same reflection reason; the demo catches the shutdown abort. | Fails with the LLVM mismatch at pipeline build. |
| `metal` | Works. Generates valid `atomic_fetch_add_explicit` on the synthesized `_slang_coverage_0` buffer. | Pipeline builds; shader dispatches; counters not bound (reflection gap) → LCOV shows zeros / stale memory. |
| `vulkan` | Untested on this branch. | Untested. |
| `d3d12` | Untested. | Untested. |

### Mode behaviour summary

- **`--mode=compile`** works on all tested backends and produces a
  valid manifest.
- **`--mode=report`** works with any external counter buffer (hand-
  generated, captured from `slang-test`, or future integrations).
- **`--mode=dispatch`** runs architecturally on Metal (pipeline →
  dispatch → readback → LCOV all complete), but counter values are
  unreliable until the reflection gap is fixed.

## Scenarios (future expansion)

A follow-up will add `--scenario=fluid-only|mixed|edge-cases` so the
demo generates distinct particle inputs and shows a gcov-style story
of *coverage percentages rising as the test suite expands*. Blocked
on the dispatch path above.
