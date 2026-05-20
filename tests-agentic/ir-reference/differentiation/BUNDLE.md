---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:32:24+00:00
source_commit: 330c9a8d807b9f9352e4754f466d1244ae681cff
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/llm-generated/ir-reference/differentiation.md
source_doc_digest: 2c875ce08ca0ca9816c8fcc45eae38c08e60f97ad6a1cfbcd7867a8bddf21303
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/differentiation

## Intent

Tests verify the per-opcode catalog of the IR differentiation family
described in
[`docs/llm-generated/ir-reference/differentiation.md`](../../../docs/llm-generated/ir-reference/differentiation.md):
that each user-observable differentiation opcode appears in
`-dump-ir` output for the obvious AST surface that produces it
(`__fwd_diff(f)` / `fwd_diff(f)`, `__bwd_diff(f)` / `bwd_diff(f)`,
`DifferentialPair<T>(p, d)`, `.p` / `.d` projections, `detach(x)`,
`no_diff T x` parameter markers).

The primary observation mechanism is `-target spirv-asm -dump-ir -o
/dev/null -entry main -stage compute` followed by a FileCheck
against the LOWER-TO-IR section. Anchors are user-named symbols
(`func %main`, `func %f`, `func %helperFunc`, `func %blockDeriv`,
`func %scale`, `let %fwdx5Fdiff`) — the IR-dump preamble for
autodiff is very large (every `IDifferentiable` /
`IForwardDifferentiable` / `IBackwardDifferentiable` interface,
every key, every builtin float-family witness) and any
unanchored pattern risks false positives.

The angle distinguishing this bundle from `ir-reference/structure`
(which anchors `func`/`interface`/`witness_table` opcodes) and
`ir-reference/types` (which anchors the differential-pair *type*
family) is: **the differentiation-operator and pair-construction/
projection axis** — every test observes either an autodiff opcode
or the `no_diff` / `detach` markers that participate in the
autodiff system. The internal autodiff-pass opcodes
(`BackwardDifferentiate`, `BackwardDifferentiatePropagate`,
`checkpointObj`, `loopExitValue`, `PrimalParamRef`, etc.) are not
user-observable at LOWER-TO-IR and are recorded as out of scope.

## Claims enumerated

| Claim ID | Anchor | Claim (one line) | Tests |
| --- | --- | --- | --- |
| C-01 | [#makediffpair](../../../docs/llm-generated/ir-reference/differentiation.md#makediffpair) | A `DifferentialPair<T>(primal, diff)` constructor lowers to a `MakeDiffPair(%primal, %diff)` IR value. | `make-diff-pair.slang` |
| C-02 | [#differential-pair-projection](../../../docs/llm-generated/ir-reference/differentiation.md#differential-pair-projection) | The `.p` projection on a `DifferentialPair<T>` value lowers to `GetPrimal(%pair)`. | `get-primal.slang` |
| C-03 | [#differential-pair-projection](../../../docs/llm-generated/ir-reference/differentiation.md#differential-pair-projection) | The `.d` projection on a `DifferentialPair<T>` value lowers to `GetDifferential(%pair)`. | `get-differential.slang` |
| C-04 | [#forwarddifferentiate](../../../docs/llm-generated/ir-reference/differentiation.md#forwarddifferentiate) | `__fwd_diff(f)` on a `[ForwardDifferentiable]` function lowers to a `ForwardDifferentiate(%f)` IR value whose result type is the JVP `Func(DiffPair, DiffPair)` signature. | `forward-differentiate.slang` |
| C-05 | [#forwarddifferentiate](../../../docs/llm-generated/ir-reference/differentiation.md#forwarddifferentiate) | Two source occurrences of `__fwd_diff(f)` dedupe to a single hoistable `let %fwd_diff = ForwardDifferentiate(%f)` binding that both call sites share. | `forward-differentiate-dedupes.slang` |
| C-06 | [#forward-mode](../../../docs/llm-generated/ir-reference/differentiation.md#forward-mode) | `ForwardDifferentiate` has a single `baseFn` operand, named as the user `func %name`. | `forward-differentiate-base-fn-operand.slang` |
| C-07 | [#forwarddifferentiate](../../../docs/llm-generated/ir-reference/differentiation.md#forwarddifferentiate) | The unprefixed `fwd_diff(f)` alias also lowers to `ForwardDifferentiate(%f)`. | `fwd-diff-alias.slang` |
| C-08 | [#legacy-bridge](../../../docs/llm-generated/ir-reference/differentiation.md#legacy-bridge) | `__bwd_diff(f)` on a `[BackwardDifferentiable]` function lowers at LOWER-TO-IR to `LegacyBackwardDifferentiate(%apply_bwd, %remat, %ctx_t)` (not the modern `BackwardDifferentiate` opcode). | `legacy-backward-differentiate.slang` |
| C-09 | [#legacy-bridge](../../../docs/llm-generated/ir-reference/differentiation.md#legacy-bridge) | The unprefixed `bwd_diff(f)` alias also lowers to `LegacyBackwardDifferentiate(...)` at LOWER-TO-IR. | `bwd-diff-alias.slang` |
| C-10 | [#detachderivative](../../../docs/llm-generated/ir-reference/differentiation.md#detachderivative) | `detach(x)` inside a differentiable function lowers to `let %N : T = detachDerivative(%x)`. | `detach-derivative.slang` |
| C-11 | [#detachderivative](../../../docs/llm-generated/ir-reference/differentiation.md#detachderivative) | `detachDerivative` returns its operand unchanged: its result type equals the operand type. | `detach-derivative-result-type.slang` |
| C-12 | [#opcodes](../../../docs/llm-generated/ir-reference/differentiation.md#opcodes) | A `no_diff T x` parameter on a differentiable function surfaces as `param %x : Attributed(T, %no_diff)` on the entry block. | `no-diff-parameter-marker.slang` |
| C-13 | [#opcodes](../../../docs/llm-generated/ir-reference/differentiation.md#opcodes) | The `no_diff` marker is a unique module-scope `let %k : Void = no_diff` opcode; every `Attributed(T, %k)` parameter wrapper references it. | `no-diff-module-scope-marker.slang` |
| C-14 | [#makediffpair](../../../docs/llm-generated/ir-reference/differentiation.md#makediffpair) | The result type of `MakeDiffPair` is `DiffPair(T, %witness)` where `%witness` is the `IDifferentiable` witness for `T`. | `diff-pair-result-type.slang` |
| C-15 | [#makediffpair](../../../docs/llm-generated/ir-reference/differentiation.md#makediffpair) | `MakeDiffPair` works for vector primal/differential types; result is `DiffPair(Vec(T, N), witness)`. | `make-diff-pair-vector.slang` |
| C-16 | [#opcodes](../../../docs/llm-generated/ir-reference/differentiation.md#opcodes) | A single entry point that constructs a `DifferentialPair` and reads both `.p` and `.d` exercises `MakeDiffPair`, `GetPrimal`, and `GetDifferential` together. | `make-diff-pair-and-projections.slang` |

## Tests in this bundle

| File | Intent | Doc anchor |
| --- | --- | --- |
| `make-diff-pair.slang` | functional | `#makediffpair` |
| `get-primal.slang` | functional | `#differential-pair-projection` |
| `get-differential.slang` | functional | `#differential-pair-projection` |
| `forward-differentiate.slang` | functional | `#forwarddifferentiate` |
| `forward-differentiate-dedupes.slang` | functional | `#forwarddifferentiate` |
| `forward-differentiate-base-fn-operand.slang` | functional | `#forward-mode` |
| `fwd-diff-alias.slang` | functional | `#forwarddifferentiate` |
| `legacy-backward-differentiate.slang` | functional | `#legacy-bridge` |
| `bwd-diff-alias.slang` | functional | `#legacy-bridge` |
| `detach-derivative.slang` | functional | `#detachderivative` |
| `detach-derivative-result-type.slang` | functional | `#detachderivative` |
| `no-diff-parameter-marker.slang` | functional | `#opcodes` |
| `no-diff-module-scope-marker.slang` | functional | `#opcodes` |
| `diff-pair-result-type.slang` | functional | `#makediffpair` |
| `make-diff-pair-vector.slang` | functional | `#makediffpair` |
| `make-diff-pair-and-projections.slang` | functional | `#opcodes` |

## Out of scope (no-GPU runner)

The following opcodes are listed in `differentiation.md` but are
either produced only by internal autodiff passes (not at
LOWER-TO-IR from natural surface code) or have no AST origin that
a user can write. They are intentionally not tested here:

- **`MakeDiffRefPair`** — `(synthesized)`; pointer-typed primal/
  differential pair used inside autodiff-pass plumbing.
- **`GetDifferentialPtr`**, **`GetPrimalRef`** — `(synthesized)`
  pointer-projection counterparts; no natural user surface.
- **`ForwardDifferentiatePropagate`**, **`TrivialForwardDifferentiate`**
  — `(synthesized)`; produced by the unzip / transcribe pipeline.
- **`BackwardDifferentiate`** itself — the doc lists `__bwd_diff`
  as its AST origin, but LOWER-TO-IR actually emits
  `LegacyBackwardDifferentiate`. The modern opcode appears only
  after later passes. See `legacy-backward-differentiate.slang`
  and the doc gap below.
- **`BackwardDifferentiatePrimal`**, **`BackwardDifferentiatePropagate`**,
  **`BackwardRemat`**, **`TrivialBackwardDifferentiate*`** —
  `(synthesized)` by the unzip pass.
- **`BackwardFromLegacyBwdDiffFunc`** / `BackwardPrimalFromLegacyBwdDiffFunc`
  / `BackwardRematFromLegacyBwdDiffFunc` /
  `BackwardPropagateFromLegacyBwdDiffFunc` — `(synthesized)`
  legacy-bridge extraction opcodes.
- **`FunctionCopy`**, **`SynthesizedForwardDerivativeWitnessTable`**,
  **`SynthesizedBackwardDerivativeWitnessTable`**,
  **`MakeIDifferentiableWitness`**,
  **`SynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc`**
  — derivative-witness synthesis, internal. (Note:
  `SynthesizedForwardDerivativeWitnessTable` and
  `SynthesizedBackwardDerivativeWitnessTable` do happen to appear
  near the LOWER-TO-IR output for every user `[Forward/Backward
  Differentiable]` function, but the doc lists them as
  `(synthesized)` with no AST origin; their presence is a
  side-effect of the lowering, not a documented surface mapping.)
- **`LoadReverseGradient`**, **`ReverseGradientDiffPairRef`**,
  **`PrimalParamRef`**, **`DiffParamRef`** — autodiff temporaries
  that do not survive past the splitting / back-prop pass.
- **`DiffTypeInfo`** — `(synthesized)` type-info container,
  internal.
- **`checkpointObj`**, **`loopExitValue`**, **`ReportCheckpointStore`**
  — checkpointing markers inserted by the reverse-mode pass; no
  natural surface form at LOWER-TO-IR.

## Doc gaps observed

- The `### BackwardDifferentiate` notable-opcode discussion names
  `__bwd_diff` as the AST origin of `BackwardDifferentiate`, but
  the actual LOWER-TO-IR opcode emitted by
  `slang-lower-to-ir.cpp` for `__bwd_diff(f)` is
  `LegacyBackwardDifferentiate(%apply_bwd, %remat, %ctx_t)` — the
  legacy-bridge form. The modern `BackwardDifferentiate` opcode is
  not produced at LOWER-TO-IR at all; it is synthesized later by
  the unzip pass when converting the legacy form to the modern
  primal-/propagate-/remat-triple. A one-line note clarifying
  that `BackwardDifferentiate` is an unzip-pass opcode (and that
  `LegacyBackwardDifferentiate` is the LOWER-TO-IR spelling) would
  prevent test-author confusion. (See
  `legacy-backward-differentiate.slang` and `bwd-diff-alias.slang`.)
- The `### Differential-pair construction` and
  `### Differential-pair projection` tables list every opcode's
  AST origin as `(synthesized)`, but `MakeDiffPair`, `GetPrimal`,
  and `GetDifferential` are all routinely produced from
  user-written surface code (`DifferentialPair<T>(p, d)`
  constructor calls and `.p`/`.d` field accesses). The
  AST-origin column should distinguish "synthesized by autodiff
  passes" from "naturally produced from user surface at
  LOWER-TO-IR". (See `make-diff-pair.slang`,
  `get-primal.slang`, `get-differential.slang`.)
- The doc nowhere documents the **`no_diff`** parameter marker as
  an IR opcode. At LOWER-TO-IR, `no_diff T x` parameters surface
  as `param %x : Attributed(T, %no_diff)` where `%no_diff` is a
  module-scope `let %k : Void = no_diff` IR value. The
  `no_diff` opcode should appear in the catalog (alongside or in
  the `Autodiff temporaries` table) with its `Attributed(...)`
  parameter-wrapper role spelled out. (See
  `no-diff-parameter-marker.slang`,
  `no-diff-module-scope-marker.slang`.)
- The `### ForwardDifferentiate` discussion says "Specialization
  replaces the opcode with the actual JVP function once `baseFn`
  is fully known" but does not document the LOWER-TO-IR form
  before specialization (`let %fwd_diff : Func(DiffPair(T, %w),
  DiffPair(T, %w)) = ForwardDifferentiate(%f)`). A one-line
  example showing the pre-specialization shape would anchor
  reference tests. (See `forward-differentiate.slang`.)
- The `### detachDerivative` discussion describes the semantic
  (returns operand unchanged, blocks derivative) but does not
  show the LOWER-TO-IR form (`let %N : T = detachDerivative(%x)`
  with operand and result type equal). A one-line example would
  prevent over-specific pattern construction. (See
  `detach-derivative.slang`, `detach-derivative-result-type.slang`.)
- The doc does not describe the **`fwd_diff` / `bwd_diff`
  unprefixed aliases**. The parser accepts both `__fwd_diff(f)`
  and `fwd_diff(f)` (similarly for `bwd_diff`); both lower
  identically. A note that the unprefixed aliases exist would
  clarify the surface. (See `fwd-diff-alias.slang`,
  `bwd-diff-alias.slang`.)
