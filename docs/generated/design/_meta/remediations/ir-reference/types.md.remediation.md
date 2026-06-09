---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/types.md
review_report: ../../reviews/ir-reference/types.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/types.md

## Summary

Both findings were fixed; none were rejected, deferred, or escalated.
A prompt-required `Enum` notable callout was added, and every
`## Opcodes` sub-table now cites a `slang-lower-to-ir.cpp` helper or an
`IRBuilder::get*/create*Type` builder that produces its opcodes.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `ir-reference-types.md:61` requires an `Enum` notable; `Enum` parent at `source/slang/slang-ir-insts.lua:137`, created by `IRBuilder::createEnumType` at `source/slang/slang-ir.cpp:5067-5072` with the tag type. | Added a `### Enum` notable callout explaining the `tagType` operand and that cases are child insts. |
| F-002 | fixed | `ir-reference-types.md:84-85` requires each sub-table to cite a lowering visitor or `IRBuilder::getXType` helper; most omitted it. Helpers verified in `source/slang/slang-ir-insts.h` (`getBasicType`, `getArrayTypeBase`, `getFuncType`, `getVectorType`, `createEnumType`, `getPtrType`, `getAnyValueType`, `getTupleType`, `getWitnessTableType`, `getBindExistentialsType`, `getTorchTensorType`, `getBackwardDiffIntermediateContextType`, `getTaggedUnionType`, `getType`, `createStructType`/`createClassType`). | Added a one-line builder/lowering citation to each sub-table (16 new lines plus 3 amended intros). |
