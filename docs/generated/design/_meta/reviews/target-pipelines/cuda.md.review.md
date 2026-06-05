---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:37+00:00
target_doc: target-pipelines/cuda.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 697128ce2b824225966f830d21ac028faff0ac0feee6366f3549b55ad9f0d9b1
source_commit: 05132edd86435f217f95634406f85184e58991f8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 1
  major: 0
  minor: 0
  nit: 0
---

# Review report for target-pipelines/cuda.md

## Summary
The CUDA page has the required target-pipeline shape and all checked relative links resolve at the recorded source commit. One ordering error remains: three reachable `SLANG_PASS` calls are documented in Phase A even though the source runs them later in Phase C.

## Items checked
- Read `regenerate.py show target-pipelines/cuda.md`, the CUDA prompt, `_common.md`, and dependency docs.
- Checked front matter, required sections, and the CUDA-specific `CUDASource` / `CUDAHeader` / `PTX` branches against `source_commit` `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 155 relative links in the page at the recorded source commit.
- Spot-checked CUDA claims for OptiX uniform collection, skipped entry-point uniform movement, `synthesizeActiveMask`, CUDA varying-param legalization, `lowerImmutableBufferLoadForCUDA`, `shouldLegalizeExistentialAndResourceTypes = false`, and nvrtc downstream handling.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: CUDA legalization, lowering, phi elimination` | The Phase A diagram/table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but `linkAndOptimizeIR` runs those passes later in Phase C, after the target-specific active-mask / texture-format region and before entry-point legalization. This makes the ordered CUDA pipeline materially wrong. | `source/slang/slang-emit.cpp:1955-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` in the Phase C range, not in the Phase A range around `source/slang/slang-emit.cpp:982-1001`. | Move those three nodes and table rows from Phase A to Phase C, placing them after the CUDA `synthesizeActiveMask` / filtered `resolveTextureFormat` point and before `legalizeEntryPointVaryingParamsForCUDA`; update the Phase A prose and numbering accordingly. |
