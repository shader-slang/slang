---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T17:30:00+00:00
target_doc: ir-reference/generics-and-existentials.md
review_report: ../../reviews/ir-reference/generics-and-existentials.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/generics-and-existentials.md

## Summary

Two major findings addressed: a new `### Type-flow specialization`
section was added covering the set, tagged-union, and dispatcher
clusters; the duplicate `key`, `indexedFieldKey`, `witness_table`,
`witness_table_entry`, `interface_req_entry`, `thisTypeWitness`,
and `TypeEqualityWitness` rows were replaced with a cross-link
paragraph pointing back to `structure.md`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-insts.lua:2880-2972` and `:2975-3121` define a substantial type-flow-specialization opcode cluster (sets, tagged unions, tag operations, dispatchers, existential specializations); it is clearly existential/generic machinery and belongs here rather than in `misc.md`. | Added `### Type-flow specialization` with three sub-tables: "Sets and set elements" (`TypeSet`, `FuncSet`, `WitnessTableSet`, `GenericSet`, `Unbounded*Element`, `Uninitialized*Element`, `None*Element`); "Tagged unions and tag operations" (`MakeTaggedUnion`, `CastInterfaceToTaggedUnionPtr`, the `Get*Tag*` family, `GetElementFromTag`, `GetTagOfElementInSet`); and "Dispatchers and existential specialization" (`GetDispatcher`, `GetSpecializedDispatcher`, `SpecializeExistentialsIn{Func,Type}`, `WeakUse`, `FuncTypeOf`). |
| F-002 | fixed | The duplicate rows were already correctly listed in `structure.md` (the owning page for structural opcodes); having them in both places was the issue. | Replaced the `### Witness tables and witness facts` opcode table with a three-bullet cross-link paragraph pointing back to `structure.md` for the structural rows and naming the placeholder witnesses (`thisTypeWitness`, `TypeEqualityWitness`) that the dispatch path consumes. |
