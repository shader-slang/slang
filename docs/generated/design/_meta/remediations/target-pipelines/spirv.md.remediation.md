---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T20:30:00+00:00
target_doc: target-pipelines/spirv.md
review_report: ../../reviews/target-pipelines/spirv.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/spirv.md

## Summary

Promoted `applyGLSLLiveness` from the filtered-out list into Phase
C, gated on `shouldTrackLiveness() && isKhronosTarget`, in both the
diagram and the table.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The gate at `slang-emit.cpp` lines 2347-2352 is `shouldTrackLiveness() && isKhronosTarget(targetRequest)`, which is satisfied by SPIR-V direct-emit (the doc had wrongly claimed it ran only on the via-GLSL path). | Added `applyGLSLLiveness` as a new row 35 in the Phase C table with the correct gate and a citation to lines 2347-2352. Removed the `applyGLSLLiveness (only when shouldTrackLiveness and Khronos non-direct)` entry from the filtered-out list. Added a `liveKhrGate` / `aGL` node pair to the mermaid diagram between `LivenessUtil::addRangeEnds` and `replaceLocationIntrinsicsWithRaytracingObject`. Updated the gate-summary table to say `applyGLSLLiveness` fires on every Khronos target (SPIR-V direct-emit and via-GLSL) rather than only the via-GLSL path. Renumbered subsequent rows (35-39 → 36-40, `checkUnsupportedInst` is now row 41). |
