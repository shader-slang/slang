---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T20:08:09Z
target_doc: target-pipelines/metal.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: aede24230ce39b7bd26f5c9691dcc18345d10b4564888b02e08b241529a399de
source_commit: f40a5af1027e6fd524a5855f77d62bddffd998c9
checklist:
  factual_accuracy: partial
  cross_references: partial
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 7
severity_breakdown:
  critical: 2
  major: 2
  minor: 3
  nit: 0
---

# Review report for target-pipelines/metal.md

## Summary

The document provides extensive, mostly accurate coverage of the Metal pipeline, but its diagrams need correction before they can serve as a control-flow reference. Most importantly, Phase B reverses a deliberately enforced cleanup/validation ordering, while Phase C presents four conditional passes as unconditional. The remaining findings concern required summaries and inaccurate navigation or explanatory claims.

## Items checked

- Verified against the **current working tree**, which matches the document’s `source_commit` for all of `source/`; the source comparison produced no differences.
- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show target-pipelines/metal.md`.
- Checked more than 20 behavioral claims, including intermediate-target selection, accumulating lowering flags, coverage-width clamping, output-parameter wrappers, decoration retargeting, resource bindings, packed vectors, address spaces, opcode producers, logging, literal suffixes, and downstream compiler flags.
- Inspected all 225 explicit line-citation expressions, including repeated references and their associated ranges, against the cited working-tree files.
- Compared Phase A–C pass ordering and gates with `linkAndOptimizeIR`, and Phase D with source emission and downstream dispatch.
- Resolved all 222 Markdown link occurrences (122 distinct destinations), checked the three local heading anchors, and confirmed all 12 referenced generated pages are manifest entries. Dependency documents were consulted only for cross-reference checks.
- Checked required front-matter fields, section structure, diagrams, and Liquid safety. The document is 95,941 bytes, within its 98,304-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Phase B diagram, lines 422–425 | The diagram runs `checkGetStringHashInsts` before the direct `eliminateDeadCode` call. This reverses the actual dependency and contradicts the correctly ordered table rows 42–43. | `source/slang/slang-emit.cpp:1800–1811` explicitly says DCE “Must run before the getStringHash check below” because leftover callees can otherwise cause legal code to be rejected. DCE runs at 1804; validation runs at 1811. | Route `performTypeInlining` through the direct DCE node first, then through the nonessential-validation diamond and optional `checkGetStringHashInsts`, before `lowerTuples`. |
| F-002 | critical | Phase C diagram, lines 643–649 | The diagram makes `simplifyIR`, `collectCooperativeMetadata`, `unexportNonEmbeddableIR`, and `checkUnsupportedInst` unconditional. Their runtime gates are material to the page’s stated purpose and are already acknowledged in its table. | `source/slang/slang-emit.cpp:2738–2742` gates simplification on `!minimalOptimization`; 2930–2936 gates cooperative metadata on capabilities; 2940–2942 gates unexporting on `EmbedDownstreamIR`; 2960–2961 gates unsupported-instruction checks on `!shouldPerformMinimumOptimizations()`. | Add four gate diamonds with true and bypass edges, preserving the source order and matching Phase C rows 24, 36, 37, and 40. |
| F-003 | major | Conditional gates, lines 930–1019 | The required consolidated gate table is split into four tables and omits `shouldLegalizeExistentialAndResourceTypes`, despite that predicate controlling a substantial branch in the Phase B diagram. The validation-options row also omits `checkForMissingReturns`. | `docs/generated/design/_meta/prompts/_common.md:374–378` requires one table covering every diagram gate. `source/slang/slang-emit.cpp:1982–2074` implements the omitted legalization branch; 1693–1705 nests missing-return checking under nonessential validation. | Consolidate the tables, retaining grouped rows. Add the existential/resource legalization predicate and its controlled passes, and include `checkForMissingReturns` under both of its gates. |
| F-004 | major | Phase A–C diagram endings, lines 176, 454, 650 | All required summaries of `validateIRModuleIfEnabled` checkpoints are absent. Consequently, the phase diagrams omit this required aspect of the pipeline contract entirely. | `docs/generated/design/_meta/prompts/_common.md:361–362` requires a comment beneath each diagram summarizing these calls. Examples occur at `source/slang/slang-emit.cpp:1080`, 1717, 2039, 2067, 2568, 2836, and 2912. | Add concise comments beneath the affected phase diagrams identifying the validation checkpoints omitted from the nodes. |
| F-005 | minor | Phase C row 21, line 674; Resource-type spellings, lines 923–927 | Two implementation pointers are inaccurate: the table locates `specializeAddressSpaceForMetal` in the generic specialization file, and the depth-texture discussion attributes its statement-emission case to `tryEmitInstExprImpl`. | `source/slang/slang-ir-metal-legalize.cpp:458–461` defines the Metal specialization wrapper. `source/slang/slang-emit-metal.cpp:422` starts `tryEmitInstStmtImpl`, whose depth-texture case is at 697–712. | Change row 21’s File link to `slang-ir-metal-legalize.cpp`; change the depth-texture discussion to `tryEmitInstStmtImpl`. |
| F-006 | minor | Notable passes: parameter transformations, lines 1252–1256 | The claim that the source does not explain Metal’s grouping with CPU/CUDA “for either pass” is false for `transformParamsToConstRef`. | `source/slang/slang-emit.cpp:2676–2681` states that struct/array parameters use const references for performance on Metal, C++, and CUDA, and explains that Metal requires array parameters created by matrix legalization to be passed by reference. | Replace the blanket absence claim with this documented rationale and ordering constraint. Limit any remaining uncertainty to `undoParameterCopy`. |
| F-007 | minor | Logging discussion, lines 770–772; downstream discussion, final paragraph | The document says `cross-cutting/targets.md` describes `metallib_3_2` and the individual Metal version atoms. That page discusses capability infrastructure but does not describe those atoms. The links resolve, but promise content absent from their destination. | `docs/generated/design/cross-cutting/targets.md:176–186` instead directs readers to the capability-atoms reference and `source/slang/slang-capabilities.capdef`; the page contains no `metallib_3_1`, `metallib_3_2`, or `metallib_4_0` descriptions. | Link the atom-specific claims directly to `source/slang/slang-capabilities.capdef`, retaining the targets-page link only for general capability-system context. |