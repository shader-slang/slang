---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:01:36Z
target_doc: ir-reference/structure.md
review_report: ../../reviews/ir-reference/structure.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 1
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/structure.md

## Summary
Two findings: one rejected-bogus, one fixed. F-001 is bogus — the `generic` callout already says the body ends with `return_val` / `IRReturn`, not `yield`, matching `findGenericReturnVal` and the Lua comment. F-002 mis-cited its example rows (the four it named already carried wrappers) but flagged a real gap: filled every remaining `—` wrapper cell whose opcode has a FIDDLE-generated wrapper, per the "from `struct_name` or implicit" rule. Edits touch only the wrapper column, no watched source, so `source_commit` is unchanged.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Doc line 98 and the `### generic` callout already read "ends with `return_val` / `IRReturn`"; the word "yield" appears only as the value the generic yields, matching `source/slang/slang-ir-insts.lua:809`. `findGenericReturnVal` casts the terminator to `IRReturn` (`source/slang/slang-ir.cpp:9743`). The reviewer's premise that the callout names `yield` as the terminator is false. | — |
| F-002 | fixed | Wrapper column is mandatory and admits implicit wrappers (`_common.md` line 234: "from `struct_name` or implicit"). Reviewer's four cited rows already had wrappers; the genuinely-empty cells were `call`, `global_generic_param`, `indexedFieldKey`, `thisTypeWitness`, `TypeEqualityWitness`, `SymbolAlias`, all of which have wrappers: `IRCall` (`source/slang/slang-ir-insts.h:1697`), `IRGlobalGenericParam` (`:2266`), `IRIndexedFieldKey` (`source/slang/slang-ir.h:1765`), `IRThisTypeWitness` (`source/slang/slang-ir.h:1730`), `IRTypeEqualityWitness` (`build/source/slang/fiddle/slang-ir-insts.h.fiddle:8307`), `IRSymbolAlias` (`cast<IRSymbolAlias>` at `source/slang/slang-ir-link.cpp:1445`). | Filled wrapper cells `—` -> `IRCall` / `IRGlobalGenericParam` / `IRIndexedFieldKey` / `IRThisTypeWitness` / `IRTypeEqualityWitness` / `IRSymbolAlias` |
