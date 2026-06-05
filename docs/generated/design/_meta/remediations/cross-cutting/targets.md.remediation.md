---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: cross-cutting/targets.md
review_report: ../../reviews/cross-cutting/targets.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/targets.md

## Summary

The review reported one major source-alignment finding, which was fixed. The `### Vocabulary` example for `raytracing` used a stale expansion contradicted by the authoritative alias declaration at `source/slang/slang-capabilities.capdef:1312` (`alias raytracing = GL_EXT_ray_tracing | _sm_6_3 | cuda`). The example was updated to match the declaration.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-capabilities.capdef:1312` declares `raytracing` as the disjunction of `GL_EXT_ray_tracing`, `_sm_6_3`, and `cuda`; the page's expansion came from the stale top-of-file example comment. | Replaced the `raytracing` example expansion with the actual `GL_EXT_ray_tracing` / `_sm_6_3` / `cuda` alternatives. |
