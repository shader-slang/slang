---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:20:06Z
target_doc: pipeline/04-ast-to-ir.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: d36bd32950d8e216a645e2f63012cb6f30c73a414e519ae8b99f0ce0aa00a0c6
source_commit: be3c344414f6541ba6731813b36591a94cd05482
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 1
  major: 1
  minor: 1
  nit: 0
---

# Review report for pipeline/04-ast-to-ir.md

## Summary

The document covers all required sections, and its line citations and links resolve correctly. The most important correction is the asserted one-to-one mapping between direct interface members and IR requirement entries: accessor requirements explicitly violate that invariant. The cache-resolution description also needs correction, and the document exceeds the per-document prompt’s size limit.

## Items checked

- Reviewed the current working tree, using the supplied equivalence of all `source/` files to the document’s source commit. No files were written.
- Read `_review.md`, `_common.md`, the document-specific prompt, and the watched-path list returned by `regenerate.py show pipeline/04-ast-to-ir.md`.
- Checked every explicit line-number citation: 70 cited positions/range endpoints in `source/slang/slang-lower-to-ir.cpp` and six in the IR headers, AST header, core module, IR implementation, and compile-request implementation. All point to the described code.
- Spot-checked more than 20 behavioral claims, including lowering entry points, imported-interface borrowing, early-inline prelinking, dump timing, compound value materialization, implicit l-value casts, caching, builtin operators, void casts, witness substitution, generic requirement environments, requirement keys, synthesized conformances, pack-count witnesses, switch diagnostics, recursion limits, entry-point attributes, layout alignment, and debug-source handling.
- Resolved all 38 Markdown link occurrences; checked generated peer targets against the manifest and the semantic-checking section anchor. Read dependency excerpts only for cross-reference checks.
- Checked required section order, table coverage, front-matter fields, identifier/file-name presence, and Liquid safety. Measured 39,887 UTF-8 bytes: below the manifest’s 49,152-byte cap but above the prompt’s 32 KB limit.
- Behavioral verification was source-based; example shaders were not compiled.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### Associated-type bounds`, lines 369–372 | The claim that the requirement loop emits `exactly one entry per direct interface member` states a false IR-shape invariant. Properties and subscripts produce entries for their accessors, while default-implementation containers and other non-requirement members are skipped. A property with both getter and setter therefore contributes multiple entries. | `source/slang/slang-lower-to-ir.cpp:12353–12381`: the loop skips `InterfaceDefaultImplDecl`, iterates `getDirectMemberDeclsOfType<AccessorDecl>()`, and calls `addEntry(accessorKey, accessorDeclRef)` for each qualifying accessor. The counting loop at lines 12163–12184 implements the same distinction. | Restrict the one-entry statement to relocated associated-type constraint requirements. Explicitly retain accessor expansion and non-requirement filtering; preserve the accurate explanation that nested associated-type/callable constraints are no longer scanned to manufacture extra entries. |
| F-002 | major | Whole document | At 39,887 bytes, the page violates the document-specific requirement `Document length under 32 KB.` The larger manifest cap does not remove this separate prompt requirement. | `docs/generated/design/_meta/prompts/pipeline-04-ast-to-ir.md:49`; measured target file size: 39,887 bytes. | Condense the document below 32,000 bytes while retaining the required sections. Shorten detailed witness examples and ancillary debug/work-graph explanations, using existing cross-references where appropriate. |
| F-003 | minor | `### LoweredValInfo and the lowering environment`, lines 103–106 | The text attributes resolution to `lowerValWithCache`, saying it `resolves the Val`. The helper only looks up and stores the supplied key. `lowerVal` resolves before calling it, whereas `lowerType` passes its input type directly. | `source/slang/slang-lower-to-ir.cpp:3099–3108` contains no resolution; line 3115 performs `val->resolve()` in `lowerVal`; lines 3129–3136 pass `type` directly to the cache and visitor. | State that `lowerVal` resolves its argument before cache lookup, `lowerType` keys the cache with its supplied type, and the shared helper caches only completed lowering results. |