---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T17:30:00+00:00
target_doc: ir-reference/values.md
review_report: ../../reviews/ir-reference/values.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/values.md

## Summary

Three major findings addressed: removed the `Undefined`
grouping-parent row, added the missing concrete value-like opcodes
in two new sub-tables ("Strings and native pointers", "Object and
CUDA helpers"), and added a comprehensive
`### Constexpr arithmetic and casts` sub-table.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-insts.lua:857-894` shows `Undefined` as the grouping parent of `LoadFromUninitializedMemory` and `Poison`; it is not itself an opcode. | Removed the `Undefined` row from `### Undefined and default-construct` and added a one-sentence note above the table identifying it as the grouping parent of the two remaining concrete rows. |
| F-002 | fixed | `source/slang/slang-ir-insts.lua:946`, `:1067`, and `:1139-1149` define `allocObj`, `CUDA_LDG`, `getNativeStr`, `makeString`, `getNativePtr`, `getManagedPtrWriteRef`, `ManagedPtrAttach`, and `ManagedPtrDetach` as concrete value-producing opcodes. | Added `### Strings and native pointers` with `makeString`, `getNativeStr`, `getNativePtr`, `getManagedPtrWriteRef`, `ManagedPtrAttach`, `ManagedPtrDetach`; added `### Object and CUDA helpers` with `allocObj` and `CUDA_LDG`. Both sub-tables inserted between "Memory" and "Aggregate constructors". |
| F-003 | fixed | `source/slang/slang-ir-insts.lua:3142-3174` defines `constexprAdd` through `constexprEnumCast` as hoistable arithmetic / cast variants used for `IntVal`-class lowering. | Added `### Constexpr arithmetic and casts` between "Result / Optional / Conditional helpers" and "Notable opcodes", with rows for all 29 constexpr opcodes. |
