# Prompt: docs/generated/tests/conformance/basics-program-execution/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/basics-program-execution/`,
anchored to
[`docs/language-reference/basics-program-execution.md`](../../../../language-reference/basics-program-execution.md).

## Claim extraction strategy

The source doc (~102 lines, three top-level sections) is primarily an
execution-model description. Most sentences describe the _model_ (what
conceptually happens) rather than compiler-observable syntax. The
extraction strategy is:

1. Treat a sentence as a **testable claim** only when Slang syntax or
   an intrinsic directly exposes the behaviour at compile time (emission)
   or runtime (compute execution).
2. Treat pure model-description sentences (e.g. "a wave is a group of
   threads", "graphics invocations are target-defined") as
   **non-normative** unless they name a specific intrinsic, attribute,
   or keyword.
3. When a claim names an intrinsic or keyword, write an **emission
   pair**: HLSL + SPIR-V directives on the same test file using
   distinct filecheck prefixes (CHECK / CHECK-SPV).
4. When a claim has a runtime-observable dimension via CPU compute,
   add a COMPARE_COMPUTE -cpu functional test as well.

## High-value testable claims

- **Thread coordinates** (`SV_DispatchThreadID`, `SV_GroupID`,
  `SV_GroupThreadID`): the compute dispatch grid structure maps these
  system values to entry-point parameters.
- **`[numthreads(x,y,z)]` attribute**: the group dimensions are
  preserved in emitted HLSL and SPIR-V (LocalSize).
- **Invocation range**: with numthreads(N,1,1), tid.x lies in [0, N).
- **`groupshared` attribute**: emits as groupshared (HLSL) /
  Workgroup storage class (SPIR-V).
- **`GroupMemoryBarrier()` / `GroupMemoryBarrierWithGroupSync()`**:
  both barriers compile and emit in HLSL/SPIR-V.
- **Wave size via `WaveGetLaneCount()`**: emits SubgroupSize in SPIR-V,
  WaveGetLaneCount in HLSL.
- **Wave-tangled ballots** (`WaveActiveBallot`): emits
  OpGroupNonUniformBallot in SPIR-V.
- **Wave-tangled reductions** (`WaveActiveSum`): emits
  OpGroupNonUniformIAdd Reduce in SPIR-V.
- **`discard` statement** (fragment-only inactive thread): emits
  `discard` in HLSL, OpKill in SPIR-V.

## What NOT to test here

- Wave shapes (adjacent vs non-adjacent) — target-defined, not
  observable.
- Partial wave fill-up — scheduling detail, not observable.
- "Thread group model applies only to compute kernels" — the Slang
  compiler doesn't reject groupshared in other stages; it's a semantic
  model statement, not a compile-time gate.
- SIMT model description (Remark 1/2) — non-normative remarks.
- Lockstep execution semantics — not observable without a
  synchronization correctness harness.
- Helper thread derivatives — GPU-only, no direct intrinsic to test.
- Graphics launch invocations (fragment, vertex) that are not the
  discard claim — target-defined and not testable via slang-test CPU.
