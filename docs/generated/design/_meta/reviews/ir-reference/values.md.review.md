---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:22+00:00
target_doc: ir-reference/values.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 50a5584b2851342292d4b982e8c4767f3127bd44d5e4d4de95333b7b3e0e7fa5
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: partial
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for ir-reference/values.md

## Summary
The page has valid front matter, required sections, and resolving links. I found one minor cross-reference/source-alignment issue: the see-also section sends readers to `misc.md` for bitfield opcodes that are actually documented on this page.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/values.md`.
- Read `_common.md`, `ir-reference-values.md`, the target document including front matter, dependency docs, and watched source files.
- Resolved the document's relative Markdown links and checked peer generated-doc links against the generated-doc tree.
- Checked required value-family sections, table columns, front matter, literal payload notes, memory rows, aggregate rows, conversion rows, constexpr rows, and notable-opcode coverage.
- Spot-checked more than 10 factual claims against source for `boolConst`, `integer_constant`, `float_constant`, `string_constant`, `LoadFromUninitializedMemory`, `Poison`, `defaultConstruct`, `add`, `irem`, `frem`, `logicalAnd`, `select`, `BuiltinCast`, `var`, `load`, `store`, `get_field`, `getElementPtr`, `swizzle`, `makeVector`, `makeUInt64`, `makeOptionalNone`, and `constexprAdd`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## See also` | The bullet for `misc.md` says that page covers `bitfieldExtract` and `bitfieldInsert`, but the actual opcode rows for both are in this document and `misc.md` does not list them. This misdirects readers looking for those opcodes. | `docs/generated/design/ir-reference/values.md:125-126` lists `bitfieldExtract` and `bitfieldInsert`; `docs/generated/design/ir-reference/values.md:406-408` claims `misc.md` covers them; `docs/generated/design/ir-reference/misc.md` has no matching opcode rows. | Remove the bitfield-opcode wording from the `misc.md` see-also bullet, or replace it with examples that actually live in `misc.md`, such as type-introspection predicates or pack helpers. |
