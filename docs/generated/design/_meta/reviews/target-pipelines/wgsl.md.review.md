---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:06:52+00:00
target_doc: target-pipelines/wgsl.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: f76d76915e55fca2f6089859682d44515d2961d21271a2b24e0eda6e9187f22f
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 1
  major: 0
  minor: 2
  nit: 0
---

# Review report for target-pipelines/wgsl.md

## Summary
The WGSL page has the required sections, distinguishes source-only and Tint downstream paths, and all checked relative links resolve at the recorded source commit. The main issue is that three passes shown in Phase A actually run later in Phase C. Two smaller issues affect source-target line citations and the WGSL-specific gate table.

## Items checked
- Read `regenerate.py show target-pipelines/wgsl.md`, the WGSL prompt, `_common.md`, and dependency docs.
- Checked front matter, required sections, `WGSL` / `WGSLSPIRV` / `WGSLSPIRVAssembly` handling, `legalizeIRForWGSL`, `specializeAddressSpaceForWGSL`, and Tint downstream handling against `source_commit` `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 147 relative links in the page at the recorded source commit.
- Spot-checked 13 WGSL claims, including source-target reduction, non-Khronos SSBO lowering, WGSL byte-address-buffer options, `resolveTextureFormat`, `legalizeIRForWGSL`, `floatNonUniformResourceIndex`, `legalizeLogicalAndOr`, address-space specialization, and loop absence.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: WGSL legalization, lowering, phi elimination` | The Phase A diagram/table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but the source runs those passes in Phase C, after `resolveTextureFormat` and before `legalizeIRForWGSL`. This makes the ordered WGSL pipeline materially wrong. | `source/slang/slang-emit.cpp:1955-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` in the Phase C range, not in the Phase A range around `source/slang/slang-emit.cpp:982-1001`. | Move those three nodes and rows from Phase A to Phase C, placing them after `resolveTextureFormat` and before `legalizeIRForWGSL`; update row numbering and Phase A prose. |
| F-002 | minor | Intro paragraph | The source-target reduction citation covers `WGSLSPIRV` but not `WGSLSPIRVAssembly`. The assembly target first maps to `WGSLSPIRV`, then the default source-target mapping reduces that intermediate target to `WGSL`. | `source/slang/slang-code-gen.cpp:271-272` maps `WGSLSPIRV` to `WGSL`; `source/slang/slang-code-gen.cpp:1059-1060` maps `WGSLSPIRVAssembly` to `WGSLSPIRV`. | Reword the sentence to describe the two-step assembly path and cite both line ranges. |
| F-003 | minor | `## Conditional gates`, WGSL-specific runtime predicates | The `isWGPUTarget(targetRequest)` row says that predicate selects the WGSL `legalizeByteAddressBufferOps` options, but those options are selected by an explicit `CodeGenTarget::WGSL`, `WGSLSPIRV`, and `WGSLSPIRVAssembly` switch arm instead. | `source/slang/slang-emit.cpp:1844-1859` sets the WGSL byte-address-buffer options in a target switch; `source/slang/slang-emit.cpp:2038-2040` and `source/slang/slang-emit.cpp:2217-2246` cover different WGSL predicates. | Move `legalizeByteAddressBufferOps` out of the `isWGPUTarget` row or add a separate row for the explicit WGSL target switch. |
