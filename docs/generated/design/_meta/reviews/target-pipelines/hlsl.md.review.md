---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:36+00:00
target_doc: target-pipelines/hlsl.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: f2252c95d32fee4775bd65d49036aee55db1092712080349ad9e8984834f2521
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
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

# Review report for target-pipelines/hlsl.md

## Summary
The page is broadly aligned with the HLSL path through `linkAndOptimizeIR`, and the required sections, front matter, links, HLSL/DXIL/DXBytecode coverage, loop statement, and phase-table columns are present. Two issues need remediation: Phase D routes HLSL artifacts through the SPIR-V-only `createArtifactFromIR` helper, and Phase B omits reachable option-gated `SLANG_PASS` calls.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show target-pipelines/hlsl.md` and used the target document's front-matter commit and digest in this report.
- Read the target document, `_review.md`, `_common.md`, `target-pipelines-hlsl.md`, and dependency docs `pipeline/04-ast-to-ir.md`, `pipeline/05-ir-passes.md`, `pipeline/06-emit.md`, `ir-reference/index.md`, and `cross-cutting/targets.md`.
- Checked the ordered HLSL-reachable `SLANG_PASS` calls in `source/slang/slang-emit.cpp` from `linkIR` through `checkUnsupportedInst`, including HLSL/D3D gates, byte-address-buffer options, liveness and phi-elimination handling, and filtered sibling-target branches.
- Verified target-specific HLSL legalization and emit references in `slang-ir-hlsl-legalize.cpp`, `slang-ir-legalize-binary-operator.cpp`, `slang-ir-byte-address-legalize.*`, `slang-ir-wrap-structured-buffers.cpp`, `slang-emit-hlsl.cpp`, and `slang-emit-hlsl-prelude.cpp`.
- Checked the downstream DXIL/DXBytecode path in `source/slang/slang-code-gen.cpp`, `source/slang/slang-pass-through.cpp`, and `source/compiler-core/slang-dxc-compiler.cpp`.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase D: HLSL emit and downstream tools`; intro paragraph | The document says downstream binary requests diverge in `createArtifactFromIR`, and Phase D includes a `createArtifactFromIR` row that "wraps the HLSL text as an `IArtifact`." That helper is not on the HLSL, DXIL, or DXBytecode path; it is a SPIR-V direct-emission helper. This sends readers to the wrong artifact construction and downstream-compile entry points. | `source/slang/slang-emit.cpp:3070-3072` says `createArtifactFromIR` is "used internally by emitSPIRVForEntryPointsDirectly", and `source/slang/slang-emit.cpp:3254-3260` is its only call site. HLSL text artifacts are created in `emitEntryPointsSourceFromIR` at `source/slang/slang-emit.cpp:2752-2753`. DXIL/DXBytecode select HLSL as the source target at `source/slang/slang-code-gen.cpp:246-266`, emit that source at `source/slang/slang-code-gen.cpp:527-541`, and enter the downstream path at `source/slang/slang-code-gen.cpp:1161-1167`. | Remove `createArtifactFromIR` from the HLSL Phase D diagram/table and intro. Replace it with the actual HLSL text artifact creation in `emitEntryPointsSourceFromIR`, and describe DXIL/DXBytecode as flowing through `emitWithDownstreamForEntryPoints` after `_getDefaultSourceForTarget` maps them to `CodeGenTarget::HLSL`. |
| F-002 | major | `## Phase B: Specialization and type legalization`; `## Conditional gates` | Phase B omits HLSL-reachable `SLANG_PASS` calls under option gates: the minimal-optimization branch after `performForceInlining` runs `applySparseConditionalConstantPropagation` and `eliminateDeadCode`, and `VulkanEmitReflection` runs `addUserTypeHintDecorations`. The target-pipeline contract requires every reachable `SLANG_PASS` call to appear in exactly one phase table, with the selecting gate recorded. | `source/slang/slang-emit.cpp:1555-1567` runs `applySparseConditionalConstantPropagation` and `eliminateDeadCode` when `fastIRSimplificationOptions.minimalOptimization` is true. `source/slang/slang-emit.cpp:1606-1609` runs `addUserTypeHintDecorations` when `CompilerOptionName::VulkanEmitReflection` is set. The Phase B table jumps from `performForceInlining` to `simplifyIR` and then to combined-texture/resource legalization without these rows. | Add the missing Phase B nodes and rows with gates `fastIRSimplificationOptions.minimalOptimization` and `getBoolOption(VulkanEmitReflection)`. Update the conditional-gates section so those option-set toggles list the passes they control, and keep the row order matching `slang-emit.cpp`. |
