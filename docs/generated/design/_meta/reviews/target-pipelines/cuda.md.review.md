---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T20:10:19Z
target_doc: target-pipelines/cuda.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 95a77f15a5bef977010d78ba05dee31d08f1bbe567b2ba2a3d7fe18b716f2ddf
source_commit: f40a5af1027e6fd524a5855f77d62bddffd998c9
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: fail
  front_matter_validity: pass
finding_count: 9
severity_breakdown:
  critical: 3
  major: 2
  minor: 4
  nit: 0
---

# Review report for target-pipelines/cuda.md

## Summary

The pass tables largely match the current orchestration, but several behavioral explanations require correction. Most importantly, the proposed interface-dispatch example cannot produce the claimed OptiX diagnostic: the diagnostic scan skips unresolved callees. The introduction also misidentifies the CUDA-header pipeline, and the matrix discussion incorrectly excludes valid buffer-element loads.

## Items checked

- Read `_review.md`, `_common.md`, and the document-specific prompt obtained through `regenerate.py show target-pipelines/cuda.md`.
- Verified against the **current working tree**, whose `source/` contents equal the document’s recorded source commit, as specified for this review.
- Checked all explicit source line-number citations, including 267 occurrences of “line”/“lines” references and their repeated citations.
- Compared the four phase tables—136 numbered rows—with orchestration and target dispatch. Spot-checked over 25 behavioral claims, including PTX routing, CUDA-header routing, coverage metadata, autodiff gates, OptiX uniforms and diagnostics, matrix storage, immutable loads, binding-name rewriting, and CUDA emission.
- Resolved all 116 distinct file targets across 215 Markdown links; confirmed all 12 referenced generated pages exist in the manifest. Consulted dependency documents only for cross-reference checks.
- Checked required sections, diagram conventions, front-matter keys and formats, and Liquid safety. The document is 98,115 bytes, within its 98,304-byte cap.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Introduction, lines 33–34; Phase A, lines 120–121 | `CUDAHeader is emitted ... through the CUDASource IR pipeline` is false. Headers retain `CUDAHeader`, which explains the later documented differences in cooperative-vector lowering, autodiff-context expansion, and constant inlining. | `source/slang/slang-code-gen.cpp:1270,1281–1285` includes `CUDAHeader` and constructs `CodeGenContext subContext(this, target, extensionTracker)`. Only PTX is mapped to `CUDASource` at lines 268–269. | State that PTX uses a `CUDASource` intermediate, while CUDA headers retain `CUDAHeader`. Replace the assertion that `CUDASource` arms cover all three targets with the appropriate source/header distinction. |
| F-002 | critical | `legalizeEntryPointVaryingParamsForCUDA`, lines 1082–1097 | The proposed interface-method example allegedly leaves a `lookupWitness` callee that the residual scan diagnoses. That scan cannot detect this shape: unresolved callees are skipped before terminate-reaching analysis. | `source/slang/slang-ir-legalize-varying-params.cpp:1779–1786` resolves only an `IRFunc` or a specialization whose base is an `IRFunc`; lines 2004–2006 explicitly execute `continue` when resolution fails. Lines 1772–1778 document the resolution boundary. | Delete the proposed dynamic-dispatch test and its claimed outcome. Explain that the diagnostic concerns **resolvable**, terminate-reaching residual callees; unresolved indirect calls are outside this analysis. |
| F-003 | critical | `lowerImmutableBufferLoadForCUDA`, lines 1232–1241; Phase C rows 18–20 | The claim that the matrix arm is unreachable for buffer elements overstates matrix lowering. CUDA can retain natural-layout, row-major matrices. The cited CUDA builtin builder also creates an array of scalars, not row vectors. Phase C’s integer-matrix-to-array explanation is consequently misleading for CUDA. | `source/slang/slang-ir-lower-buffer-element-type.cpp:2698–2714` preserves matrices with the default layout and Natural rules. `source/slang/slang-ir-legalize-matrix-types.cpp:18–40,61–65` excludes CUDA from matrix-to-vector-array legalization. `source/slang/slang-ir-lower-cuda-builtin-types.cpp:163–167` builds a scalar array. | Describe storage wrapping as conditional on layout/type policy. Retain matrix-typed buffer loads as possible inputs to the immutable-load pass. Distinguish the buffer storage builder from CUDA kernel-signature lowering, and qualify the Phase C ordering notes as shared-pipeline rationale rather than CUDA transformations. |
| F-004 | major | Phase A–D diagrams, lines 139–163, 277–375, 485–550, 661–679 | The diagrams do not satisfy the control-flow contract. Numerous conditional passes appear on unconditional chains; five validation calls are collapsed into `checks`; Phase D omits its table’s `sourceEmitter->init` node. The required validation-call summary is absent. | `_common.md`, “Target-pipeline page contract,” requires gate diamonds with bypass arms, one rectangle per `SLANG_PASS`, and a validation summary. Examples of omitted gates/calls appear in `source/slang/slang-emit.cpp:1087–1104,1693–1710,2930–2942,3092`. | Draw each reachable pass separately, restore conditional diamonds and bypass paths, include emitter initialization, and summarize interleaved `validateIRModuleIfEnabled` calls beneath the diagrams. |
| F-005 | major | `## See also`, lines 1309–1325 | The required user-facing target-documentation link is missing, although a CUDA/OptiX section exists. | `_common.md`, “Target-pipeline page contract,” item 9 requires this link when documentation exists. `docs/user-guide/09-targets.md:355` begins `## CUDA and OptiX`. | Add a relative link to `../../../user-guide/09-targets.md#cuda-and-optix`. |
| F-006 | minor | `## Conditional gates`, lines 794–908 | The contract requires a **single** consolidated table, but this section contains four separate tables, with a different column schema in the last one. | `_common.md`, “Target-pipeline page contract,” item 6 explicitly specifies “a single table consolidating every gate expression.” | Merge the tables into one consistent schema, preserving the required ordering of flag, option, context, and CUDA-specific groups. |
| F-007 | minor | `collectOptiXEntryPointUniformParams`, lines 1026–1028 | The explanation that the collected parameter `is already a global` describes the wrong representation. It is a temporary `IRParam`, replaced by an SBT-pointer instruction and removed. | `source/slang/slang-ir-optix-entry-point-uniforms.cpp:259` uses `builder.createParam`; lines 218 and 228–232 insert it into the entry block, replace its uses with `kIROp_GetOptiXSbtDataPtr`, and delete it. | Explain that SBT accesses replace the collected parameter, so no collected uniform parameter remains to move into global scope. |
| F-008 | minor | `lowerImmutableBufferLoadForCUDA`, lines 1190–1192 | Calling this `the one pass in linkAndOptimizeIR that exists solely for this target family` contradicts other CUDA-only passes documented on the same page. | `source/slang/slang-emit.cpp:1902–1904,2354–2358,2439–2442` contains CUDA-family-only calls to OptiX intersection legalization, active-mask synthesis, and CUDA varying-parameter legalization. | Replace the exclusivity claim with “a CUDA-specific optimization.” |
| F-009 | minor | `The autodiff gate`, lines 948–951 | `This gate matters more on CUDA than on any other target` introduces an unsupported importance ranking and usage claim. The source establishes an IR-content predicate, not relative target usage or importance. | `source/slang/slang-emit.cpp:423–434,522–528` sets the flag from IR constructs; lines 1592–1598 select finalization or stripping without a CUDA-specific condition. The common contract prohibits editorial opinions and unsupported claims. | Remove the ranking and usage generalization. State that CUDA modules containing the listed autodiff constructs take the finalization path, while modules without them take the stripping path. |