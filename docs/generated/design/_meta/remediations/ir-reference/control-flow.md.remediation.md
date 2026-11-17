---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:10:08Z
target_doc: ir-reference/control-flow.md
review_report: ../../reviews/ir-reference/control-flow.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for ir-reference/control-flow.md

## Summary

Five findings were verified against the current source and fixed in the doc (F-001, F-002, F-003, F-004, F-006); one (F-005) was deferred. The doc shrank from 32,619 to about 31,990 bytes, well under the 65,536 cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-lower-to-ir.cpp` lines 15641-15647 and 15701 gate `simplifyCFG` on `!minimumOptimizations`. | Reworded the pass-sequence paragraph and the `ifElse` empty-arm paragraph as conditional. |
| F-002 | fixed | The line-14379 scope covers only coverage instrumentation; `lowerDecl` (line 14834) sets the declaration location and `lowerStmt` (line 9719) restores it. | Replaced the line-14379 explanation in the `missingReturn` callout. |
| F-003 | fixed | Verified 20 hand-written wrappers and 6 generated ones in `slang-ir-insts.h`; unnamed Lua operands have accessors there. | Added `‡` markers, a count and legend, `†` accessor-derived names, `(variadic)`, and `(3 unnamed)` for `gpuForeach`. |
| F-004 | fixed | `slang-ir.cpp` line 6521 constructs `conditionalBranch`; `createBlock`/`emitBlock` are at lines 5457/5488; params come from `lowerFuncDeclInContext` (line 14047). | Replaced `(synthesized)`, and named producers in the `block` and `param` rows. |
| F-005 | deferred | Regrouping into four headings would remove headings (`#terminators-switches`, `#terminators-unconditional-branches`, etc.) that `docs/generated/tests/design/ir-reference/control-flow/*.slang` reference via `doc_ref`. The rows are already grouped, only more finely. | Needs a coordinated update of those test anchors in a later cycle. — |
| F-006 | fixed | The cache and simplification-iteration detail is pass-level material outside the opcode scope; the `mightHaveSideEffects` claim for terminators is verified. | Cut the cache paragraphs and pointed to `../pipeline/05-ir-passes.md`. |
