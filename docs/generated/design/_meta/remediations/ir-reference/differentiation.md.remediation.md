---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T17:30:00+00:00
target_doc: ir-reference/differentiation.md
review_report: ../../reviews/ir-reference/differentiation.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for differentiation.md

## Summary

Two major findings addressed: a new "Autodiff temporaries"
sub-table now lists the four backward-autodiff placeholder opcodes,
and `DiffTypeInfo` is now documented here. The `misc.md`
remediation in this same cycle removes its prior row from that
page.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-insts.lua:1110-1121` defines `LoadReverseGradient`, `ReverseGradientDiffPairRef`, `PrimalParamRef`, and `DiffParamRef` as backward-autodiff placeholders; the page already explains the splitting / back-prop flow and these are first-class entries in that flow. | Added `### Autodiff temporaries` between "Synthesized derivative witnesses" and "Checkpointing and rematerialization", with rows for all four opcodes. |
| F-002 | fixed | `source/slang/slang-ir-insts.lua:1006-1010` annotates `DiffTypeInfo` as "witness tables for differential type info", which is squarely differential metadata; the row in `misc.md` was incidental. | Added a one-row `### Differential type info` sub-table for `DiffTypeInfo`; the paired `misc.md` remediation removes the duplicate. |
