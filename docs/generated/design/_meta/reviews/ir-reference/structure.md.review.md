---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:22+00:00
target_doc: ir-reference/structure.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 50a5584b2851342292d4b982e8c4767f3127bd44d5e4d4de95333b7b3e0e7fa5
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 1
  major: 0
  minor: 0
  nit: 0
---

# Review report for ir-reference/structure.md

## Summary
The page has valid front matter, required sections, and resolving links. The central issue is that it describes `interface_req_entry` instances as children of `interface`, but the source builds them as operands on `IRInterfaceType`; that distinction is important for anyone writing an IR traversal.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/structure.md`.
- Read `_common.md`, `ir-reference-structure.md`, the target document including front matter, dependency docs, and watched source files.
- Resolved the document's relative Markdown links and checked peer generated-doc links against the generated-doc tree.
- Checked required structure, table columns, front matter, `GlobalValueWithCode`, module/global-state rows, struct keys, interface entries, witness tables, symbol aliases, and notable-opcode coverage.
- Spot-checked more than 10 factual claims against source for `module`, `func`, `generic`, `param`, `call`, `global_var`, `global_param`, `globalConstant`, `field`, `key`, `builtinRequirementKey`, `indexedFieldKey`, `interface`, `interface_req_entry`, `witness_table`, `witness_table_entry`, `thisTypeWitness`, `TypeEqualityWitness`, and `SymbolAlias`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### Interface internals` and `### witness_table_entry vs interface_req_entry` | The document says `interface` is a parent container whose `interface_req_entry` values are children, but lowering allocates operand slots on `IRInterfaceType` and writes each requirement entry into those operands. A reader following the doc would walk children and miss interface requirements. | `source/slang/slang-ir-insts.lua:656` declares `interface` as `global = true`, not `parent = true`; `source/slang/slang-lower-to-ir.cpp:11697-11698` creates an `IRInterfaceType` with `operandCount`; `source/slang/slang-lower-to-ir.cpp:11915-11919` stores each `interface_req_entry` with `irInterface->setOperand(entryIndex, constraintEntry)`. | Replace the child/parent wording and operand cell for `interface` with an operand-based description: `interface_req_entry` instances are operands of the `IRInterfaceType`, while witness-table entries are children of `witness_table`. |
