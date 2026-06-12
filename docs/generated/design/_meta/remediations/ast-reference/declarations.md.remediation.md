---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ast-reference/declarations.md
review_report: ../../reviews/ast-reference/declarations.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/declarations.md

## Summary

The single major finding was fixed (fixed=1; no rejections, deferrals,
or escalations). F-001 reported a dangling grammar anchor in the
`AttributeDecl` row. The link now points at the existing
`#attributes-and-decorations` heading in the grammar page.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `docs/generated/design/syntax-reference/grammar.md:252` is `## Attributes and decorations` (anchor `#attributes-and-decorations`); no `#modifiers-and-attributes` heading exists. | Changed the `AttributeDecl` grammar link from `#modifiers-and-attributes` to `#attributes-and-decorations`. |
