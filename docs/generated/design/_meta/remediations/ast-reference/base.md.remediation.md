---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:10:00Z
target_doc: ast-reference/base.md
review_report: ../../reviews/ast-reference/base.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/base.md

## Summary

All three findings were verified against the current source and fixed with minimal edits. Fixed: 3; no rejections, deferrals or escalations. The document grew by 29 bytes (18,478 bytes, well under the 49,152 cap).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `source/slang/slang-parser.cpp` `DeclGroupBuilder::addToGroup` calls `CompleteDecl` with the shared modifiers for each member. | DeclBase section: replaced inheritance explanation with `DeclGroupBuilder`/`CompleteDecl` attachment. |
| F-002 | fixed | Confirmed: `Scope` is `class Scope : public NodeBase` in `source/slang/slang-ast-base.h`. | Support types intro: most entries are non-node helpers in the support-types header; `Scope` is the exception. |
| F-003 | fixed | Confirmed: `SyntaxClass<T>::isSubClassOf<U>()` in `source/slang/slang-ast-support-types.h` places no concreteness restriction. | `SyntaxClass<T>` row: "concrete AST class" -> "AST class, including abstract bases". |
