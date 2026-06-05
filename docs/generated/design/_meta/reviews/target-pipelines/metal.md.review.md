---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:06:52+00:00
target_doc: target-pipelines/metal.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 751b986b2d853e8242f650fcb4a698ce747155b40fac3ebc58e2361363790674
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
  major: 3
  minor: 0
  nit: 0
---

# Review report for target-pipelines/metal.md

## Summary
The Metal page has the requested structure and all checked relative links resolve at the recorded source commit. The most important issue is that several entry-point-shape passes are documented in Phase A even though source runs them later in Phase C. The page also omits a reachable Metal-only late-lowering block and relaxes required phase-diagram rules.

## Items checked
- Read `regenerate.py show target-pipelines/metal.md`, the Metal prompt, `_common.md`, and dependency docs.
- Checked front matter, required sections, the `Metal` / `MetalLib` / `MetalLibAssembly` branches, `legalizeIRForMetal`, Metal emitter construction, and Apple `metal` downstream handling against `source_commit` `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 153 relative links in the page at the recorded source commit.
- Spot-checked 14 Metal claims, including the MetalParameterBlock lowering, `wrapCBufferElementsForMetal`, `legalizeIRForMetal`, `specializeAddressSpaceForMetal`, `legalizeImageSubscript`, Metal pointer lowering, `DescriptorHandle<T>` emission, and loop absence.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: Metal legalization, lowering, phi elimination` | The Phase A diagram/table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but the source runs those passes in Phase C before the target legalization switch. This makes the ordered Metal pipeline materially wrong. | `source/slang/slang-emit.cpp:1955-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` in the Phase C range, not in the Phase A range around `source/slang/slang-emit.cpp:982-1001`. | Move those three nodes and rows from Phase A to Phase C, placing them before `legalizeIRForMetal`; update row numbering and Phase A prose. |
| F-002 | major | `## Phase C: Metal legalization, lowering, phi elimination` | The Phase C table stops before a reachable Metal-only late-lowering block. For Metal targets, `linkAndOptimizeIR` runs an additional `lowerBufferElementTypeToStorageType` with `MetalPointerLowering`, then `performForceInlining`, a second `eliminatePhis`, and another `simplifyNonSSAIR` before final validation and emit. | `source/slang/slang-emit.cpp:2402-2437` contains the `if (isMetalTarget(targetRequest))` block with those four `SLANG_PASS` calls. | Add this Metal-only block after the existing `simplifyNonSSAIR` row in Phase C, with gates and notes explaining the `MetalPointerLowering` policy and the second phi-elimination pass. |
| F-003 | major | `## Phase C: Metal legalization, lowering, phi elimination` | The Phase C diagram includes explicit skipped nodes for CUDA, GLSL, SPIR-V, WGSL, and array-return branches that cannot fire for Metal. The target-pipeline contract requires sibling-target branches to be filtered out rather than drawn as pseudo-pass nodes. | `docs/generated/design/_meta/prompts/_common.md:320-323` says branches that cannot fire for the target are filtered out. The source switches at `source/slang/slang-emit.cpp:1927-1950` and `source/slang/slang-emit.cpp:2203-2206` show these skipped branches are not Metal pass calls. | Remove the skipped pseudo-nodes from the Metal Phase C diagram and keep the non-Metal exclusions only in the filtered-out prose. |
| F-004 | major | `## Phase B: Specialization and type legalization` | The Phase B diagram states that conditional gates are omitted for readability, but the contract requires conditional gates to be diamond nodes in each phase diagram. The companion table has gates, but the required control-flow-aware diagram is incomplete. | `docs/generated/design/_meta/prompts/_common.md:311-319` requires conditional gates and loop edges in phase diagrams. The document text under the Phase B diagram says conditional gates are omitted. | Add the Phase B gate diamonds or split Phase B into smaller diagrams so the required gate structure remains visible. |
