---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:37+00:00
target_doc: target-pipelines/metal.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 751b986b2d853e8242f650fcb4a698ce747155b40fac3ebc58e2361363790674
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
  major: 1
  minor: 0
  nit: 0
---

# Review report for target-pipelines/metal.md

## Summary
The Metal page has the requested structure and all checked relative links resolve at the recorded source commit. Two source-alignment issues remain: the common Phase A/Phase C ordering error, and a Metal-only late lowering block that is missing from the Phase C table.

## Items checked
- Read `regenerate.py show target-pipelines/metal.md`, the Metal prompt, `_common.md`, and dependency docs.
- Checked front matter, required sections, the `Metal` / `MetalLib` / `MetalLibAssembly` branches, `legalizeIRForMetal`, Metal emitter construction, and Apple `metal` downstream handling against `source_commit` `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 153 relative links in the page at the recorded source commit.
- Spot-checked Metal claims for the MetalParameterBlock lowering, `wrapCBufferElementsForMetal`, `legalizeIRForMetal`, `specializeAddressSpaceForMetal`, `legalizeImageSubscript`, `DescriptorHandle<T>` emission, and loop absence.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: Metal legalization, lowering, phi elimination` | The Phase A diagram/table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but the source runs those passes in Phase C before the target legalization switch. This makes the ordered Metal pipeline materially wrong. | `source/slang/slang-emit.cpp:1955-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` in the Phase C range, not in the Phase A range around `source/slang/slang-emit.cpp:982-1001`. | Move those three nodes and rows from Phase A to Phase C, placing them before `legalizeIRForMetal`; update row numbering and Phase A prose. |
| F-002 | major | `## Phase C: Metal legalization, lowering, phi elimination` | The Phase C table stops before a reachable Metal-only late-lowering block. For Metal targets, `linkAndOptimizeIR` runs an additional `lowerBufferElementTypeToStorageType` with `MetalPointerLowering`, then `performForceInlining`, a second `eliminatePhis`, and another `simplifyNonSSAIR` before final validation and emit. | `source/slang/slang-emit.cpp:2402-2437` contains the `if (isMetalTarget(targetRequest))` block with those four `SLANG_PASS` calls. | Add this Metal-only block after the existing `simplifyNonSSAIR` row in Phase C, with gates and notes explaining the `MetalPointerLowering` policy and the second phi-elimination pass. |
