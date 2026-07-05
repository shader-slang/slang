---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:34:23+00:00
target_doc: target-pipelines/wgsl.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 893e68384601fb6107ed1d9426d6ba0a0ad7b13bd39f42f529bf8c28e6020a47
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for target-pipelines/wgsl.md

## Summary
The WGSL target-pipeline page is structurally close to the prompt contract, but it has a few source-alignment problems. The most important issue is that the `WGSLSPIRVAssembly` downstream path is documented as stopping at Tint, while the source routes that variant through Tint to SPIR-V and then through glslang to SPIR-V assembly.

## Items checked
- Ran `regenerate.py show target-pipelines/wgsl.md` and reviewed only the target document, `_common.md`, `target-pipelines-wgsl.md`, the five `depends_on` docs, and the resolved watched-file set printed by the driver.
- Checked front matter for all required keys, the recorded target-doc source commit, the warning string, and a 64-character hex watched-path digest copied from the target document.
- Verified the required target-pipeline sections are present: Source, High-level phase diagram, Phase A-D sections, Conditional gates, Loops in the pipeline, Notable passes, and See also.
- Spot-checked source-backed claims for `linkAndOptimizeIR`, `lowerGLSLShaderStorageBufferObjectsToStructuredBuffers`, `lowerCooperativeVectors`, `lowerCombinedTextureSamplers`, `legalizeByteAddressBufferOps` WGSL options, `legalizeIRForWGSL`, `floatNonUniformResourceIndex`, `legalizeLogicalAndOr`, WGSL buffer-element lowering policy, `specializeAddressSpaceForWGSL`, `eliminatePhis`, `applyVariableScopeCorrection`, `WGSLSourceEmitter`, and WGSL line-directive handling.
- Checked the WGSL legalizer and emitter claims against `slang-ir-wgsl-legalize.cpp`, `slang-ir-legalize-varying-params.cpp`, `slang-ir-legalize-binary-operator.cpp`, `slang-emit-wgsl.cpp`, `slang-emit-c-like.cpp`, and the relevant ranges of `slang-emit.cpp`; consulted the downstream routing files only for the page's `WGSLSPIRVAssembly` downstream-tool claim.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Phase D: WGSL emit and downstream tools`, lines 584-622; `### Downstream Tint`, lines 802-808 | The downstream path for `WGSLSPIRVAssembly` is incomplete. The page shows one downstream node, `Tint WGSL to SPIR-V`, for both `WGSLSPIRV` and `WGSLSPIRVAssembly`, and says the resulting SPIR-V module is what client code consumes. That omits the additional SPIR-V-to-SPIR-V-assembly transition required for the assembly target. | `source/slang/slang-code-gen.cpp:1059-1104` maps `WGSLSPIRVAssembly` to intermediate `WGSLSPIRV` before disassembly, and `source/slang/slang-global-session.cpp:218-225` registers `WGSL -> WGSLSPIRV` through Tint and `WGSLSPIRV -> WGSLSPIRVAssembly` through glslang. | Update Phase D's diagram, table, conditional-gate row, and downstream callout to show `WGSLSPIRVAssembly` as `WGSL -> (downstream) Tint -> SPIR-V -> (downstream) glslang -> SPIR-V assembly`; keep bare `WGSL` as source-only and `WGSLSPIRV` as Tint-only. |
| F-002 | minor | `### specializeAddressSpaceForWGSL`, lines 727-735 | The WGSL address-space list includes `push_constant`, but the watched WGSL legalizer/emitter paths do not assign or emit that address space for WGSL. The listed WGSL emitter address spaces are `function`, `private`, `storage`, `uniform`, and `workgroup`. | `source/slang/slang-ir-wgsl-legalize.cpp:243-328` assigns `Uniform`, `Global`, `Function`, `ThreadLocal`, and `GroupShared`; `source/slang/slang-emit-wgsl.cpp:331-356` emits `uniform`, `storage`, `function`, `private`, and `workgroup`, with no `PushConstant` case. | Remove `push_constant` from the WGSL address-space list, or explicitly qualify it as an IR/SPIR-V address-space concept that this WGSL emitter path does not write. |
| F-003 | minor | `### legalizeLogicalAndOr`, lines 737-744 | The notable-pass text says the pass rewrites vector logical operators "into element-wise selects." The source instead casts vector operands/results to boolean vector types and rebuilds `And` / `Or`; for array-lowered matrices it loops over array elements and emits `And` / `Or` per element. No select emission is present in this pass. | `source/slang/slang-ir-legalize-binary-operator.cpp:179-310` handles `kIROp_And` and `kIROp_Or`, uses `emitCast`, `emitAnd`, `emitOr`, and `emitMakeArray`, and does not emit selects. | Reword this callout to say the pass legalizes operand and result types for vector/array logical `And` / `Or`; delete the "element-wise selects" wording. |
| F-004 | minor | `## See also`, lines 810-824 | The target-pipeline contract asks for a user-facing target documentation link when one exists, but the See also list omits the WGSL user-guide page. | `docs/generated/design/_meta/prompts/_common.md:352-359` requires linking user-facing target documentation when present, and `docs/user-guide/a2-03-wgsl-target-specific.md` exists. | Add a See also bullet for `[../../../user-guide/a2-03-wgsl-target-specific.md](../../../user-guide/a2-03-wgsl-target-specific.md)`. |
