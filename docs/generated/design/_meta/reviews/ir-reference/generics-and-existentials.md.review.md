---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:06:21Z
target_doc: ir-reference/generics-and-existentials.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 56404e70e333b4f755dead63d0132446c9ebb690ea7e2d1f06bbbb9e8de4f68c
source_commit: 8341c7e537a3f5cb3a60cd062066ffd630da40d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 6
severity_breakdown:
  critical: 1
  major: 0
  minor: 5
  nit: 0
---

# Review report for ir-reference/generics-and-existentials.md

## Summary

The document substantially matches the current source, with accurate opcode coverage and useful distinctions between active producers and unused builders. The most important correction concerns interface representation: requirement entries are global instructions referenced through interface operands, not children of the interface. Five smaller findings address conditional behavior, producer attribution, wrapper counting, and a missing required source link.

## Items checked

- Read `_review.md`, `_common.md`, and the document-specific prompt obtained through `regenerate.py show`.
- Verified against the **current working tree**, which the operator states matches the document’s source commit throughout `source/`.
- Checked every located line-number citation, covering 170 distinct file/line starting locations and the cited `SetBase` comment range. Locations resolve; two producer citations actually identify consumers.
- Spot-checked more than 10 claims, including specialization operands, witness lookup keys, existential construction and projections, global generic diagnostics, interface requirements, set canonicalization, tagged-union operand order, wrapper generation, optional-constraint lowering, and unused builders.
- Checked the 52 concrete opcode rows against Lua definitions and relevant C++ wrappers/builders.
- Resolved all 119 relative-link occurrences, representing 35 distinct destinations; checked generated destinations against the manifest and the referenced section anchors. Dependency documents were consulted only for cross-reference checks.
- Checked required sections, table structure, front matter, Mermaid conventions, Liquid safety, and the 65,536-byte size cap; the document is 53,090 bytes.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### Witness tables and witness facts`, lines 299–307 | The example says the interface has three `interface_req_entry` **children**. This describes the wrong IR relationship and could lead readers to traverse children instead of operands. | `source/slang/slang-lower-to-ir.cpp:12187` allocates the interface with requirement operands; line 12346 uses `irInterface->setOperand(entryIndex, entry)`. `source/slang/slang-ir.cpp:5325–5336` creates each entry with `addGlobalValue(this, entry)`. | Replace “children” with “operands referencing global requirement-entry instructions.” Preserve the three-key example and explain that these entries are not nested under the interface. |
| F-002 | minor | `### wrapExistential`, lines 532–537 | The claim that targeting an `InterfaceType` produces `makeExistential` omits conditions and an additional identity case. Consequently, the unconditional conclusion about alternative spellings is too strong. | `source/slang/slang-ir.cpp:5037–5055` requires `slotArgCount >= 2`; when `slotArgs[0]` is `DynamicType`, it returns `value` unchanged. Otherwise it extracts the bound value and builds `makeExistential`. | State the slot-count condition and the `DynamicType` identity case. Limit the canonicalization claim to the concrete-interface wrapping case handled by this branch. |
| F-003 | minor | `#### Dispatchers and existential specialization`, lines 448–451 | `FuncTypeOf` is called “the one row in this table” not produced by type-flow specialization. The same table includes two unproduced dispatcher opcodes and `SpecializeExistentialsInType`, produced by ordinary specialization. | `source/slang/slang-ir-specialize.cpp:3214` constructs `SpecializeExistentialsInType`; `source/slang/slang-ir-insts.h:4580` and `:4601` define the uncalled dispatcher builders. | Replace the exclusivity claim with “`FuncTypeOf` is produced during AST lowering by `lowerFuncDependentType`.” Retain the verified construction-site explanation. |
| F-004 | minor | `## Source`, lines 50–61 | “Twenty-one” hand-written opcode wrappers counts the abstract `IRSetBase` alongside concrete wrappers. There are 20 hand-written concrete wrappers in the opcode tables, plus this abstract base. | `source/slang/slang-ir-insts.h:3009–3011` declares `IRSetBase` with `FIDDLE(baseInst())`; the concrete set wrappers at lines 3080 and 3087 use `leafInst()`. | State “Twenty concrete opcode wrappers are hand-written, together with the abstract `IRSetBase`,” and retain the existing declaration list. |
| F-005 | minor | `#### Tagged unions and tag operations`, lines 432–433 | The AST-origin cells cite dynamic-dispatch lines 1115 and 1119 as producers. These locations register processing of existing instructions, rather than constructing them. The other cited locations in those cells are valid producers. | `source/slang/slang-ir-lower-dynamic-dispatch-insts.cpp:1114–1120` calls `processInstsOfType` and dispatches to lowering functions. Construction occurs at lines 1245 and 1290. | Remove 1115 and 1119 from the producer citations; retain 1245 and 1290 respectively. |
| F-006 | minor | `## Source` | The source section omits the required link to `slang-ir.h`, despite relying on IR infrastructure and hoistable/global flags throughout the page. | `docs/generated/design/_meta/prompts/_common.md`, IR-reference family contract, item 2, requires including `source/slang/slang-ir.h` when the family relies on infrastructure declared there. | Add a relative Markdown link to `../../../../source/slang/slang-ir.h` in the source section, identifying its role in core IR infrastructure and flags. |