---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:06:28+00:00
target_doc: ir-reference/types.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 5ac7df35674b391db414495e8be54b9c8c58690cd2b324a3a4c6804a1748f586
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for ir-reference/types.md

## Summary
The page is broadly source-aligned: front matter is valid, links resolve, and the Type-family opcode table covers the concrete Lua entries I checked. The remaining issues are prompt-contract gaps: `Enum` is required as a notable opcode but is absent, and many sub-tables do not include the required lowering or builder citation.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/types.md` and verified the prompt, dependency list, watched paths, target source commit, and digest against the document front matter.
- Read the target doc, `_common.md`, `ir-reference-types.md`, and the dependency docs `cross-cutting/ir-instructions.md`, `pipeline/04-ast-to-ir.md`, and `ast-reference/types.md`.
- Resolved all 23 relative links in the target document against `52339028a2aa703271533454c6b9528a534bac31`.
- Checked all required IR-reference sections and extracted 154 opcode rows from the `## Opcodes` tables.
- Spot-checked more than 10 factual claims against `source/slang/slang-ir-insts.lua`, including `Void`, `Int`, `Array`, `UnsizedArray`, `Func`, `Vec`, `Mat`, `Enum`, `Ptr`, `AnyValueType`, `BindExistentials`, `BoundInterface`, `TextureType`, `witness_table_t`, `RateQualified`, `TaggedUnionType`, and `OptionalNoneType`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Notable opcodes`, lines 360-449 | The per-document prompt requires `Enum` to be covered in `## Notable opcodes`, but the section has no `Enum` callout. The table row alone does not explain the parent opcode behavior or case children that the prompt asks to highlight. | `docs/generated/design/_meta/prompts/ir-reference-types.md:61` requires `Enum` coverage; `source/slang/slang-ir-insts.lua:137` defines `Enum` with `tagType` and `parent = true`; `source/slang/slang-ir.cpp:5068-5070` creates `kIROp_EnumType` with the tag type. | Add a `### Enum` notable callout explaining the `tagType` operand and parent relationship to enum case children. |
| F-002 | major | `## Opcodes`, lines 95-358 | The type prompt requires at least one row in each sub-table to cite a `slang-lower-to-ir.cpp` visitor or `IRBuilder::getXType` helper, but many sub-tables only name AST categories or synthesized origins. For example, the basic scalar rows use `BasicExpressionType(...)`, and the array rows use `ArrayExpressionType`, with no concrete lowering or builder citation in those sub-tables. | `docs/generated/design/_meta/prompts/ir-reference-types.md:84-85` requires one such citation per sub-table; concrete helpers exist in `source/slang/slang-ir.cpp:2943` for `IRBuilder::getVectorType`, `source/slang/slang-ir.cpp:5053-5070` for struct, class, and enum creation, and `source/slang/slang-lower-to-ir.cpp:897` for the `lowerType` family. | Add at least one concrete lowering visitor or `IRBuilder::getXType` helper citation to every type sub-table, either in an AST-origin cell or a short sentence immediately before the table. |
