---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:01:57Z
target_doc: ast-reference/declarations.md
review_report: ../../reviews/ast-reference/declarations.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/declarations.md

## Summary

All four findings were verified against the current source and fixed. The doc shrank from about 39.8 KB to about 30.3 KB, well under its cap. Front-matter `generated_at` was refreshed; the watched-paths digest is unchanged.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ast-decl.h` lines 321-335 (`VarDeclBase`), 428-431 (`AggTypeDecl`), 668 (`returnType`), 700 (`body`) confirm the inherited fields. | Replaced every `(inherits)` with inherited fields marked "(from ...)"; `(no additional state)` for `ScopeDecl` and `SemanticDecl`. |
| F-002 | fixed | `source/slang/slang-check-modifier.cpp:946-960` synthesizes a modifier for a user attribute struct; `source/slang/slang-parser.cpp:11055-11066` and 11243 build it from `__attributeTarget`. | Attributed construction to `parseAttributeTargetModifier`; kept the line 1527 use-site check. |
| F-003 | fixed | `_common.md` AST contract item 5 forbids code blocks; callouts must be 2-5 sentences. | Condensed callouts and removed both fenced Slang examples (see also F-004). |
| F-004 | fixed | `ast-reference-declarations.md` forbids type-checking semantics and IR lowering. | Removed attribute validation and E31002 detail, global-generic diagnostics and `-specialize`, and shadow-var lowering; linked to `../pipeline/03-semantic-check.md`. |
