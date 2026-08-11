---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:20:00Z
target_doc: ir-reference/structure.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 7
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 2
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/structure.md

## Summary

Seven gaps, all reported by `design/ir-reference/structure`. Five were confirmed
and fixed: the `Func(...)` operand order, the `type_param` surface and its
two-inst lowering, the fact that `-dump-ir` never prints the `module` inst, the
misleading default-implementation wording, and the two `SymbolAlias` insts told
apart by result type. Two are deferred for the same reason: they ask for printed
IR forms that no watched source states and no test in the bundle pins, and the
only build in the tree is Linux x86-64 on an arm64 host, so I could not run the
compiler to observe them. Nothing was escalated; none of the seven was a
compiler defect.

Two of the fixes are broader than the gap asked for, because the source said
more than the gap did. `Func(...)` can carry a trailing `IRAttr` that is neither
result nor parameter, which a reader counting operands would misread — the gap
did not mention it. And the requirement-entry rewording is anchored to the
counting loop that actually does the skipping, rather than left as prose.

## Actions

| Gap ID       | Action   | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                | Fix summary                                                                                              |
| ------------ | -------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| b9e17ab8c4e8 | fixed    | Confirmed in watched `source/slang/slang-ir.h:1626-1640`, which documents the operand layout directly: `[0]` result type, `[1..paramEnd)` parameter types, then an optional trailing `IRAttr` that `getParamCount()` / `getParamType()` skip.                                                                                                                                                                                                                                           | added the printed `Func(Int, Int, Int)` form, the operand order, and the trailing-attr caveat to §`func` |
| 27bdc3599a05 | fixed    | Two halves, both confirmed. The two-inst lowering is in watched `slang-lower-to-ir.cpp:10854-10858` and `10877-10881`, where a constraint decl parented by a `GlobalGenericParamDecl` emits its own `emitGlobalGenericParam(witnessType)`. The `type_param T : IFoo;` spelling and both printed result types are pinned by the bundle's own passing test `global-generic-param-type-param.slang`.                                                                                       | added the spelling to the `global_generic_param` row and a paragraph on the two insts                    |
| 52d3221d1b09 | fixed    | Confirmed in watched `source/slang/slang-ir.cpp:8401-8407`: `dumpIRModule` iterates `getGlobalInsts()` and dumps each child, emitting no line for the `module` inst itself.                                                                                                                                                                                                                                                                                                             | added a note to §`module` that the container is observable only through its children                     |
| cacdc30cbae7 | fixed    | Confirmed in watched `slang-lower-to-ir.cpp:12101`: the skip is inside the loop computing the interface's operand count, and it skips the `InterfaceDefaultImplDecl` — the synthesized carrier of the default body — not the requirement, which still contributes its one entry. The gap's reading of the old wording was correct.                                                                                                                                                      | reworded the requirement-list paragraph and anchored it to the counting loop                             |
| cb92e1d6601c | fixed    | Printed forms pinned by the bundle's own passing test `symbol-alias-from-export-type-alias.slang`, whose CHECK-DAGs require one `: Type = SymbolAlias(%FooImpl)` and one `: witness_table_t(%IFoo) = SymbolAlias(...)`, both matched by numeric id rather than by name. I documented the result-type distinction and the absent name hint; I did not document the gap's further claim that the source name survives in an `[export(...)]` decoration, which nothing available confirms. | added a dump excerpt and a sentence to §`SymbolAlias`                                                    |
| d75271bc7d3f | deferred | Cannot confirm the printed form. The gap asks for a dump excerpt of a `builtinRequirementKey` definition together with its decoration; no watched source states the rendering, and the bundle's tests pin only the fragment `= builtinRequirementKey(`. Observing it needs a runnable compiler, and the tree's build is Linux x86-64 on an arm64 host.                                                                                                                                  | —                                                                                                        |
| d984deecc3c7 | deferred | Cannot confirm either half. No watched source shows a `Ptr(Bool)` run-once guard global for a function-`static` with a runtime initializer, and the bundle's `global-var-function-static.slang` shows only a bare `global_var %c : Ptr(Int);` with no initializer block and no guard — which neither confirms nor refutes the gap, since that test uses a constant initializer. Documenting either shape would assert an unverified claim about emitted IR.                             | —                                                                                                        |
