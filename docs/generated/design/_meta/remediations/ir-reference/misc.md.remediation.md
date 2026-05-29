---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T17:30:00+00:00
target_doc: ir-reference/misc.md
review_report: ../../reviews/ir-reference/misc.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/misc.md

## Summary

Three major findings addressed: added "System opcodes" and "Tensor
and runtime helpers" sub-tables for the unclaimed concrete opcodes
the catch-all had been missing; removed the `CastStorageToLogicalBase`
and `LiveRangeMarker` grouping-parent rows; removed `DiffTypeInfo`
(now owned by `differentiation.md`).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-insts.lua:14-18` defines `nop` and `Unrecognized` (system-level placeholders); `:1528-1576` defines runtime/tensor helpers (`makeArrayList`, `makeTensorView`, `allocTorchTensor`, `TorchGetCudaStream`, `TorchTensorGetView`, `allocateOpaqueHandle`); the cooperative-matrix/vector ops from that same range belong on `resources-and-atomics.md` (added there in this cycle); the constexpr arithmetic/cast cluster at `:3142-3174` is value-like and was added to `values.md` in this cycle. | Added `### System opcodes` and `### Tensor and runtime helpers` to the top of `## Opcodes`. |
| F-002 | fixed | `source/slang/slang-ir-insts.lua:2517-2522` and `:2701` show `CastStorageToLogicalBase` and `LiveRangeMarker` as grouping parents with no IR opcode of their own. | Removed both rows; folded a short note above each affected table identifying the parent and its concrete children; renamed the "CastStorageToLogicalBase" notable-opcode block to "Storage / logical casts" so the existing prose still applies. |
| F-003 | fixed | `source/slang/slang-ir-insts.lua:1006-1010` annotates `DiffTypeInfo` as differential-type-info; the paired `differentiation.md` remediation adds the canonical row there. | Removed the `DiffTypeInfo` row from the "Pack and expansion" table. |
