---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:05:53+00:00
target_doc: target-pipelines/hlsl.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: d6ab7e839f67ff67089c6ff596134280c2acd4d4480e7715012652269230eb0f
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: pass
finding_count: 8
severity_breakdown:
  critical: 1
  major: 5
  minor: 2
  nit: 0
---

# Review report for target-pipelines/hlsl.md

## Summary
The page accurately records most HLSL-reachable gates and all 118 citation-bearing lines point at the stated source locations, but several behavioral descriptions and required diagram/table details are wrong or incomplete. The most important error is that Phase D sends the assembly targets directly through the compile branches; the source instead compiles an intermediate binary target and then invokes a downstream disassembler.

## Items checked
- Read the target document, `_common.md`, its per-document prompt, all resolved watched files, and the five `depends_on` documents reported by `regenerate.py show`.
- Verified the recorded source commit exists and equals review-time `HEAD`, then checked every line-number citation on the 118 citation-bearing lines against that commit.
- Spot-checked more than 10 claims, including both required-pass scans, HLSL emitter construction, byte-address-buffer options, ray-payload gates, barrier validation, logical-operator legalization, phi elimination, variable-scope correction, named-constant emission, artifact creation, and DXC/fxc dispatch.
- Resolved relative links and peer-document references; `regenerate.py lint target-pipelines/hlsl.md` completed cleanly.
- Compared every HLSL-reachable `SLANG_PASS` region in `linkAndOptimizeIR` with the four phase diagrams and ordered tables.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Phase D diagram and table, lines 564-603 | The page treats `DXILAssembly` and `DXBytecodeAssembly` as direct `emitWithDownstreamForEntryPoints` compile targets. They instead recurse through the corresponding binary target and then disassemble that binary with a downstream tool. | `source/slang/slang-code-gen.cpp:1119-1141` handles all assembly targets by compiling `_getIntermediateTarget(target)` and calling `dissassembleWithDownstream`; only `DXIL` and `DXBytecode` enter `emitWithDownstreamForEntryPoints` at lines 1191-1197. | Give each assembly target a branch to its binary intermediate followed by a `(downstream) disassemble` node. Restrict the DXC/fxc compile-row gates to `DXIL` and `DXBytecode`, respectively. |
| F-002 | major | After the Phase D table, lines 605-606 | The statement that “all validation and optimization is delegated to DXC or fxc” contradicts the page and source: Slang performs numerous validation and optimization passes before emitting HLSL. | `source/slang/slang-emit.cpp:2394-2404` runs `validateVectorsAndMatrices` and `eliminateDeadCode`; lines 2736-2739 collect metadata and run `checkUnsupportedInst`. | Say that no SPIR-V tools apply and that downstream HLSL validation/optimization continues in DXC or fxc after Slang's own IR validation and optimization. |
| F-003 | major | Phase C row 7 and `legalizeLogicalAndOr`, lines 490 and 906-912 | The page says `legalizeLogicalAndOr` rewrites vector operations into element-wise selects. The implementation emits no selects: it converts vector operands/results to boolean vectors as needed, while lowered matrices represented as arrays are rebuilt from per-element `And`/`Or` instructions. | `source/slang/slang-ir-legalize-binary-operator.cpp:179-245` handles vector casts and `And`/`Or`; lines 246-297 rebuild arrays from per-element `And`/`Or`. | Replace the select description with the actual vector-boolean coercion and lowered-matrix array reconstruction behavior. |
| F-004 | major | `applyVariableScopeCorrection`, lines 925-930 | The explanation attributes this pass to live-range-marker scoping and claims DXC requires declarations at the outermost enclosing scope. The implementation instead repairs values defined in a loop and used after that loop: it hoists `IRVar`, spills storable values through a function-entry variable, or clones unstorable instructions at use sites. | `source/slang/slang-ir-variable-scope-correction.cpp:131-200` detects uses outside loop scope; lines 203-244 implement spill/reload or cloning. | Describe the loop-scope producer/use mismatch and the three repair strategies. Remove the unsupported live-range-marker and universal outermost-declaration claims. |
| F-005 | major | Phase B ordered table, lines 297-370 | Three HLSL-reachable `SLANG_PASS` call sites have no distinct table row: `stripAutoDiffDecorations`, the minimal-mode `eliminateDeadCode` after the first fast-simplification gate, and the minimal-mode `eliminateDeadCode` after matrix legalization. Notes attached to the alternative rows do not satisfy the one-row-per-call coverage rule. | `source/slang/slang-emit.cpp:1446-1452`, `source/slang/slang-emit.cpp:1589-1595`, and `source/slang/slang-emit.cpp:1938-1941`; the coverage rule is `docs/generated/design/_meta/prompts/_common.md:364-368`. | Add separate ordered rows for all three call sites and keep their mutually exclusive gates explicit. |
| F-006 | major | Phase diagrams, lines 116-137, 205-294, and 402-479 | The diagrams flatten many conditional calls into linear edges; Phase A explicitly says all gates were omitted. The target-pipeline contract requires every conditional gate to be a diamond with true and false fall-through arms. | `docs/generated/design/_meta/prompts/_common.md:314-324` defines the mandatory diagram convention; examples omitted from Phase A include the gates at `source/slang/slang-emit.cpp:1053-1058` and `source/slang/slang-emit.cpp:1075-1076`. | Add diamond nodes for every conditional pass call, including option, target, capability, and `RequiredLoweringPassSet` gates, with false paths rejoining the sequence. |
| F-007 | minor | Opening paragraph, lines 12-22 | The first body paragraph explains coverage but never identifies the intended reader, which is mandatory universal content. | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first paragraph to state both coverage and intended reader; the per-doc prompt identifies a compiler developer tracing HLSL pass order and downstream flow at `docs/generated/design/_meta/prompts/target-pipelines-hlsl.md:12-15`. | Add the intended-reader clause to the first paragraph. |
| F-008 | minor | Phase B row 55 and `legalizeEmptyRayPayloadsForHLSL`, lines 355 and 840-846 | The page generalizes the motivation to “DXR requires non-empty ray payload structs.” The implementation documents the narrower requirement: DXIL/HLSL with NVAPI needs a non-empty payload because `NvInvokeHitObject` expects a payload argument. | `source/slang/slang-ir-hlsl-legalize.cpp:252-255`. | Narrow both descriptions to the DXIL/HLSL-with-NVAPI compatibility requirement stated by the implementation. |

## No-issues notes
- All mandatory front-matter fields are present, and the watched-path digest is a valid 64-character hexadecimal value.
- The two accumulating `calcRequiredLoweringPassSet` scans and the cited HLSL pass gates match `slang-emit.cpp`.
- The HLSL byte-address-buffer section correctly uses default options and only enables `useBitCastFromUInt` for DX 5.0-or-earlier profiles.
- Named work-graph types and barrier flags are correctly traced through the HLSL emitter and prelude helpers.
