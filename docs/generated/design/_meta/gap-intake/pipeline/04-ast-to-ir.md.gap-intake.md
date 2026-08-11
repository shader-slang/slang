---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:44:19Z
target_doc: pipeline/04-ast-to-ir.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 10
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 2
  escalated_to_finding: 0
---

# Gap-intake report for pipeline/04-ast-to-ir.md

## Summary

Ten gaps, all from `design/pipeline/04-ast-to-ir`. Eight were confirmed in
the watched paths and fixed; two are deferred. Nothing was escalated — the
one `drift-from-source` gap turned out to be a documentation error, not a
compiler defect: the checker really does stamp
`kIROp_SynthesizedForwardDerivativeWitnessTable` on the synthesized
differentiability conformance, so the doc's parenthetical naming
`kIROp_ForwardDifferentiate` was simply wrong. The largest correction is
the one the operator flagged in advance: the `LOWER-TO-IR` dump is *not*
the raw output of the lowering walk, because `prelinkIR` and the whole
mandatory pass block run before it — that is now stated once, in
`Inputs and outputs`, and it is the answer to the prelinking gap rather
than a pointer at a later stage. The two deferred gaps both ask for
material that cannot be produced without running the compiler, which this
host cannot do (Linux x86-64 build, arm64 host). The page is now 32,729
bytes against a 32,768-byte cap; that budget is the reason every fix is
terse, and it is the main obstacle to any further work on this page.

Two fixes go slightly beyond the gap text, in both cases because the
gap's `Suggested addition` was wrong on the source. The `constexpr*`
example the gap proposed (`N / 2` reaching `visitBuiltinOperationIntVal`)
was replaced with the one that has a verified dump line, `int b[N + 1]`
→ `constexprAdd(1 : Int, %N)`, together with the correction that `+`,
`-`, `*` and unary `-` never form a `BuiltinOperationIntVal` at all
(`slang-ast-val.h:243-247` asserts it) and reach the same opcodes through
`visitPolynomialIntVal`. The `LoweredValInfo` flavor table also records
that `Subscript` has no construction site anywhere in
`slang-lower-to-ir.cpp`, which is why no Slang surface could be paired
with it.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 02d54d396116 | fixed | `source/slang/slang-check-decl.cpp:9490` and `:15113` pass `kIROp_SynthesizedForwardDerivativeWitnessTable` as the `synthesisOp` that becomes `SynthesizedModifier::op`; `source/slang/slang-lower-to-ir.cpp:11390-11394` emits `(IROp)synModifier->op`; the opcode is declared at `source/slang/slang-ir-insts.lua:2847`. Corroborated by `differentiable-conformance-is-intrinsic-witness-value.slang` (`= SynthesizedForwardDerivativeWitnessTable([[FWD]])`). Source agrees with the observation, so the doc was wrong. | corrected the conformance parenthetical to `kIROp_SynthesizedForwardDerivativeWitnessTable` and moved `kIROp_ForwardDifferentiate` onto the `SynthesizedFuncDecl` bullet |
| ed8892764aff | fixed | `source/slang/slang-lower-to-ir.cpp:15572` calls `prelinkIR`, `:15664` runs `performMandatoryEarlyInlining`, `:15770` runs DCE — all before `dumpIR(..., "LOWER-TO-IR", ...)` at `:15797`. `04b-pre-link-passes/phase-c-mandatory-early-inlining-pulls-in-core-helper.slang` pins the inlined `add(%a, %b)` under `### LOWER-TO-IR:`. | added a sentence to `Inputs and outputs` stating the prelinked body is already in the `LOWER-TO-IR` dump, normally inlined, and that the dump is not raw lowering output |
| 8abfb42eaf38 | fixed | `source/slang/slang-lower-to-ir.cpp:2009-2029` (`visitPolynomialIntVal` emits `constexprMul`/`constexprAdd`, constant term first); `source/slang/slang-ast-val.h:243-247` asserts `Add`/`Sub`/`Mul`/`Neg` never form a `BuiltinOperationIntVal` and that an all-constant fold is a `ConstantIntVal`; `source/slang/slang-ir-insts.lua:3408-3437` declares the hoistable `constexpr*` family. Verified dump line from `cross-cutting/ir-instructions/value-constexpr-add-ir.slang:49`. | added the `int b[N + 1]` / `constexprAdd(1 : Int, %N)` example and the `PolynomialIntVal` split |
| 080381fc3fa2 | fixed | `source/slang/slang-lower-to-ir.cpp:1124` (`BoundStorage` when the accessor set is more than a lone `get`, per `:1090-1116`), `:6394` (`BoundMember` for a callable member-expr), `:7739` (`ExtractedExistential` in l-value context), `:7768` (`ImplicitCastedLValue` from `LValueImplicitCastExpr`/`OutImplicitCastExpr`), `:7795` (matrix swizzle), `:7880` (vector swizzle); `LoweredValInfo::subscript` (`:405`) has no call site. Use-site insts from `bound-storage-property-read-emits-accessor-call.slang` and `swizzled-lvalue-assign-lowers-to-swizzled-store.slang`. | replaced the inline flavor list with a flavor → Slang-surface table and a note that `Subscript` is never constructed |
| e8b7f6b842fe | fixed | `source/slang/slang-lower-to-ir.cpp:6355-6364` and `:7481,7560-7567` (`IndexExpr` → `emitElementExtract`/`emitElementAddress`, mnemonics `getElement`/`getElementPtr` at `slang-ir-insts.lua:1273-1274`), `:5580-5595` (`__subscript` reaches lowering as an `InvokeExpr` routed to `lowerStorageReference`), `:7645-7654` (`AssignExpr` → `assignExpr`), `:7191-7195` with `source/slang/slang-ir.cpp:4377,4431-4440` (`emitCast` style table), `:9065-9098` with `source/slang/slang-ir.cpp:6447-6455` (`emitBreak`/`emitContinue` are `emitBranch`). Composite type spellings from `lower-optional-int.slang`, `lower-tuple-heterogeneous.slang`, `lower-parameter-block-narrow-struct.slang`. | added five mapping-table rows (`IndexExpr`, `AssignExpr`, `BuiltinCastExpr`, `BreakStmt`/`ContinueStmt`, composite generics) and pointed the element-type spellings at `ir-reference/types.md` instead of duplicating them |
| a797d351abfe | fixed | The only `addLayoutDecoration` calls in `source/slang/slang-lower-to-ir.cpp` are at `:16444`, `:16478`, `:16526`, all inside `TargetProgram::createIRModuleForLayout` (`:16353`); nothing on the `generateIRForTranslationUnit` path attaches one. | rewrote the bullet to say layout intent is not materialized at lowering and to point at `04c-layout-ir.md` |
| ec22217dbfdc | fixed | `source/standard-modules/experimental/workgraph.slang:10,17-39` declares `[ExperimentalModule] module workgraph` and the `attribute_syntax` for `NodeLaunch` / `NodeMaxDispatchGrid` / `NodeDispatchGrid` / `MaxRecords` / `NodeID`; `source/slang/slang-session.cpp:1768-1776` diagnoses `NeedToEnableExperimentFeature` unless `CompilerOptionName::ExperimentalFeature` is set (`-experimental-feature`, `source/slang/slang-options.cpp:1222`); `source/slang/slang-lower-to-ir.cpp:14485-14525` is the attribute → decoration mapping. Compile options corroborated by `entry-point-node-launch-attribute-lowers-to-string-decoration.slang`. | added the required `import experimental.workgraph` plus `-experimental-feature` / `-stage node` / `lib_6_8` to the work-graph paragraph |
| f84a0d200976 | fixed | `source/slang/slang-parser.cpp:1966-1976` parses `where countof(Pack) == <expr>` into a `GenericVariadicPackCountConstraintDecl`; the spelling is exercised by `tests/language-feature/generics/diagnose-variadic-pack-count-constraint.slang:3`. | added the surface spelling `void f<let N : int, each T>(T x) where countof(T) == N` to the section's opening sentence |
| 8fa595c02daf | deferred | The `assign` switch (`source/slang/slang-lower-to-ir.cpp:10249-10619`) handles `Ptr`, `SwizzledLValue`, `SwizzledMatrixLValue`, `BoundStorage`, `BoundMember`, `ExtractedExistential`, `ImplicitCastedLValue`; only `None` / `Simple` / `Subscript` reach the `UnsupportedAssignmentTarget` arm at `:10614`, and semantic checking rejects an r-value assignment target before lowering, so no user-reachable left-hand side is nameable from the source alone. `MaximumTypeNestingLevelExceeded` needs a type that recurses past `kMaxIRInvokeLoweringRecursionDepth` (128) in `visitInvokeExprImpl`, which is trial-and-error without a compiler. Both need a `slangc` run (unavailable: Linux x86-64 build, arm64 host) to produce a reproducer and its `E####` code; the codes themselves live in `source/slang/slang-diagnostic-defs.h`, outside this page's `watched_paths`. | — |
| eb9ddcf6e85a | deferred | `source/slang/slang-lower-to-ir.cpp:11107-11115` confirms the specialization itself, and `:10916-10938` shows `getWitnessTableBaseDeclRef` returns null only when the conformance's base type is not a `DeclRefType` — which no ordinary source-level `struct S : IFoo` produces, so the gap's "without the base specialization" half may not be reachable from Slang at all. No test in this bundle or in `ir-reference/*` pins a `witness_table_entry` line for a conformance to a generic interface, and the difference cannot be asserted without running the compiler. Needs a bundle test that dumps a generic-interface conformance before the entry's value operand can be described. | — |
