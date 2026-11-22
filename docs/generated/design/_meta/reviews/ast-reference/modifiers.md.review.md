---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:57:51Z
target_doc: ast-reference/modifiers.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: b01b225b8bde27c3d88361d42256af2a347366a98b51c393066f46d210f81d54
source_commit: 56b1b0b83fb96f88f36da7e37cd9c2c7f27af70c
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 7
severity_breakdown:
  critical: 1
  major: 3
  minor: 3
  nit: 0
---

# Review report for ast-reference/modifiers.md

## Summary

The document covers every concrete modifier class, but needs factual and contract corrections. Most importantly, its `sin` example describes an opcode-binding mechanism that the actual declaration does not use. Existing links and both explicit line-number citations resolve correctly.

## Items checked

- Reviewed against the **current working tree**; `git diff --quiet 2518a942d59b1228fc8cc70a37739baab82b6002 -- source/` confirmed that tracked source matches the document’s source commit.
- Read `_review.md`, `_common.md`, the document-specific prompt, and the resolved watched paths from `regenerate.py show ast-reference/modifiers.md`.
- Compared 282 FIDDLE-declared classes against 273 table rows: all 272 concrete classes appear; one abstract class also appears.
- Checked more than 15 claims, including modifier-list ownership, inherited attribute fields, unchecked-attribute construction, double brackets, optional commas, qualified-name flattening, receiver-mode mappings and defaults, reference-keyword registration and restrictions, diagnostic numbers, interpolation conflict groups, differentiation spellings, work-graph spellings and fields, intrinsic binding, and unrecognized GLSL layout handling.
- Verified both explicit line citations: `slang-check-modifier.cpp:1672–1674` and `slang-ast-modifier.h:1754`.
- Resolved all 79 link occurrences, covering 26 distinct destinations, six anchored destinations, and 11 generated peer pages registered in the manifest. Dependency documents were consulted only for cross-reference checks.
- Checked required sections, table columns, front-matter keys and formats, Mermaid syntax, and Liquid safety. The document is 63,719 bytes, within its 65,536-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### IntrinsicOpModifier and the core module binding`, lines 601–607 | The example claims that `sin` in `core.meta.slang` carries an `IntrinsicOpModifier` selecting a sine opcode. The actual declaration resides in `hlsl.meta.slang` and implements target-specific bodies using `__target_switch`, `__intrinsic_asm`, and SPIR-V assembly. This teaches the wrong lowering mechanism. | `source/slang/hlsl.meta.slang:15939–15955` declares `T sin(T x)` with target-specific bodies. A genuine opcode-binding example is `__intrinsic_op(select) T select(...)` in `source/slang/core.meta.slang:1117`. | Replace the `sin` example with the scalar `select` declaration at `core.meta.slang:1117`, describing its `select` opcode binding. |
| F-002 | minor | `## Nodes`, line 290, `GLSLUnparsedLayoutModifier` | `Raw text of a layout qualifier deferred to later parsing` incorrectly describes an error marker as deferred syntax storage. The class has no raw-text field, and parsing immediately diagnoses this result. | `source/slang/slang-ast-modifier.h:463–467` declares an empty subclass. `source/slang/slang-parser.cpp:1101–1105` creates it when `=` is absent; `:10874–10878` emits `UnrecognizedGlslLayoutQualifierOrRequiresAssignment`. | Describe it as the marker produced for an unrecognized or missing-assignment GLSL layout qualifier, immediately diagnosed by the parser. |
| F-003 | minor | `## Source`, lines 47–58; `### Work-graph node attribute spellings`, lines 697–704 | The document places differentiability attribute spellings outside `core.meta.slang`, in `diff.meta.slang`. Several central spellings are declared in `core.meta.slang`; only part of the family resides in `diff.meta.slang`. | `source/slang/core.meta.slang:489`, `:498`, and `:517` declare `ForwardDifferentiable`, `BackwardDifferentiable`, and `Differentiable`. `source/slang/diff.meta.slang:44` and `:83` declare explicit derivative bindings. | State that the family spans both files: core differentiability markers in `core.meta.slang`, derivative bindings and related annotations in `diff.meta.slang`. |
| F-004 | major | `## Nodes`, throughout | Numerous parsed modifiers and attributes have `(none)` in the mandatory Grammar column, including `RefReadOnlyModifier`, `ForceUnrollAttribute`, `MutatingAttribute`, and every work-graph attribute. This substantially violates the required distinction between parsed syntax and synthesis-only nodes. | `docs/generated/design/_meta/prompts/ast-reference-modifiers.md:21–25` requires grammar links for parsed nodes. Source spellings are registered in `source/slang/slang-parser.cpp:11156`, `source/slang/core.meta.slang:4560`, `:4776`, and `source/standard-modules/experimental/workgraph.slang:17–51`. | Audit all `(none)` cells. Link parsed keywords/layout qualifiers to `grammar.md#modifiers` and source attributes to `grammar.md#attributes-and-decorations`; retain `(none)` for nodes without source syntax. |
| F-005 | major | `## Nodes`, lines 146–574, especially line 524 | The required single concrete-node table is split into 26 tables and includes the abstract `ReceiverParamModeSpecifierAttribute`. Concrete coverage itself is complete. | `docs/generated/design/_meta/prompts/_common.md:133–134` requires a single table; `:158–161` excludes abstract bases. `source/slang/slang-ast-modifier.h:1457–1458` explicitly declares `FIDDLE(abstract)` for this class. | Consolidate the rows into one table and remove `ReceiverParamModeSpecifierAttribute` from it, retaining that base in the hierarchy. |
| F-006 | major | `## See also`, lines 782–800 | The required parsing-page link is absent, and this section contains no pipeline-page link. Pipeline links elsewhere do not fulfill the section’s explicit contract. | `docs/generated/design/_meta/prompts/_common.md:153–156`: “Always include the parsing page when the family is parsed.” | Add `../pipeline/02-parse-ast.md` to `See also`; include `../pipeline/03-semantic-check.md` for the checking references already used in the body. |
| F-007 | minor | Lines 389, 528, 545, 711–739 | Detailed modifier-checking rules exceed the prompt’s explicit semantic-behavior exclusion. Several summaries also exceed the required short-clause form and contain links, emphasis, or bold markup. | `docs/generated/design/_meta/prompts/ast-reference-modifiers.md:67–68` excludes modifier semantics. `_common.md:142` requires “One short clause; no markup beyond inline code.” Examples include the long `EnumClassModifier` diagnostic explanation and bold `both` in the `BackwardDifferentiableAttribute` summary. | Reduce summaries to node shape and source spelling; remove non-inline-code markup. Replace extended receiver/reference checking rules and diagnostic explanations with concise representation descriptions and a semantic-checking-page link. |