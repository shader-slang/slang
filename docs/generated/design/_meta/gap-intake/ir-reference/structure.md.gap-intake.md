---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: ir-reference/structure.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 7
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/structure.md

## Summary

All seven gaps were recorded as fixed and then reopened because a regenerated bundle still reports them. Each was re-decided against the current source, and each is `fixed`: 7 fixed, 0 rejected-bogus, 0 rejected-out-of-scope, 0 deferred, 0 escalated. None is a compiler defect. The sections the gaps anchor to already carried the requested text, but every citation under them had drifted from the source (the page was generated at 48c746dc1), so the same refresh re-derived the cited lines against 2518a942d and corrected them. The reopening is therefore most plausibly the bundle having been regenerated against older section text; the page now states each behaviour with citations that match the current files. The `.slang` bundle was not re-run, and `slangc` (a Linux binary in this tree) cannot run here, so the printed dump excerpts are carried forward from the earlier intake's real `-dump-ir` runs and the checked-in bundle tests, and are not re-observed.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| d75271bc7d3f | fixed | Section `builtinRequirementKey` shows the printed key and decoration pair and the named `StructKey` contrast. Source re-confirmed: `source/slang/slang-lower-to-ir.cpp:1748-1862` (`getInterfaceRequirementKey` creates the hoistable key and adds `IRBuiltinRequirementDecoration` at 1832-1842; `addNameHint` only on the `StructKey` path, line 1855); `source/slang/slang-ir-insts.h:3651` passes `nullptr` type; `source/slang/slang-ir.cpp:7970-7975` prints `_` for a null type; `scrubName` at `slang-ir.cpp:7752-7821`; kind 1 is `DifferentialType` (`slang-ast-support-types.h:2056-2060`). New since the first intake: the role is also derived for the conformance requirement of a built-in associated type (`slang-lower-to-ir.cpp:1778-1812`). | re-anchored the cited lines and added the derived-role origin to the `builtinRequirementKey` row |
| b9e17ab8c4e8 | fixed | Section `func` states the `Func(Int, Int, Int)` dump form, the result-first operand order and the trailing-attr caveat. Re-confirmed in `source/slang/slang-ir.h:1626-1640` (operand layout comment of `IRFuncType`) and `slang-ir.cpp:804` (`getFirstParam` forwards to the first block). | re-anchored `getFirstParam` to line 804; text otherwise unchanged |
| d984deecc3c7 | fixed | Section Global state shows both printed shapes. Re-confirmed in `slang-lower-to-ir.cpp:11622-11628` (module-scope initializer goes into a child block through `beginGlobalVarInitializer`, defined at 11560) and `11713-11799` (`lowerFunctionStaticVarDecl`: bodyless variable, second `Ptr(Bool)` `global_var` returning `false` at 11771-11774, `emitIfElse` at 11786). The guard condition is `if (auto initExpr = decl->initExpr)` at line 11756, not a constant-ness test. A third `global_var` producer now exists, `visitUniformParameterShadowVarDecl` (line 11646), and was added to the row. | updated all line citations (guard test now 11756, function range 11713-11799) and added the uniform-shadow producer to the `global_var` row |
| 27bdc3599a05 | fixed | Section Global state names the `type_param T : IFoo;` spelling and the two-inst lowering. Re-confirmed in `slang-lower-to-ir.cpp:10584-10590` (`visitGenericTypeConstraintDecl` emits `emitGlobalGenericWitnessTableParam(supType)` for a constraint parented by a `GlobalGenericParamDecl`), `slang-ir-insts.h:4391-4398` (type param typed `Type`, witness param typed `WitnessTableType`), and `slang-lower-to-ir.cpp:10658` (the parameter itself). The page previously cited two witness-emitting visitors; only this one remains. The other constraint kinds (10611, 10626, 10649) were added. | rewrote the citation to the single current visitor, added the other constraint kinds, corrected the AST class citations to `slang-ast-decl.h:626,634` |
| 52d3221d1b09 | fixed | Section `module` notes that `-dump-ir` never prints the `module` inst. Re-confirmed in `source/slang/slang-ir.cpp:8486-8491`: `dumpIRModule` iterates `getGlobalInsts()` and dumps each child with no line for the module inst. Module creation is `IRModule::create` at `slang-ir.cpp:5092`. The page's version-range sentence was also stale (range is now 37..37, `slang-ir.h:2254-2255`) and was rewritten from the comment at `slang-ir.h:2244-2253`. | corrected `dumpIRModule` to line 8486 and replaced the outdated module-version claim |
| cb92e1d6601c | fixed | Section `SymbolAlias` shows both alias insts with their `Type` / `witness_table_t(...)` result types and states that they print unnamed. Re-confirmed in `slang-lower-to-ir.cpp:12514-12545` (`visitAggTypeDecl` wraps the aliased type via `emitSymbolAlias`, then `ensureDecl`s each nested `InheritanceDecl`) and `10996-11018` (`visitInheritanceDecl` emits the witness alias and adds an import or export decoration). `emitSymbolAlias` creates no name hint (`slang-ir.cpp:3385`). New since the first intake: a generic alias decl takes the `finishOuterGenerics` path and creates no `SymbolAlias`; this was added. The gap's claim that the alias name survives only in an `[export(...)]` decoration is still not documented as a name-hint statement; the decoration is mangled-name based and I did not re-observe its printed form. | added the generic-alias caveat to the `SymbolAlias` section |
| cacdc30cbae7 | fixed | Section `witness_table_entry` vs `interface_req_entry` states that a requirement with a default implementation contributes one entry and that the synthesized `InterfaceDefaultImplDecl` is what is skipped. Re-confirmed in `slang-lower-to-ir.cpp:12162-12183` (operand-count loop with the skip at 12166) and `shouldDeclBeTreatedAsInterfaceRequirement` at 1705-1744 (default-impl stub is not a requirement). The entry-building loop skips it again at 12358. | corrected the cited line to 12166; wording unchanged |
