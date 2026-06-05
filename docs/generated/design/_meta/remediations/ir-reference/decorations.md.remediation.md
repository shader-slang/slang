---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/decorations.md
review_report: ../../reviews/ir-reference/decorations.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/decorations.md

## Summary

One major finding was fixed and no findings were rejected, deferred, or
escalated. The `## Notable opcodes` section was missing the
prompt-required callout for the control-flow hint decorations, so a
short subsection was added covering `branch`, `flatten`, and
`loopControl`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `docs/generated/design/_meta/prompts/ir-reference-decorations.md:68-69` requires a `branch` / `flatten` / `loopControl` notable callout; it was absent from `## Notable opcodes`. The opcodes are defined at `source/slang/slang-ir-insts.lua:1636-1642`. | Added a `### branch / flatten / loopControl` notable subsection before `### KeepAliveDecoration`, describing them as backend-consumed control-flow hints. |
