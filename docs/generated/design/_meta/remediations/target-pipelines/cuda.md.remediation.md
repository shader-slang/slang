---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:17:53Z
target_doc: target-pipelines/cuda.md
review_report: ../../reviews/target-pipelines/cuda.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/cuda.md

## Summary

All five review findings were verified against source at the target commit and
all five were correct and in-contract, so each was fixed with a minimal edit.
Two critical findings (F-001 PTX divergence inside `linkAndOptimizeIR`, F-002
wrong Phase-D artifact helper), two major findings (F-003 missing
`addUserTypeHintDecorations`, F-004 missing minimal-optimization SCCP+DCE arm),
and one minor finding (F-005 diagram/table label mismatch) were corrected. No
findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified at `source/slang/slang-emit.cpp:1163-1167`, `1183-1202`, `1204-1214`: only `CUDASource`/`CUDAHeader` take the OptiX-uniform and skip arms; `PTX` falls through the `default` arms and runs `collectEntryPointUniformParams`, `moveEntryPointUniformParamsToGlobalScope`, and `removeTorchAndCUDAEntryPoints` inside `linkAndOptimizeIR`. | Rewrote the intro so PTX is described as diverging inside `linkAndOptimizeIR`; split the Phase-A entry-point-uniform switch in the diagram into `CUDASource/CUDAHeader` and `PTX` branches; converted the two skip rows into numbered `(PTX)` rows plus a PTX `collectEntryPointUniformParams` row. |
| F-002 | fixed | Verified `createArtifactFromIR` (`source/slang/slang-emit.cpp:3070-3080`) is the SPIR-V-only helper that calls `emitSPIRVFromIR`; CUDA text is wrapped via `ArtifactUtil::createArtifactForCompileTarget` + `StringBlob::moveCreate` inside `emitEntryPointsSourceFromIR` (`source/slang/slang-emit.cpp:2752-2753`). | Replaced the `createArtifactFromIR` Phase-D diagram node and table row 7 with the direct text-wrapping performed in `emitEntryPointsSourceFromIR`. |
| F-003 | fixed | Verified `source/slang/slang-emit.cpp:1606-1609`: `addUserTypeHintDecorations` runs under `getBoolOption(VulkanEmitReflection)` with no CUDA-excluding gate, after `lowerCombinedTextureSamplers` and before `legalizeEmptyArray`. | Added the `addUserTypeHintDecorations` node to the Phase-B diagram, row 51b to the Phase-B table, and a `VulkanEmitReflection` row to the Option-set toggles gate table. |
| F-004 | fixed | Verified `source/slang/slang-emit.cpp:1555-1572`: under `fastIRSimplificationOptions.minimalOptimization` the pipeline runs `applySparseConditionalConstantPropagation` + `eliminateDeadCode`, otherwise `simplifyIR`. | Added a `minimalOptimization` diamond plus SCCP and paired `eliminateDeadCode` nodes to the Phase-B diagram and rows 49a/49b to the table, and qualified the existing `simplifyIR` row 49 as the else arm. |
| F-005 | fixed | Verified `source/slang/slang-emit.cpp:1623-1627`: `inlineGlobalConstantsForLegalization` is forced for `target == CodeGenTarget::CUDASource` only among the CUDA family; table row 54 and the prose already say CUDASource only. | Changed the Phase-B diagram node label from `CUDA always` to `CUDASource only` so it agrees with the table and prose. |
