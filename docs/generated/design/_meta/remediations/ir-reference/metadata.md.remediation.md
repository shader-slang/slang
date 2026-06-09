---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/metadata.md
review_report: ../../reviews/ir-reference/metadata.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/metadata.md

## Summary

Both findings were fixed; none were rejected, deferred, or escalated.
A debug-info row now cites its introducing IR pass, and the
`## Notable opcodes` section gained the two prompt-required callouts
that were missing.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `ir-reference-metadata.md:64` requires at least one debug-info row to cite the introducing pass; `DebugValue` is created by `emitDebugValue` at `source/slang/slang-ir-insert-debug-value-store.cpp:175,212`. | Changed the `DebugValue` row `AST origin` cell from `(synthesized)` to cite `emitDebugValue` in `slang-ir-insert-debug-value-store.cpp`. |
| F-002 | fixed | `ir-reference-metadata.md:42-46` requires `Layout` and `DebugScope` notable coverage; both were absent. `Layout` parent at `source/slang/slang-ir-insts.lua:2652`; `DebugScope` at `:2767` with operands `scope, inlinedAt` per `source/slang/slang-ir-insts.h:2664-2669`. | Added `### Layout` (parent, relation to `LayoutDecoration`) and `### DebugScope` (operand pair, scope nesting) notable callouts. |
