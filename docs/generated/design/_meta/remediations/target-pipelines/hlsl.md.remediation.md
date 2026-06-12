---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:17:56Z
target_doc: target-pipelines/hlsl.md
review_report: ../../reviews/target-pipelines/hlsl.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/hlsl.md

## Summary

Both findings were verified against the source at `eb9403ef` and
fixed. F-001 (critical) corrected the Phase D artifact-construction
path: `createArtifactFromIR` is the SPIR-V-direct helper and is not
on the HLSL path, so the intro, the Phase D diagram, and the Phase D
table now reference `createArtifactForCompileTarget` in
`emitEntryPointsSourceFromIR` and route DXIL/DXBytecode through
`emitWithDownstreamForEntryPoints`. F-002 (major) added the two
omitted Phase B `SLANG_PASS` calls (the `minimalOptimization`
SCCP+DCE branch and `addUserTypeHintDecorations` under
`VulkanEmitReflection`) to the diagram, the ordered table, and the
conditional-gates section.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified at `slang-emit.cpp:3070-3072` that `createArtifactFromIR` is "used internally by emitSPIRVForEntryPointsDirectly" and is only called at line 3260 inside `emitSPIRVForEntryPointsDirectly`. The HLSL text artifact is created at `slang-emit.cpp:2752` (`createArtifactForCompileTarget`) inside `emitEntryPointsSourceFromIR`, and DXIL/DXBytecode dispatch through `emitWithDownstreamForEntryPoints` (`slang-code-gen.cpp:353`) after `_getDefaultSourceForTarget` (`slang-code-gen.cpp:246-266`) maps them to `CodeGenTarget::HLSL`. | Replaced `createArtifactFromIR` in the intro, the Phase D diagram, and Phase D table row 7 with `createArtifactForCompileTarget`; added an `emitWithDownstreamForEntryPoints` dispatch node and noted it on the DXC/fxc rows. |
| F-002 | fixed | Verified at `slang-emit.cpp:1555-1567` that the `fastIRSimplificationOptions.minimalOptimization` branch runs `applySparseConditionalConstantPropagation` + `eliminateDeadCode` (else-arm is the already-listed `simplifyIR`), and at `slang-emit.cpp:1606-1609` that `addUserTypeHintDecorations` runs under `getBoolOption(VulkanEmitReflection)`. Both are HLSL-reachable (no sibling-target gate), so the contract's coverage rule requires them in a phase table. | Added the SCCP+DCE minimal-optimization rows and the `addUserTypeHintDecorations` row to the Phase B diagram and ordered table (renumbering subsequent rows), and added the `getBoolOption(VulkanEmitReflection)` option-set toggle. |
