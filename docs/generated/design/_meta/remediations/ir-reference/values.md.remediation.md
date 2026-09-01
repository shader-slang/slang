---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:58:00Z
target_doc: ir-reference/values.md
review_report: ../../reviews/ir-reference/values.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/values.md

## Summary

All five findings were fixed. The missing `Reshape and pack helpers` subsection
now exists, the false `ReinterpretOptional` producer claim is replaced by a
"no producer at HEAD" classification that also records the stale source comment
behind it, four aggregate-constructor origin cells gained their real AST
producers, the descriptor-handle conversion count was corrected from four to
six, and the IR-pass mechanics in that callout were trimmed to a producer
classification plus a link. The document was edited, so `mark-fresh` is needed.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                                                                                                                                                                                                                                | Fix summary                                                                                                                                                                                              |
| ---------- | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | `docs/generated/design/_meta/prompts/ir-reference-values.md:48-49` names `Reshape and pack helpers` as its own table and the page had none. The arithmetic/logical split was left alone: `_common.md:230-232` explicitly encourages sub-tables for large groups, so two tables covering the prompt's `Arithmetic and logic` group is not a contract violation, whereas an absent group is.                                               | Moved `matrixReshape`, `vectorReshape`, `getTupleElement`, and `getTargetTupleElement` out of `Aggregate constructors` into a new `### Reshape and pack helpers` table; rows unchanged apart from F-003. |
| F-002      | fixed  | Confirmed. `source/slang/slang-ir-typeflow-set.cpp:266-274` carries a comment claiming it emits the opcode but returns `openOptional(...)`, which builds the if-else itself; there is no `emitReinterpretOptional` and no `kIROp_ReinterpretOptional` construction anywhere in `source/`, and `source/slang/slang-ir-lower-reinterpret.cpp:228-260` only consumes existing instances. The stale comment is a real (harmless) source bug. | Conversions row origin changed to "no producer at HEAD" with the stale-comment explanation; live-but-unproduced count raised from four to five and a paragraph added for the opcode.                     |
| F-003      | fixed  | Verified each producer in `source/slang/slang-lower-to-ir.cpp`: `visitPackExpr` at 6542 calls `emitMakeValuePack` at 6550, `visitEachExpr` at 6554 calls `emitGetTupleElement` at 6558, `visitMakeArrayFromElementExpr` at 6757 calls `emitMakeArrayFromElement` at 6765, and the initializer-list path calls `emitMakeMatrix` at 6887 and `emitMakeTuple` at 6965. `makeValuePack` was wrongly listed as purely synthesized.            | Added the AST classes and visitor line numbers to the `makeMatrix`, `makeArrayFromElement`, `makeTuple`, `makeValuePack`, and `getTupleElement` origin cells.                                            |
| F-004      | fixed  | Confirmed: the conversions table at lines 297-302 holds six descriptor-handle conversion opcodes, matching the six Lua entries in `source/slang/slang-ir-insts.lua` at 2756-2757 and 2773-2778, but the callout counted four by skipping the `uint2` pair.                                                                                                                                                                               | Callout now says six opcodes in three pairs, names the `uint2` pair explicitly, and the closing contrast reads "These six".                                                                              |
| F-005      | fixed  | `ir-reference-values.md:72-75` forbids IR-pass behaviour. The peephole fold was pure optimization behaviour with no bearing on opcode shape or origin; the buffer-element-type detail was reduced to the producer identification the `AST origin` classification needs, as the reviewer allows.                                                                                                                                          | Deleted the peephole-fold sentence; replaced the `convertOriginalToLowered` / `convertLoweredToOriginal` mechanics with a named producing pass plus a link to `../pipeline/05-ir-passes.md`.             |
