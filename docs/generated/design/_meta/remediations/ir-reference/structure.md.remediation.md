---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:20:00Z
target_doc: ir-reference/structure.md
review_report: ../../reviews/ir-reference/structure.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/structure.md

## Summary

All five minor findings were verified against the source at the
recorded commit and fixed; none was rejected, deferred, or escalated.
Four fixes are single-cell corrections in the opcode tables and the
witness-table prose; the fifth merges the audience sentence into the
opening paragraph as the common contract requires. The nine
generic/existential rows this page owns were left untouched.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed. `source/slang/slang-lower-to-ir.cpp:11927-11938` shows `visitVarDecl` sending only true globals to `lowerGlobalVarDecl` and function-statics to `lowerFunctionStaticVarDecl`, which at `:11825-11834` delegates a `const` to `lowerFunctionStaticConstVarDecl`. | `### Global state`: `global_var` origin now cites `lowerFunctionStaticVarDecl` for mutable function-statics; `globalConstant` origin gained function-`static` `const` via `lowerFunctionStaticConstVarDecl`. |
| F-002 | fixed | Confirmed. `source/slang/slang-ast-decl.h:575` declares `GlobalGenericParamDecl : public AggTypeDecl`, but line 583 declares `GlobalGenericValueParamDecl : public VarDeclBase`. | `global_generic_param` row: split the shared derivation claim into the two correct base classes. |
| F-003 | fixed | Confirmed. `docs/generated/design/_meta/prompts/ir-reference-structure.md:38` requires lambda lowering in this cell, and `source/slang/slang-check-expr.cpp:7933-7939` creates the lambda's `FuncDecl` and stores it on the lambda struct. | `func` row: added the synthesized `FuncDecl` that checking stores on a lambda to the AST-origin cell. |
| F-004 | fixed | Confirmed. `source/slang/slang-lower-to-ir.cpp:12086-12108` counts one entry per `AccessorDecl` of a property or subscript and `continue`s past `InterfaceDefaultImplDecl`, so "one entry per direct interface member" overcounts and undercounts. | `witness_table_entry` / `interface_req_entry` prose: reworded to one entry per requirement-bearing member, with accessors contributing their own entries and default-impl decls skipped. |
| F-005 | fixed | Confirmed. `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first body paragraph to state both coverage and intended reader; the page split them across two paragraphs. | Introduction: merged the audience sentence into the opening paragraph. |
