---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:01:29Z
target_doc: ast-reference/expressions.md
review_report: ../../reviews/ast-reference/expressions.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/expressions.md

## Summary

All five findings were verified against the current source and fixed in the
document. Edits were minimal; the two long callouts were condensed, which also
paid for the additions and left the doc well under its size cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/compiler-core/slang-lexer.cpp` lines 1309-1347 round or truncate to Half/Float; `source/slang/slang-parser.cpp` line 9180 stores the result unchanged. | Table cell and callout now say suffix-converted value; parser adds no rounding. |
| F-002 | fixed | `source/slang/slang-parser.cpp` line 8838 stores a parsed block, line 8844 a direct `ReturnStmt`. | Removed "same shape"; stated the `bodyStmt` difference. |
| F-003 | fixed | Declarations at `source/slang/slang-ast-expr.h` lines 231, 263, 339, 355 have these types. | Added `List<SourceLoc>`, `TryClauseType`, `SourceLoc` x2 to the cells. |
| F-004 | fixed | Callouts exceeded the 2-5 sentence guidance in `_common.md`. | Condensed the LiteralExpr and Type-expression callouts, keeping source-backed facts. |
| F-005 | fixed | Prompt "Forbidden content" excludes IR lowering of expressions. | Dropped the lowering explanation; linked `../pipeline/04-ast-to-ir.md`. |
