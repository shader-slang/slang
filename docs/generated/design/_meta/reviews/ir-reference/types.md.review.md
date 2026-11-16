---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:29:00Z
target_doc: ir-reference/types.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: fc1be7181a600580a7302abe6daad827bab78a630769ceed21fea8ce8e0b5c0a
source_commit: 801b9e6e57d41f5cef1a5d54c0eeff9e0db4fee6
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 6
severity_breakdown:
  critical: 1
  major: 1
  minor: 3
  nit: 1
---

# Review report for ir-reference/types.md

## Summary

The catalog covers all 170 concrete type opcodes, and its explicit line citations resolve to the described source locations. It needs corrections before approval: most importantly, the pointer callout misstates an operand’s representation and the meaning of `hasAddressSpace()`. Producer attribution also remains substantially incomplete under the common contract.

## Items checked

- Reviewed against the **current working tree**, which the operator identifies as matching the document’s source commit throughout `source/`. No files were written.
- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show ir-reference/types.md`.
- Parsed the Lua instruction hierarchy: verified 865 total concrete opcodes, all 170 type-family rows, every row’s H/P/G flags, and all immediate hierarchy groups.
- Checked every explicit line-number citation, including repeated citations and ranges, against the corresponding source text.
- Spot-checked more than 25 behavioral claims, including intrinsic lowering, scalar aliases, matrix layout, function attributes, enum construction, interface requirements, differentiation contexts, descriptor handles, and tagged-union operand order.
- Resolved all 80 Markdown link occurrences, covering 40 distinct destinations; verified all 12 generated-page destinations appear in the manifest. Read dependency material only for cross-reference checks.
- Checked required sections, table columns, front-matter keys and formats, Mermaid structure, Liquid safety, and the 68,609-byte document against its 98,304-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Notable opcodes`, lines 727–738 | The claim that the three optional pointer operands are `IRIntLit`s is wrong: `dataLayout` is an `IRType*`. Also, `hasAddressSpace()` does not indicate whether an operand “was really recorded”; an explicitly recorded `AddressSpace::Generic` returns false. These statements would lead a pass author to interpret valid operands incorrectly. | `source/slang/slang-ir.h:1595–1607`: `hasAddressSpace()` requires both operand presence and a non-Generic value; `getDataLayout()` returns `IRType*`. | Describe access qualifier and address space as integer literals, and data layout as a type operand. Define `hasAddressSpace()` as testing for a non-Generic address space; use `getOperandCount() > 2` when testing physical operand presence. |
| F-002 | major | `## Opcodes`, throughout | Fifty-two AST-origin cells still say `(synthesized)`, omitting the required producer distinction. This also hides source-declared types: texture shape markers and tensor-addressing types have intrinsic declarations. | `docs/generated/design/_meta/prompts/_common.md:274` explicitly retires `(synthesized)`. Concrete examples: `source/slang/slang-ir.cpp:5457–5459` constructs blocks using `getBasicBlockType()`; `source/slang/hlsl.meta.slang:544–546` declares `__Shape1D`; `source/slang/hlsl.meta.slang:29748–29757` declares `TensorLayout` and its constructor intrinsic. | Replace the 52 placeholders with verified producer functions/passes or intrinsic declarations and their lowering route. For example, name `IRBuilder::createBlock`, `__Shape1D` via `lowerSimpleIntrinsicType`, and `TensorLayout` via that same lowering helper. Use `no producer at HEAD` only after a source-wide search finds none. |
| F-003 | minor | `### Witness-table types`, line 432 | `Both are built by IRBuilder::getWitnessTableType` conflates the ordinary witness-table type with its integer-ID form. | `source/slang/slang-ir-lower-dynamic-dispatch-insts.cpp:1798` constructs the latter with `builder.getWitnessTableIDType((IRType*)interfaceType)`. | State that `witness_table_t` uses `getWitnessTableType`, while `witness_table_id_t` uses `getWitnessTableIDType`. |
| F-004 | minor | `### Set-theoretic types`, lines 619–624 | The blanket statement that the wrappers are hand-written and “each takes a single IRSetBase operand” contradicts two rows immediately below it: `TaggedUnionType` takes two operands, and `OptionalNoneType` is nullary and generated. | `source/slang/slang-ir-insts.h:3101–3105` reads tagged-union operands 0 and 1; `source/slang/slang-ir-insts.lua:874–880` declares nullary `OptionalNoneType`, covered by the generated-wrapper template at `source/slang/slang-ir-insts.h:3157–3197`. | Limit the single-set description to `UntaggedUnionType`, `ElementOfSetType`, and `SetTagType`; explicitly distinguish the two-operand tagged union and generated nullary optional-none type. |
| F-005 | minor | `### The work-graph record types`, lines 820–832 | The mandatory checklist requires both `[shader("node")]` and `-stage node`. The explicit stage option is redundant when the attribute supplies the stage. | `source/slang/slang-check-shader.cpp:2820–2845`, `resolveStageOfProfileWithEntryPoint`, derives the stage from `EntryPointAttribute` and fills an unknown profile stage with it. | Present `-stage node` as optional when `[shader("node")]` already specifies the entry-point stage, rather than requiring both. |
| F-006 | nit | Opcode summaries, lines 522 and 631 | Summary cells use emphasis markup (`*named*`, `**first**`), contrary to the family’s inline-code-only rule. | `docs/generated/design/_meta/prompts/_common.md:275`: “One short clause; no markup beyond inline code.” | Remove the emphasis markers from these summary cells. |