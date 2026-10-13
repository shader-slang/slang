---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:25:21Z
target_doc: ir-reference/control-flow.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 11
actions:
  fixed: 9
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/control-flow.md

## Summary

No gap was escalated: all three `drift-from-source` gaps share one
root cause that is documented compiler design, not a defect.
`generateIRForTranslationUnit` runs a fixed block of mandatory passes
between the last statement visitor and the `LOWER-TO-IR` dump
(`source/slang/slang-lower-to-ir.cpp` lines 15572-15770, dump at
15797), so the "first `-dump-ir` snapshot" the reporting tests observe
is already post-`lowerErrorHandling`, post-`lowerDefer`, post-SCCP and
post-`simplifyCFG`. That single rule is now stated once, in a new
`### What a lowering dump shows` callout, and the five gaps that are
instances of it (`32fe5f46f2a3`, `28bd3d6676a6`, `ef8670879a7e`,
`19229132854c`, and the `while (true)` half of `75969ee7db6c`) point at
it rather than repeating it. Nine gaps were fixed, one was rejected as
bogus (`813e8869b359` — the operand order it asks for is already in the
`Operands` column, and the family contract forbids code blocks in
`## Notable opcodes`), and one was deferred (`671845da605d` — the
constraint is enforced in `slang-emit-spirv.cpp` /
`slang-emit-glsl.cpp`, outside this page's `watched_paths`).

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 32fe5f46f2a3 | fixed | `source/slang/slang-lower-to-ir.cpp:15580-15581` calls `lowerDefer` inside `generateIRForTranslationUnit`, ahead of the `LOWER-TO-IR` dump at `:15797`; the doc's "a later IR pass" was true but read as post-hand-off. Source does not contradict the doc, so not a compiler bug. | Named `lowerDefer` and its line-15581 position in `### defer`; the general rule lives in the new `### What a lowering dump shows` callout shared with 28bd3d6676a6, ef8670879a7e, 19229132854c and 75969ee7db6c |
| 04331dedbf64 | fixed | `source/slang/slang-lower-to-ir.cpp:9059-9063` (`visitDiscardStmt` emits only `emitDiscard()`, never a terminator); printed form pinned by `docs/generated/tests/design/ir-reference/control-flow/discard-non-terminator.slang` (`discard` / `unconditionalBranch`); code and report location from `source/slang/slang-diagnostics.lua:2450-2455` (`err("entry-point-uses-unavailable-capability", 36107, ..., span { loc = "decl:Decl" })`), matching `discard-rejected-in-compute-stage.slang` | Added the in-block shape (`discard` then the `unconditionalBranch` to the post-`if` merge) and the 36107 entry-point rejection to `### discard` |
| 19229132854c | fixed | `source/slang/slang-lower-to-ir.cpp:8303-8309` — `visitIfStmt` creates three fresh blocks for any `if`/`else`, so no emitter produces the collapsed then-arm; `simplifyCFG` at `:15611` runs before the dump at `:15797`. Printed form in `ifelse-empty-then-arm.slang` | Added a third-shape paragraph to `### ifElse` attributing `trueBlock == afterBlock` to the mandatory `simplifyCFG`, not to a convenience emitter; part of the one generalized dump-ordering edit |
| 75969ee7db6c | fixed | `source/slang/slang-lower-to-ir.cpp:8689` (`emitNot`) and `:8734` (`emitIfElse(invCondition, breakLabel, merge, merge)`) for the `do`-`while` inversion; `:8568`+`:8578` show `visitWhileStmt` passes the loop head as both target and continue label; `:8435` shows `visitForStmt` uses a distinct continue block; `unreachable` break block per `slang-ir-sccp.cpp:1899-1900` reached from `:15606`. Printed forms in `loop-do-while.slang`, `loop-infinite-while-true.slang` | Added a per-statement operand paragraph to `### loop` covering `for`/`while`/`do`-`while` (more than asked: the `while` target-equals-continue identity holds for every `while`, not only `while (true)`) plus the unreachable break block |
| 988f9b8f21e3 | fixed | `source/slang/slang-lower-to-ir.cpp:15706` calls `checkForMissingReturns(module, sink, CodeGenTarget::None, true)`; codes and severities in `source/slang/slang-diagnostics.lua:4930-4942` (41009 `missing-return-error` err, 41010 `missing-return` warning); report location is `missingReturn->sourceLoc`, shown landing on the signature by `missing-return-reachable-diagnostic.slang` | Named the check, both diagnostic ids, and the signature report location in `### missingReturn and unreachable` |
| 813e8869b359 | rejected-bogus | The `### Other control-flow opcodes` table already carries the operand order the gap says is absent — the `StaticAssert` row's `Operands` cell is `condition, message` and the `Printf` row's is `format, args...`. The premise that "the `loop` and `ifElse` sections already use" example blocks is also false: neither has one, and `docs/generated/design/_meta/prompts/_common.md:250-251` states `## Notable opcodes` callouts must "not include code blocks" | — |
| 671845da605d | deferred | The thread-count rule is enforced at emit time, not in lowering: `source/slang/slang-emit-spirv.cpp:5298-5320` and `source/slang/slang-emit-glsl.cpp:1681-1692` call `verifyComputeDerivativeGroupModifiers` (defined `source/slang/slang-ir-util.cpp:2461-2492`, x and y each `% 2`) when a `RequireComputeDerivative` inst is seen. None of those files is in this page's `watched_paths`, and no bundle test pins the rejection — `require-execution-mode-markers.slang` only uses a conforming `[numthreads(2, 2, 1)]`. Needs `slang-emit-spirv.cpp` / `slang-emit-glsl.cpp` / `slang-ir-util.cpp` added to `watched_paths`, or a test that pins the diagnostic | — |
| 532e76a42314 | fixed | The statement spelling is used throughout a watched path: `source/slang/core.meta.slang:2153`, `:2169`, `:3455-3458` (`case cpp: __intrinsic_asm "int(strlen($0))";` inside `__target_switch`); `source/slang/slang-lower-to-ir.cpp:9482-9500` reads `stmt->asmText` into operand 0. Printed form in `generic-asm-terminates-target-case.slang` | Added the `__intrinsic_asm "<text>";` statement spelling to the `GenericAsm` row's `AST origin` cell |
| ef8670879a7e | fixed | The doc's claim is what the source says: `source/slang/slang-lower-to-ir.cpp` contains **no** `emitUnreachable` call (only `emitMissingReturn` at `:14373`), so the gap's suggested "all-arms-diverge lowering" does not exist. The observed `unreachable` comes from `applySparseConditionalConstantPropagation` (`:15606`), which gives a use-but-no-predecessor block a body of one `unreachable` (`source/slang/slang-ir-sccp.cpp:1896-1900`), before the dump at `:15797`. Not a compiler bug | Replaced the retired `(synthesized)` `AST origin` cell with the producing pass and added a paragraph to `### missingReturn and unreachable` explaining why the opcode is in a lowering dump; part of the one generalized dump-ordering edit |
| fc606038962b | fixed | `__target_switch` appears in a watched path at `source/slang/core.meta.slang:1076`, `:2151`, `:3453`; `visitStageSwitchStmt` (`source/slang/slang-lower-to-ir.cpp:9335-9425`) lowers the stage form to `emitSwitch` over `emitGetCurrentStage()`, and the `__stage_switch { case compute: ... }` surface plus its `switch(GetCurrentStage, ...)` printed form are pinned by `stage-switch-lowers-to-switch.slang` | Named `__stage_switch` and `__target_switch` in the two `AST origin` cells of the switches table |
| 28bd3d6676a6 | fixed | Lowering does emit both opcodes — `source/slang/slang-lower-to-ir.cpp:8969` (`emitThrow`) and `:909` (`emitTryCallInst` for `TryClauseType::Standard`) — so the doc's `AST origin` claims are correct and this is not a compiler bug. `:15574-15578` calls `lowerErrorHandling` with the comment "lowering throwing functions into functions that returns a `Result<T,E>` value, translating `tryCall` into normal `call` + `ifElse`"; the `Result` opcodes are `source/slang/slang-ir-insts.lua:1135-1139` | Added a short-lived-opcode paragraph to `### tryCall and throw` naming `lowerErrorHandling` and the `Result` opcodes that replace them; part of the one generalized dump-ordering edit |
