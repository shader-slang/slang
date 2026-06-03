---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T17:30:00+00:00
target_doc: ir-reference/structure.md
review_report: ../../reviews/ir-reference/structure.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/structure.md

## Summary

One major finding addressed by renaming three Opcode cells to match
the Lua entry names exactly.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Lua entries at `source/slang/slang-ir-insts.lua:640-656` are `struct`, `class`, and `interface`; the C++ wrapper names belong in the wrapper column only. | Changed the three Opcode cells to `struct`, `class`, `interface` (lines 110, 111, 124); wrapper column unchanged. |
