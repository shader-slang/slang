---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/misc.md
review_report: ../../reviews/ir-reference/misc.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/misc.md

## Summary

The single finding was fixed; none were rejected, deferred, or
escalated. The two concrete children of `ForceVarIntoStructTemporarilyBase`
were missing from the catch-all tables and have been added.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `ir-reference-misc.md:16` names `ForceVarIntoStructTemporarilyBase` as a typical misc inhabitant; its children at `source/slang/slang-ir-insts.lua:1541-1548` were absent and no sibling page lists them. Behavior verified at `source/slang/slang-ir-hlsl-legalize.cpp:62-130`. | Added a `### Variable struct-wrapping legalization` sub-table with rows for `ForceVarIntoStructTemporarily` and `ForceVarIntoRayPayloadStructTemporarily`. |
