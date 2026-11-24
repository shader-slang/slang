---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: ir-reference/metadata.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 1
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/metadata.md

## Summary

One gap was open, `11e94ebd0a8a` (`DebugScope` / `DebugNoScope` operands and
producers), and it is `fixed`: the breakdown is one `fixed` and nothing
deferred, rejected or escalated. The observation is correct and the source
agrees with it, so this was a documentation error rather than a compiler
bug. The document was refreshed against the current source in the same pass,
which is what removed the stale `DebugNoScope` claims; the `DebugScope` row
and callout now describe the optional second operand and the caller-scope
restore the inliner emits.

## Actions

| Gap ID       | Action | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          | Fix summary                                                                                                                                                                                                       |
| ------------ | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 11e94ebd0a8a | fixed  | Confirmed in watched source. `source/slang/slang-ir-insts.lua:3044` declares `DebugNoScope` with `min_operands = 0` and `:3040-3043` declares `DebugScope` with `min_operands = 1`. `source/slang/slang-ir-insts.h:2823-2830` gives `IRDebugScope` `getScope()` and a `getInlinedAt()` that returns null unless there are 2 operands, and `:2832-2836` gives `IRDebugNoScope` no accessor. `source/slang/slang-ir.cpp:3711-3721` builds `DebugScope` with 1 or 2 operands and `emitDebugNoScope` with 0, and no other file calls `emitDebugNoScope`. `source/slang/slang-ir-inline.cpp:378` emits the caller-scope restore with `emitDebugScope(callerScope, callDebugInlinedAt)`; `:346` only reads an existing `DebugNoScope`. | `DebugScope` operands now `scope, inlinedAt?`; `DebugNoScope` marked no producer at HEAD and dropped from the inliner and `-g1` lists; added `DebugLexicalBlock` and `DebugScope` / `DebugNoScope` callouts |
