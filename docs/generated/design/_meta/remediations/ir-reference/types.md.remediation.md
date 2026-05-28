---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T17:30:00+00:00
target_doc: ir-reference/types.md
review_report: ../../reviews/ir-reference/types.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/types.md

## Summary

One major finding addressed by removing the duplicate `interface`
row. The canonical row remains under `### Existentials and
interfaces` and already cross-links to `structure.md` for the
container role.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The Lua source at `source/slang/slang-ir-insts.lua:656` has a single concrete `interface` opcode; the canonical row at line 210 already cross-links to `structure.md` for the container role. | Removed the duplicate `interface` row from `### Struct, class, interface containers`; renamed the heading to `### Struct and class containers` and noted the cross-link to the existentials section for `interface`. |
