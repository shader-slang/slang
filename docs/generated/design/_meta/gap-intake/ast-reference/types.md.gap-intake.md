---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:45:00Z
target_doc: ast-reference/types.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 6
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for ast-reference/types.md

## Summary

Six gaps were acted on: five `fixed` and one `deferred`; nothing was
escalated, because in every case the watched source agreed with the
observation rather than with the document. Four of the fixes are
`## Nodes` row rewrites (`NamedExpressionType`, `AtomicType`, and the
pseudo-type rows `OverloadGroupType` / `InitializerListType` /
`ErrorType` / `BottomType`), all taken from the class comments in
`source/slang/slang-ast-type.h` and the `__magic_type` declarations in
`source/slang/core.meta.slang`; the fifth adds the `descriptor_handle`
capability gate to the descriptor-heap section from
`source/slang/hlsl.meta.slang`. The `BasicExpressionType, ...` callout
gained a boundary paragraph anchored on the `__builtin_type` loop in
`core.meta.slang` and `_determineIntegerLiteralType` in
`slang-parser.cpp`. The single deferral is the `BorrowInParamType`
printing surface, whose behaviour lives entirely in
`source/slang/slang-ast-type.cpp` and `source/slang/slang-ast-print.cpp`
— neither of which is in this page's `watched_paths`.

Two side notes for the operator, both outside the queue and therefore
not edited: the `## Source` section still says the `.meta.slang` module
sources "are not among this page's watched paths" and proposes adding
`hlsl.meta.slang` to the manifest, but `regenerate.py show` already
resolves `core.meta.slang` and `hlsl.meta.slang` as watched, so that
paragraph is stale. And `ASTPrinter::addDeclParams`
(`source/slang/slang-ast-print.cpp:1565-1571`) renders only
`InOutModifier` and `OutModifier`, silently dropping `__constref` and
`__ref` from overload-candidate signatures — a plausible compiler
defect with no existing finding.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 891e2c74c784 | fixed | `source/slang/core.meta.slang:1137-1139` declares one scalar leaf per `BaseType` tag via `__builtin_type($(int(kBaseTypes[tt].tag)))`, with intrinsic-op operators (1235-1246); `source/slang/slang-parser.cpp:8467-8605` (`_determineIntegerLiteralType`) widens an unsuffixed decimal `int` -> `int64` and diagnoses `IntegerLiteralTooLarge` at 8541/8563 for anything past `int64`; the unsigned-wrap half is the bundle test `basictype-uint-wrap-overflow.slang` (and its recorded `uint32_t` / `0` output), the literal half `basictype-literal-too-large-rejected.slang` (E10012) | added a value-boundary paragraph to `### BasicExpressionType, VectorExpressionType, MatrixExpressionType` covering native-width wrap and the too-large-literal diagnostic |
| d17d7508aea8 | fixed | `source/slang/slang-ast-type.h:937-949` — the class comment is only "A type alias of some kind (e.g., via `typedef`)"; it declares both `_toTextOverride` and `_createCanonicalTypeOverride`, and `Type::getCanonicalType()` in `source/slang/slang-ast-base.h` resolves through the latter, so nothing in the source promises the alias name to diagnostics | reworded the `NamedExpressionType` row: prints under the alias name, but `getCanonicalType()` resolves it away to the aliased type |
| a77c5bf8b18d | deferred | The `borrow ` spelling is produced by `BorrowInParamType::_toTextOverride` (`source/slang/slang-ast-type.cpp:1406-1409`) and reaches a function signature through `FuncType::_toTextOverride` (`source/slang/slang-ast-type.cpp:1471-1483`), while candidate notes take a different route — `ASTPrinter::getDeclSignatureString` (`source/slang/slang-check-overload.cpp:3620`) into `ASTPrinter::addDeclParams` (`source/slang/slang-ast-print.cpp:1534-1571`), which renders only `inout` and `out`. None of those three files is in this page's `watched_paths`, so the naming of the printing context needs a `watched_paths` expansion (at minimum `source/slang/slang-ast-type.cpp`); the stripping half additionally looks like a defect in `ASTPrinter` rather than documentable behaviour | — |
| 7a67d2ac24b2 | fixed | `source/slang/slang-ast-type.h:16` ("the type of a reference to an overloaded name"), `:26-27` ("initializer-list expression (before it has been coerced to some other type)"), `:37` ("the type of an expression that was erroneous"), `:48` ("bottom/empty type that has no values") plus `:1059-1060` and `:1069-1070` (bottom type as a never-returning result type and a cannot-fail error type); bundle tests `overloadgrouptype-ambiguous-rejected.slang`, `initializerlisttype-collapse-to-target.slang`, `errortype-cascading-suppression.slang`, `functype-error-type.slang` | rewrote the four pseudo-type Summary cells to name the source shape that produces each; the `TypeType` row was left alone because it already carries the `float(2)` example the gap asks for |
| e868d4c06329 | fixed | `source/slang/core.meta.slang:4097-4131` — `__magic_type(AtomicType)` on `struct Atomic<T : IAtomicable>`, whose `load` / `store` / `exchange` / `compareExchange` are `__intrinsic_op($(kIROp_Atomic*))`, extended with `add` / `sub` / `max` / `min` at 4134-4155; the per-target lowering is pinned by the bundle test `atomictype-wrapper.slang` (`InterlockedAdd` / `atomicAdd` / `atomic_fetch_add_explicit` / `OpAtomicIAdd`) | expanded the `AtomicType` row to the `Atomic<T>` spelling, gave it the `type ref` grammar link, and stated that an update lowers to the target's atomic instruction |
| c49ca9b56cf4 | fixed | `source/slang/hlsl.meta.slang:27593` and `:27602` put `[require(glsl_hlsl_spirv_wgsl, descriptor_handle)]` on the `__ResourceDescriptorHeapType` / `__SamplerDescriptorHeapType` subscripts, matching the handle constructors at `:27568` and `:27577`; the comment at `:27589-27590` states the gate is repeated on the subscript so the diagnostic lands at the indexing site. The gap's `undefined identifier` symptom is not documented — the bundle README (`## Untested claims`, row 3) attributes it to the generating runner's slangc predating this surface | added a capability-gate sentence to `### UntypedResourceHandleType and UntypedSamplerHandleType` |
