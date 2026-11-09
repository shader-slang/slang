---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:04:36Z
target_doc: ir-reference/decorations.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 7003d9ffd97f502ef36deaf8cec832320bcf008e9a79f32f8c5f2570288a7ffc
source_commit: 8341c7e537a3f5cb3a60cd062066ffd630da40d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 6
severity_breakdown:
  critical: 0
  major: 5
  minor: 1
  nit: 0
---

# Review report for ir-reference/decorations.md

## Summary

The document covers all 201 concrete decoration opcodes, and its explicit line-number citations match the current source. However, the reference tables do not satisfy several mandatory contract requirements. The most important gap is the producer column: many entries conceal provenance behind generic synthesis labels, including decorations actually produced during AST lowering.

## Items checked

- Reviewed against the **current working tree**, which the operator confirms equals the document’s `source_commit` throughout `source/`; confirmed HEAD matches the supplied review commit.
- Read `_review.md`, `_common.md`, the document-specific prompt, and `regenerate.py show ir-reference/decorations.md`.
- Compared all 201 opcode names with the Lua family’s concrete entries; checked family flags and identified 64 handwritten wrappers among the table entries.
- Checked all 36 explicit line-citation expressions, including their ranges and the additional `11690` reference.
- Spot-checked more than 10 claims, including target-intrinsic operand forms, profile packing, all 16 stage values, interpolation values and lowering, capability filtering, shader-64-bit-indexing production, work-graph lowering, synthesized parameter groups, builtin-interface marking, differentiable-call production, linkage production, and keep-alive handling.
- Resolved all 45 relative-link occurrences, covering 23 distinct targets; verified all 10 linked generated pages occur in the manifest. Consulted dependency pages only for cross-reference checks.
- Checked required front-matter keys, heading/table structure, Mermaid structure, Liquid safety, and the 60,206-byte document against its 98,304-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Opcodes`, throughout | Twenty-two operand cells use the explicitly prohibited `(variadic, min=N)` form for Lua entries that provide only `min_operands`. This obscures the contract’s distinction between unnamed operands and variadic instructions. | `source/slang/slang-ir-insts.lua:1773` declares `layout` with `min_operands = 1`; `source/slang/slang-ir-insts.lua:1783` does likewise for `loopMaxIters`. `_common.md`, IR-reference “Operands” contract, requires `(N unnamed)` or accessor-derived names marked with `†`. | Replace all 22 cells with `(N unnamed)` using the declared count, or verified accessor names with a `†` legend. Also normalize the constructor cell and distinguish child instructions from operands in the dictionary-decoration row. |
| F-002 | major | `## Opcodes`, AST origin column; especially lines 216–217, 270–271, 309, 356–357 | Thirty-six rows contain only `(synthesized)`, and many AST-origin rows omit the required producing function. Some labels are substantively misleading: the differentiable-call markers say `(synthesized by autodiff)`, while `import` and `export` are attributed to declarations bearing those names rather than linkage classification. | `source/slang/slang-lower-to-ir.cpp:5341` produces both call markers in `visitTreatAsDifferentiableExpr`; `:1431` selects import/export decorations through `isImportedDecl`; `:12840` lowers ray-payload read/write semantics; `:14713` lowers `NonDynamicUniformAttribute`. `_common.md` requires actual producers and rejects generic synthesis labels. | Audit every origin cell. Name the AST class/modifier plus lowering function, or the responsible pass/function. Correct these examples explicitly. Where no construction site exists, use `no producer at HEAD` and explain that status in the summary. |
| F-003 | major | `## Source`, lines 30–34; all C++ wrapper cells | The required handwritten-wrapper markers, legend, and count are absent. Describing handwritten wrappers as merely occasional does not replace the required distinction: 64 listed wrappers are handwritten. | `source/slang/slang-ir-insts.h:138`, `:217`, and `:1805` explicitly define `IRIntrinsicOpDecoration`, `IRNameHintDecoration`, and `IRLayoutDecoration`; `source/slang/slang-ir.h.lua:151` skips generating structs already present. | Mark all 64 handwritten wrappers consistently, add the legend and count, and explain that generation supplies the remaining wrappers. |
| F-004 | major | `## Family hierarchy`, lines 66–98 | The diagram mixes semantic table buckets with Lua nesting without distinguishing them, and omits the abstract `LinkageDecoration` and `DerivativeGroupDecoration` groups. It therefore does not provide the required schema hierarchy. | `source/slang/slang-ir-insts.lua:2252` groups `import` and `export` under `LinkageDecoration`; `:2390` groups the quad/linear variants under `DerivativeGroupDecoration`. `_common.md` requires a hierarchy mirroring Lua nesting. | Show all actual abstract groups and their nesting, including these two. Remove semantic buckets from the schema diagram or clearly separate them from actual Lua nodes. |
| F-005 | major | `## Notable opcodes` | The specifically required `Differentiable` / `BackwardDifferentiable` callout is missing. Those historical decoration names are absent from the current family, so inventing rows would also be incorrect; the contract requires explaining unavailable requested material. | `docs/generated/design/_meta/prompts/ir-reference-decorations.md`, “Notable opcodes,” requires this coverage. `source/slang/slang-lower-to-ir.cpp:10821` describes differentiability becoming an interface conformance requirement rather than the historical decoration representation. | Add a brief callout explaining that these are not current decoration opcodes, describe the current representation at a high level, and link to `differentiation.md` and the lowering page’s differentiability section. Do not add nonexistent opcodes to the tables. |
| F-006 | minor | Opcode summaries and source citations, including lines 108, 163, 208, 346 | Several summaries contain multiple explanatory sentences instead of one short clause; source files are also cited only as backticked basenames in several rows. Both violate explicit formatting requirements. | `_common.md`, universal source-link rule and IR-reference “Summary” contract; `docs/generated/design/_meta/prompts/ir-reference-decorations.md`, quality checklist. Examples include the multi-sentence `streamOutputTypeDecoration` summary and unlinked autodiff filenames on line 346. | Move extended explanations into existing notable-opcode callouts, reduce summaries to one short clause, and turn source-file citations into resolving relative Markdown links. |
