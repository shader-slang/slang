---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:18:14+00:00
target_doc: target-pipelines/cuda.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 7274909025eca0a18bb6ccaa2a1ad7cbbbbebe01136637c939696e0458eecc22
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 2
  major: 2
  minor: 1
  nit: 0
---

# Review report for target-pipelines/cuda.md

## Summary
The CUDA page has the required overall shape, valid front matter, and all checked relative links resolve at the recorded source commit. The strongest remaining issue is that the PTX path is still partly collapsed into the `CUDASource` / `CUDAHeader` path: the intro says PTX diverges only after `linkAndOptimizeIR`, while Phase A leaves PTX-reachable `SLANG_PASS` calls as unnumbered skip notes. Phase D also points readers at the SPIR-V-only `createArtifactFromIR` helper for CUDA text artifacts.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show target-pipelines/cuda.md` and used the target front matter source commit and digest in this report.
- Read the CUDA target doc, `_common.md`, `target-pipelines-cuda.md`, and dependency docs `pipeline/04-ast-to-ir.md`, `pipeline/05-ir-passes.md`, `pipeline/06-emit.md`, `ir-reference/index.md`, and `cross-cutting/targets.md`.
- Resolved all 156 relative Markdown links at `eb9403ef595a99c2ff6def1d538dbd7a792d9371`; no dangling links were found.
- Checked required target-pipeline sections, phase table columns, conditional-gate grouping, loop coverage, adjacent-target coverage, front matter keys, and downstream PTX/NVRTC claims.
- Verified more than 20 factual claims against source at the target commit, including ordered `SLANG_PASS` calls in `linkAndOptimizeIR`, CUDA/PTX switch arms, `shouldLegalizeExistentialAndResourceTypes`, cooperative-vector lowering, CUDA varying-param legalization, immutable-buffer-load lowering, phi elimination, CUDA source emission, and PTX pass-through selection.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Intro and `## Phase A: Link and entry-point prep` | The page says `PTX` diverges only in `createArtifactFromIR` and downstream compile dispatch, and Phase A lists `collectEntryPointUniformParams`, `moveEntryPointUniformParamsToGlobalScope`, and `removeTorchAndCUDAEntryPoints` only as skipped notes. In source, `PTX` diverges inside `linkAndOptimizeIR`: it falls through the default arms and runs those three `SLANG_PASS` calls, while only `CUDASource` / `CUDAHeader` take the OptiX-uniform and skip arms. | `source/slang/slang-emit.cpp:1163-1177` sends only `CUDASource` / `CUDAHeader` to `collectOptiXEntryPointUniformParams`; `PTX` reaches `collectEntryPointUniformParams`. `source/slang/slang-emit.cpp:1183-1212` excludes only `CUDASource` / `CUDAHeader` from `moveEntryPointUniformParamsToGlobalScope` and `removeTorchAndCUDAEntryPoints`, so `PTX` runs both default-arm passes. | Split Phase A into explicit `CUDASource` / `CUDAHeader` and `PTX` branches, add numbered PTX rows for the three default-arm passes, and change the intro so PTX is not described as diverging only after `linkAndOptimizeIR`. |
| F-002 | critical | `## Phase D: CUDA emit and downstream tools` | The Phase D diagram and table route CUDA output through `createArtifactFromIR`, but that helper is documented in source as the internal helper for `emitSPIRVForEntryPointsDirectly` and immediately calls `emitSPIRVFromIR`. CUDA source emission instead wraps `sourceWriter` text directly in an artifact inside `emitEntryPointsSourceFromIR`. | `source/slang/slang-emit.cpp:2752-2754` creates the CUDA-family text artifact with `ArtifactUtil::createArtifactForCompileTarget` and `StringBlob::moveCreate(finalResult)`. `source/slang/slang-emit.cpp:3070-3080` says `createArtifactFromIR` is used by `emitSPIRVForEntryPointsDirectly` and calls `emitSPIRVFromIR`; `source/slang/slang-emit.cpp:3254-3260` calls it from the direct SPIR-V path. | Replace the `createArtifactFromIR` node and row in Phase D with the direct artifact wrapping performed in `emitEntryPointsSourceFromIR`; keep the downstream `PTX` / NVRTC branch as a later artifact transition rather than a call to the SPIR-V helper. |
| F-003 | major | `## Phase B: Specialization and type legalization` | The ordered Phase B table omits the reachable `addUserTypeHintDecorations` `SLANG_PASS`. The source runs it whenever `CompilerOptionName::VulkanEmitReflection` is true, without a CUDA-excluding target gate, so it is part of the target pipeline under that option. | `source/slang/slang-emit.cpp:1606-1609` checks `getBoolOption(CompilerOptionName::VulkanEmitReflection)` and then calls `SLANG_PASS(addUserTypeHintDecorations)` before `legalizeEmptyArray`. | Add `addUserTypeHintDecorations` after `lowerCombinedTextureSamplers` and before `legalizeEmptyArray` in the Phase B diagram and table, and add the `VulkanEmitReflection` gate to `## Conditional gates`. |
| F-004 | major | `## Phase B: Specialization and type legalization` | The Phase B table shows only the non-minimal `simplifyIR` path after `performForceInlining`, but source runs a different pair of `SLANG_PASS` calls when `fastIRSimplificationOptions.minimalOptimization` is true: `applySparseConditionalConstantPropagation` followed by `eliminateDeadCode`. Omitting that conditional arm violates the ordered target-pipeline contract for reachable pass calls. | `source/slang/slang-emit.cpp:1555-1571` selects `applySparseConditionalConstantPropagation` and `eliminateDeadCode` under `fastIRSimplificationOptions.minimalOptimization`, otherwise `simplifyIR`. | Add a minimal-optimization diamond in Phase B and rows for `applySparseConditionalConstantPropagation` and the paired `eliminateDeadCode`, with the existing `simplifyIR` row on the false branch. |
| F-005 | minor | `## Phase B: Specialization and type legalization` diagram | The Phase B diagram labels `inlineGlobalConstantsForLegalization` as `CUDA always`, but the source only forces it for `CodeGenTarget::CUDASource` among the CUDA family when `shouldLegalizeExistentialAndResourceTypes` is false. The prose and table later say `CUDAHeader` and `PTX` skip it, so the diagram is the inconsistent part. | `source/slang/slang-emit.cpp:1623-1627` runs `inlineGlobalConstantsForLegalization` for `target == CodeGenTarget::CUDASource`, CPU kernel targets, or `options.shouldLegalizeExistentialAndResourceTypes`; `source/slang/slang-emit.cpp:2663-2666` sets that option false for `SourceLanguage::CUDA`. | Change the diagram label to `CUDASource only` or add a gate matching the table expression so the diagram agrees with the source and companion table. |
