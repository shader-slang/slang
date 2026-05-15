---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T20:30:00+00:00
target_doc: target-pipelines/cuda.md
review_report: ../../reviews/target-pipelines/cuda.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/cuda.md

## Summary

Both findings addressed: the liveness-marker passes are now in
Phase C, and the adjacent-target description of `PyTorchCppBinding`
names `TorchCppSourceEmitter` instead of `CPPSourceEmitter`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The CUDA Phase C section omitted the liveness markers that run for every target when liveness tracking is enabled. | Inserted `LivenessUtil::addVariableRangeStarts` (new row 24) and `LivenessUtil::addRangeEnds` (new row 26) into the Phase C table with the `shouldTrackLiveness()` gate, and renumbered the subsequent rows (`simplifyNonSSAIR` → 27, ..., `checkUnsupportedInst` → 32). Added matching gate nodes and edges to the mermaid diagram. |
| F-002 | fixed | `slang-emit.cpp` lines 2523-2525 construct `TorchCppSourceEmitter`, not `CPPSourceEmitter`. | Updated the `PyTorchCppBinding` bullet in `## Adjacent targets` to say the emit arm is `TorchCppSourceEmitter` (linking to `slang-emit-torch.cpp`) and citing the construction site in `slang-emit.cpp`. |
