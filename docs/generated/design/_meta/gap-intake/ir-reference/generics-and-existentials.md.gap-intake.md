---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:28:34Z
target_doc: ir-reference/generics-and-existentials.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 11
actions:
  fixed: 11
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/generics-and-existentials.md

## Summary

No gap was escalated: every observation in the queue was confirmed in
the document's watched paths, and none contradicted what the source
says. All eleven gaps were `fixed` with six edits, because several
gaps are the same underlying rule seen from different opcodes: the
`type_param` surface and its `E38207` diagnostic became one paragraph
under Generic application; the two conformance-registration gaps
(`createExistentialObject` and the interface-typed entry-point
parameter) became one paragraph under Existential construction; and
the three witness-table gaps became one block plus a clause on the
existing generic-requirement rule. Two gaps also asked for compiler
*fixes* rather than documentation — the repeated / unlocated `E50100`
on non-CUDA targets and the repeated `E50101` — and that half was
deliberately **not** written into the document; see the flags below.

Two things the operator should act on separately. First, two suspected
diagnostic-quality defects, both confirmed as mechanisms in the
watched source but not reproducible here (no runnable `slangc` on this
host): `seedGlobalScope`
(`source/slang/slang-ir-typeflow-specialize.cpp:1554`) emits `E50100`
per interface-typed global param with **no** dedup set, while its
sibling paths keep one (`diagnosedEntryPointInterfaceParams:8272`,
`diagnosedNoTypeConformancesInterfaces:8288`), and the location it
passes is the hoisted global param's `sourceLoc` rather than a use
site — which matches the reported "no location, repeated 2-3x on
SPIR-V/HLSL/GLSL/Metal/WGSL/CPP, single located span on CUDA", since
CUDA skips `moveEntryPointUniformParamsToGlobalScope` and instead
reaches the dedup'd `diagnoseEntryPointInterfaceParamIfNeeded:3522`.
Likewise `DynamicDispatchOnPotentiallyUninitializedExistential:3592`
has no dedup set while the same class runs to a fixpoint, which is why
`E50101` repeats. Second, three citations needed source outside
`watched_paths`: `source/slang/slang-options.cpp` (the
`-conformance <typeName>:<interfaceName>[=<sequentialID>]` spelling,
line 736), `source/slang/slang-diagnostics.lua` (the `E50101` number,
line 5402) and `source/slang/slang-parser.cpp` (`type_param` at 10726,
`where optional` at 1947). Each was additionally corroborated by a
bundle test, but the manifest should probably gain
`slang-diagnostics.lua` at least.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| b8b6a66f3770 | fixed | `kIROp_FuncTypeOf` has exactly one construction site in `source/`: `lowerFuncDependentType`, `source/slang/slang-lower-to-ir.cpp:2498` (emit at 2504), reached from `visitFwdDiffFuncType` (2521); the type-flow pass constructs it nowhere. Surface confirmed by `docs/generated/tests/design/ir-reference/generics-and-existentials/func-type-of-fwd-diff.slang`. | added a note after the Dispatchers table stating `FuncTypeOf` is the one row produced at lowering rather than by the type-flow pass, naming `fwd_diff` on a `[Differentiable]` function as the surface; refreshed the row's line citation 2492 -> 2498 |
| 7ddc83b2a017 | fixed | `collectExistentialTables` (`source/slang/slang-ir-typeflow-specialize.cpp:8173`) collects only global witness tables surviving the link; the entry-point-parameter site is `diagnoseEntryPointInterfaceParamIfNeeded:3541`, the flag is named at 1567/1580 and the code `50100` at 1580. | generalized with 99b58df73eed into one paragraph under Existential construction stating the linkage-conformance requirement and `E50100`, naming the entry-point-parameter site; the per-target no-location / repeated emission was deliberately not documented and is flagged in the Summary as a suspected defect |
| 8d47528eb4a0 | fixed | `analyzeExtractExistentialWitnessTable` (`source/slang/slang-ir-typeflow-specialize.cpp:3554`) reports `DynamicDispatchOnPotentiallyUninitializedExistential` at 3592 and returns `none()` when the witness-table set has an uninitialized element; code from `source/slang/slang-diagnostics.lua:5402` and `docs/generated/tests/coverage/specialize/uninitialized-existential-dispatch-diag.slang`. | added a paragraph after the set-elements note stating that an interface local must be assigned on every path reaching a dynamic dispatch, citing the diagnostic; the repeat was not documented and is flagged in the Summary |
| 99b58df73eed | fixed | `source/slang/slang-ir-typeflow-specialize.cpp:8173` (`collectExistentialTables`), diagnostic sites 1583 / 3506 / 3541, flag named in the comments at 1567 and 1580; spelling `-conformance <typeName>:<interfaceName>[=<sequentialID>]` at `source/slang/slang-options.cpp:736`; `docs/generated/tests/design/ir-reference/generics-and-existentials/create-existential-object-dynamic-object.slang` compiles with `-conformance A:IFoo=0`. | one generalized paragraph under Existential construction (shared with 7ddc83b2a017) covering the runtime-tagged form, the `-conformance` option and `E50100` |
| 7c93f3557071 | fixed | `specializeGlobalGenericParameters` (`source/slang/slang-ir-specialize.cpp:3273`) names the surface and the two-parameter shape in its diagnose loop, 3337-3339 and 3360-3362 ("a conjunction `type_param T : IFoo & IBar`"); `source/slang/slang-parser.cpp:10726` registers `type_param`; `global-generic-param.slang` and `global-generic-param-with-constraint.slang` pin both dumped forms. | generalized with 2df1d0ed5c23 into one paragraph after the Generic application table giving the `type_param T;` / `type_param T : IFoo;` spellings and the one-witness-parameter-per-constraint rule |
| 2df1d0ed5c23 | fixed | `Diagnostics::UnspecializedGlobalGenericParamWithUses` emitted at `source/slang/slang-ir-specialize.cpp:3350` (and 3374 for the witness-only case) for any `global_generic_param` retaining a use after specialization; the code `E38207` is named in the comment at 3417 and the message text is at `source/slang/slang-diagnostics.lua:4622-4623`. | second half of the same Generic application paragraph: an unbound module-scope parameter used in code is reported as `E38207`, and a `bind_global_generic_param` must supply a value first |
| c7fd9b8d332e | fixed | `visitIsTypeExpr` (`source/slang/slang-lower-to-ir.cpp:7426`) emits `GetSequentialID` only under `declWitness->isOptional()` (7443, compared against `-1` as `UInt`); the ordinary-existential arm emits `IsType` at 7452. `where optional` parsed at `source/slang/slang-parser.cpp:1947`; `get-sequential-id-optional-constraint.slang` pins the sentinel `4294967295 : UInt`. | rewrote the `GetSequentialID` callout to name the two-part surface (`where optional T : IFoo` plus `T is IFoo`) and to state that a test on an ordinary existential emits `IsType` instead |
| 0cd2b3d9e8fc | fixed | `canDeclLowerToAGeneric` (`source/slang/slang-lower-to-ir.cpp:14886`) returns true for an `InheritanceDecl`, so a conformance under a generic lowers to a generic witness table; `specialize-generic-witness-table.slang` pins `generic %N : witness_table_t(%IFoo)` and the nested `call specialize(%N, specialize(%N, Int), specialize(%N, Int))`. | added a worked witness-table case to the `specialize` callout (`struct Box<T> : IFoo` plus a constrained helper) showing the nested `specialize` in the outer call's argument list; kept as prose, since the family contract forbids code blocks under `## Notable opcodes` |
| 297674261ea5 | fixed | `shouldDeclBeTreatedAsInterfaceRequirement` (`source/slang/slang-lower-to-ir.cpp:1676`) returns false for a `PropertyDecl` (comment at 1711-1714); the interface-lowering loop at 12275 descends into the accessors and keys one entry each (12281). Printed key spelling from `scrubName` (`source/slang/slang-ir.cpp:7667`, `.` -> `_` -> `x5F`) and `interface-req-entry-per-accessor.slang`. | added a minimal `interface IHasProp { property int val { get; set; } int plain(int x); }` example and the resulting three `interface_req_entry` keys after the witness-facts table |
| f2e77cef25b9 | fixed | `emitDeclRef` lowers a `GenericAppDeclRef`'s base with `IRBuilder::getGenericKind()` (`source/slang/slang-lower-to-ir.cpp:15067`) and passes that type straight into `emitLookupInterfaceMethodInst` at 15170; `Generic` is the Lua mnemonic for `GenericKind` (`source/slang/slang-ir-insts.lua:331`); `lookup-witness-generic-requirement-specialize.slang` pins `let %g : Generic = lookupWitness(...)`. | extended the "a requirement can itself be generic" rule with the lookup's `Generic` result type and the fact that only the `specialize` wrapped around it is callable; refreshed lines 11018 -> 11030 and 14860 -> 14886 in the same paragraph |
| 66bcb2cd5432 | fixed | Requirement keys get their name from `addNameHint` (`source/slang/slang-lower-to-ir.cpp:1657`), which adds nothing when `getNameForNameHint` (1548) returns empty for a nameless decl (1561-1564); the synthesized bound constraint decl has no name, so its key prints bare, as pinned by `interface-req-entry-assoc-type-bound-sibling.slang` (`interface_req_entry(%{{[0-9]+}}, witness_table_t(%IBar))` next to `%IHasAssocx5FA`). | added a paragraph noting the bound's sibling entry is keyed by an unnamed decl and prints as a bare `%N`, so readers cannot match it by name |
