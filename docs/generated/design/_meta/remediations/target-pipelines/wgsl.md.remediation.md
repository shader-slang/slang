---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: target-pipelines/wgsl.md
review_report: ../../reviews/target-pipelines/wgsl.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/wgsl.md

## Summary

All three findings were fixed. The three entry-point-shape passes
were moved from Phase A to Phase C (after `resolveTextureFormat`,
before `legalizeIRForWGSL`) with both tables renumbered. The intro
now describes the two-step `WGSLSPIRVAssembly` -> `WGSLSPIRV` -> `WGSL`
source-target reduction, and the byte-address-buffer options gate was
moved from the `isWGPUTarget` row to a dedicated `CodeGenTarget` switch
row.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-emit.cpp:1952-1962` runs the three varying passes after `resolveTextureFormat` (1943-1950) and before `legalizeIRForWGSL` (2025), not in the Phase A region. | Moved `translateGlobalVaryingVar`/`resolveVaryingInputRef`/`fixEntryPointCallsites` from Phase A (diagram+rows 6-8) into Phase C (new rows 4-6); renumbered Phase A (→17) and Phase C (→34). |
| F-002 | fixed | `source/slang/slang-code-gen.cpp:1059-1060` maps `WGSLSPIRVAssembly`→`WGSLSPIRV`; `271-272` maps `WGSLSPIRV`→`WGSL`. | Reworded the intro to describe the two-step reduction and cite both line ranges. |
| F-003 | fixed | `source/slang/slang-emit.cpp:1852-1860` sets the WGSL byte-address options in a `CodeGenTarget::WGSL`/`WGSLSPIRV`/`WGSLSPIRVAssembly` switch, not via `isWGPUTarget`. | Removed `legalizeByteAddressBufferOps` from the `isWGPUTarget` gate row and added a dedicated target-switch row. |
