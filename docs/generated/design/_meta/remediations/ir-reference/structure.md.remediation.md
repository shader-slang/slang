---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/structure.md
review_report: ../../reviews/ir-reference/structure.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/structure.md

## Summary

Both findings were fixed; none were rejected, deferred, or escalated.
The `## Source` paragraph now cites the lowering functions that
actually exist, and the `witness_table` opcode is documented with its
concrete-type operand and result-type conformance plus a dedicated
notable callout.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The cited `lowerProgram` / `lowerCallableDecl` / `lowerGenericDecl` / `lowerStructDecl` / `lowerInterfaceDecl` / `lowerInheritanceDecl` do not exist; real symbols are `lowerFuncDecl` (`slang-lower-to-ir.cpp:14089`), `visitGenericDecl` (`:14100`), `visitAggTypeDecl` (`:11938`, calls `createStructType` `:11994`), `visitInterfaceDecl` (`:11520`), `visitInheritanceDecl` (`:10671`), `lowerGlobalVarDecl` (`:11199`), `generateIRForTranslationUnit`/`ensureAllDeclsRec` (`:14672`). | Rewrote the `## Source` lowering sentence to cite the real visitor/helper names. |
| F-002 | fixed | `slang-ir.cpp:4992-4998` records `concreteType` as operand 0 and the interface in the `WitnessTableType` result; `slang-ir-insts.h:2211-2216` exposes `getConcreteType()` / `getConformanceType()`. `ir-reference-structure.md:58-59` requires a `witness_table` notable. | Updated the `witness_table` row's Operands/Summary to name `concreteType` and result-type conformance, and added a `### witness_table` notable callout. |
