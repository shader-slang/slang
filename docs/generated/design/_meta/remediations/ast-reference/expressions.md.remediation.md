---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:12:55Z
target_doc: ast-reference/expressions.md
review_report: ../../reviews/ast-reference/expressions.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 1, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for ast-reference/expressions.md

## Summary

The review contained one major finding, which was verified against the source and fixed. The `PartiallyAppliedGenericExpr` row listed a nonexistent `knownGenericArgs` field; the source declares `providedOrdinaryArgs` instead. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against `source/slang/slang-ast-expr.h` at commit eb9403ef: `PartiallyAppliedGenericExpr` declares `baseGenericDeclRef` and `providedOrdinaryArgs: List<Val*>`, with no `knownGenericArgs` field. | Replaced `knownGenericArgs: List<Val*>` with `providedOrdinaryArgs: List<Val*>` in the `PartiallyAppliedGenericExpr` row. |
