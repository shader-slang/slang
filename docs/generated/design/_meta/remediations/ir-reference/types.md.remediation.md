---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:16:04Z
target_doc: ir-reference/types.md
review_report: ../../reviews/ir-reference/types.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 2, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for ir-reference/types.md

## Summary
Both findings from the review were verified against source at `eb9403ef595a99c2ff6def1d538dbd7a792d9371` and both were correct and in-contract, so both were fixed. F-001 (the `interface` row claiming `interface_req_entry` children) was corrected to describe the entries as operands. F-002 (the two missing sentinel-like Type opcodes) was fixed by adding rows for `AfterBaseType` and `AfterRawPointerTypeBase`. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified: `slang-ir-insts.lua:656` declares `interface` as `global = true` (not `parent`), and `slang-lower-to-ir.cpp:11915-11918` stores each `interface_req_entry` via `irInterface->setOperand(entryIndex, constraintEntry)` after `createInterfaceType(operandCount, ...)` at `:11697-11698`. The entries are operands, not children; the existing `G` flag was already correct. | Changed the `interface` Operands cell from `(children: interface_req_entry)` to `(operands: interface_req_entry...)` and updated the summary to state the requirements are operands set via `setOperand`, with a cross-link to structure.md for the `witness_table`-children distinction. |
| F-002 | fixed | Verified: `slang-ir-insts.lua:42` declares `{ AfterBaseType = {} }` (direct child of `Type`, no `hoistable`) and `:67` declares `{ AfterRawPointerTypeBase = {} }` (inside the `hoistable` `RawPointerTypeBase` group). `process_inst` (`:3312-3324`) marks both `is_leaf` with a defaulted `struct_name`, and `slang-ir.h.lua:288-290` emits a `kIROp_` enum per leaf; stable names 18 and 26 are assigned in `slang-ir-insts-stable-names.lua:22,30`. They are concrete leaf opcodes, not abstract group parents, so the coverage rule requires them in the table. | Added a row for `AfterBaseType` to the Basic scalar types table (em-dash C++ wrapper, blank Flags since it does not inherit `hoistable`) and a row for `AfterRawPointerTypeBase` to the Raw and RTTI pointers table (em-dash wrapper, `H` flag inherited from its group); both summaries explain the range-marker role. |
