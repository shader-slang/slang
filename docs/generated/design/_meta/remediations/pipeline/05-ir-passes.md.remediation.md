---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T19:30:00+00:00
target_doc: pipeline/05-ir-passes.md
review_report: ../../reviews/pipeline/05-ir-passes.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/05-ir-passes.md

## Summary

Promoted the shared-utilities subsection to a top-level `## Pass
utilities` heading as the prompt requires. Took the opportunity to
expand the per-row descriptions so each utility now states what it
contributes (clone, dominators, walking, opcode info, stable names).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The prompt mandates a top-level `## Pass utilities` section; the doc only had `### Shared utilities (not passes)` nested under `## Pass categories`. | Removed the `### Shared utilities (not passes)` block from inside `## Pass categories` and added a new top-level `## Pass utilities` section just after `## Pass categories`, preserving every file row and adding a "Purpose" column with a short description of each utility. |
