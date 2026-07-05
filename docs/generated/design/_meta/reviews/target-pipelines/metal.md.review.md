---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:46:47+00:00
target_doc: target-pipelines/metal.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 3bfc164e382505a7acce894d60950a1812eb10280d5da247c705758df95dccb7
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
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
The page is mostly aligned with the Metal pass sequence in `linkAndOptimizeIR`, but its downstream-target narrative is misleading for `MetalLibAssembly`. The most important problem is that the document describes the normal assembly target as skipping `wrapCBufferElementsForMetal`, while the public emission path first produces `MetalLib` from `Metal` source and therefore runs the `CodeGenTarget::Metal` source pipeline before disassembly.

## Items checked
- Ran `regenerate.py show target-pipelines/metal.md` and reviewed the target document, `_common.md`, the per-document prompt, and the resolved watched files for this page.
- Checked the target document front matter for all required keys, the recorded source commit, the required warning string, and a 64-character hex watched-path digest.
- Read dependency context from `pipeline/04-ast-to-ir.md`, `pipeline/05-ir-passes.md`, `pipeline/06-emit.md`, `ir-reference/index.md`, and `cross-cutting/targets.md`.
- Verified the required target-pipeline sections: Source, High-level phase diagram, four phase sections, Conditional gates, Loops in the pipeline, Notable passes, and See also.
- Spot-checked more than 10 source-backed claims, including `linkAndOptimizeIR`, `legalizeIRForMetal`, `legalizeEntryPointVaryingParamsForMetal`, `lowerCombinedTextureSamplers`, `legalizeEmptyTypes`, `wrapCBufferElementsForMetal`, `legalizeByteAddressBufferOps`, `specializeAddressSpaceForMetal`, `MetalSourceEmitter::emitFuncParamLayoutImpl`, `emitEntryPointsSourceFromIR`, and the `MetalLib` / `MetalLibAssembly` downstream transitions.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Intro, lines 16-22; Phase B, lines 178-182; Notable passes, lines 836-840 | The document says `MetalLibAssembly` skips `wrapCBufferElementsForMetal`, including the claim `a direct request for MetalLibAssembly would not get the wrap`. That is not the normal public emission path: a `MetalLibAssembly` request first emits an intermediate `MetalLib`, and the `MetalLib` request emits intermediate `Metal` source, so `linkAndOptimizeIR` sees `CodeGenTarget::Metal` and runs the switch arm containing `wrapCBufferElementsForMetal`. | `source/slang/slang-code-gen.cpp:1047-1058` maps `MetalLibAssembly` to intermediate `MetalLib`; `source/slang/slang-code-gen.cpp:1089-1101` emits that intermediate target; `source/slang/slang-code-gen.cpp:246-270` maps `MetalLib` to source target `Metal`; `source/slang/slang-code-gen.cpp:527-531` emits that source target; `source/slang/slang-emit.cpp:1812-1816` runs `wrapCBufferElementsForMetal` for `CodeGenTarget::Metal`. | Remove the skip/inconsistency narrative for `MetalLibAssembly`. State that the assembly target is produced through the `MetalLib` downstream path and therefore inherits the Metal-source IR pass sequence; if the raw `CodeGenTarget::MetalLibAssembly` switch behavior is worth mentioning, clearly mark it as not the ordinary `_emitEntryPoints` route. |
| F-002 | major | Phase D, lines 620-622 and 642-651; Conditional gates, lines 717-720; Downstream Apple `metal` compiler, lines 891-895 | The Phase D table collapses `MetalLib` and `MetalLibAssembly` into one `compile` step gated by both binary targets, and the prose says the Apple `metal` compiler produces `.metallib` "or its disassembly." The source has two downstream steps: `MetalLib` compiles Metal source through the `MetalC` transition, while `MetalLibAssembly` first gets an intermediate `MetalLib` artifact and then disassembles it with `metal-objdump --disassemble`. | `source/slang/slang-global-session.cpp:217-234` declares separate `Metal` to `MetalLib` and `MetalLib` to `MetalLibAssembly` transitions; `source/slang/slang-code-gen.cpp:1089-1111` disassembles the intermediate artifact for assembly targets; `source/compiler-core/slang-metal-compiler.cpp:45-63` implements Metal AIR disassembly with `metal-objdump --disassemble`. | Split the Phase D downstream branch into `Metal` source-only, `MetalLib` compile via Apple `metal`, and `MetalLibAssembly` intermediate `MetalLib` plus `(downstream) metal-objdump --disassemble`. Update the conditional gates table and notable downstream callout to name the disassembler explicitly. |

## No-issues notes
- The front matter uses the required keys and preserves the target document's recorded `source_commit` and `watched_paths_digest`.
- The Metal legalization callout matches `legalizeIRForMetal`: it runs `legalizeSubpassInputsForMetal`, `legalizeEntryPointVaryingParamsForMetal`, and `processInst`.
- The main Phase C table correctly captures the Metal byte-address-buffer options, `floatNonUniformResourceIndex` textual mode, default phi elimination, late Metal pointer lowering, and `collectMetadata(targetProgram, *metadata)`.
