---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:37+00:00
target_doc: target-pipelines/spirv.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 45c7187ec9e14c4b9df481b096e07d7166024478913e182332f481afa116f29f
source_commit: 05132edd86435f217f95634406f85184e58991f8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 1
  major: 0
  minor: 1
  nit: 0
---

# Review report for target-pipelines/spirv.md

## Summary
The SPIR-V page covers the required sections, downstream chain, and the two documented SPIR-V loops, and all checked relative links resolve at the recorded source commit. The main issue is a pass-ordering error shared with the other target pages: three passes are shown in Phase A but actually run in Phase C.

## Items checked
- Read `regenerate.py show target-pipelines/spirv.md`, the SPIR-V prompt, `_common.md`, and dependency docs.
- Checked front matter, required sections, direct-emit precondition, Phase A-D source ranges, SPIR-V legalizer loop bounds, forward-declared-pointer fixup loop, and downstream `spirv-link` / `spirv-val` / `spirv-opt` handling against `source_commit` `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 176 relative links in the page at the recorded source commit.
- Spot-checked SPIR-V claims for `legalizeEntryPointsForGLSL`, `removeRawDefaultConstructors`, SPIR-V phi options, `applyGLSLLiveness`, `legalizeIRForSPIRV`, `simplifyIRForSpirvLegalization`, and the disabled in-source `optimizeSPIRV` block.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: SPIR-V legalization, lowering, phi elimination` | The Phase A diagram/table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but the source runs those passes in Phase C, after `resolveTextureFormat` and before `legalizeEntryPointsForGLSL`. This makes the ordered SPIR-V pipeline materially wrong. | `source/slang/slang-emit.cpp:1955-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` in the Phase C range, not in the Phase A range around `source/slang/slang-emit.cpp:982-1001`. | Move those three nodes and rows from Phase A to Phase C, placing them after `resolveTextureFormat` and before `legalizeEntryPointsForGLSL`; update row numbering and Phase A prose. |
| F-002 | minor | `## Phase C: SPIR-V legalization, lowering, phi elimination` | The Phase C row for `legalizeEmptyTypes` links the pass to `slang-ir-legalize-empty-array.cpp`, but the pass is defined in `slang-ir-legalize-types.cpp`. | `source/slang/slang-ir-legalize-types.cpp:4026` defines `void legalizeEmptyTypes(...)`; no matching definition exists in `source/slang/slang-ir-legalize-empty-array.cpp`. | Change the `legalizeEmptyTypes` row's File cell to `source/slang/slang-ir-legalize-types.cpp`. |
