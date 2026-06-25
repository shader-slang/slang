---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:14:29Z
target_doc: ir-reference/structure.md
review_report: ../../reviews/ir-reference/structure.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/structure.md

## Summary

The review raised one critical finding (F-001), which I verified
against source at the document's commit and fixed. The action
breakdown is one fixed finding, with no rejections, deferrals, or
escalations.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against source at eb9403ef: `slang-ir-insts.lua` declares `interface` with `global = true` (not `parent = true`, unlike the neighbouring `struct`/`class` which are `parent = true`), and `slang-lower-to-ir.cpp` creates `IRInterfaceType` with `operandCount` (line ~11697) then stores each requirement via `irInterface->setOperand(entryIndex, constraintEntry)` (line ~11918). So `interface_req_entry` instances are operands of the `IRInterfaceType`, not children. Operand-vs-child shape is squarely within the IR-reference family contract. | Reworded the Interface-internals intro and `interface` table row to describe `interface_req_entry` as operands rather than children, and changed the `witness_table_entry vs interface_req_entry` notable to say `interface_req_entry` is an operand of an `InterfaceType` rather than living inside an `InterfaceType` parent. |
