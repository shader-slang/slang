---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:27+00:00
target_doc: ir-reference/types.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 64be22b621bde4e26ac349ba999894219b13a0f0d103c6e61d02970a8258d1bc
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 3
  minor: 2
  nit: 0
---

# Review report for ir-reference/types.md

## Summary

The page is exhaustive and most source-backed claims are accurate, but it has five findings. Most importantly, every `C++ wrapper` cell omits the required `IR` prefix, so the table systematically gives `struct_name` values rather than exact C++ wrapper symbols; the operand cells and per-subtable citations also do not follow the generation contract.

## Items checked

- Ran `regenerate.py show ir-reference/types.md` and reviewed the target document, the common contract, its per-document prompt, and the resolved watched-file set (`source/slang/slang-ir-insts.h`, `source/slang/slang-ir-insts.lua`, `source/slang/slang-ir.cpp`, `source/slang/slang-ir.h`, `source/slang/slang-lower-to-ir.cpp`).
- Verified all 56 line-number references against source at `53b76e6d3009b8e6434d41573524c7ce5c499d23`; the watched sources are unchanged from that commit.
- Spot-checked more than 10 factual claims, including the 170 live Type-family opcodes, Lua nesting and flags, wrapper generation, builder exclusions, AST lowering visitors, pointer and texture layouts, work-graph records, differentiation types, tagged-union operand order, and untyped-pointer legalization.
- Compared all 170 opcode rows against the live `Type` entries in `source/slang/slang-ir-insts.lua`; no live opcode is omitted or duplicated.
- Resolved all 62 relative-link occurrences (31 unique targets), including generated peers and handwritten design pages; none is missing.
- Recomputed the watched-path digest and confirmed it matches the document front matter; also verified the mandatory sections, table columns, size cap, and report-model identity requirements.

## Findings

| ID    | Severity | Location                                                                | Description                                                                                                                                                                                                                                                                                                                                                             | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                | Recommendation                                                                                                                                                                                                                       |
| ----- | -------- | ----------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| F-001 | major    | `## Opcodes`, all 170 rows (lines 184-529)                              | Every `C++ wrapper` cell omits the `IR` prefix. For example, the page gives `VectorType`, `ArrayType`, and `StructType`, but the required exact wrapper symbols are `IRVectorType`, `IRArrayType`, and `IRStructType`. This is systematic and can direct readers to unrelated AST symbols.                                                                              | The family contract requires an `IRFoo` struct name (`docs/generated/design/_meta/prompts/_common.md` lines 236-238). The generated declaration template is `struct IR$(inst.struct_name)` in `source/slang/slang-ir-insts.h` lines 3113-3148; hand-written wrappers likewise include `IR`, such as `IRSetTagType` at line 3050.                                                                                                        | Prefix every wrapper cell with `IR`, using the exact generated or hand-written C++ symbol (for example, `IRVectorType` and `IRStructType`).                                                                                          |
| F-002 | major    | `## Opcodes`, operand legend and affected rows (lines 167-175, 245-529) | The page substitutes inferred C++ or construction-site layouts marked `†` for Lua entries that omit `operands`, although the per-document prompt requires the literal `(see Lua)` in that case. It also uses forms such as `types...` and `(same as Ptr)` where the family contract requires `(variadic)` for variadic opcodes and compact Lua operand names otherwise. | `docs/generated/design/_meta/prompts/ir-reference-types.md` lines 35-39 specifies `(see Lua)` for entries without explicit operands, and `_common.md` lines 238-240 specifies `(variadic)`. For example, `TorchTensor` has no `operands` field in `source/slang/slang-ir-insts.lua` line 222, while the page reports `elementType†`; `ForwardDiffFuncType` through `RematFuncType` likewise have no declared operands at lines 209-213. | Restore the contracted operand vocabulary in every row: use Lua names where declared, `(variadic)` for variadic entries, `(see Lua)` where `operands` is absent, and move useful inferred runtime layouts into notable-opcode prose. |
| F-003 | major    | `## Opcodes`, 11 of 20 subtables                                        | The per-document quality checklist requires at least one row in each subtable to cite a producing lowering visitor or `IRBuilder::getXType` helper. Eleven subtables, including `Basic scalar types`, `Storage-only floating-point`, `Arrays`, and `Work-graph record types`, have no such row-level citation; nine subtables do name a producer in a row.              | `docs/generated/design/_meta/prompts/ir-reference-types.md` lines 84-85 requires one producing-symbol citation per subtable. For example, `Basic scalar types` has no row citation despite `visitBasicExpressionType` calling `getBasicType` at `source/slang/slang-lower-to-ir.cpp` lines 2839-2842, and `Arrays` has no row citation despite `visitArrayExpressionType` at lines 2861-2873.                                           | Add one source-linked producing visitor or builder helper to at least one row in each currently uncited subtable.                                                                                                                    |
| F-004 | minor    | `## Notable opcodes` / `### The work-graph record types`, line 666      | The page says `The four Empty* variants are nullary`, but the ten-opcode subgroup contains only three `Empty*` opcodes: `EmptyNodeInput`, `EmptyNodeOutput`, and `EmptyNodeOutputArray`.                                                                                                                                                                                | `source/slang/slang-ir-insts.lua` lines 232-280 lists the subgroup; its only `Empty*` entries are at lines 252, 277, and 278.                                                                                                                                                                                                                                                                                                           | Change `four` to `three`.                                                                                                                                                                                                            |
| F-005 | minor    | Introductory paragraphs, lines 12-20                                    | The universal contract requires the first body paragraph to state both what the page covers and who it is for. The page puts the intended audience in a separate second paragraph.                                                                                                                                                                                      | `docs/generated/design/_meta/prompts/_common.md` lines 65-66 requires both facts in the first paragraph.                                                                                                                                                                                                                                                                                                                                | Merge the intended-reader sentence into the first paragraph.                                                                                                                                                                         |

## No-issues notes

- The opcode catalog is exhaustive: all 170 live Lua Type-family leaves appear exactly once.
- The family hierarchy includes every immediate Lua subgroup and correctly distinguishes nested resource and parameter-group bases.
- All required front-matter keys are present, and the watched-path digest recomputes exactly.
