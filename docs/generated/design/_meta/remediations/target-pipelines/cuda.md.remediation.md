---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: target-pipelines/cuda.md
review_report: ../../reviews/target-pipelines/cuda.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/cuda.md

## Summary

All seven findings were fixed. The three entry-point-shape passes
moved from Phase A to Phase C; PTX-divergent behavior was documented
for the Phase A uniform/torch switches, `lowerCooperativeVectors`, and
`inlineGlobalConstantsForLegalization`; `floatNonUniformResourceIndex`
was corrected from skipped to a real CUDA Phase C pass; `removeTorchKernels`
was removed from the PyTorch adjacent bullet; and the Phase D table
header was fixed to `Pass`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-emit.cpp:1952-1962` runs the three varying passes after `synthesizeActiveMask`/`resolveTextureFormat` and before `legalizeEntryPointVaryingParamsForCUDA` (2017), not in Phase A. | Moved `translateGlobalVaryingVar`/`resolveVaryingInputRef`/`fixEntryPointCallsites` from Phase A (diagram+rows 6-8) into Phase C (new rows 4-6); renumbered both tables. |
| F-002 | fixed | `source/slang/slang-emit.cpp:1124-1173` puts only `CUDASource`/`CUDAHeader` in the OptiX/skip case lists; `PTX` falls through the `default` arms (1137, 1147, 1173). | Updated Phase A prose and the three affected table rows to state PTX runs `collectEntryPointUniformParams`, `moveEntryPointUniformParamsToGlobalScope`, and `removeTorchAndCUDAEntryPoints`. |
| F-003 | fixed | `source/slang/slang-emit.cpp:1505-1506` runs `lowerCooperativeVectors` unconditionally in the `default` arm reached by `CUDAHeader`/`PTX`. | Updated the Phase B prose, diagram node label, table row 47 gate, and conditional-gates row to distinguish the `CUDASource` capability gate from the `CUDAHeader`/`PTX` unconditional default arm. |
| F-004 | fixed | `source/slang/slang-emit.cpp:1584-1586` short-circuits only `target == CUDASource`; `2624-2627` sets `shouldLegalizeExistentialAndResourceTypes=false` for CUDA, so `CUDAHeader`/`PTX` skip the pass. | Reworded the four "CUDA always runs `inlineGlobalConstantsForLegalization`" claims (Phase B prose, table row, two Notable-passes sections) to scope it to `CUDASource`. |
| F-005 | fixed | `source/slang/slang-emit.cpp:2033-2035` calls `floatNonUniformResourceIndex` for every non-SPIR-V target; the four-way gate at 2038 governs only `legalizeLogicalAndOr`. | Converted the skipped pseudo-node to a real Phase C pass (new row 8, gate `!isSPIRV`) and removed `floatNonUniformResourceIndex` from the filtered-out list, keeping `legalizeLogicalAndOr` skipped. |
| F-006 | fixed | `source/slang/slang-emit.cpp:1353-1357` (PyTorch arm) does not call `removeTorchKernels`; `1359-1363` (CUDA arm) does. | Removed `removeTorchKernels` from the PyTorch adjacent-target bullet and noted it is CUDASource/CUDAHeader-only. |
| F-007 | fixed | `docs/generated/design/_meta/prompts/_common.md:326-335` requires the column header `Pass`. | Renamed the Phase D table header column `Pass / step` to `Pass`. |
