---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:06:00Z
target_doc: ast-reference/base.md
review_report: ../../reviews/ast-reference/base.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/base.md

## Summary

All four findings were verified against the source and the per-document
prompt, and all four were fixed. Two were factual or contract wording in
a single line each (`Scope`, the `DeclBase` family line); one removed
concrete-leaf class names from the root prose and two support-table
cells; one added a second role sentence to the `NodeBase` and `Stmt`
callouts. The front matter was not touched.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                                                  | Fix summary                                                                                                                                                             |
| ---------- | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | Confirmed: `source/slang/slang-parser.cpp:144-152` (`Parser::PushScope`) creates a `Scope` and stores it in `containerDecl->ownedScope`, declared at `source/slang/slang-ast-decl.h:141`. The unqualified "not parsed" wording was wrong.                  | Root-hierarchy bullet and the `Scope` support-table cell now say it is not a syntax node but is created during parsing and attached to container declarations           |
| F-002      | fixed  | `docs/generated/design/_meta/prompts/ast-reference-base.md:44-46` lists `DeclBase` among the roots that must read `(no dedicated family page)`.                                                                                                            | `Family page:` line for `DeclBase` changed to `(no dedicated family page)`                                                                                              |
| F-003      | fixed  | `docs/generated/design/_meta/prompts/ast-reference-base.md:61-62` forbids concrete leaves on this page; the checklist at lines 57-58 also limits named classes to the three watched headers, which excludes `DeclGroup` (`source/slang/slang-ast-decl.h`). | `DeclBase` prose now says "a concrete group node" and links `declarations.md`; `DeclRef<T>` and `BuiltinOperationKind` cells replaced leaf names with family-page links |
| F-004      | fixed  | `docs/generated/design/_meta/prompts/ast-reference-base.md:37` requires 2-4 sentences per root; both callouts had one.                                                                                                                                     | Added one role sentence to `NodeBase` (shared state for all allocated nodes) and to `Stmt` (`ModifiableSyntaxNode` supplies loc and `Modifiers`)                        |
