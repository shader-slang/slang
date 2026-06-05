---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: pipeline/overview.md
review_report: ../../reviews/pipeline/overview.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/overview.md

## Summary

The single nit was fixed by renaming the `## End-to-end flow` Mermaid
node IDs from PascalCase to camelCase, as the prompt's quality
checklist requires. The visible node labels were left unchanged.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `docs/generated/design/_meta/prompts/pipeline-overview.md:45` requires camelCase Mermaid node IDs; the diagram used PascalCase (`Source`, `Lex`, ...). | Renamed IDs to `source`, `lexPreprocess`, `parse`, `semanticCheck`, `lower`, `irPasses`, `emit`, `targetArtifact`, preserving labels. |
