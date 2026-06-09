---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ast-reference/expressions.md
review_report: ../../reviews/ast-reference/expressions.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/expressions.md

## Summary

Both minor findings were fixed (fixed=2; no rejections, deferrals, or
escalations). F-001 corrected the `OperatorExpr` row that wrongly called
the class an abstract intermediate. F-002 corrected the claim that
adjacent string literals are merged at lex time.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-expr.h:275` declares `OperatorExpr` with plain `FIDDLE()`, not `FIDDLE(abstract)`, so it is a concrete class. | Reworded the `OperatorExpr` summary to call it the shared base for the infix, prefix, postfix, select, and short-circuit operator forms instead of "abstract intermediate". |
| F-002 | fixed | `source/slang/slang-parser.cpp:8681-8695` concatenates adjacent `StringLiteral` tokens into `StringLiteralExpr::value` during parsing, not at lex time. | Changed "merged at lex time" to merged during expression parsing after lexing produces adjacent string-literal tokens. |
