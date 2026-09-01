---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:02:00Z
target_doc: ir-reference/control-flow.md
review_report: ../../reviews/ir-reference/control-flow.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/control-flow.md

## Summary

All three findings were verified against the source and fixed; none
was rejected, deferred, or escalated. The critical finding was
confirmed: the page inverted the soundness precondition recorded at
`source/slang/slang-ir-dce.h:27-29`, so the stale-entry claim was
replaced with the real lifetime rule. The other two fixes name the
actual lowering visitors in the opcode tables and give `emitBlock`
its own line citation.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                                                                                                                                                                                             | Fix summary                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| ---------- | ------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | Confirmed. `source/slang/slang-ir-dce.h:27-29` states sharing is sound _only while_ no pass adds an `IRAnnotation` or removes a purity decoration; the page had turned that precondition into a claim that such staleness is conservative. `source/slang/slang-ir-util.cpp:1698-1709` caches unconditionally and `:1684-1693` scans annotation users, so a stale `false` is unsafe, not conservative. | Rewrote the closing sentence of `### Moving and deleting control-flow instructions` as the lifetime rule, citing `slang-ir-util.cpp` 1670-1709, the `addAnnotation` warning at `slang-ir-insts.h` 3620-3623, and the per-iteration `clear()` at `slang-ir-ssa-simplification.cpp` line 79.                                                                                                                                                                                                                                                                                              |
| F-002      | fixed  | Confirmed. No AST-origin cell named a `visit*` function, and cells cited `slang-lower-to-ir.cpp` as backticked prose, which `_common.md:43-45` forbids. Subtables were kept as-is (regrouping would contradict the settled per-family layout); instead each subtable now names a real producer.                                                                                                       | Replaced the bare filename prose in 13 AST-origin cells with the verified visitor and line number (`visitReturnStmt` 8831, `visitExpandExpr` 6565, `visitBreakStmt` 9059, `visitContinueStmt` 9077, `visitForStmt` 8410, `visitIfStmt` 8280, `visitSwitchStmt` 9495, `visitTargetSwitchStmt` 9425, `visitThrowStmt` 8942, `visitTryExpr` 8020, `visitDeferStmt` 8919, `visitIntrinsicAsmStmt` 9470, `visitDiscardStmt` 9053, `visitGpuForeachStmt` 8739, `visitSelectExpr` 7989 / `startBlock` 8193). The no-continuation subtable keeps `lowerFuncDeclInContext`, its actual producer. |
| F-003      | fixed  | Confirmed. `source/slang/slang-ir.cpp:5447` defines `IRBuilder::emitBlock`, outside the cited 6331-6560 range.                                                                                                                                                                                                                                                                                        | `## Source`: split `emitBlock` out with its own line 5447 citation; the branch emitters keep the 6331-6560 range.                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
