# Prompt: docs/generated/tests/conformance/basics-execution-divergence-reconvergence/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/basics-execution-divergence-reconvergence/`,
anchored to
[`docs/language-reference/basics-execution-divergence-reconvergence.md`](../../../../language-reference/basics-execution-divergence-reconvergence.md).

## Doc summary

The doc (≈189 lines) defines execution divergence and reconvergence for
wave/subgroup execution in Slang. Claims fall into four sections:

1. **Preamble** (lines 1–31): Definitions — uniform path, convergence scopes
   (thread-group-uniform and wave-uniform), mutually-convergent thread set.
   These are definitional; the only testable claim here is the two-scope
   enumeration.

2. **Divergence and Reconvergence in Structured Control Flow** (`#divergence`,
   lines 32–152): Normative claims about `if`, `switch`, and `for`/loop
   statements — when divergence occurs and where reconvergence happens. The
   doc describes the structural control flow behaviour; these are observable
   in SPIRV/HLSL/GLSL emission via `OpSelectionMerge`, `OpSwitch`,
   `OpLoopMerge`, etc.

3. **Thread-Group-Tangled Functions on Divergent Paths** (lines 153–159):
   One normative claim: such functions may only be called on thread-group-
   uniform paths; calling them on divergent paths is undefined behaviour.
   No slang-test directive can directly observe UB; however the compile-
   time capability-check path may surface for specific target + function
   combinations.

4. **Wave-Tangled Functions on Divergent Paths** (lines 160–189): Two
   normative claims: (a) not all targets support them; (b) when supported,
   only mutually-convergent threads participate. These are observable in
   SPIRV/HLSL/GLSL emission — wave ops inside a divergent `if` appear in
   separate SPIR-V merge blocks, HLSL emits `WaveActiveMin/Max` inside
   `if` arms, GLSL emits `subgroupMin/Max` inside `if` arms.

## Claim extraction strategy

- Definitional remarks (Remarks 1–4 in preamble) are non-normative; skip.
- Control-flow-structure claims (if/switch/loop) are normative but their
  runtime behavior is only observable on a GPU. Prefer emission tests that
  pin the structural SPIRV opcodes (`OpSelectionMerge`, `OpSwitch`,
  `OpLoopMerge`).
- Wave-op claims inside divergent branches are observable in emission on
  any target that supports the wave op.
- Thread-group-tangled UB claim has no slang-test-observable surface;
  mark as untested with `gpu-other`.

## What NOT to test here

- Actual wave-op result values (WaveActiveMin, WaveActiveMax) — require a
  GPU runner. Write `COMPARE_COMPUTE -vk` tests for CI; mark locally-ignored.
- Thread-group-tangled functions (barriers, GroupMemoryBarrier) on divergent
  paths — undefined behavior is not testable via assertion; mark untested.
- Uniformity analysis diagnostics — the doc does not specify which
  diagnostic fires when; skip.
