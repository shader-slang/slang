---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:01:24Z
target_doc: ast-reference/statements.md
review_report: ../../reviews/ast-reference/statements.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/statements.md

## Summary

Both findings were verified against the current source and fixed in the document. F-001 corrected the `IfStmt` predicate type in prose; F-002 condensed the three oversized callouts to the 2-5 sentence contract. Action breakdown: 2 fixed, 0 rejected, 0 deferred, 0 escalated. The document shrank from 25,891 to 22,740 bytes, well under its cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-stmt.h:86-88` declares `Expr* predicate` and two `Stmt*` branches; the prose was wrong. | IfStmt callout now reads "an `Expr*` predicate and two `Stmt*` branches". |
| F-002 | fixed | `_common.md` AST reference contract item 5 requires 2-5 sentence callouts; the flagged callouts were far longer. | Condensed BlockStmt/SeqStmt, Loop family and TargetSwitchStmt callouts, keeping parser line citations and dropping repeated table content and extended diagnostic examples. |
