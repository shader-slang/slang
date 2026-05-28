---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:32:24+00:00
source_commit: 330c9a8d807b9f9352e4754f466d1244ae681cff
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/generated/design/ir-reference/differentiation.md
source_doc_digest: 2c875ce08ca0ca9816c8fcc45eae38c08e60f97ad6a1cfbcd7867a8bddf21303
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/differentiation

## Intent
Tests verify the per-opcode catalog of the IR differentiation family
described in
[`docs/generated/design/ir-reference/differentiation.md`](../../../design/ir-reference/differentiation.md):
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


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| detach(x) inside a differentiable function lowers to detachDerivative(%x). | functional | [#detachderivative](../../../design/ir-reference/differentiation.md#detachderivative) | [`detach-derivative.slang`](detach-derivative.slang) |
| detachDerivative returns its operand unchanged with the same scalar result type at LOWER-TO-IR. | functional | [#detachderivative](../../../design/ir-reference/differentiation.md#detachderivative) | [`detach-derivative-result-type.slang`](detach-derivative-result-type.slang) |
| Reading .d off a returned DifferentialPair lowers to GetDifferential(%pair). | functional | [#differential-pair-projection](../../../design/ir-reference/differentiation.md#differential-pair-projection) | [`get-differential.slang`](get-differential.slang) |
| Reading .p off a returned DifferentialPair lowers to GetPrimal(%pair). | functional | [#differential-pair-projection](../../../design/ir-reference/differentiation.md#differential-pair-projection) | [`get-primal.slang`](get-primal.slang) |
| ForwardDifferentiate has exactly one operand (baseFn): the user-named func value to differentiate. | functional | [#forward-mode](../../../design/ir-reference/differentiation.md#forward-mode) | [`forward-differentiate-base-fn-operand.slang`](forward-differentiate-base-fn-operand.slang) |
| ForwardDifferentiate is hoistable so two syntactic __fwd_diff(f) occurrences on the same base function dedupe to one ForwardDifferentiate(%f) IR value. | functional | [#forwarddifferentiate](../../../design/ir-reference/differentiation.md#forwarddifferentiate) | [`forward-differentiate-dedupes.slang`](forward-differentiate-dedupes.slang) |
| The unprefixed fwd_diff(f) form also lowers to ForwardDifferentiate(%f), same as __fwd_diff. | functional | [#forwarddifferentiate](../../../design/ir-reference/differentiation.md#forwarddifferentiate) | [`fwd-diff-alias.slang`](fwd-diff-alias.slang) |
| __fwd_diff(f) on a [ForwardDifferentiable] function lowers to a ForwardDifferentiate(%f) IR value whose result type is the JVP signature Func(DiffPair, DiffPair). | functional | [#forwarddifferentiate](../../../design/ir-reference/differentiation.md#forwarddifferentiate) | [`forward-differentiate.slang`](forward-differentiate.slang) |
| The unprefixed bwd_diff(f) form also lowers to LegacyBackwardDifferentiate at LOWER-TO-IR, same as __bwd_diff. | functional | [#legacy-bridge](../../../design/ir-reference/differentiation.md#legacy-bridge) | [`bwd-diff-alias.slang`](bwd-diff-alias.slang) |
| __bwd_diff(f) on a [BackwardDifferentiable] function lowers at LOWER-TO-IR to LegacyBackwardDifferentiate(%apply_bwd, %remat, %ctx_t), not to BackwardDifferentiate. | functional | [#legacy-bridge](../../../design/ir-reference/differentiation.md#legacy-bridge) | [`legacy-backward-differentiate.slang`](legacy-backward-differentiate.slang) |
| DifferentialPair<T>(primal, diff) constructor at a __fwd_diff call site lowers to MakeDiffPair(%primal, %diff). | functional | [#makediffpair](../../../design/ir-reference/differentiation.md#makediffpair) | [`make-diff-pair.slang`](make-diff-pair.slang) |
| MakeDiffPair produces a value of type DiffPair(T, witness); the witness operand is the IDifferentiable witness for T. | functional | [#makediffpair](../../../design/ir-reference/differentiation.md#makediffpair) | [`diff-pair-result-type.slang`](diff-pair-result-type.slang) |
| MakeDiffPair works on vector primal/differential types; result type is DiffPair(Vec(...), witness). | functional | [#makediffpair](../../../design/ir-reference/differentiation.md#makediffpair) | [`make-diff-pair-vector.slang`](make-diff-pair-vector.slang) |
| A no_diff parameter surfaces as Attributed(T, %no_diff) on the function entry block at LOWER-TO-IR. | functional | [#opcodes](../../../design/ir-reference/differentiation.md#opcodes) | [`no-diff-parameter-marker.slang`](no-diff-parameter-marker.slang) |
| A single entry point that constructs a DifferentialPair then reads both .p and .d exercises MakeDiffPair, GetPrimal, and GetDifferential in one IR dump. | functional | [#opcodes](../../../design/ir-reference/differentiation.md#opcodes) | [`make-diff-pair-and-projections.slang`](make-diff-pair-and-projections.slang) |
| The no_diff parameter marker is implemented by a unique module-scope let %k : Void = no_diff value that all Attributed(T, %k) wrappers reference. | functional | [#opcodes](../../../design/ir-reference/differentiation.md#opcodes) | [`no-diff-module-scope-marker.slang`](no-diff-module-scope-marker.slang) |


## Untested claims
| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| **`checkpointObj`**, **`loopExitValue`**, **`ReportCheckpointStore`** — checkpointing markers inserted by the reverse-mode pass; no natural surface form at LOWER-TO-IR. | (unclassified) | [#checkpointobj](../../../design/ir-reference/differentiation.md#checkpointobj) | Reason and explanation to be refined by the next regeneration. |
| **`LoadReverseGradient`**, **`ReverseGradientDiffPairRef`**, **`PrimalParamRef`**, **`DiffParamRef`** — autodiff temporaries that do not survive past the splitting / back-prop pass. | (unclassified) | [#loadreversegradient](../../../design/ir-reference/differentiation.md#loadreversegradient) | Reason and explanation to be refined by the next regeneration. |
| **`BackwardDifferentiate`** itself — the doc lists `__bwd_diff` as its AST origin, but LOWER-TO-IR actually emits `LegacyBackwardDifferentiate`. The modern opcode appears only after later passes. See `legacy-backward-differentiate.slang` and the doc gap below. | internal-source-fact | [#backwarddifferentiate](../../../design/ir-reference/differentiation.md#backwarddifferentiate) | Implementation detail (C++ class hierarchy, field names, parser-callback names) with no user-observable consequence. |
| **`BackwardDifferentiatePrimal`**, **`BackwardDifferentiatePropagate`**, **`BackwardRemat`**, **`TrivialBackwardDifferentiate*`** — `(synthesized)` by the unzip pass. | link-stage-only | [#backwarddifferentiateprimal](../../../design/ir-reference/differentiation.md#backwarddifferentiateprimal) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |
| **`BackwardFromLegacyBwdDiffFunc`** / `BackwardPrimalFromLegacyBwdDiffFunc` / `BackwardRematFromLegacyBwdDiffFunc` / `BackwardPropagateFromLegacyBwdDiffFunc` — `(synthesized)` legacy-bridge extraction opcodes. | link-stage-only | [#backwardfromlegacybwddifffunc](../../../design/ir-reference/differentiation.md#backwardfromlegacybwddifffunc) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |
| **`DiffTypeInfo`** — `(synthesized)` type-info container, internal. | link-stage-only | [#difftypeinfo](../../../design/ir-reference/differentiation.md#difftypeinfo) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |
| **`ForwardDifferentiatePropagate`**, **`TrivialForwardDifferentiate`** — `(synthesized)`; produced by the unzip / transcribe pipeline. | link-stage-only | [#forwarddifferentiatepropagate](../../../design/ir-reference/differentiation.md#forwarddifferentiatepropagate) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |
| **`FunctionCopy`**, **`SynthesizedForwardDerivativeWitnessTable`**, **`SynthesizedBackwardDerivativeWitnessTable`**, **`MakeIDifferentiableWitness`**, **`SynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc`** — derivative-witness synthesis, internal. (Note: `SynthesizedForwardDerivativeWitnessTable` and `SynthesizedBackwardDerivativeWitnessTable` do happen to appear near the LOWER-TO-IR output for every user `[Forward/Backward Differentiable]` function, but the doc lists them as `(synthesized)` with no AST origin; their presence is a side-effect of the lowering, not a documented surface mapping.) | link-stage-only | [#functioncopy](../../../design/ir-reference/differentiation.md#functioncopy) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |
| **`GetDifferentialPtr`**, **`GetPrimalRef`** — `(synthesized)` pointer-projection counterparts; no natural user surface. | link-stage-only | [#getdifferentialptr](../../../design/ir-reference/differentiation.md#getdifferentialptr) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |
| **`MakeDiffRefPair`** — `(synthesized)`; pointer-typed primal/ differential pair used inside autodiff-pass plumbing. | link-stage-only | [#makediffrefpair](../../../design/ir-reference/differentiation.md#makediffrefpair) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#backwarddifferentiate](../../../design/ir-reference/differentiation.md#backwarddifferentiate) | drift-from-source | The `### BackwardDifferentiate` notable-opcode discussion names `__bwd_diff` as the AST origin of `BackwardDifferentiate`, but the actual LOWER-TO-IR opcode emitted by `slang-lower-to-ir.cpp` for `__bwd_diff(f)` is `LegacyBackwardDifferentiate(%apply_bwd, %remat, %ctx_t)` — the legacy-bridge form. The modern `BackwardDifferentiate` opcode is not produced at LOWER-TO-IR at all; it is synthesized later by the unzip pass when converting the legacy form to the modern primal-/propagate-/remat-triple. | A one-line note clarifying that `BackwardDifferentiate` is an unzip-pass opcode (and that `LegacyBackwardDifferentiate` is the LOWER-TO-IR spelling) would prevent test-author confusion. (See `legacy-backward-differentiate.slang` and `bwd-diff-alias.slang`.) |
| [#differential-pair-construction](../../../design/ir-reference/differentiation.md#differential-pair-construction) | undocumented-behavior | The `### Differential-pair construction` and `### Differential-pair projection` tables list every opcode's AST origin as `(synthesized)`, but `MakeDiffPair`, `GetPrimal`, and `GetDifferential` are all routinely produced from user-written surface code (`DifferentialPair<T>(p, d)` constructor calls and `.p`/`.d` field accesses). The AST-origin column should distinguish "synthesized by autodiff passes" from "naturally produced from user surface at LOWER-TO-IR". (See `make-diff-pair.slang`, `get-primal.slang`, `get-differential.slang`.) |  |
| [#nodiff](../../../design/ir-reference/differentiation.md#nodiff) | undocumented-behavior | The doc nowhere documents the **`no_diff`** parameter marker as an IR opcode. At LOWER-TO-IR, `no_diff T x` parameters surface as `param %x : Attributed(T, %no_diff)` where `%no_diff` is a module-scope `let %k : Void = no_diff` IR value. The `no_diff` opcode should appear in the catalog (alongside or in the `Autodiff temporaries` table) with its `Attributed(...)` parameter-wrapper role spelled out. (See `no-diff-parameter-marker.slang`, `no-diff-module-scope-marker.slang`.) |  |
| [#forwarddifferentiate](../../../design/ir-reference/differentiation.md#forwarddifferentiate) | undocumented-behavior | The `### ForwardDifferentiate` discussion says "Specialization replaces the opcode with the actual JVP function once `baseFn` is fully known" but does not document the LOWER-TO-IR form before specialization (`let %fwd_diff : Func(DiffPair(T, %w), DiffPair(T, %w)) = ForwardDifferentiate(%f)`). | A one-line example showing the pre-specialization shape would anchor reference tests. (See `forward-differentiate.slang`.) |
| [#detachderivative](../../../design/ir-reference/differentiation.md#detachderivative) | undocumented-behavior | The `### detachDerivative` discussion describes the semantic (returns operand unchanged, blocks derivative) but does not show the LOWER-TO-IR form (`let %N : T = detachDerivative(%x)` with operand and result type equal). | A one-line example would prevent over-specific pattern construction. (See `detach-derivative.slang`, `detach-derivative-result-type.slang`.) |
| [#fwddiff](../../../design/ir-reference/differentiation.md#fwddiff) | undocumented-behavior | The doc does not describe the **`fwd_diff` / `bwd_diff` unprefixed aliases**. The parser accepts both `__fwd_diff(f)` and `fwd_diff(f)` (similarly for `bwd_diff`); both lower identically. | A note that the unprefixed aliases exist would clarify the surface. (See `fwd-diff-alias.slang`, `bwd-diff-alias.slang`.) |
