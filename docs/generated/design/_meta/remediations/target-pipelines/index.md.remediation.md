---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: target-pipelines/index.md
review_report: ../../reviews/target-pipelines/index.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/index.md

## Summary

The single minor finding was fixed. The post-table paragraph that
named per-pass behavior was generalized so the index no longer
documents individual passes, as the index contract requires.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The target-pipeline index contract in `docs/generated/design/_meta/prompts/_common.md` forbids per-pass details on the index page. | Replaced the paragraph naming `eliminatePhis` register-allocation settings and `specializeAddressSpaceForMetal`/`specializeAddressSpaceForWGSL` placement with a generalized pointer to the per-target pages. |
