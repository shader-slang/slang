---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: ir-reference/misc.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 6
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 1
---

# Gap-intake report for ir-reference/misc.md

## Summary

One gap stays escalated: `6527e8b0b50a`, the `asDynamicUniform` lifetime, whose source behaviour is unchanged. The four REOPENED gaps (`287d78f2e773`, `35bd0499af6c`, `352bf73e9bfd`, `03a328b5c0e3`) were re-decided against the tree at `2518a942d5`; all four are `fixed`, and the page now states each behaviour. The reason the earlier fixes did not take is that the page's source citations had drifted (`slang-lower-to-ir.cpp` moved by hundreds of lines) and, for `03a328b5c0e3`, the legalizer the page described no longer exists where it said. Gap `4eca2b39600e` moves from `escalated-to-finding` to `fixed`: the compiler defect it reported is gone. The breakdown is five `fixed`, one `escalated-to-finding`, and no `rejected-bogus`, `rejected-out-of-scope`, or `deferred`. `slangc` in this tree is a Linux binary and could not be run, so every verdict below rests on reading source; no dump was regenerated.

## Escalated gaps

- **`6527e8b0b50a`** — `asDynamicUniform<T>` is an ungated core-module function (`source/slang/core.meta.slang:4127-4128`). The only code that erases `TreatAsDynamicUniform` is `eliminateAsDynamicUniformInst` (`source/slang/slang-ir-uniformity.cpp:473-494`, called at 470), reached only from `validateUniformity`, which `source/slang/slang-emit.cpp:1501-1503` runs only when `CompilerOptionName::ValidateUniformity` is set. Nothing else in `source/slang` mentions the opcode, so an ordinary call reaches emit; the earlier run observed an abort there. Existing finding: `docs/generated/tests/_meta/findings/as-dynamic-uniform-reaches-emit-ice.yaml`. The `__getLegalizedSPIRVGlobalParamAddr` half of the gap no longer applies: the helper is `internal` and lives in `source/slang/glsl.meta.slang:4512-4514`, so a user cannot call it; the page's row now says so.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 287d78f2e773 | fixed | `visitEachExpr` (`source/slang/slang-lower-to-ir.cpp:6011-6019`) emits `emitGetTupleElement(pack, context->expandIndex)`, and `visitExpandExpr` (6022-) adds the `int` block parameter and records it as `expandIndex`. The only `emitEachInst` callers are `visitEachIntVal` (2232), `visitEachType` (2239) and `visitEachSubtypeWitness` (2353), all `Val`-level. | re-verified the `Expand` / `Each` callout and moved its citations to 2227, 2235, 2345, 6011 and 6022; the `Each` row cites line 2353 |
| 4eca2b39600e | fixed | The compiler no longer misfolds. `visitSizeOfLikeExpr` (`source/slang/slang-lower-to-ir.cpp:5448-5459`) skips the natural-layout shortcut for a `CountOfExpr` and always emits `kIROp_CountOf`; `maybeSpecializeCountOf` (`source/slang/slang-ir-specialize.cpp:1114-1140`) folds a fixed array with an `IRIntLit` count to that count. `_isTypeOrValValidForCountOf` (`source/slang/slang-check-expr.cpp:6586-6617`) accepts type packs, tuples, fixed-size arrays and value packs and returns false for everything else, so a vector and an unsized array get `CountOfArgumentIsInvalid` (E30083, `source/slang/slang-diagnostics.lua:2096`). The finding file named by the earlier report no longer exists, so the escalation is discharged. | narrowed the `countOf` row to array, type pack, tuple and value pack, named the E30083 rejection of vector and unsized-array operands, and described the always-emit-then-fold path in the Size, alignment, count intro |
| 35bd0499af6c | fixed | `source/slang/slang-check-expr.cpp:6913` fills an omitted layout with `getScalarLayoutType()`; `source/slang/slang-lower-to-ir.cpp:5455-5459` treats a null layout and `ScalarDataLayoutType` alike. The six `IBufferDataLayout` implementations are at `source/slang/hlsl.meta.slang:33-73` and map to `DefaultLayout`, `DefaultPushConstantLayout`, `Std140Layout`, `Std430Layout`, `ScalarLayout`, `CLayout` (`source/slang/slang-ir-insts.lua:472-477`). | kept the default-operand and layout-spelling paragraph; moved its checker and `hlsl.meta.slang` citations to the current lines |
| 6527e8b0b50a | escalated-to-finding | Source unchanged; see the Escalated gaps entry. Documenting the opcode's lifetime as it is would bless an abort at emit. Existing finding: `docs/generated/tests/_meta/findings/as-dynamic-uniform-reaches-emit-ice.yaml`. | — |
| 352bf73e9bfd | fixed | `RayQuery::__init` and `HitObject::__init` carry `__intrinsic_op($(kIROp_AllocateOpaqueHandle))` (`source/slang/hlsl.meta.slang:22677`, 24237). `maybeAddReturnDestinationParam` (`source/slang/slang-lower-to-ir.cpp:3975-3987`) adds a return-destination parameter for a non-copyable result, `returnViaLastRefParam` is set at 4236, and the call site passes the destination and retypes the result `void` (5159-5185); `emitCallToDeclRef` (977-1019) then builds the inst through `emitIntrinsicInst`. `source/slang/slang-ir.cpp:9757` lists the opcode as side-effect free and the DCE loop at `slang-lower-to-ir.cpp:15654-15658` runs before the `LOWER-TO-IR` dump (15853), so it never shows in a dump. The earlier debugger observation is not repeatable here (no runnable `slangc`); the mechanism above is what the source states. | rewrote the paragraph's citations (3975, 4236, 5159-5185, 9757, 15654-15658, 15853); the paragraph still states that no shader's dump contains the inst |
| 03a328b5c0e3 | fixed | The wrapper helpers are at `source/slang/hlsl.meta.slang:21106` and 21113; their only call sites are payload arguments at 21191 (`__traceRayHLSL`), 24263 (`__hlslTraceRay`), 25209 (`__InvokeHLSL_NVAPI`), 25289 (`__InvokeHLSL_DXR`), all ray-payload spellings. The HLSL legalizer no longer owns this: the rewrite moved to `legalizeD3DCall` (`source/slang/slang-ir-ray-tracing-legalize.cpp:416-495`), which loops over each `Call`'s arguments (456-495), run only for D3D targets (666-670) from `source/slang/slang-emit.cpp:1989`; `source/slang/slang-ir-hlsl-legalize.cpp` no longer mentions the opcodes. | rewrote the section preamble to name the new pass and its three cases (struct passthrough, dummy payload, single-field temporary), corrected the signature line numbers and call-site list, and stated that the pass handles call-argument position on D3D targets only |

## Operator notes

- Source facts the page now states that moved since the page was generated: `IsVector` folds to true for a builtin vector (`source/slang/slang-ir-peephole.cpp:1959-1960`) where the old text said it always folds to false; `IsBindlessTextureNVEncodable` (`source/slang/slang-ir-insts.lua:2851`) is a new predicate with a row; the barrier-flag emit path goes through `getBarrierFlagValueInst` (`source/slang/slang-ir-util-hlsl.cpp:22`); the `getStringHash` check in `slang-emit.cpp` now follows a dead-code pass (lines 1800-1811).
- Two stale cross-references were corrected in the compiler-dictionary rows: `CompilerDictionary` is built at `slang-ir-translate.cpp:23` and `CompilerDictionaryScope` at 27 (the old text had them reversed).
- New Lua opcode `reportOptiXIntersection` (`slang-ir-insts.lua:1670`) sits in the ray-tracing payload block that `ir-reference/resources-and-atomics.md` covers; it was not added here.
- Unwatched files this cycle depended on: `source/slang/slang-ir-ray-tracing-legalize.cpp`, `source/slang/slang-ir-specialize.cpp`, `source/slang/glsl.meta.slang`, `source/slang/slang-ir-util-hlsl.cpp`. Consider adding `slang-ir-ray-tracing-legalize.cpp` to `watched_paths`, since the struct-wrapping section now describes it.
