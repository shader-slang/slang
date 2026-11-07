---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:35:29Z
target_doc: ir-reference/resources-and-atomics.md
review_report: ../../reviews/ir-reference/resources-and-atomics.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 10
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/resources-and-atomics.md

## Summary

All ten findings were verified against the current source and found valid, so all ten were fixed in the document. Net edits shrank the page from 65,434 to about 63,000 bytes (cap 65,536). Front-matter `generated_at` was refreshed; `source_commit` and `watched_paths_digest` are unchanged because the digest command returns the same value.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir.h.lua` line 75 and `slang-ir-insts.h` lines 3175-3181 both guard optional accessors | Replaced the guarded/unguarded claim; noted only manual `IRImageStore::getAuxCoord1()` is unguarded |
| F-002 | fixed | `hlsl.meta.slang` lines 34303-34323, 3462, 28547-28567 give public callers | Replaced the blanket no-public-wrapper paragraph with the actual callers; dropped the lowering-snapshot claim |
| F-003 | fixed | `core.meta.slang` line 2832 produces `MetalSetPrimitive`; emitted by `slang-emit-metal.cpp` | Removed "never emitted"; stated the producer |
| F-004 | fixed | `slang-ir-insts.h` lines 1798-1810: layout is referenced by `IRLayoutDecoration` operand 0; entry-point layout attaches to the function | Rewrote the global_param paragraph and the See-also layout bullet |
| F-005 | fixed | `slang-ir-legalize-image-subscript.cpp` lines 195-205 visit only Store/SwizzledStore; removal needs no uses | Reworded the table cell and the Notable-opcodes paragraph |
| F-006 | fixed | `hlsl.meta.slang` 6353-6379 has no atomic load/store; `core.meta.slang` 4192-4197 does | Rewrote the producer-surfaces lead-in |
| F-007 | fixed | `decorations.md` line 409 heading yields a double-hyphen fragment | Corrected the See-also link fragment |
| F-008 | fixed | `_common.md` line 272 requires `(N unnamed)` for unnamed operands | Converted all-bracket rows to `(N unnamed)`; kept `[order]` for undeclared atomic operands with a legend; buffer-store row now shows Lua order |
| F-009 | fixed | 20 bold wrappers confirmed in the tables | Added "20 of 91 hand-written" sentence |
| F-010 | fixed | Prompt forbids target-specific lowering, code blocks in Notable opcodes, schema duplication | Condensed descriptor-heap prose, turned the InterlockedAdd block into prose, removed the fixedArgCount section |
