---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:15:34Z
target_doc: name-resolution/overload-resolution.md
review_report: ../../reviews/name-resolution/overload-resolution.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/overload-resolution.md

## Summary

All five review findings were verified against the watched source at commit `eb9403ef5` and fixed. All five were factually correct and within this page's contract: three majors (F-001, F-002, F-003) corrected algorithm-step descriptions that misstated which candidate flavor is delegated, how per-argument cost is computed, and what the directions step actually checks; two minors (F-004, F-005) corrected the `PartiallyAppliedGenericExpr` data-shape claim and the operator-cache-key description. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `slang-check-overload.cpp:791-794` delegates only `Flavor::Generic` (default is `SLANG_UNEXPECTED`); `:2846-2851` creates the `UnspecializedGeneric` candidate post-inference-failure; `:1394-1405` handles `GenericArgumentInferenceFailed` in the early error path, never reaching the construction switch; `:1540-1559` builds `PartiallyAppliedGenericExpr` under `Flavor::Generic`. | Rewrote the type-check delegation bullet and the finalize construction switch so only `Flavor::Generic` is delegated/wrapped, and described `UnspecializedGeneric` as the recorded failed-inference candidate handled by the early error path. |
| F-002 | fixed | Confirmed: `slang-check-overload.cpp:824-837` calls `canCoerce` and adds the reported `cost`; `slang-check-conversion.cpp:2551-2556` reports too-high implicit conversions as possible so the overload can be selected and diagnosed at reification. | Replaced the `getImplicitConversionCostWithKnownArg`/`canConvertImplicitly` prose in the type-check step and the Conversion-costs intro with `canCoerce`-based accumulation and the report-as-possible behavior. |
| F-003 | fixed | Confirmed: `slang-check-overload.cpp:1018-1024` says general l-value checking is done elsewhere and this step only checks `this` mutability; `:1030-1039` rejects a mutating method on an immutable base, emitting `MutatingMethodOnImmutableValue`. | Rewrote the directions step to describe the mutating-`this` check and its diagnostics instead of general in/out/inout/ref l-value enforcement. |
| F-004 | fixed | Confirmed: `slang-ast-expr.h:955-964` stores `baseGenericDeclRef` and `providedOrdinaryArgs` with a comment that witness arguments are not stored there; `slang-check-overload.cpp:1552-1558` stores only the ordinary-argument prefix. | Rephrased the data-shape claim to name `baseGenericDeclRef` plus the provided ordinary-argument prefix and note witness arguments are formed later. |
| F-005 | fixed | Confirmed: `slang-check-impl.h:197-209` declares `operatorName`, `isGLSLMode`, `args[2]`; `:230-264` sets `operatorName = intrinsicOp->op` from an `IntrinsicOpModifier` on an applicable overload. | Updated the cache-key prose to include `isGLSLMode`, the two `BasicTypeKey` operands, and the intrinsic-op-modifier derivation of `operatorName`. |
