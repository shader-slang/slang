---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ast-reference/statements.md
review_report: ../../reviews/ast-reference/statements.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/statements.md

## Summary

All three findings were fixed (fixed=3; no rejections, deferrals, or
escalations). F-001 corrected the `CatchStmt` row to describe `do ...
catch` syntax. F-002 reconciled the `UniqueStmtIDNode` prose with the
table row that lists it. F-003 added the missing notable-node coverage
for `ReturnStmt`, `ContinueStmt`, `DiscardStmt`, and `EmptyStmt`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-parser.cpp:7063-7065` routes `do` then `catch` to `ParseDoCatchStatement`; `try` is parsed as an expression statement, so `CatchStmt` is `do ... catch`. | Changed the `CatchStmt` grammar text to `do-catch` and the summary from `try { ... } catch` to `do { ... } catch (e) { ... }`. |
| F-002 | fixed | The header declares `UniqueStmtIDNode` with `FIDDLE()` and a prior cycle added its table row; the prose still claimed it was excluded. | Reworded the hierarchy prose to say the row is listed as a helper (FIDDLE-declared) though not in the `Stmt` hierarchy or parsed, removing the "excluded" contradiction. |
| F-003 | fixed | `docs/generated/design/_meta/prompts/ast-reference-statements.md:37-46` requires notable callouts for `ReturnStmt`, `ContinueStmt`, `DiscardStmt`, `EmptyStmt`, which were absent. | Added a `### ReturnStmt` callout, expanded the LabelStmt callout to cover `ContinueStmt` and `DiscardStmt`, and expanded the DeclStmt/ExpressionStmt callout to cover `EmptyStmt`. |
