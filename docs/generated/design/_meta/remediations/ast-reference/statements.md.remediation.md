---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T18:00:00+00:00
target_doc: ast-reference/statements.md
review_report: ../../reviews/ast-reference/statements.md.review.md
target_doc_source_commit_before: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/statements.md

## Summary

One major finding addressed by adding a `UniqueStmtIDNode` row at
the end of the Nodes table, with a summary noting it is a
serialization / control-flow identity helper rather than a parsed
statement.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-stmt.h:92-94` declares `class UniqueStmtIDNode : public Decl` with `FIDDLE()` in the watched header, so the per-page contract requires it to be tabulated. | Added a `UniqueStmtIDNode` row to `## Nodes` with parent `Decl`, `(no parsed state)`, grammar `(none)`, and a summary that describes its identity-helper role. |
