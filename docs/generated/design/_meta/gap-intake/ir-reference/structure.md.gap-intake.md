---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-12T06:41:25Z
target_doc: ir-reference/structure.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 67149d1e03ebf1d4645ddd224ff4647a8ea5db53
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

This is a re-run of the gap-intake stage for this page. The previous pass
deferred `d75271bc7d3f` and `d984deecc3c7` on the belief that no `slangc` was
runnable in this tree; that was wrong — a native macOS-arm64 build sits at
`build-arm64/Debug/bin/slangc`. Both gaps were revisited, settled against real
`-dump-ir` output, and are now `fixed`, so the queue is 7 fixed, 0
rejected-bogus, 0 rejected-out-of-scope, 0 deferred, 0 escalated. The five gaps
fixed in the first pass are carried forward unchanged, with their original
Evidence verbatim; nothing was escalated, and none of the seven was a compiler
defect.

Two things the dump showed that the gap rows did not. First, the run-once
`Ptr(Bool)` guard for a function-`static` is not conditional on the initializer
being a *runtime* value, as `d984deecc3c7` assumed — lowering emits it whenever
`initExpr` is present at all, and the committed test's own `static int c = 0;`
produces one. Second, the guard is not the only surprise in that shape: the
variable's own `global_var` comes out *bodyless*, because the initializer is
lowered into the enclosing function under an `ifElse` rather than into a child
block, which contradicts the unqualified "an initializer lives in child blocks"
in the `global_var` row. The row was qualified accordingly.

## Actions

| Gap ID       | Action | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                | Fix summary                                                                                              |
| ------------ | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| b9e17ab8c4e8 | fixed  | Confirmed in watched `source/slang/slang-ir.h:1626-1640`, which documents the operand layout directly: `[0]` result type, `[1..paramEnd)` parameter types, then an optional trailing `IRAttr` that `getParamCount()` / `getParamType()` skip.                                                                                                                                                                                                                                           | added the printed `Func(Int, Int, Int)` form, the operand order, and the trailing-attr caveat to §`func` |
| 27bdc3599a05 | fixed  | Two halves, both confirmed. The two-inst lowering is in watched `slang-lower-to-ir.cpp:10854-10858` and `10877-10881`, where a constraint decl parented by a `GlobalGenericParamDecl` emits its own `emitGlobalGenericParam(witnessType)`. The `type_param T : IFoo;` spelling and both printed result types are pinned by the bundle's own passing test `global-generic-param-type-param.slang`.                                                                                       | added the spelling to the `global_generic_param` row and a paragraph on the two insts                    |
| 52d3221d1b09 | fixed  | Confirmed in watched `source/slang/slang-ir.cpp:8401-8407`: `dumpIRModule` iterates `getGlobalInsts()` and dumps each child, emitting no line for the `module` inst itself.                                                                                                                                                                                                                                                                                                             | added a note to §`module` that the container is observable only through its children                     |
| cacdc30cbae7 | fixed  | Confirmed in watched `slang-lower-to-ir.cpp:12101`: the skip is inside the loop computing the interface's operand count, and it skips the `InterfaceDefaultImplDecl` — the synthesized carrier of the default body — not the requirement, which still contributes its one entry. The gap's reading of the old wording was correct.                                                                                                                                                      | reworded the requirement-list paragraph and anchored it to the counting loop                             |
| cb92e1d6601c | fixed  | Printed forms pinned by the bundle's own passing test `symbol-alias-from-export-type-alias.slang`, whose CHECK-DAGs require one `: Type = SymbolAlias(%FooImpl)` and one `: witness_table_t(%IFoo) = SymbolAlias(...)`, both matched by numeric id rather than by name. I documented the result-type distinction and the absent name hint; I did not document the gap's further claim that the source name survives in an `[export(...)]` decoration, which nothing available confirms. | added a dump excerpt and a sentence to §`SymbolAlias`                                                    |
| d75271bc7d3f | fixed  | Re-run; previously deferred. Observed by running `SLANG_ASSERT=release-assert-only build-arm64/Debug/bin/slangc -target spirv-asm -dump-ir -o /dev/null -entry main -stage compute` on the bundle's `builtin-requirement-key-in-witness-table.slang`; the `LOWER-TO-IR` dump prints `[BuiltinRequirementDecoration(1 : Int)]` immediately above `let %201 : _ = builtinRequirementKey(1 : Int)`. The `_` is a null result type — watched `slang-ir-insts.h:3609` passes `nullptr` to `emitIntrinsicInst`, and watched `slang-ir.cpp:7889` prints `_` for a null type. The absent name hint is watched `slang-lower-to-ir.cpp:1826`: `addNameHint` is on the `StructKey` branch only. Kind `1` is `DifferentialType`: the same dump has `interface_req_entry(%201, associated_type)` and `witness_table_entry(%201, %Grad)`, and `builtinRequirementKey(27 : Int)` next to it keys `witness_table_t(%IDifferentiable)`, matching `BuiltinRequirementKind` order in `source/slang/slang-ast-support-types.h:1822`. Contrast form from the same command on `interface-with-requirement.slang`; escaping is watched `slang-ir.cpp:7688-7717`. | added the printed key/decoration pair, the named `StructKey` contrast, and the `_`-type note to §`builtinRequirementKey` |
| d984deecc3c7 | fixed  | Re-run; previously deferred. Both halves observed with the same `slangc -dump-ir` command on a two-case file (`static int g = 3;` at module scope plus `static int c = u;` inside a function). The `LOWER-TO-IR` dump shows `global_var %g : Ptr(Int) { block %2: return_val(3 : Int) }` — SCCP does not fold it away — and, for the function-`static`, a bodyless `global_var %c : Ptr(Int);` beside an unnamed `global_var %3 : Ptr(Bool) { block %4: return_val(false) }`, with the function body doing `load(%3)` / `ifElse` / `store(%c, %u)` / `store(%3, true)`. Confirmed in watched `slang-lower-to-ir.cpp:11817-11826` (module-scope initializer block) and `11885-11924` (guard global plus in-function `ifElse`). Line 11885 tests only `decl->initExpr`, so the guard is not conditional on a runtime initializer; re-running the command on the bundle's own `global-var-function-static.slang` (`static int c = 0;`) emits the guard too. | added both printed shapes to §Global state and qualified the `global_var` row's initializer claim |
