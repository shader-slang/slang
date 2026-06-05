---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/differentiation.md
review_report: ../../reviews/ir-reference/differentiation.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/differentiation.md

## Summary

One minor finding was fixed and no findings were rejected, deferred, or
escalated. The `## Source` section miscounted the checkpointing opcodes
as two while naming three, so the count word was corrected.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-insts.lua:1479-1487` defines three checkpointing opcodes (`checkpointObj`, `loopExitValue`, `ReportCheckpointStore`), but the text said `Two additional opcodes` while listing all three. | `## Source`: changed `Two additional opcodes` to `Three additional opcodes`. |
