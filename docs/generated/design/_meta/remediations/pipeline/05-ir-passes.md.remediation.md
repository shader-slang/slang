---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: pipeline/05-ir-passes.md
review_report: ../../reviews/pipeline/05-ir-passes.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/05-ir-passes.md

## Summary

The single structural finding was fixed by swapping the section
order so `## Adding a new pass` precedes `## Pass utilities`, matching
the per-doc prompt's required structure. Both sections' content was
preserved verbatim; only their order changed.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `docs/generated/design/_meta/prompts/pipeline-05-ir-passes.md:55-60` lists `## Adding a new pass` (item 4) before `## Pass utilities` (item 5); the doc had them reversed. | Moved the `## Pass utilities` section to immediately after `## Adding a new pass`. |
