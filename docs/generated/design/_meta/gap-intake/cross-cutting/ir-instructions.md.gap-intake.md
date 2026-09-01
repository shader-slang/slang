---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:41:08Z
target_doc: cross-cutting/ir-instructions.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 6
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for cross-cutting/ir-instructions.md

## Summary

All six gaps were fixed; none was rejected, deferred, or escalated as
a compiler defect. Every gap was a `missing-surface` / `missing-example`
question of the same shape — "what Slang code produces this opcode?" —
and each answer was confirmable in a watched `.meta.slang` or in
`slang-ir.cpp`. Two of the six `Suggested addition` hypotheses were
wrong and the document says the opposite of what they proposed:
`RequireTargetExtension`, `Abort` and `StaticAssert` are _not_
compiler-internal (all three are `__intrinsic_op` core-module functions
a user can call), and `alloca` is not the opcode for
"dynamically-sized allocations" a reader could reach but an opcode with
no producer at all in the tree. Three edits rest partly on evidence
outside the watched paths and are flagged below.

## Actions

| Gap ID       | Action | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      | Fix summary                                                                                                                                                                                      |
| ------------ | ------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| f2af7ee073b3 | fixed  | `source/slang/core.meta.slang:472-473` (`__intrinsic_op($(kIROp_RequirePrelude)) void __requirePrelude(...)`), `:475-476` (`__requireTargetExtension`), `:481-483` (`__intrinsic_op($(kIROp_StaticAssert)) void static_assert(constexpr bool, NativeString)`), `source/slang/hlsl.meta.slang:14109-14112` (`printf<each T>`), `:14130-14132` (`abort<each T>`); the `Printf` / `RequirePrelude` forms are the verified IR CHECK lines of `docs/generated/tests/design/cross-cutting/ir-instructions/control-flow-printf-ir.slang` and `control-flow-require-prelude-ir.slang` | named the calling surface of all five opcodes in the Notes cell, stating that none of them is compiler-internal (the gap's hypothesis that three were, is contradicted by the source)            |
| 3b220c71b247 | fixed  | `source/slang/slang-ir.cpp:4046-4052` is the sole `kIROp_Alloca` producer, `IRBuilder::emitAlloca(IRInst* type, IRInst* rttiObjPtr)`; declared at `source/slang/slang-ir-insts.h:3797` and unreferenced anywhere else in `source/` (the `emitAlloca` hits in `slang-emit-llvm.cpp` / `slang-llvm/` are the LLVM builder's own method); `source/slang/slang-ir-insts.lua:1150` places `alloca` next to `rtti_object`                                                                                                                                                           | added to the `alloca` Notes cell that no Slang construct reaches it, that locals always lower to `var`, and that its one producer takes an RTTI object pointer as the size                       |
| 3ffbd8c00902 | fixed  | verified CHECK lines of `docs/generated/tests/design/cross-cutting/ir-instructions/type-metal-packed-vec-emit.slang` — `StructuredBuffer<float3>` gives `packed_float3 device*` on Metal, `float3` on HLSL, `vec3` on GLSL, `vec3<f32>` on WGSL, and `ArrayStride 16` in SPIR-V; opcode shape from `source/slang/slang-ir-insts.lua:122-123`                                                                                                                                                                                                                                  | added the selecting condition (a vector used as a device-buffer element type on Metal) and the buffer-versus-other-target contrast to the `MetalPackedVec` Notes cell                            |
| 65101e9dbc06 | fixed  | `source/slang/core.meta.slang:3376-3377` declares `__intrinsic_op($(kIROp_BitCast)) T bit_cast(U value)`; `:1209` and `:1223-1225` generate, for every base-type pair, an `__intrinsic_op($(intrinsicOpCode)) __init($(kBaseTypes[ss].name) value)` whose opcode comes from `getBaseTypeConversionOp`; the two constructor forms are the verified CHECK lines of `conversion-intcast-ir.slang` (`int64_t(a)` → `intCast`) and `conversion-floatcast-ir.slang` (`float(d)` → `floatCast`), and `bit_cast` of `conversion-bitcast-ir.slang`                                     | named `bit_cast<T>()` and the built-in conversion constructors in the conversion-row Notes cell, in the same table edit as 9c7ad237ab9d and 7744ad2b760b                                         |
| 9c7ad237ab9d | fixed  | `source/slang/slang-ir-insts.lua:3408-3437` gives every `constexpr*` entry `hoistable = true`, while the runtime `add` at `:1552` has no flag — the flag difference is what makes them type-level values; `source/slang/slang-ir-insts.h:4412` / `source/slang/slang-ir.cpp:6837-6841` define `emitConstexprAdd`; the generic-value-parameter surface is the verified CHECK line `constexprAdd(1 : Int, %N)` of `value-constexpr-add-ir.slang`                                                                                                                                | added the type-level-value distinction and the `int b[N + 1]` inside `f<let N : int>` example to the `constexpr*` Notes cell                                                                     |
| 7744ad2b760b | fixed  | `source/slang/core.meta.slang:2758-2812` declares the `vector<T,2/3/4>` component constructors as `__intrinsic_op($(kIROp_MakeVector)) __init(...)`, confirmed by the verified CHECK line of `value-make-vector-ir.slang`; operand shapes from `source/slang/slang-ir-insts.lua:1064` (`makeVector`), `:1077` (`makeArray`), `:1082` (`makeStruct`), all variadic, and from `source/slang/slang-ir.cpp:4241-4260`, where `emitDefaultConstruct` builds one operand per array element and per struct field                                                                     | added a `makeVector` / `makeArray` / `makeStruct` aggregate-constructor row with the vector-constructor surface; the array-initializer surface is not stated (see the unwatched-path note below) |

## Unwatched-path notes for the operator

Three fixes are confirmed only partly, or not at all, inside the
manifest's `watched_paths` for this page, so a change to the real
owner will not mark the page stale:

- `source/slang/slang-ir-lower-buffer-element-type.cpp` owns the
  `MetalPackedVec` decision (3ffbd8c00902). Its
  `usesPackedVectorStorage` / `needsPackedVectorStorage` pair (lines
  2985-3012) and `lowerLeafLogicalType` (line 3128) say the rule
  exactly: on Metal, _every_ multi-element vector in a
  natural-layout device-memory buffer becomes packed, while constant
  and argument buffers keep the native padded MSL vector. Only the
  part the bundle test verifies was written into the document.
- `source/slang/slang-lower-to-ir.cpp` owns the `constexpr*`
  producers (9c7ad237ab9d) at lines 1925 and 2027, reached from
  `IntVal` lowering, and the array-initializer → `makeArray` path
  (7744ad2b760b) at line 6846, which also pads a short initializer
  list with default values. The `makeArray` surface was left out of
  the document for want of a watched-path citation; adding this file
  to `watched_paths` would let a later cycle state it.

The document is now 27607 bytes against its 49152-byte `size_cap`, so
none of these additions pressures the cap.
