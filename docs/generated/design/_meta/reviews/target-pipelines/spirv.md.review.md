---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:31:00+00:00
target_doc: target-pipelines/spirv.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 68a85e13aad997a240500c6924c43cbfb5c7a2705b13eee149bc97d9ad794aeb
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for target-pipelines/spirv.md

## Summary
The document is broadly aligned with the SPIR-V direct-emit path, but it needs a small remediation pass. The most important issue is that the Phase D diagram omits the active downstream `compiler->compile` spirv-opt step and places the disabled in-source `optimizeSPIRV` node after validation, which does not match source order.

## Items checked
- Ran `regenerate.py show target-pipelines/spirv.md` and used only the listed prompt, five dependency docs, and eight resolved watched source files.
- Checked front matter for all required keys, `target_doc_source_commit`, the warning string, and the 64-character hex watched-path digest.
- Resolved all 188 relative markdown links in the document body; none were broken.
- Verified required target-pipeline sections: intro, Source, High-level phase diagram, Phase A-D sections, Conditional gates, Loops in the pipeline, Notable passes, and See also.
- Spot-checked more than 10 concrete claims against watched source, including `linkAndOptimizeIR`, `emitSPIRVForEntryPointsDirectly`, `createArtifactFromIR`, `shouldRunSPIRVValidation`, `emitSPIRVFromIR`, `legalizeIRForSPIRV`, `simplifyIRForSpirvLegalization`, `processAbort`, `emitAbort`, the forward-declared-pointer loop, `legalizeEntryPointsForGLSL`, `legalizeLogicalAndOr`, `TargetProgram::shouldEmitSPIRVDirectly`, and descriptor-heap stride handling.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Phase D: IR-to-SPIR-V emit, simplification loop, downstream tools`, lines 654-745 | The Phase D diagram shows only `optimizeSPIRV [disabled]` after `spirv-val`, but the active downstream `compiler->compile` spirv-opt step that row 20 describes is missing from the diagram. The disabled `#if 0` `optimizeSPIRV` block also appears before downstream linking and validation in source, not after validation. | `source/slang/slang-emit.cpp:3106` starts the disabled `#if 0` `optimizeSPIRV` block before downstream setup; `source/slang/slang-emit.cpp:3198` creates `DownstreamCompileOptions`, and `source/slang/slang-emit.cpp:3224` calls `compiler->compile(downstreamOptions, optimizedArtifact.writeRef())`. | Update the Phase D diagram to include an active `(downstream) compiler->compile spirv-opt` node after the validation gate, and either move the disabled `optimizeSPIRV [disabled]` note to its source-order position before downstream linking or describe it only in the table/prose. |
| F-002 | minor | `## Source`, `## Phase D`, and `### simplifyIRForSpirvLegalization`, lines 49-59, 641-647, 842-865 | Several line-number citations for the SPIR-V emitter/legalizer are stale by far more than a few lines, even though the named symbols exist. Examples include `emitSPIRVFromIR` cited as line ~11598 and `legalizeIRForSPIRV` cited as line 3104; the current watched source defines them at materially different lines. | `source/slang/slang-emit-spirv.cpp:11803` defines `emitSPIRVFromIR`; `source/slang/slang-ir-spirv-legalize.cpp:3039` defines `simplifyIRForSpirvLegalization`; `source/slang/slang-ir-spirv-legalize.cpp:3265` defines `legalizeIRForSPIRV`. | Refresh the affected line-number citations in the Source, Phase D, and Loops sections to match the recorded source commit, including the loop body citation currently pointing at lines 2885-2914. |
| F-003 | minor | Opening paragraphs, lines 12-39 | The first body paragraph explains what the page covers, but it does not say who the intended reader is, which the common contract requires. The audience appears only in the per-doc prompt, not in the generated page. | `docs/generated/design/_meta/prompts/_common.md:65` says the first paragraph must state what the document covers and who its intended reader is. | Amend the opening paragraph to name the intended reader, for example a compiler developer debugging or modifying the SPIR-V direct-emit pipeline. |

## No-issues notes
- The front matter uses the target doc's recorded source commit and watched-path digest, and all mandatory fields are present.
- The direct-emit scope and via-GLSL exclusion are consistent with the per-doc prompt.
- The fixed-point-loop behavior is correctly described: `iterationCounter` and `funcIterationCount` are initialized but not incremented in the watched source.
