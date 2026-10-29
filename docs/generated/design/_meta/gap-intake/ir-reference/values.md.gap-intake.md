---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:16:45Z
target_doc: ir-reference/values.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 12
actions:
  fixed: 9
  rejected_bogus: 2
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/values.md

## Summary

Twelve gaps were acted on: nine fixed, two rejected as bogus, one
deferred. Nothing was escalated — every disagreement traced either to
the document being incomplete about a checker or dump behaviour, or to
the reporting agent reading a post-pass IR dump and attributing the
result to lowering. The two rejections are `matrixReshape` (the
core-module reshape overload really does carry
`__intrinsic_op($(kIROp_MatrixReshape))`; the observed
`getElement` / `swizzle` / `makeMatrix` shape is the peephole pass
expanding it) and integer divide-by-zero (the `div` opcode *is*
emitted; the diagnostic comes from the SCCP pass, not from semantic
check). The single deferral is the `makeStruct` / `$init` question,
which cannot be settled from this document's watched paths.

Several fixes cite files outside `watched_paths` —
`slang-check-expr.cpp`, `slang-check-conversion.cpp`,
`slang-ir-ssa.cpp`, `slang-ir-legalize-empty-array.cpp`, and
`hlsl.meta.slang`. The document already cited four of those five
before this cycle, but the manifest does not watch any of them, so the
new prose will not be re-checked when they change; see the note at the
end of this report.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| e8307921ab20 | deferred | `source/slang/slang-lower-to-ir.cpp:6774,6975` confirms the documented origin is live: the struct branch of `visitInitializerListExpr` does call `emitMakeStruct`. Whether a given struct initializer list reaches it, or is first rewritten into a synthesized `$init` call, is decided by `_coerceInitializerListToStruct` in `source/slang/slang-check-conversion.cpp:855-960` — whose `isCStyleType` / `useCStyleInitialization` branching I could not pin down well enough to state a rule, and which is outside this doc's `watched_paths`. Needs a `watched_paths` expansion plus a follow-up trace. | — |
| 45202f25ddbd | fixed | `source/slang/slang-ir.cpp:4291-4295` (the `kIROp_CoopMatrixType` case of `emitDefaultConstruct`) is the only caller of `emitMakeCoopMatrixFromScalar` (`source/slang/slang-ir.cpp:4940`); no `__intrinsic_op` naming the opcode exists in any `*.meta.slang`. | replaced the bare `(synthesized)` on the `makeCoopMatrixFromScalar` row with its one real producer and stated it is unreachable from a call in Slang source |
| 2f6a6fb9cd5e | rejected-bogus | The premise is wrong. `div` / `irem` are emitted normally; the "divide by zero" diagnostic the test observes is raised by the SCCP constant-propagation pass at `source/slang/slang-ir-sccp.cpp:1071-1088` and `1091-1110`, i.e. after lowering. The only front-end site, `source/slang/slang-ast-val.cpp:2474-2479`, folds `IntVal` constant expressions and cannot fire for the test's `uniform int a`. Writing "those cases produce no opcode at all" would document a falsehood, and the real mechanism is IR-pass behaviour that this page's prompt assigns to `pipeline/05-ir-passes.md`. | — |
| 5d56c6c244e9 | fixed | `source/slang/slang-ir.cpp:4457-4458` confirms the documented Enum→Enum cell. The decomposition is real: `source/slang/slang-check-conversion.cpp:2197-2231` coerces the operand to the target enum's tag type and wraps the result, and `2175-2195` makes the first leg an enum-to-tag `BuiltinCastExpr`, so two nested casts reach `emitCast` separately. | noted on the `EnumCast` row that a source-level enum-to-enum conversion is split by the checker into `CastEnumToInt` plus `CastIntToEnum` |
| bf32bebdebee | fixed | The suggested "(synthesized)" marking is wrong — there is an AST origin. `source/slang/slang-lower-to-ir.cpp:7756-7773` builds the `ImplicitCastedLValue` from `Out`/`InOutImplicitCastExpr`, and `10181-10197` is the only consumer of that flavor. The checker builds the node only for a differing l-value argument type at `source/slang/slang-check-expr.cpp:4333-4378`, gated by `_canLValueCoerceScalarType` (`3854-3878`), which admits only same-width integer pairs and their vector / matrix forms. | named the real AST nodes on both rows and added an `out` / `inout` argument casts subsection giving the exact reachability condition |
| 83c0ba7f1d2d | fixed | `source/slang/core.meta.slang:3376-3377` declares `bit_cast<T, U>` with `__intrinsic_op($(kIROp_BitCast))`; `source/slang/hlsl.meta.slang:8391-8500` declares `asuint` / `asint` / `asfloat` / `asdouble` with `__intrinsic_asm` inside a `__target_switch`, so they carry no opcode. | added `bit_cast<T, U>` as the direct core-module surface on the `bitCast` row and recorded that the `as*` builtins stay calls resolved by the target-intrinsic mechanism |
| 86c9dc4f8a2a | fixed | `source/slang/slang-ir.cpp:4170-4307` is the switch: everything absent from it (resource / sampler types, interface types, generic type parameters, associated types) reaches `default: break;`, as do an array or cooperative vector whose element count is not an `IRIntLit` or exceeds the 4096 cap coded at `4280-4282`. `source/slang/slang-ir-insts.h:3849` shows `fallback` defaults to `true`. | named the concrete non-decomposable result types under `defaultConstruct` |
| 55020f37a0c7 | fixed | `source/slang/slang-ir.cpp:7822-7832` folds every `IRConstant` into its use sites unless the dump mode is `Detailed`; `8175-8208` gives the per-class inline spelling, and `VoidLit` has no case there so it falls to the generic `dump(opInfo.name)` at `8275`. The printed forms are pinned by the bundle's `literals-inline-payload.slang` CHECK lines (`add(%a, 42 : Int)`, `return_val(void_constant)`). | added a dump-rendering paragraph with a two-line example and the "no definition line" rule to Literal payload encoding |
| 5ae92a1f6461 | fixed | `source/slang/slang-lower-to-ir.cpp:10249-10345` (`assign`, `SwizzledLValue` case at `10297`): it emits `swizzleSet` only in the `default:` branch, i.e. when the base did not reduce to a `Ptr`. `10042-10093` shows a `BoundStorage` reduces to a pointer only through a `ref` accessor, and `1102-1124` shows a `property` / `__subscript` with more than a getter becomes a `BoundStorage`. Property syntax matched against `tests/autodiff/property-accessor-3.slang`. | gave the concrete non-addressable destination (a `get` / `set` property or subscript with no `ref` accessor) with a minimal example, and cross-linked it from the `swizzleSet` row |
| 17ea93402227 | rejected-bogus | The documented origin is correct: the matrix generator in `source/slang/core.meta.slang:2861-2867` emits `__intrinsic_op(<kIROp_MatrixReshape>) __init(matrix<T,rr,cc,L> value)` for every larger source shape, so `float2x2(m3)` does lower to `matrixReshape`. The observed `getElement` / `swizzle` / `makeMatrix` triple is that opcode after `source/slang/slang-ir-peephole.cpp:1575-1618` expands it row by row — a later-pass artifact, not the lowering the `AST origin` column describes. | — |
| c4300e3e8f75 | fixed | `source/slang/slang-lower-to-ir.cpp` has exactly one `emitGetTupleElement` call, in `visitEachExpr` at `6560-6569`, so the documented `MemberExpr` origin has no producer. `source/slang/core.meta.slang:1929-1934` documents tuple member names as swizzle components, and `source/slang/slang-check-expr.cpp:8251` (`checkTupleSwizzleExpr`, reached from `8983`) rewrites the `MemberExpr` accordingly, which is why `t._0` lowers to `swizzle`. | dropped the non-existent `MemberExpr` origin from the `getTupleElement` row and recorded that positional tuple access lowers to `swizzle` |
| a2cc6cfbadbe | fixed | `source/slang/slang-ir.cpp:3289` and `3297` are the two emitters. `emitLoadFromUninitializedMemory` has exactly two callers, both in SSA construction (`source/slang/slang-ir-ssa.cpp:1033` and `1121`, on the no-reaching-store paths of `readVarRec`). `getPoison` has many callers spread across legalization and autodiff, e.g. `source/slang/slang-ir-legalize-empty-array.cpp:89-142`, `slang-ir-legalize-vector-types.cpp:97`, `slang-ir-lower-conditional-type.cpp:138`, `slang-ir-glsl-legalize.cpp:4200`, `slang-ir-autodiff-rev.cpp:120-122`. | replaced `(synthesized)` on both rows with the producing pass, naming SSA construction for `LoadFromUninitializedMemory` and the legalization / autodiff callers of `getPoison` |

## Operator notes

- `target_doc_source_commit_after` is the SHA supplied with the task
  (`ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e`). The working tree's
  `HEAD` at intake time was `c64470bcd3013990310ba15148c633f64d50896c`;
  all line citations above are against that tree.
- Five files this cycle's prose depends on are not in
  `watched_paths` for `ir-reference/values.md`:
  `source/slang/slang-check-expr.cpp`,
  `source/slang/slang-check-conversion.cpp`,
  `source/slang/hlsl.meta.slang`, `source/slang/slang-ir-ssa.cpp`, and
  `source/slang/slang-ir-legalize-empty-array.cpp`. The first three are
  the ones that matter: this page's `AST origin` column repeatedly
  describes checker decisions, and the checker is unwatched.
- Gap `e8307921ab20` is blocked specifically on
  `source/slang/slang-check-conversion.cpp`.
