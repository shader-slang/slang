---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:06:28+00:00
target_doc: ir-reference/values.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 5ac7df35674b391db414495e8be54b9c8c58690cd2b324a3a4c6804a1748f586
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for ir-reference/values.md

## Summary
The page has valid front matter, all required top-level sections, and all relative links resolve at the recorded source commit. Two major issues remain: several prompt-required value opcodes are missing from the tables, and `logicalAnd` plus `logicalOr` are described with source-language short-circuit semantics that the IR instructions do not have.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/values.md` and verified the prompt, dependency list, watched paths, target source commit, and digest against the document front matter.
- Read the target doc, `_common.md`, `ir-reference-values.md`, and the dependency docs `cross-cutting/ir-instructions.md`, `pipeline/04-ast-to-ir.md`, `ast-reference/expressions.md`, and `ir-reference/types.md`.
- Resolved all 22 relative links in the target document against `52339028a2aa703271533454c6b9528a534bac31`.
- Checked required sections and extracted 144 opcode rows from the `## Opcodes` tables.
- Spot-checked more than 10 factual claims against `source/slang/slang-ir-insts.lua`, `source/slang/slang-ir-insts.h`, `source/slang/slang-ir.cpp`, and `source/slang/slang-lower-to-ir.cpp`, including literal payloads, `LoadFromUninitializedMemory`, `Poison`, `defaultConstruct`, arithmetic op operands, comparison op wrappers, conversion op operands, `var`, `load`, `store`, field and element access, swizzles, aggregate constructors, optional helpers, and constexpr rows.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Opcodes`, lines 215-255 | The values prompt explicitly includes aggregate and conversion helpers that are absent from the tables. In particular, `makeValuePack`, `makeCombinedTextureSampler`, `packAnyValue`, and `unpackAnyValue` are concrete Lua opcodes in the watched ranges, but none has a row in this page. | `docs/generated/design/_meta/prompts/ir-reference-values.md:38-47` lists `Pack*`, `Unpack*`, and `makeCombinedTextureSampler`; `source/slang/slang-ir-insts.lua:970-971` defines `makeValuePack` and `makeCombinedTextureSampler`; `source/slang/slang-ir-insts.lua:1040-1041` defines `packAnyValue` and `unpackAnyValue`. | Add rows for these opcodes in the relevant aggregate or conversion sub-tables, or revise the manifest and prompt if ownership is intentionally moved to another IR-reference page. |
| F-002 | major | `### Logical`, lines 132-133 | The table describes `logicalAnd` and `logicalOr` as short-circuiting, but the IR opcodes are ordinary two-operand instructions. Short-circuit behavior belongs to source-expression lowering before both operands become IR values, not to these opcodes themselves. | `source/slang/slang-ir-insts.lua:1465-1471` defines `logicalAnd` and `logicalOr` with `left, right` operands; `source/slang/slang-ir.cpp:6730-6739` emits `kIROp_And` and `kIROp_Or` from already supplied `left` and `right` operands. | Change the summaries to describe boolean AND and boolean OR over already-lowered operands, and remove the word `Short-circuit` from both rows. |
