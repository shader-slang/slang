---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:17:22Z
target_doc: pipeline/overview.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: c0c551c26b5c6b8cdcd2fff95ae01f9403452087dcb8bccb74fe55a8a5992b68
source_commit: be3c344414f6541ba6731813b36591a94cd05482
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 1
  major: 0
  minor: 1
  nit: 1
---

# Review report for pipeline/overview.md

## Summary

The page provides a largely accurate pipeline map, with valid links and exact line-number citations. Its most important error is turning a translation-unit dictionary ordering limitation into an unconditional claim about `slangc` source-file ordering. The page also exceeds its roadmap remit in several passages and uses a different stage-heading level than its prompt requires.

## Items checked

- Reviewed against the **current working tree**, which equals the document’s recorded source commit for all of `source/`, as specified; the source-tree diff reported no differences.
- Read `_review.md`, `_common.md`, the per-document prompt, and resolved watched paths using `regenerate.py show pipeline/overview.md`.
- Verified all three line-number citations: `linkAndOptimizeIR` at `source/slang/slang-emit.cpp:1015`, `emitEntryPointsSourceFromIR` at line 2968, and `checkAllTranslationUnits` at `source/slang/slang-compile-request.cpp:702`.
- Spot-checked more than 10 behavioral claims: preprocessing before parsing; deferred body parsing; semantic visitor organization; conditional-expression block parameters; short-circuit lowering; mandatory SSA construction; phi elimination; SPIR-V optimizer gating; GLSL-specific legalization; Torch binding selection; textual/binary dispatch; loaded-module registration; entry-point checking; and compile-request service access.
- Confirmed the count of 167 `slang-ir-*.cpp` files and checked thread-group emission spellings in the target emitters.
- Resolved all 44 relative-link occurrences, covering 34 distinct destinations; confirmed all 12 linked generated pages are manifest entries. Read the architecture dependency only for cross-reference checks.
- Checked required sections, Mermaid structure, front-matter keys and formats, and the 11,103-byte document against its 24,576-byte cap. No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Driver entry points`, lines 207–212 | The claim that with two command-line source files “the later file can `import` the earlier one” and “the reverse order does not resolve” incorrectly generalizes the sibling translation-unit dictionary behavior. Ordinary `.slang` command-line files share one translation unit; furthermore, missing dictionary entries do not prevent filesystem module lookup. | `source/slang/slang-options.cpp:1726`: “All of the input .slang files will be grouped into a single logical translation unit.” `source/slang/slang-session.cpp:1745` checks the supplied dictionary, while line 1881 subsequently searches for module files. `source/slang/slang-compile-request.cpp:717–725` supports only the narrower checked-sibling registration limitation. | Delete the command-line ordering conclusion. State that later translation units are unavailable **through this request-local dictionary**, while ordinary Slang CLI inputs share a translation unit and module lookup can still resolve files independently. |
| F-002 | minor | `### AST → IR lowering`, lines 101–115; `### IR passes`, lines 135–153 | Detailed phi construction/elimination, optimizer gating, and seven target spellings go beyond the overview’s explicit roadmap scope. | `docs/generated/design/_meta/prompts/pipeline-overview.md`, “Quality checklist”: “the per-stage docs do the deep dive, this one is purely a roadmap”; required structure asks for one paragraph describing each stage’s input and output. | Reduce these passages to brief descriptions of SSA lowering and target-dependent transformations; use the existing links to `04-ast-to-ir.md`, `05-ir-passes.md`, and `06-emit.md` for implementation details. |
| F-003 | nit | `## Stages`, lines 40, 57, 74, 92, 125, 162 | All six stage headings use level 3, whereas the per-document contract explicitly requests level-2 subsections. Stage coverage itself is complete. | `docs/generated/design/_meta/prompts/pipeline-overview.md`, required structure item 3: “a level-2 subsection per stage.” | Change the six stage headings from `###` to `##` to match the stated contract. |