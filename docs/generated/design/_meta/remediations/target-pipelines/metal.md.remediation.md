---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: target-pipelines/metal.md
review_report: ../../reviews/target-pipelines/metal.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for target-pipelines/metal.md

## Summary

Three findings were fixed and one deferred. The three entry-point-shape
passes were moved from Phase A to Phase C (before `legalizeIRForMetal`);
the reachable Metal-only late-lowering block (`lowerBufferElementTypeToStorageType`
with `MetalPointerLowering`, `performForceInlining`, a second `eliminatePhis`,
and a second `simplifyNonSSAIR`) was added to the Phase C diagram and table;
and the sibling-target skipped pseudo-nodes were removed from the Phase C
diagram. F-004 (adding gate diamonds to the Phase B diagram) was deferred
because it requires reconstructing the whole ~64-pass Phase B control-flow
graph, a generation-scale rewrite.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-emit.cpp:1952-1962` runs the three varying passes after specialization and before the target switch (`legalizeIRForMetal` at 1997), not in Phase A. | Moved `translateGlobalVaryingVar`/`resolveVaryingInputRef`/`fixEntryPointCallsites` from Phase A (diagram+rows 6-8) into Phase C (new rows 3-5); renumbered Phase A (→17) and Phase C. |
| F-002 | fixed | `source/slang/slang-emit.cpp:2402-2437` runs a Metal-only `lowerBufferElementTypeToStorageType` (MetalPointerLowering), `performForceInlining`, `eliminatePhis`, `simplifyNonSSAIR` after the main `simplifyNonSSAIR`. | Added the four-pass Metal-only block to the Phase C diagram and table (new rows 32-35) with gates/notes; Phase C now 40 rows. |
| F-003 | fixed | `docs/generated/design/_meta/prompts/_common.md:320-323` requires sibling-target branches to be filtered out of phase diagrams, not drawn as pseudo-nodes. | Removed the `synthesizeActiveMask`, `resolveTextureFormat`, and `legalizeArrayReturnType` skipped pseudo-nodes from the Phase C diagram (they remain in the filtered-out prose). |
| F-004 | deferred | Adding the required gate diamonds means rebuilding the entire ~64-pass Phase B control-flow diagram (as on the SPIR-V/WGSL pages); this is a generation-scale rewrite, not a surgical edit, and the companion Phase B table already encodes every gate. Follow-up: regenerate the page or rebuild the Phase B diagram with gate diamonds. | — |
