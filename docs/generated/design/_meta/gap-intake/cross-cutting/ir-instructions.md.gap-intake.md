---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: cross-cutting/ir-instructions.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
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

This is a second intake pass over six gaps recorded `fixed` that a regenerated bundle still reports (REOPENED). None is a compiler defect; all six were re-decided as `fixed` (six fixed, none rejected, deferred or escalated). The earlier edits already stated each behaviour, so the doc was strengthened where the gap asked for something the text lacked: an explicit "Slang surface" lead-in for the five control-flow opcodes, a buffer-element versus local example for `MetalPackedVec`, and the `makeArray` initializer-list surface. Behaviour was re-confirmed against current source at 2518a942d, and the stale line citations and counts elsewhere in the doc were refreshed in the same pass.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| f2af7ee073b3 | fixed | `source/slang/core.meta.slang:519-530` (`__requirePrelude`, `__requireTargetExtension`, `static_assert` as `__intrinsic_op`), `source/slang/hlsl.meta.slang:15221-15222` (`printf<each T>`), `:15241-15242` (`abort<each T>`) | Control-flow Notes cell now opens the reachability statement with "Slang surface:" and names all five calling spellings; none is compiler-internal |
| 3b220c71b247 | fixed | `source/slang/slang-ir.cpp:4078-4084` is the only `kIROp_Alloca` producer (`emitAlloca`, RTTI object pointer as size); `source/slang/slang-ir-insts.h:3841` declaration; no caller in `source/` (the sole non-emit hits are the builder itself); `source/slang/slang-ir-insts.lua:1150` | `alloca` cell already states no Slang construct reaches it and locals lower to `var`; citation moved to line 4078 |
| 3ffbd8c00902 | fixed | `source/slang/slang-ir-lower-buffer-element-type.cpp:3141-3153` (`usesPackedVectorStorage`: natural layout, `StorageBuffer`/`UserPointer` only), `:3162-3168` (`needsPackedVectorStorage`: element count > 1); `source/slang/slang-ir-insts.lua:122-123` | Added the `StructuredBuffer<float3>` element versus `float3` local contrast and the deciding functions to the `MetalPackedVec` cell (file is outside watched_paths) |
| 65101e9dbc06 | fixed | `source/slang/core.meta.slang:3452` (`bit_cast`), `:1259` (`getBaseTypeConversionOp` feeding the generated conversion `__init`s) | Conversion cell names `bit_cast<T>(x)` and the built-in conversion constructors (`int64_t(a)`, `float(d)`); no change needed beyond re-verification |
| 9c7ad237ab9d | fixed | `source/slang/slang-ir-insts.lua:3454-3478` (every `constexpr*` entry `hoistable = true`), `:1552` runtime `add` unflagged; `source/slang/slang-ir-insts.h:4456`, `source/slang/slang-ir.cpp:6869` (`emitConstexprAdd`) | `constexpr*` cell already carries the generic-value-parameter `int b[N + 1]` example; re-verified |
| 7744ad2b760b | fixed | `source/slang/core.meta.slang:2851-2893` (`MakeVector` constructors), `source/slang/slang-ir.cpp:4268-4291` (`emitDefaultConstruct` builds `makeStruct`/`makeArray`), `source/slang/slang-lower-to-ir.cpp:6242-6314` (`visitInitializerListExpr` emits `makeArray`, padding short lists) | Aggregate-constructor row now also states the array-initializer surface for `makeArray` (lower-to-ir is outside watched_paths) |

## Unwatched-path notes for the operator

`slang-lower-to-ir.cpp` (array initializer, `constexpr*` producers) and `slang-ir-lower-buffer-element-type.cpp` (Metal packing rule) own behaviour this page now cites but are not in its `watched_paths`; a change there will not mark the page stale. Gap 65101e9dbc06 and 9c7ad237ab9d needed no text change, so if the bundle still reports them after regeneration they may be failing on form (a requested table column) rather than content.
