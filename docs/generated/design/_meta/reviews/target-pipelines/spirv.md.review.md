---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:06:52+00:00
target_doc: target-pipelines/spirv.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 45c7187ec9e14c4b9df481b096e07d7166024478913e182332f481afa116f29f
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 1
  major: 1
  minor: 2
  nit: 0
---

# Review report for target-pipelines/spirv.md

## Summary
The SPIR-V page covers the required sections and the two documented SPIR-V loops, and all checked relative links resolve at the recorded source commit. The main issue is a pass-ordering error shared with the other target pages: three passes are shown in Phase A but actually run in Phase C. The downstream-tool section also overstates the validation-after-link flow.

## Items checked
- Read `regenerate.py show target-pipelines/spirv.md`, the SPIR-V prompt, `_common.md`, and dependency docs.
- Checked front matter, required sections, direct-emit precondition, Phase A-D source ranges, SPIR-V legalizer loop bounds, forward-declared-pointer fixup loop, and downstream `spirv-link` / `spirv-val` / `spirv-opt` handling against `source_commit` `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 176 relative links in the page at the recorded source commit.
- Spot-checked 16 SPIR-V claims, including `legalizeEntryPointsForGLSL`, `removeRawDefaultConstructors`, SPIR-V phi options, `applyGLSLLiveness`, `legalizeIRForSPIRV`, `simplifyIRForSpirvLegalization`, downstream linking, validation, and the disabled in-source `optimizeSPIRV` block.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: SPIR-V legalization, lowering, phi elimination` | The Phase A diagram/table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but the source runs those passes in Phase C, after `resolveTextureFormat` and before `legalizeEntryPointsForGLSL`. This makes the ordered SPIR-V pipeline materially wrong. | `source/slang/slang-emit.cpp:1955-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` in the Phase C range, not in the Phase A range around `source/slang/slang-emit.cpp:982-1001`. | Move those three nodes and rows from Phase A to Phase C, placing them after `resolveTextureFormat` and before `legalizeEntryPointsForGLSL`; update row numbering and Phase A prose. |
| F-002 | major | `## Phase D: IR-to-SPIR-V emit, simplification loop, downstream tools` | The Phase D prose and diagram show validation after optional `spirv-link`, implying the linked artifact is what `spirv-val` checks. In source, linking may replace `artifact`, but validation still calls `compiler->validate` on the original `spirv` buffer. | `source/slang/slang-emit.cpp:3116-3132` may replace `artifact` with `linkedArtifact`, while `source/slang/slang-emit.cpp:3135-3138` validates `spirv.getBuffer()` rather than the linked artifact. | Clarify that `spirv-val` validates the freshly emitted SPIR-V buffer in this code path, or adjust the diagram so linking and validation do not imply validation of the linked artifact. |
| F-003 | minor | `## Phase C: SPIR-V legalization, lowering, phi elimination` | The Phase C row for `legalizeEmptyTypes` links the pass to `slang-ir-legalize-empty-array.cpp`, but the pass is defined in `slang-ir-legalize-types.cpp`. | `source/slang/slang-ir-legalize-types.cpp:4026` defines `void legalizeEmptyTypes(...)`. | Change the `legalizeEmptyTypes` row's File cell to `source/slang/slang-ir-legalize-types.cpp`. |
| F-004 | minor | `## Notable passes`, `Downstream spirv-link / spirv-val / spirv-opt chain` | The closing sentence tells future readers they should re-enable the disabled `optimizeSPIRV` block to recover original behavior. That is an editorial action recommendation rather than descriptive source documentation, and the source only shows a disabled block. | `source/slang/slang-emit.cpp:3053-3060` contains the disabled `#if 0` block but no source comment recommending that readers re-enable it. | Delete the future-reader recommendation or replace it with a neutral statement that the in-source `optimizeSPIRV` block is currently disabled. |
