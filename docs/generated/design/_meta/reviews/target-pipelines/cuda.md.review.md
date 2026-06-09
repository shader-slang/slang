---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:26+00:00
target_doc: target-pipelines/cuda.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 697128ce2b824225966f830d21ac028faff0ac0feee6366f3549b55ad9f0d9b1
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 7
severity_breakdown:
  critical: 1
  major: 4
  minor: 2
  nit: 0
---

# Review report for target-pipelines/cuda.md

## Summary
The CUDA page has the required high-level shape and all checked relative links resolve at the recorded source commit. However, the ordered pass view is only partially aligned with `linkAndOptimizeIR`: Phase A includes three passes that actually run in Phase C, and several PTX-reachable branches are either omitted or described as skipped. The most important fix is to split the CUDA source/header path from the PTX path wherever the source uses explicit `CodeGenTarget` switch arms.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show target-pipelines/cuda.md` and used the target front matter source commit and digest in this report.
- Read the CUDA target doc, `_common.md`, `target-pipelines-cuda.md`, and the dependency docs `pipeline/04-ast-to-ir.md`, `pipeline/05-ir-passes.md`, `pipeline/06-emit.md`, `ir-reference/index.md`, and `cross-cutting/targets.md`.
- Resolved all 155 relative Markdown links at `52339028a2aa703271533454c6b9528a534bac31`; no dangling links were found.
- Checked the required target-pipeline sections, front matter keys, Phase D table columns, conditional-gate grouping, loops section, adjacent-target section, and peer links.
- Verified more than 20 factual claims against source at the target commit, including the `linkAndOptimizeIR` phase ranges, OptiX uniform collection, PTX branch behavior, cooperative-vector lowering, existential/resource legalization options, CUDA immutable-load lowering, `synthesizeActiveMask`, CUDA varying-param legalization, phi elimination options, and nvrtc pass-through.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase A: Link and entry-point prep` and `## Phase C: CUDA legalization, lowering, phi elimination` | The Phase A diagram and table place `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites` before uniform collection, but the source runs those passes later in the Phase C range after target-specific active-mask and texture-format handling. This makes the ordered CUDA pipeline materially wrong. | `source/slang/slang-emit.cpp:982-1001` has only SSBO lowering, `translateEntryPointInParamToBorrow`, `replaceGlobalConstants`, and `bindExistentialSlots` in this early region; `source/slang/slang-emit.cpp:1952-1962` runs `translateGlobalVaryingVar`, `resolveVaryingInputRef`, and `fixEntryPointCallsites`. | Move those three nodes and rows from Phase A to Phase C, placing them after the CUDA `synthesizeActiveMask` and filtered `resolveTextureFormat` point and before `legalizeEntryPointVaryingParamsForCUDA`; update Phase A prose and row numbering. |
| F-002 | major | `## Phase A: Link and entry-point prep` | The page covers `PTX`, but Phase A describes the CUDA family as skipping `collectEntryPointUniformParams`, `moveEntryPointUniformParamsToGlobalScope`, and `removeTorchAndCUDAEntryPoints`. That is true for `CUDASource` and `CUDAHeader`, but `PTX` falls through the default arms and runs those passes. | `source/slang/slang-emit.cpp:1124-1138` sends only `CUDASource` and `CUDAHeader` to `collectOptiXEntryPointUniformParams`, so `PTX` reaches `collectEntryPointUniformParams`; `source/slang/slang-emit.cpp:1144-1148` runs `moveEntryPointUniformParamsToGlobalScope` in the default arm; `source/slang/slang-emit.cpp:1165-1173` runs `removeTorchAndCUDAEntryPoints` in the default arm. | Split the Phase A flow into a CUDA source/header arm and a PTX arm, or explicitly add PTX rows for those default passes instead of showing them as skipped for the whole CUDA target family. |
| F-003 | major | `## Phase B: Specialization and type legalization` | The page gates `lowerCooperativeVectors` only on `case CUDASource` with the `optix_coopvec` capability check, omitting the PTX path. In source, `PTX` reaches the `default` switch arm and runs `lowerCooperativeVectors` unconditionally for this switch. | `source/slang/slang-emit.cpp:1492-1506` breaks for SPIR-V and HLSL, conditionally runs the pass for `CUDASource`, and then runs `lowerCooperativeVectors` in the `default` arm that includes `PTX`. | Add the PTX/default branch to the Phase B diagram, table, and conditional-gates section, distinguishing it from the `CUDASource` capability-gated branch. |
| F-004 | major | `## Phase B: Specialization and type legalization` and `## Notable passes` | The page repeatedly says CUDA always runs `inlineGlobalConstantsForLegalization`, but the source only short-circuits on `CodeGenTarget::CUDASource` among the CUDA family. With CUDA source-language emission, the broader existential/resource legalization option is set false, so the generated text overstates this pass for `CUDAHeader` and `PTX`. | `source/slang/slang-emit.cpp:1577-1588` runs `inlineGlobalConstantsForLegalization` for `CUDASource`, CPU kernel targets, or when `shouldLegalizeExistentialAndResourceTypes` is true; `source/slang/slang-emit.cpp:2624-2627` sets that option false for `SourceLanguage::CUDA`. | Change the CUDA-wide wording to say the pass is always forced for `CUDASource`; separately document whether `CUDAHeader` and `PTX` reach it through another option path or omit it from those rows. |
| F-005 | major | `## Phase C: CUDA legalization, lowering, phi elimination` | The page marks `floatNonUniformResourceIndex` as skipped for CUDA, but the source calls it for every non-SPIR-V target before the narrower `legalizeLogicalAndOr` gate. The document appears to apply the later four-way gate to the wrong pass. | `source/slang/slang-emit.cpp:2033-2035` calls `floatNonUniformResourceIndex` whenever the target is not SPIR-V; the four-way D3D/Khronos/WGPU/Metal condition begins only at `source/slang/slang-emit.cpp:2038-2040` for `legalizeLogicalAndOr`. | Add `floatNonUniformResourceIndex` as a Phase C pass for CUDA with the non-SPIR-V gate, and keep only `legalizeLogicalAndOr` in the skipped CUDA list. |
| F-006 | minor | `## Adjacent targets` | The PyTorch adjacent-target bullet lists `removeTorchKernels` as part of the PyTorch arm, but the source PyTorch branch does not call that pass. `removeTorchKernels` is in the CUDA source/header branch instead. | `source/slang/slang-emit.cpp:1353-1357` runs `generateHostFunctionsForAutoBindCuda`, `lowerBuiltinTypesForKernelEntryPoints`, `generatePyTorchCppBinding`, and `handleAutoBindNames` for `PyTorchCppBinding`; `source/slang/slang-emit.cpp:1359-1363` runs `removeTorchKernels` for `CUDASource` and `CUDAHeader`. | Remove `removeTorchKernels` from the PyTorch adjacent-target list and mention it only in the CUDA source/header branch. |
| F-007 | minor | `## Phase D: CUDA emit and downstream tools` | The Phase D table header uses `Pass / step` instead of the required `Pass` column. The contract requires the ordered phase table columns to be exactly `#`, `Pass`, `File`, `Gate`, and `Notes`. | `docs/generated/design/_meta/prompts/_common.md:326-335` defines the required table columns; the target doc Phase D table header uses `Pass / step`. | Rename the Phase D table column to `Pass`, keeping downstream compiler rows in that column as pass-like steps if needed. |
