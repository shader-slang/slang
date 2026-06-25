---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:22+00:00
target_doc: ir-reference/types.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 50a5584b2851342292d4b982e8c4767f3127bd44d5e4d4de95333b7b3e0e7fa5
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 1
  major: 1
  minor: 0
  nit: 0
---

# Review report for ir-reference/types.md

## Summary
The page has valid front matter, required sections, and resolving links. I found one critical representation error for `interface` and one Type-family coverage gap: two concrete stable Type opcodes are absent from the opcode tables.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/types.md`.
- Read `_common.md`, `ir-reference-types.md`, the target document including front matter, dependency docs, and watched source files.
- Resolved the document's relative Markdown links and checked peer generated-doc links against the generated-doc tree.
- Checked required Type-family sections, table columns, front matter, hierarchy coverage, notable-opcode coverage, and selected dependency cross-references.
- Spot-checked more than 10 factual claims against source for `Void`, `Bool`, `Int`, `UIntPtr`, `CapabilitySet`, `AnyValueType`, `RawPointerType`, `Array`, `UnsizedArray`, `Func`, `Vec`, `Mat`, `Enum`, `TextureType`, `interface`, `associated_type`, `Ptr`, `witness_table_t`, `RateQualified`, `TaggedUnionType`, and `OptionalNoneType`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### Existentials and interfaces` | The `interface` row says the opcode owns `interface_req_entry` children, but source lowering stores those entries as operands on `IRInterfaceType`, and the Lua declaration is `global`, not `parent`. This can mislead pass authors into traversing children and missing requirements. | `source/slang/slang-ir-insts.lua:656` declares `interface` as `global = true`; `source/slang/slang-lower-to-ir.cpp:11697-11698` creates the interface with an operand count; `source/slang/slang-lower-to-ir.cpp:11915-11919` writes entries with `irInterface->setOperand(entryIndex, constraintEntry)`. | Change the operands cell and summary to say `interface_req_entry` instances are operands of `IRInterfaceType`, and cross-link to `structure.md` for the representation distinction from `witness_table` children. |
| F-002 | major | `## Opcodes` | The Type-family table omits `AfterBaseType` and `AfterRawPointerTypeBase`. They are concrete stable opcode entries in the Type family even though they look like range sentinels, so the IR-reference coverage rule requires them to be listed or explicitly explained. | `source/slang/slang-ir-insts.lua:42` declares `AfterBaseType`; `source/slang/slang-ir-insts.lua:67` declares `AfterRawPointerTypeBase`; `source/slang/slang-ir-insts-stable-names.lua:22` and `source/slang/slang-ir-insts-stable-names.lua:30` assign both stable opcode names. | Add rows for both sentinel-like Type opcodes, with summaries that explain their range-marker role, or add an explicit note in the table if the prompt is changed to exclude stable sentinel opcodes. |
