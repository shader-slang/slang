---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T20:10:50+00:00
target_doc: target-pipelines/spirv.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: a50fb58bbc9789d4e73034b91a5610e1ced90f57c3c4eb3b19560e9cb5d4c072
source_commit: f40a5af1027e6fd524a5855f77d62bddffd998c9
checklist:
  factual_accuracy: partial
  cross_references: partial
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 6
severity_breakdown:
  critical: 1
  major: 2
  minor: 3
  nit: 0
---

# Review report for target-pipelines/spirv.md

## Summary

The document closely tracks the current pipeline, but needs corrections before acceptance. The most important error is its claim that SPIR-V uniquely changes phi-elimination defaults: the documented settings already match the defaults, invalidating the accompanying comparison of emitted code. Other findings concern pointer ordering, diagram/table consistency, a broken anchor, and overgeneralized gating explanations.

## Items checked

- Read `_review.md`, `_common.md`, the target’s generation prompt, and `regenerate.py show target-pipelines/spirv.md`.
- Verified against the **current working tree**. `git diff` against the document’s source commit showed no differences under `source/`; confirmed HEAD matches the supplied report commit.
- Checked all 204 explicit line-number citation expressions, including repeated references, against their source locations.
- Spot-checked more than 20 behavioral claims: direct-emit dispatch, accumulating lowering flags, autodiff branching, optional/result ordering, resource legalization, byte-address options, phi elimination, address-space specialization, both documented loops, abort lowering, descriptor pointers and strides, debug emission, depth modes, gather capabilities, flat inputs, switch widths, float-to-bool casts, and downstream link/validate/optimize gates.
- Resolved 230 Markdown links: all 102 distinct file targets exist both locally and at the document’s source commit. Checked both local anchors and confirmed all five referenced generated peers are manifest entries; dependency documents were inspected only for cross-reference checks.
- Checked required sections, phase diagrams/tables, front-matter fields, Liquid safety, and the size cap: 114,952 bytes against 131,072.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Notable passes`, lines 1098–1112 | The claim that SPIR-V is the only backend that `flips both knobs`, and that the default mode would leave an `OpPhi`, contradicts the actual defaults. Both explicitly assigned SPIR-V values already equal the default settings. | `source/slang/slang-ir-eliminate-phis.h:13–14` initializes `eliminateCompositeTypedPhiOnly = false` and `useRegisterAllocation = true`. `source/slang/slang-ir-eliminate-phis.cpp:458–468` allocates temporaries for scalar as well as composite phis when the former is false. | Keep the explicit assignments described at `slang-emit.cpp:2788–2794`, but state that they currently restate the defaults. Remove the claim of unique SPIR-V settings and the default-mode `OpPhi` comparison. |
| F-002 | minor | `### Forward-declared pointer fixup`, lines 1072–1076 | The statement that the `module ends with all forward pointer declarations` confuses pointer definitions with forward declarations and the types section with the whole module. | `source/slang/slang-emit-spirv.cpp:2609–2617` records the `OpTypePointer` definition and separately emits `OpTypeForwardPointer`; its comment explicitly says the definition moves to the end of the Types & Constants section. Lines 77–90 place globals and functions after that section. | Say that the loop ensures pointee types are emitted, then moves the recorded pointer **definitions** to the end of their ConstantsAndTypes section. |
| F-003 | major | `### Option-set toggles`, line 976 | The link `#embedding-source-at--g1` has no corresponding heading or explicit anchor. | `docs/generated/design/target-pipelines/spirv.md:1310` contains the relevant heading, `### Debug levels and -debug-info-include-source` with inline-code formatting; no “Embedding source” heading exists. | Retarget the link to `#debug-levels-and--debug-info-include-source` and update its label to match that section. |
| F-004 | major | Phase A/B/D tables; `## Conditional gates` | The required diagram/table correspondence is incomplete. Phase B draws `clearTranslationDictionary`, `reportCheckpointIntermediates`, and `checkStaticAssert` without companion rows; Phase A draws the build-identifier emission and lowering-set scan without distinct rows; Phase D draws `createArtifactFromIR` without a row and lists `addAssociated(metadata)` without a node. Conditional gates are also split into five tables instead of the required single consolidated table. | `docs/generated/design/_meta/prompts/_common.md:363–378` requires one row per pass node and a single gates table. The omitted Phase B calls exist in `source/slang/slang-emit.cpp` between `removeWeakUseInsts` and `performTypeInlining`, at lines 1906–1909, and at line 2178 respectively; metadata attachment is at line 3798. | Reconcile the diagrams and tables, adding ordered rows for the displayed direct calls and a metadata-attachment node before the final artifact. Consolidate the gates into one table grouped in the prescribed order. |
| F-005 | minor | Phase diagrams, especially lines 158–164 and 254–261 | Gate labels repeatedly replace source expressions with prose or incomplete expressions, despite the prompt’s verbatim-expression requirement. Examples include `reqSet.debugInfo and DebugInfoLevel::None`, `getBoolOption ValidateUniformity`, and `not isSpecializationDisabled`. The first omits the debug-level comparison altogether. | `docs/generated/design/_meta/prompts/target-pipelines-spirv.md`, “Diagram conventions,” requires verbatim gate expressions with limited abbreviation. Actual conditions appear at `source/slang/slang-emit.cpp:1099–1101`, `1501`, and `1555`. | Use quoted Mermaid diamond labels containing the actual C++ predicates, retaining operators, calls, and comparisons. Define any shortened receiver aliases explicitly and apply them consistently. |
| F-006 | minor | Introduction, lines 67–72 | The universal explanation that every gated construct is produced before the last scan is too strong. Some gated constructs are produced later, and correctness depends on explicitly updating a flag rather than on the scans alone. | `source/slang/slang-emit.cpp:1757–1761` runs `lowerTaggedUnionTypes` after the second scan and sets `requiredLoweringPassSet.reinterpret = true` when it creates reinterpret instructions. The document itself correctly describes this exception in Phase B row 31. | Qualify the explanation: the scans accumulate flags, and later producers must update affected flags explicitly. Name `lowerTaggedUnionTypes` updating `reinterpret` as the concrete exception. |