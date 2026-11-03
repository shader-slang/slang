---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: ir-reference/control-flow.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 9
actions:
  fixed: 9
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/control-flow.md

## Summary

No gap was escalated: the three `drift-from-source` gaps all trace to
one documented design fact, that `generateIRForTranslationUnit` runs
mandatory passes before the `LOWER-TO-IR` dump, so the dump is not the
raw visitor output. This pass re-ran intake for the nine REOPENED
gaps (all previously `fixed`). The doc text already carried each
fix, and the reporting bundle's README predates the current doc
(README `source_doc_digest` 8a79... against the doc digest recorded in
`docs/generated/tests/_meta/freshness.json`), so the reopened verdict
looks like a stale README rather than a missing fix. Independently, the
fixes were re-verified against current source, and every line citation
in the doc was re-derived, since most had drifted. Nine gaps were
re-decided as `fixed`.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 32fe5f46f2a3 | fixed | `source/slang/slang-lower-to-ir.cpp:15614` (`lowerDefer`) runs before the `LOWER-TO-IR` dump at `:15853`; `lowerErrorHandling` at `:15611`; `visitDeferStmt` at `:8495` emits `defer`. Source agrees with the doc, so not a compiler bug | `### defer` and `### What a lowering dump shows` now cite the current lines of `lowerDefer` and the dump; stale lines refreshed |
| 04331dedbf64 | fixed | `source/slang/slang-lower-to-ir.cpp:8631-8635` (`visitDiscardStmt` emits only `emitDiscard()`); `:7885` `emitBranchIfNeeded(afterBlock)` ends the then-arm; `source/slang/slang-diagnostics.lua:2706-2710` (36107, `span { loc = "decl:Decl" }`) | `### discard` keeps the in-block shape; the 36107 sentence no longer claims no `discard` opcode is built, because the capability check is outside the watched paths |
| 19229132854c | fixed | `source/slang/slang-lower-to-ir.cpp:7865-7867` three fresh blocks for `if`/`else`; `:7932` `emitIf` for else-less; `simplifyCFG` at `:15644` precedes the dump | `### ifElse` third-shape paragraph retained, line citation updated to 7865-7867 |
| 75969ee7db6c | fixed | `source/slang/slang-lower-to-ir.cpp:8257` (`emitNot`), `:8304` (`emitIfElse(invCondition, breakLabel, mergeBlock, mergeBlock)`), `:8282` (branch-coverage variant via `loopExitBlock`); `:8134` while continue label is the loop head; `:7999` for continue block; `slang-ir-sccp.cpp:1900` | `### loop` per-statement paragraph kept; all citations re-derived; added the coverage-traced exit variant |
| 988f9b8f21e3 | fixed | `source/slang/slang-lower-to-ir.cpp:14432` (`emitMissingReturn`), `:14379` (source loc set to `decl->loc`), `:15739` (`checkForMissingReturns`, `CodeGenTarget::None`); `source/slang/slang-ir-missing-return.cpp` (`diagnoseMissingReturnForTarget`); `source/slang/slang-diagnostics.lua:5315-5333` (41009, 40025, 41010) | `### missingReturn and unreachable` now also names error 40025 for Slang 202c and later, and which targets get 41009 |
| 532e76a42314 | fixed | `source/slang/core.meta.slang:2217`, `:2232` (`__intrinsic_asm` as a statement in `__target_switch` arms); `source/slang/slang-lower-to-ir.cpp:9098-9122` (`visitIntrinsicAsmStmt` emits `kIROp_GenericAsm`, operand 0 the string) | `GenericAsm` row `AST origin` cell carries the `__intrinsic_asm "<text>";` spelling; visitor line updated to 9098 |
| ef8670879a7e | fixed | `source/slang/slang-lower-to-ir.cpp` has no `emitUnreachable` call; `source/slang/slang-ir-sccp.cpp:1880-1901` gives a use-but-no-predecessor block one `unreachable`; SCCP runs at `slang-lower-to-ir.cpp:15639`, before the dump at `:15853`. Not a compiler bug | `unreachable` row and prose attribute it to SCCP, now also listing the all-arms-return `if`/`else` and `while (true)` cases, with the SCCP line |
| fc606038962b | fixed | `source/slang/core.meta.slang:1126`, `:2215` (`__target_switch`); `source/slang/slang-lower-to-ir.cpp:8948-8959` (`visitStageSwitchStmt` uses `emitGetCurrentStage` and a `switch`), `:9048` (`visitTargetSwitchStmt`) | `switch` and `targetSwitch` rows name `__stage_switch` and `__target_switch`; visitor lines updated |
| 28bd3d6676a6 | fixed | `source/slang/slang-lower-to-ir.cpp:929` (`emitTryCallInst`), `:940` and `:8541` (`emitThrow`); `:15611` calls `lowerErrorHandling` ahead of the dump at `:15853`; `source/slang/slang-ir-insts.lua:1135-1139` (Result opcodes) | `### tryCall and throw` short-lived-opcode paragraph kept with current lines; `emitCallToVal` line corrected to 872 |
