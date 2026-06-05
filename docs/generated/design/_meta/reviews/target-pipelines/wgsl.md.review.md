---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:37+00:00
target_doc: target-pipelines/wgsl.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: f76d76915e55fca2f6089859682d44515d2961d21271a2b24e0eda6e9187f22f
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

# Review report for target-pipelines/wgsl.md

## Summary
The WGSL page has the required sections, distinguishes source-only and Tint downstream paths, and all checked relative links resolve at the recorded source commit. One ordering error remains: three passes shown in Phase A actually run later in Phase C.

## Items checked
- Read `regenerate.py show target-pipelines/wgsl.md`, the WGSL prompt, `_common.md`, and dependency docs.
- Checked front matter, required sections, `WGSL` / `WGSLSPIRV` / `WGSLSPIRVAssembly` handling, `legalizeIRForWGSL`, `specializeAddressSpaceForWGSL`, and Tint downstream handling against `source_commit` `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 147 relative links in the page at the recorded source commit.
- Spot-checked WGSL claims for source-target reduction, non-Khronos SSBO lowering, WGSL byte-address-buffer options, `resolveTextureFormat`, `legalizeIRForWGSL`, `floatNonUniformResourceIndex`, `legalizeLogicalAndOr`, and loop absence.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: WGSL legalization, lowering, phi elimination` | The Phase A diagram/table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but the source runs those passes in Phase C, after `resolveTextureFormat` and before `legalizeIRForWGSL`. This makes the ordered WGSL pipeline materially wrong. | `source/slang/slang-emit.cpp:1955-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` in the Phase C range, not in the Phase A range around `source/slang/slang-emit.cpp:982-1001`. | Move those three nodes and rows from Phase A to Phase C, placing them after `resolveTextureFormat` and before `legalizeIRForWGSL`; update row numbering and Phase A prose. |
