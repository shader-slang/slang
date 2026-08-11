---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:44:02Z
target_doc: ir-reference/misc.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 7
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 3
---

# Gap-intake report for ir-reference/misc.md

## Summary

Three gaps were escalated as compiler defects, each already covered by
an existing finding: the `getStringHash` non-literal SIGSEGV, the
`countof`-on-array fold, and `asDynamicUniform` reaching emit. All
three are `drift-from-source` or `undocumented-behavior` rows where the
watched source states the intent the document already records and the
compiler does something else, so the document was left alone for them.
Three gaps were fixed with edits confirmed against watched paths — the
`Each` producers versus the `getTupleElement` a value-level `each`
actually lowers to, the default and accepted spellings of the
`sizeOf` / `alignOf` `dataLayout` operand, and the two
`__forceVarIntoStructTemporarily` signatures plus the call-argument
position the HLSL legalizer requires. One gap was deferred: the
`allocateOpaqueHandle` example cannot be produced or verified without
running `slangc`, which this host cannot do (the checked-out build is
Linux x86-64, the host is arm64).

Two of the three escalations were reported as documentation problems
and are recorded here as defects instead, which is the point of the
escalation channel: writing "`countof` of an array yields the element
alignment" or "`TreatAsDynamicUniform` reaches emit and aborts every
backend" into the reference would have blessed both bugs.

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
  it elsewhere.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 287d78f2e773 | fixed | The observation is right and the row was not: `source/slang/slang-lower-to-ir.cpp:6560-6569` (`visitEachExpr`) emits `getTupleElement(pack, index)` against the `int` parameter `visitExpandExpr` adds to the `Expand` region's block (`6571-6600`), so a value-level `each` never reaches `Each`. The three `emitEachInst` callers are all `Val`-level: `visitEachIntVal` (`2198-2204`), `visitEachType` (`2206-2211`), `visitEachSubtypeWitness` (`2316-2325`) — matching the row's existing AST-origin cell. | narrowed the `Each` row's summary and added a paragraph to the `Expand` / `Each` callout naming the three `Val`-level producers and the `getTupleElement` form a dump of an expansion body actually shows |
| a6197cfe99b6 | escalated-to-finding | `source/slang/slang-ir-string-hash.cpp:105-118` contains exactly the diagnostic the document describes, so the document is backed by the source; the unchecked `cast` behind `getStringLit()` (operand typed `IRStringLit` at `source/slang/slang-ir-insts.lua:1627-1629`) means the null test never fires and the compiler dies first. Compiler defect, not a documentation defect. Existing finding: `docs/generated/tests/_meta/findings/getstringhash-nonliteral-argument-sigsegv.yaml`. | — |
| 4eca2b39600e | escalated-to-finding | `source/slang/slang-check-expr.cpp:6329-6357` admits array operands on purpose, so the row's "element count of a fixed-size array" is what the source intends; `source/slang/slang-lower-to-ir.cpp:6007` and `6053` return `size.alignment` for a `CountOfExpr` that folds, which is the observed element-type-sized answer. Existing finding: `docs/generated/tests/_meta/findings/countof-on-array-returns-element-size.yaml`. The row's `vector` operand is separately inaccurate (`_isTypeOrValValidForCountOf` never admits a vector); left unedited under the escalation rule and noted below. | — |
| 35bd0499af6c | fixed | `source/slang/slang-check-expr.cpp:6652` fills an omitted layout with `getScalarLayoutType()`, and `source/slang/slang-lower-to-ir.cpp:6007-6013` treats a null layout and `ScalarDataLayoutType` identically, which is why the one-argument form still folds. The accepted spellings are the six `IBufferDataLayout` implementations in `source/slang/hlsl.meta.slang:28-71`, each `__intrinsic_type`d to a layout opcode in `source/slang/slang-ir-insts.lua:472-477`. Dumped tokens pinned by the bundle's `sizeof-alignof-generic.slang` (`ScalarLayout`) and `sizeof-explicit-data-layout.slang` (`Std140Layout` / `Std430Layout`). | added a paragraph under Size, alignment, count naming `ScalarDataLayout` as the default operand and listing the six layout spellings with their dumped opcode names |
| 6527e8b0b50a | escalated-to-finding | The marker's only remover is `eliminateAsDynamicUniformInst` (`source/slang/slang-ir-uniformity.cpp:472-494`), reached only from `validateUniformity`, gated on `CompilerOptionName::ValidateUniformity` at `source/slang/slang-emit.cpp:1358-1360`; `source/slang/core.meta.slang:4034-4035` shows `asDynamicUniform<T>` carries no capability or visibility gate, so a legal call reaches emit and aborts. Documenting that as the opcode's lifetime would bless the abort. Existing finding: `docs/generated/tests/_meta/findings/as-dynamic-uniform-reaches-emit-ice.yaml`. | — |
| 352bf73e9bfd | deferred | The row's AST origin is correct — `RayQuery::__init()` (`source/slang/hlsl.meta.slang:21207-21209`) and `HitObject::__init()` (`22768-22769`) both carry `__intrinsic_op($(kIROp_AllocateOpaqueHandle))` — so nothing in the document is wrong; what the gap asks for is a shader whose dump contains the inst. Which surface form reaches the `__init` is decided in the unwatched checker, and confirming a candidate requires running `slangc`, which this host cannot do (the tree's build is Linux x86-64, the host is arm64). Needs a run on a Linux builder, or a `watched_paths` expansion covering the checker's default-construct path. | — |
| 03a328b5c0e3 | fixed | `source/slang/hlsl.meta.slang:19653-19663` gives both signatures (`__generic<T> Ref<T> __forceVarIntoStructTemporarily(inout T maybeStruct)` and the ray-payload twin); their only call sites are payload arguments — `__traceRayHLSL` at `19749`, `__hlslTraceRay` at `22802`, `__InvokeHLSL_NVAPI` at `23739`, `__InvokeHLSL_DXR` at `23815`. `source/slang/slang-ir-hlsl-legalize.cpp:139-160` shows `searchChildrenForForceVarIntoStructTemporarily` only inspects `IRCall` arguments, so a wrapper in any other position is never rewritten. | added the two signatures and the call-argument restriction to the Variable struct-wrapping legalization preamble, and named the real intrinsic on the ray-payload row's AST-origin cell |

## Operator notes

- `target_doc_source_commit_after` is the SHA supplied with the task
  (`ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e`); all line citations are
  against the working tree at intake time.
- Files this cycle's reasoning depends on that are **not** in
  `watched_paths` for this page:
  `source/slang/slang-check-expr.cpp` (the `sizeof` default layout and
  the `countof` operand admissibility),
  `source/slang/slang-ir-string-hash.cpp` (the `getStringHash`
  diagnostic the document already cites),
  `source/slang/slang-ir-uniformity.cpp` and
  `source/slang/slang-emit.cpp` (the `TreatAsDynamicUniform` remover
  and its gate), and `source/slang/slang-ir-hlsl-legalize.cpp` (also
  already cited by the document). The first is the one that matters
  most: this page's `AST origin` column repeatedly depends on checker
  decisions, and the checker is unwatched.
- Gap `4eca2b39600e` leaves a known inaccuracy in place by design: the
  `countOf` row still lists `vector` as an operand kind, which
  `_isTypeOrValValidForCountOf` rejects. Once the array fold is fixed,
  that row should be re-checked in the same pass — a follow-up doc gap
  or a `fixed` action in a later cycle, not an edit made under an
  escalation.
