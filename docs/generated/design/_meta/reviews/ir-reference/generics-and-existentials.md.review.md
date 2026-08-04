---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:25:38+00:00
target_doc: ir-reference/generics-and-existentials.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: e27926ca78614bca20d3b57a5268d5884f642e04074ed66afbbed157eadbfdd7
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 2
  minor: 3
  nit: 0
---

# Review report for ir-reference/generics-and-existentials.md

## Summary
The page covers the main scattered opcode groups and its links/front matter look sound, but several table rows do not match the source operand shapes. The most important remediation is to restore prompt-required notable coverage for existential projections and `TypeEqualityWitness`, then fix the opcode-shape rows that would mislead someone reading IR.

## Items checked
- Ran `regenerate.py show ir-reference/generics-and-existentials.md` and reviewed the target document, common contract, per-document prompt, dependency docs, and the resolved watched files.
- Checked front matter for all required generated-doc keys, the recorded source commit, warning string, and 64-character hex watched-path digest.
- Resolved the document's relative links to generated peer pages, watched source files, and the two `docs/design/` rationale pages.
- Verified the required IR-reference sections: Source, Family hierarchy, Opcodes, Notable opcodes, and See also.
- Spot-checked source claims for `specialize`, `lookupWitness`, `GetSequentialID`, `bind_global_generic_param`, `globalValueRef`, `rtti_object`, `packAnyValue`, `unpackAnyValue`, `witness_table_entry`, `interface_req_entry`, `makeExistential`, `makeExistentialWithRTTI`, `createExistentialObject`, `wrapExistential`, `extractExistentialValue`, `extractExistentialType`, `extractExistentialWitnessTable`, `TypeSet`, `GetDispatcher`, `WeakUse`, `FuncTypeOf`, `getInterfaceRequirementKey`, `emitSpecializeInst`, `emitLookupInterfaceMethodInst`, and `emitMakeExistential`.
- Checked the prose line-number claims around the Lua ranges for structural, existential, RTTI, set, dispatcher, and specialization-key entries.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Notable opcodes`, lines 229-338 | The per-document prompt requires notable coverage for `ExtractExistentialValue` / `ExtractExistentialType` / `ExtractExistentialWitnessTable` and `TypeEqualityWitness`, but the notable section has no callout for either group. | `docs/generated/design/_meta/prompts/ir-reference-generics-and-existentials.md:58` requires the three extract projections, and `docs/generated/design/_meta/prompts/ir-reference-generics-and-existentials.md:64` requires `TypeEqualityWitness`. The source declares `TypeEqualityWitness` with `subType, superType` operands at `source/slang/slang-ir-insts.lua:853`. | Add short notable callouts for the three existential projections and for `TypeEqualityWitness`, or explicitly explain any source-backed reason a required item is not applicable. |
| F-002 | major | `## Opcodes` / Witness tables and witness facts table, line 146 | The `witness_table` row lists only children in the Operands column, but a witness table has operand 0 as the concrete conforming type; the conformance interface is carried by the result `WitnessTableType`. This omission contradicts both the source and the dependency page's structure reference. | `source/slang/slang-ir-insts.h:2241` exposes `IRWitnessTable::getConcreteType()` as `getOperand(0)`, and `source/slang/slang-ir.cpp:5142` creates a witness table with `subType` as the operand and `getWitnessTableType(baseType)` as the result type. | Change the Operands cell to include `concreteType` plus children, matching `structure.md`, and keep the explanation that entries are child instructions. |
| F-003 | minor | `## Opcodes` / Generic application table, line 95 | The `specialize` row says the AST origin is `GenericAppExpr`, but that visitor is explicitly unimplemented during lowering. The source emits `specialize` while lowering a `GenericAppDeclRef` substitution inside `emitDeclRef`. | `source/slang/slang-lower-to-ir.cpp:7386` has `visitGenericAppExpr` call `SLANG_UNIMPLEMENTED_X`, while `source/slang/slang-lower-to-ir.cpp:14738` handles `GenericAppDeclRef` and emits `emitSpecializeInst` at `source/slang/slang-lower-to-ir.cpp:14796`. | Change the AST-origin cell to reference `GenericAppDeclRef` / generic-substitution lowering in `emitDeclRef`, not `GenericAppExpr`. |
| F-004 | minor | `## Opcodes` / Runtime type information table, line 159 | The `rtti_object` row says its operands are `(variadic)`, but the C++ wrapper documents one operand: the type the RTTI object describes. | `source/slang/slang-ir-insts.h:2244` says `IRRTTIObject` has 1 operand specifying the type, and the Lua entry at `source/slang/slang-ir-insts.lua:1063` names `rtti_object` with the `RTTIObject` wrapper. | Change the Operands cell from `(variadic)` to a single type operand, for example `type`, and keep the summary focused on the RTTI record. |
| F-005 | minor | `## Opcodes` / Dispatchers and existential specialization table, lines 214-215 | The `WeakUse` and `FuncTypeOf` rows list no operands, but both are constructed with one operand in the watched source: `WeakUse` wraps the weakly referenced inst, and `FuncTypeOf` is emitted from the lowered function-as-type operand. | `source/slang/slang-ir-insts.h:3526` implements `IRBuilder::getWeakUse(IRInst* inst)` with one operand, and `source/slang/slang-lower-to-ir.cpp:2432` emits `kIROp_FuncTypeOf` with one `irType` operand. | Update the Operands cells to `inst` for `WeakUse` and `value` or `func` for `FuncTypeOf`; if the Lua schema lacks explicit names, note that these are source-usage names. |
