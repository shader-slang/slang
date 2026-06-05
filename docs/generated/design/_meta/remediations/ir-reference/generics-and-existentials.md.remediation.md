---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/generics-and-existentials.md
review_report: ../../reviews/ir-reference/generics-and-existentials.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/generics-and-existentials.md

## Summary

Both findings were fixed; none were rejected, deferred, or escalated.
The witness-table / witness-fact section was converted from a bullet
list into a proper opcode sub-table (as the prompt's "Witness facts"
group requires), and the `## Source` range list gained the missing
type-flow specialization range.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `ir-reference-generics-and-existentials.md:35` requires a Witness-facts opcode sub-table; the section used bullets. Opcode shapes verified at `source/slang/slang-ir-insts.lua:816-824` and `:1042-1048`, consistent with the rows in `ir-reference/structure.md`. | Replaced the bullet list under `### Witness tables and witness facts` with a 7-row sub-table (`witness_table`, `witness_table_entry`, `interface_req_entry`, `thisTypeWitness`, `TypeEqualityWitness`, `key`, `indexedFieldKey`). |
| F-002 | fixed | Page documents the type-flow opcodes (`TypeSet`, `GetDispatcher`, tagged-union/tag ops) defined at `source/slang/slang-ir-insts.lua:2926-3162`, but `## Source` omitted that range. | Added a bullet to `## Source` citing the type-flow specialization range (~2916-3162). |
