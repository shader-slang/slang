---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:56:05Z
target_doc: ast-reference/types.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: fbf01df9017d66463a64e01622276ae0c4dd5286d0596b4b1a494ffe03ab0b91
source_commit: 56b1b0b83fb96f88f36da7e37cd9c2c7f27af70c
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 0
  minor: 3
  nit: 0
---

# Review report for ast-reference/types.md

## Summary

The document covers all concrete classes and has valid cross-references and front matter. Three localized corrections are needed: the exclusive claim about magic-type registration overlooks builtin scalar registration, descriptor-handle lowering needs a more precise phase description, and one table summary exceeds the required format.

## Items checked

- Reviewed against the **current working tree**, which the operator identifies as matching the document’s source commit throughout `source/`; the source diff against that commit was empty.
- Read the review prompt, common generation contract, and document-specific prompt; ran `regenerate.py show ast-reference/types.md`.
- Compared all 123 concrete `FIDDLE()` classes in `slang-ast-type.h` with the 123 table rows: no missing or extra classes.
- Spot-checked more than 20 claims, including `Val → Type` inheritance, canonicalization, reflection storage, parser entry points, vector/matrix accessors, pointer qualifiers, function operand order, parameter-mode decoding, existential caches, pack operands, literal boundaries, and descriptor-heap capability gates.
- Verified all four explicit line-number citations: `slang-ast-type.cpp` lines 124, 1392, and 1400, and `slang-ast-type.h` line 50. All match.
- Resolved all 56 Markdown link occurrences, covering 20 distinct destinations; confirmed all nine referenced generated pages are registered in the manifest and checked the three distinct fragment targets. Dependency documents were consulted only for cross-reference checks.
- Checked required section order, table columns, front-matter keys and formats, Mermaid syntax, and Liquid safety. The document is 35,226 bytes, below its 65,536-byte cap.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `### DeclRefType`, lines 302–309 | The statement that a core-module type “only gets its own C++ class here when it is bound to one with `__magic_type(...)`” overlooks scalar builtins registered through `__builtin_type`. The distinction concerning intrinsic-only work-graph records does not establish this exclusive rule. | `source/slang/core.meta.slang:1187` declares scalar types using `__builtin_type`; `source/slang/slang-ast-type.cpp:2250` implements `BasicExpressionType::getBaseType()` by reading `BuiltinTypeModifier`. | Qualify the rule to acknowledge `__builtin_type` scalar registration alongside `__magic_type`, while retaining the narrower explanation about intrinsic-only work-graph records. |
| F-002 | minor | `### UntypedResourceHandleType and UntypedSamplerHandleType`, lines 455–456 | Saying handles become `uint` “during IR lowering” obscures the distinction between producing handle IR during AST lowering and eliminating surviving handles in the later target pipeline. | `source/slang/slang-ast-type.h:488` names the `lowerUntypedResourceHandleToUInt` IR pass. `source/slang/slang-emit.cpp:2137–2144` explicitly distinguishes AST-to-IR production from the later gated pass invocation. | Replace the timing phrase with “by the `lowerUntypedResourceHandleToUInt` IR pass before emission,” retaining the existing cross-reference for details. |
| F-003 | minor | `## Nodes`, `NamedExpressionType` row, line 150 | The Summary cell contains several sentences, implementation discussion, and a Markdown source link. The family contract requires one short clause with no markup beyond inline code. | `docs/generated/design/_meta/prompts/_common.md`, AST reference family contract, defines Summary as “One short clause; no markup beyond inline code.” The relevant implementation is `source/slang/slang-ast-type.cpp:1392–1406`. | Reduce the cell to a short description of an alias type. Move the printing/canonicalization explanation and citations into a `NamedExpressionType` callout under `## Notable nodes`. |