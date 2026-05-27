---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T20:30:00+00:00
target_doc: target-pipelines/metal.md
review_report: ../../reviews/target-pipelines/metal.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/metal.md

## Summary

Added the two liveness-marker passes that flank `eliminatePhis` to
both the Phase C diagram and the table, gated on
`shouldTrackLiveness()`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `LivenessUtil::addVariableRangeStarts` and `addRangeEnds` run for every target around `eliminatePhis` when liveness tracking is enabled ([slang-emit.cpp lines 2307-2325](../../../source/slang/slang-emit.cpp)); the Metal Phase C section omitted them. | Inserted `LivenessUtil::addVariableRangeStarts` (new row 25) and `LivenessUtil::addRangeEnds` (new row 27) into the Phase C table with the `shouldTrackLiveness()` gate, and renumbered the subsequent rows (`simplifyNonSSAIR` → 28, ..., `checkUnsupportedInst` → 33). Added matching gate nodes and edges to the mermaid diagram. |
