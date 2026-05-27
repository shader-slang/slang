---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T18:00:00+00:00
target_doc: ast-reference/base.md
review_report: ../../reviews/ast-reference/base.md.review.md
target_doc_source_commit_before: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/base.md

## Summary

One major finding addressed by removing the `Scope` node from the
root-hierarchy diagram. `Scope` is already listed in the Support
types table with the same identity as a `NodeBase` used only as a
lookup helper, which is its accurate role.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-base.h:111-112` shows `Scope` as a concrete (non-abstract) `NodeBase`; the root-hierarchy diagram is reserved for the abstract roots that group every parsed AST family. | Removed the `NodeBase --> Scope` edge from the `## Root hierarchy` mermaid diagram; the existing prose at line 68 (`Scope is a concrete NodeBase ... not a syntax node...`) and the `Scope` row in `## Support types` (line 263) still document it. |
