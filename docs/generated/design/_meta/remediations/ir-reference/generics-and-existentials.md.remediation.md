---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:00:25Z
target_doc: ir-reference/generics-and-existentials.md
review_report: ../../reviews/ir-reference/generics-and-existentials.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 5
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/generics-and-existentials.md

## Summary
All five findings describe defects that are not present in the current target document; the document already matches the watched source on every point (a prior remediation cycle had already applied them). The review re-flagged the already-corrected text even though its `target_doc_source_commit` equals the doc's current `source_commit` (`c21ead2...`). No edits were made this cycle; every finding is rejected-bogus with source evidence. The action breakdown is 5 rejected-bogus and nothing else.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Required notable callouts already present: target doc line 274 covers `extractExistentialValue` / `extractExistentialType` / `extractExistentialWitnessTable` and line 288 covers `TypeEqualityWitness` (`subType, superType`, `source/slang/slang-ir-insts.lua:853`). | — |
| F-002 | rejected-bogus | Target doc line 146 already lists `concreteType (plus children: witness_table_entry)` and states operand 0 is the conforming type with the interface on the result `WitnessTableType`, matching `getConcreteType()`=`getOperand(0)` at `source/slang/slang-ir-insts.h:2241`. | — |
| F-003 | rejected-bogus | Target doc line 95 already cites `GenericAppDeclRef` substitution in `emitDeclRef`, not `GenericAppExpr`; `visitGenericAppExpr` is `SLANG_UNIMPLEMENTED_X` (`source/slang/slang-lower-to-ir.cpp:7386`), real path emits `emitSpecializeInst` from `GenericAppDeclRef` (`:14738`, `:14796`). | — |
| F-004 | rejected-bogus | Target doc line 159 already shows the Operands cell as `type`, not `(variadic)`, consistent with the 1-operand `IRRTTIObject` doc at `source/slang/slang-ir-insts.h:2244`. | — |
| F-005 | rejected-bogus | Target doc lines 214-215 already list `inst` for `WeakUse` and `func` for `FuncTypeOf`, each with a source-usage-name note, matching `IRBuilder::getWeakUse(IRInst*)` (`source/slang/slang-ir-insts.h:3526`) and the single `kIROp_FuncTypeOf` type operand (`source/slang/slang-lower-to-ir.cpp:2432`). | — |
