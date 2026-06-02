---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T20:30:00+00:00
target_doc: target-pipelines/wgsl.md
review_report: ../../reviews/target-pipelines/wgsl.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/wgsl.md

## Summary

Rephrased the intro to distinguish source-pipeline sharing (via the
source-target reduction in `slang-code-gen.cpp`) from
`linkAndOptimizeIR` switch-arm membership; the `WGSLSPIRV*` variants
share the WGSL pipeline because they map to source target `WGSL`,
not because the arms list them.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Several `linkAndOptimizeIR` arms list only `CodeGenTarget::WGSL`; the doc previously implied all three targets appeared in every arm. | Rewrote the second paragraph of the intro to explain the source-target reduction at `source/slang/slang-code-gen.cpp:269-272` and to call out arms like `slang-emit.cpp:1947-1952` and `slang-emit.cpp:2074-2077` that name only `CodeGenTarget::WGSL`, noting that those arms still fire for `WGSLSPIRV*` because of the source-target reduction. |
