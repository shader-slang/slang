---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T12:00:00+00:00
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: 415a50cb05f7c62902b743f24a4fc8a5a1e26f0059d645d4973f127e8f110ee4
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/autodiff

White-box characterization tests targeting under-exercised branches in the
automatic-differentiation IR passes: `source/slang/slang-ir-autodiff-fwd.cpp`
(~35% covered, forward-mode transcription), `slang-ir-autodiff-transpose.cpp`
(~25%, reverse-mode transpose), and `slang-ir-autodiff-primal-hoist.cpp` (~44%,
primal checkpointing / loop-state hoisting). These pin the compiler's _current
observed behaviour_ (not a spec). The design doc
`ir-reference/differentiation.md` is the manifest `source_doc`; the real
white-box target is named in each test's `//META: covers=` field.

## Intent

Drive `__fwd_diff` / `__bwd_diff` over differentiable functions with control
flow (if/else, bounded loops), nested calls, vector inputs, and `IDifferentiable`
structs, and classify the actual result. Two vehicles are used:

- **Structural emission** (`//TEST:SIMPLE -target hlsl/cuda`): pins the stable
  tokens the passes synthesise — the forward-derivative function `s_fwd_<f>`,
  the differential-pair struct `DiffPair_float_*` with `primal_*`/`differential_*`
  fields, the reverse propagation function `s_bwdProp_<f>`, and the callable
  context struct `s_bwdCallableCtx_<f>`. FileCheck is absent locally so these
  report `ignored` on verify; CI validates them.
- **Runtime value** (`//TEST:COMPARE_COMPUTE -cpu`): pins the derivative the CPU
  runtime computes. This validates locally. Because the autodiff result is hard
  to confirm against an independent oracle (only the compiler's own output is
  available here), every value test carries
  `//META: characterization-unverified=true`; a wrong derivative is a finding,
  not a green test.

INTERPRET / `slangi` is **not** used for the reverse-mode value tests: every
`__bwd_diff` call aborts the bytecode VM (see Findings below), while the
identical function runs correctly on `-cpu`. Forward-mode runs correctly on
`slangi`, but `-cpu` is used uniformly for value tests so the suite has one
locally-validating vehicle.

## Functional coverage

| Test                                                                                     | What it pins                                                                                                                                           | covers=                                         |
| ---------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ | ----------------------------------------------- |
| [`fwd-square-emits-fwd-derivative-fn.slang`](fwd-square-emits-fwd-derivative-fn.slang)   | `__fwd_diff` over `x*x` emits forward-derivative fn `s_fwd_f_0` and a `DiffPair_float_0` struct with `primal_0`/`differential_0` fields (HLSL + CUDA). | source/slang/slang-ir-autodiff-fwd.cpp          |
| [`fwd-ifelse-branch-derivative.slang`](fwd-ifelse-branch-derivative.slang)               | `__fwd_diff` over an if/else propagates the tangent through the taken branch: primal 8, tangent 12 at x=2 for `x*x*x`.                                 | source/slang/slang-ir-autodiff-fwd.cpp          |
| [`fwd-vector-dot-tangent.slang`](fwd-vector-dot-tangent.slang)                           | `__fwd_diff` over `dot(v,v)` with a float3 tangent yields primal 14, directional derivative 12.                                                        | source/slang/slang-ir-autodiff-fwd.cpp          |
| [`bwd-square-emits-bwd-prop-fn.slang`](bwd-square-emits-bwd-prop-fn.slang)               | `__bwd_diff` over `x*x` emits backward-propagation fn `s_bwdProp_f_0` and callable-context struct `s_bwdCallableCtx_f_0` (HLSL).                       | source/slang/slang-ir-autodiff-transpose.cpp    |
| [`bwd-ifelse-branch-grad.slang`](bwd-ifelse-branch-grad.slang)                           | `__bwd_diff` over an if/else reverses the taken branch: gradient 12 at x=2 for `x*x*x` with seed 1.0.                                                  | source/slang/slang-ir-autodiff-transpose.cpp    |
| [`bwd-nested-call-grad.slang`](bwd-nested-call-grad.slang)                               | `__bwd_diff` differentiates through a nested call `g(x)+x` (g=x\*x): chain-rule gradient 7 at x=3.                                                     | source/slang/slang-ir-autodiff-transpose.cpp    |
| [`bwd-struct-field-grads.slang`](bwd-struct-field-grads.slang)                           | `__bwd_diff` over an `IDifferentiable` struct `f(s)=s.a*s.b` accumulates per-field gradients (a=5, b=2) into the struct Differential.                  | source/slang/slang-ir-autodiff-transpose.cpp    |
| [`bwd-for-loop-maxiters-grad.slang`](bwd-for-loop-maxiters-grad.slang)                   | `__bwd_diff` over a `[MaxIters(3)]` accumulating loop hoists loop-carried primal state and yields gradient 12 (6x at x=2).                             | source/slang/slang-ir-autodiff-primal-hoist.cpp |
| [`bwd-loop-emits-reverse-prop-context.slang`](bwd-loop-emits-reverse-prop-context.slang) | `__bwd_diff` over a bounded loop emits `s_bwdProp_poly_0`, context struct `s_bwdCallableCtx_poly_0`, and a re-emitted reverse loop (`for(;;)`) (HLSL). | source/slang/slang-ir-autodiff-primal-hoist.cpp |

## Unreachable gaps

| Gap                                                                                                                               | Why not targeted                                                                                                                                                                                                                                                                                                                 |
| --------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `SLANG_UNEXPECTED("unexpected pair type")` / `SLANG_UNEXPECTED("unknown make differential pair op")` in slang-ir-autodiff-fwd.cpp | Defensive aborts on IR pair-shapes the upstream lowering never produces for well-formed `[Differentiable]` input; not driveable to a clean result from the CLI.                                                                                                                                                                  |
| `Diagnostics::InternalCompilerError` sites in slang-ir-autodiff-fwd.cpp (arithmetic transcription, call transcription)            | Defensive internal-error paths guarding inst shapes the transcriber should never receive after type-check; reaching them would itself be a compiler bug, so a finding (not a test) is the correct artefact.                                                                                                                      |
| `Diagnostics::Unimplemented` literal/inst fall-through in fwd transcription                                                       | Reachable only with IR opcodes the front-end does not emit inside a differentiable function body at this commit; would require a future/unsupported construct to trigger.                                                                                                                                                        |
| `EncounteredNonDifferentiableFunctionDuringHigherOrderDiff` and `CannotMixDifferentiableValueAndPtrOutputs`                       | Reachable diagnostics, but they belong to a check/legalization-adjacent surface already exercised by the dedicated `tests/autodiff/` suite (e.g. higher-order-diff and ptr-output negatives); deferred to keep this bundle on the structural transcription/transpose/hoist hint area rather than re-covering existing negatives. |
| Reverse-mode value coverage via `slangi`/INTERPRET                                                                                | Blocked by a VM abort on every `__bwd_diff` call (filed finding `slangi-bwd-diff-working-set-oob`); `-cpu` COMPARE_COMPUTE is used instead.                                                                                                                                                                                      |

## Doc gaps observed

| Anchor                                                                                                                      | Kind            | Gap                                                                                                                                                                                                                                                                                                                                                                          | Suggested addition                                                                                                                                                                                                 |
| --------------------------------------------------------------------------------------------------------------------------- | --------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [#reverse-mode](../../../design/ir-reference/differentiation.md#reverse-mode)                                               | missing-surface | The doc describes the `BackwardDifferentiate` / `BackwardDifferentiatePropagate` opcodes but never names the user-visible emitted artefacts the transpose pass produces — the `s_bwdProp_<f>` propagation function and the `s_bwdCallableCtx_<f>` callable-context struct that carries hoisted primal state — so a reader cannot tell what to expect in generated HLSL/CUDA. | Add a short "emitted form" note under Reverse-mode naming the `s_bwdProp_<orig>` function and `s_bwdCallableCtx_<orig>` context struct (and `s_fwd_<orig>` for forward-mode), with a one-line example of each.     |
| [#checkpointing-and-rematerialization](../../../design/ir-reference/differentiation.md#checkpointing-and-rematerialization) | missing-surface | The doc covers checkpoint/rematerialization opcodes but does not state the user-surface requirement that a loop inside a differentiable function must carry `[MaxIters(n)]` or `[ForceUnroll]` (E30510), nor that the reverse pass re-emits the loop.                                                                                                                        | Add a "loops in differentiable functions" subsection noting the `[MaxIters]`/`[ForceUnroll]` requirement and the diagnostic code, since this is the first thing a reader hits when reverse-differentiating a loop. |
