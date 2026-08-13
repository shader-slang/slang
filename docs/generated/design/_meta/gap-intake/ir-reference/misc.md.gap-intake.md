---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-12T06:47:15Z
target_doc: ir-reference/misc.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 67149d1e03ebf1d4645ddd224ff4647a8ea5db53
gap_count: 7
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 3
---

# Gap-intake report for ir-reference/misc.md

## Summary

This is a re-run of the intake for this page; the three escalations
below are unchanged and are carried forward verbatim, each already
covered by an existing finding: the `getStringHash` non-literal
SIGSEGV, the `countof`-on-array fold, and `asDynamicUniform` reaching
emit. The one change is gap `352bf73e9bfd`, which the previous cycle
deferred on the false premise that no `slangc` was available: a native
arm64 `slangc` built from HEAD settled it, so the verdict moves from
`deferred` to `fixed`. The final breakdown is four `fixed`, three
`escalated-to-finding`, and no `rejected-bogus`,
`rejected-out-of-scope`, or `deferred`. The other three `fixed`
verdicts (`287d78f2e773`, `35bd0499af6c`, `03a328b5c0e3`) and their
edits stand from the previous cycle and were not revisited.

The `allocateOpaqueHandle` question turned out not to need an example
at all: the inst is emitted by AST lowering for every `RayQuery` /
`HitObject` local, but it is dead by construction and the mandatory
dead-code pass inside `generateIRForTranslationUnit` always deletes it
before the first `-dump-ir` snapshot, so no shader can be written
whose dump contains it. That is now stated in the document instead of
the example the gap asked for.

## Escalated gaps

- **`a6197cfe99b6`** — `getStringHash` on a non-literal `String`.
  The document's claim is verbatim in the source:
  `checkGetStringHashInsts`
  (`source/slang/slang-ir-string-hash.cpp:105-118`) tests
  `inst->getStringLit() == nullptr` and raises
  `Diagnostics::GetStringHashMustBeOnStringLiteral`. The guard cannot
  fire, because the Lua entry types the operand `IRStringLit`
  (`source/slang/slang-ir-insts.lua:1627-1629`) so the generated
  `getStringLit()` is an unchecked `cast<IRStringLit>` of operand 0,
  which returns a non-null mis-typed pointer for a non-literal operand;
  the process then dies before any diagnostic is printed. Existing
  finding:
  `docs/generated/tests/_meta/findings/getstringhash-nonliteral-argument-sigsegv.yaml`.
- **`4eca2b39600e`** — `countof` of a fixed-size array. The checker
  deliberately admits array operands
  (`_isTypeOrValValidForCountOf`,
  `source/slang/slang-check-expr.cpp:6329-6357`, which accepts type
  packs, tuples, arrays and value packs and rejects everything else),
  so the documented "element count of a fixed-size array" is the
  intended semantics. `visitSizeOfLikeExpr`
  (`source/slang/slang-lower-to-ir.cpp:6003-6055`) then computes the
  natural layout of the array — a `CountOfExpr` has no
  `dataLayoutType`, so the `!dataLayoutType` branch at 6007 runs — and
  returns `size.alignment` for anything that is not a `SizeOfExpr`
  (line 6053). That is why `float[1]`, `float[3]` and `float[7]` all
  report 4 and a `double` array reports 8: the fold yields the element
  type's natural alignment, not a count. Existing finding:
  `docs/generated/tests/_meta/findings/countof-on-array-returns-element-size.yaml`.
- **`6527e8b0b50a`** — `asDynamicUniform` / `TreatAsDynamicUniform`
  lifetime. `asDynamicUniform<T>` is an ungated, non-`internal`
  core-module function (`source/slang/core.meta.slang:4034-4035`), and
  the only code that erases the marker is
  `eliminateAsDynamicUniformInst`
  (`source/slang/slang-ir-uniformity.cpp:472-494`), reached only from
  `validateUniformity`, which `source/slang/slang-emit.cpp:1358-1360`
  runs only when `CompilerOptionName::ValidateUniformity` is set. An
  ordinary call therefore passes the checker, survives every pass and
  aborts each backend with an internal error rather than being stripped
  or diagnosed. Existing finding:
  `docs/generated/tests/_meta/findings/as-dynamic-uniform-reaches-emit-ice.yaml`.
  The same gap's `__getLegalizedSPIRVGlobalParamAddr` half has no
  finding of its own; the marker is consumed on the SPIR-V path
  (`source/slang/slang-ir-spirv-legalize.cpp:351`) and nothing removes
  it elsewhere. This cycle reproduced that half directly — see the
  operator notes.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 287d78f2e773 | fixed | The observation is right and the row was not: `source/slang/slang-lower-to-ir.cpp:6560-6569` (`visitEachExpr`) emits `getTupleElement(pack, index)` against the `int` parameter `visitExpandExpr` adds to the `Expand` region's block (`6571-6600`), so a value-level `each` never reaches `Each`. The three `emitEachInst` callers are all `Val`-level: `visitEachIntVal` (`2198-2204`), `visitEachType` (`2206-2211`), `visitEachSubtypeWitness` (`2316-2325`) — matching the row's existing AST-origin cell. | narrowed the `Each` row's summary and added a paragraph to the `Expand` / `Each` callout naming the three `Val`-level producers and the `getTupleElement` form a dump of an expansion body actually shows |
| a6197cfe99b6 | escalated-to-finding | `source/slang/slang-ir-string-hash.cpp:105-118` contains exactly the diagnostic the document describes, so the document is backed by the source; the unchecked `cast` behind `getStringLit()` (operand typed `IRStringLit` at `source/slang/slang-ir-insts.lua:1627-1629`) means the null test never fires and the compiler dies first. Compiler defect, not a documentation defect. Existing finding: `docs/generated/tests/_meta/findings/getstringhash-nonliteral-argument-sigsegv.yaml`. | — |
| 4eca2b39600e | escalated-to-finding | `source/slang/slang-check-expr.cpp:6329-6357` admits array operands on purpose, so the row's "element count of a fixed-size array" is what the source intends; `source/slang/slang-lower-to-ir.cpp:6007` and `6053` return `size.alignment` for a `CountOfExpr` that folds, which is the observed element-type-sized answer. Existing finding: `docs/generated/tests/_meta/findings/countof-on-array-returns-element-size.yaml`. The row's `vector` operand is separately inaccurate (`_isTypeOrValValidForCountOf` never admits a vector); left unedited under the escalation rule and noted below. | — |
| 35bd0499af6c | fixed | `source/slang/slang-check-expr.cpp:6652` fills an omitted layout with `getScalarLayoutType()`, and `source/slang/slang-lower-to-ir.cpp:6007-6013` treats a null layout and `ScalarDataLayoutType` identically, which is why the one-argument form still folds. The accepted spellings are the six `IBufferDataLayout` implementations in `source/slang/hlsl.meta.slang:28-71`, each `__intrinsic_type`d to a layout opcode in `source/slang/slang-ir-insts.lua:472-477`. Dumped tokens pinned by the bundle's `sizeof-alignof-generic.slang` (`ScalarLayout`) and `sizeof-explicit-data-layout.slang` (`Std140Layout` / `Std430Layout`). | added a paragraph under Size, alignment, count naming `ScalarDataLayout` as the default operand and listing the six layout spellings with their dumped opcode names |
| 6527e8b0b50a | escalated-to-finding | The marker's only remover is `eliminateAsDynamicUniformInst` (`source/slang/slang-ir-uniformity.cpp:472-494`), reached only from `validateUniformity`, gated on `CompilerOptionName::ValidateUniformity` at `source/slang/slang-emit.cpp:1358-1360`; `source/slang/core.meta.slang:4034-4035` shows `asDynamicUniform<T>` carries no capability or visibility gate, so a legal call reaches emit and aborts. Documenting that as the opcode's lifetime would bless the abort. Existing finding: `docs/generated/tests/_meta/findings/as-dynamic-uniform-reaches-emit-ice.yaml`. | — |
| 352bf73e9bfd | fixed | Re-run with `build-arm64/Debug/bin/slangc` (2026.14.1-80-g6122d03def). The inst is emitted and then deleted, so no example can exist. Emission: an lldb breakpoint on `Slang::IRBuilder::emitIntrinsicInst` conditioned on `$x2 == 429` (`kIROp_AllocateOpaqueHandle`) fires for both `RayQuery<0> q;` and `RayQuery<0> q = RayQuery<0>();`, twice per `RayQuery` local, never for a shader with none, with the stack `visitVarDecl` -> `assignExpr` -> `DestinationDrivenRValueExprLoweringVisitor::visitInvokeExpr` -> `visitInvokeExprImpl` -> `emitCallToDeclRef` -> `emitIntrinsicInst`, and `argCount == 1`. Deletion: following that inst pointer to `Slang::IRInst::removeAndDeallocate` gives the stack `eliminateDeadCode` -> `DeadCodeEliminationContext::processInst` -> `eliminateDeadInstsRec`, called from `generateIRForTranslationUnit`. The watched-path mechanism behind both: `source/slang/slang-lower-to-ir.cpp:4238-4251` and `4781-4785` give the non-copyable `__init` a return-destination parameter, `5714-5741` retypes the result `void` and passes the destination as the sole operand, `source/slang/slang-ir.cpp:9670` puts `kIROp_AllocateOpaqueHandle` on the no-side-effects list, and `source/slang/slang-lower-to-ir.cpp:15621-15625` runs DCE over every function ahead of the `LOWER-TO-IR` dump at `15797`. Corroborated by `slangc -dump-ir -target hlsl -stage compute -entry main`: `grep -c allocateOpaqueHandle` is 0 across all 77 dumped passes for both spellings. | replaced the "no constructor expression" summary with the operand the call site actually passes and added a paragraph under Tensor and runtime helpers explaining why the inst never reaches a dump |
| 03a328b5c0e3 | fixed | `source/slang/hlsl.meta.slang:19653-19663` gives both signatures (`__generic<T> Ref<T> __forceVarIntoStructTemporarily(inout T maybeStruct)` and the ray-payload twin); their only call sites are payload arguments — `__traceRayHLSL` at `19749`, `__hlslTraceRay` at `22802`, `__InvokeHLSL_NVAPI` at `23739`, `__InvokeHLSL_DXR` at `23815`. `source/slang/slang-ir-hlsl-legalize.cpp:139-160` shows `searchChildrenForForceVarIntoStructTemporarily` only inspects `IRCall` arguments, so a wrapper in any other position is never rewritten. | added the two signatures and the call-argument restriction to the Variable struct-wrapping legalization preamble, and named the real intrinsic on the ray-payload row's AST-origin cell |

## Operator notes

- `target_doc_source_commit_after` is this run's `HEAD`
  (`67149d1e03ebf1d4645ddd224ff4647a8ea5db53`); all line citations are
  against the working tree at intake time. The `_before` value is the
  `source_commit` still recorded in the page's front-matter.
- The previous cycle's block on `352bf73e9bfd` — "this host cannot run
  `slangc`" — was wrong. A native macOS-arm64 build exists at
  `build-arm64/Debug/bin/slangc`, and it is what settled the gap.
  `slang-test` does not build in this tree (`external/imgui` is
  incomplete), so `slangc` was invoked directly; `-target spirv` also
  needs a downstream `spirv-opt` that is unavailable, so `-target
  hlsl` was used for the dumps.
- Confirmed with the compiler, outside the queue: `countof(float4)` is
  rejected with `error[E30083]: invalid countof argument` ("argument
  to countof can only be a type pack or tuple"), so the `countOf`
  row's `vector` operand is wrong at HEAD. It is still left unedited
  under gap `4eca2b39600e`'s escalation rule; the row should be
  re-checked in the same pass that fixes the array fold.
- Also confirmed with the compiler, outside the queue: a plain call to
  `__getLegalizedSPIRVGlobalParamAddr` (declared without `internal` or
  any capability gate at `source/slang/core.meta.slang:4037-4039`)
  passes the checker and aborts every non-SPIR-V backend —
  `internal error[E99999]: ... unexpected IR opcode during code emit`
  on `-target hlsl` and `-target glsl`, and
  `error[E99997]: ... Unknown addressspace encountered.` on `-target
  metal`. **No finding covers this**; one needs to be opened on the
  tests side. No id is asserted here.
- Files this cycle's reasoning depends on that are **not** in
  `watched_paths` for this page:
  `source/slang/slang-check-expr.cpp` is watched, but
  `source/slang/slang-check-decl.cpp` (which decides whether a local
  gets a synthesized default-constructor call) and
  `source/slang/slang-ir-dce.cpp` (`shouldInstBeLiveIfParentIsLive`,
  which consults `mightHaveSideEffects`) are not. Neither claim in the
  document rests on them alone: the emission and the deletion were
  both observed under a debugger, and the mechanism is cited to
  watched lines in `slang-lower-to-ir.cpp` and `slang-ir.cpp`.
- `source/slang/slang-emit-c-like.cpp:3290` and
  `source/slang/slang-emit-spirv.cpp:2995` (both unwatched) make
  `allocateOpaqueHandle` a no-op at emit, which is why deleting it
  early is harmless rather than a defect. That reasoning is recorded
  here and deliberately kept out of the document.
