---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: pipeline/04-ast-to-ir.md
review_report: ../../reviews/pipeline/04-ast-to-ir.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04-ast-to-ir.md

## Summary

The single finding was fixed. The `## Adjacent pipelines` bullet
claimed the layout IR module is absolutely "not fed into
`linkAndOptimizeIR`", but `linkIR` adds an existing layout module to
its module list so layout-decorated globals participate in linking.
The wording was narrowed to match the source.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-link.cpp:2120-2127` adds `getExistingIRModuleForLayout()` to `irModules` so layout-decorated symbols are considered during linking, contradicting the absolute claim. | Reworded the bullet to say the layout module is not the executable module and skips mandatory passes, but is considered by `linkIR` (cited at lines 2120-2127). |
