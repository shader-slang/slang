---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:16:06Z
target_doc: ir-reference/differentiation.md
review_report: ../../reviews/ir-reference/differentiation.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 1, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for ir-reference/differentiation.md

## Summary

The review had one finding (F-001, critical), which I verified against source and fixed. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against `source/slang/slang-lower-to-ir.cpp`: `visitBackwardDifferentiateExpr` (line 5675) calls `SLANG_UNEXPECTED("BackwardDifferentiateExpr present during IR lowered")` (line 5678), so `__bwd_diff` expression lowering does not emit `BackwardDifferentiate`. The opcode is actually emitted by `visitBackwardDifferentiateVal` (line 2508), a witness-table `Val` lowering path. The contract's AST-origin column uses `(synthesized)` for IR-pass-introduced opcodes and forbids guessing AST nodes. | Changed the `BackwardDifferentiate` row's AST origin from `BackwardDifferentiateExpr (__bwd_diff(...))` to `(synthesized)`, and rewrote the `## Source` paragraph so it states `__fwd_diff` lowers to `ForwardDifferentiate`, `BackwardDifferentiate` comes from the `Val` lowering path, and the `__bwd_diff` expression form is rejected with `SLANG_UNEXPECTED`. |
