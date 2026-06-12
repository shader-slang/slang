---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: target-pipelines/hlsl.md
review_report: ../../reviews/target-pipelines/hlsl.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/hlsl.md

## Summary

Both findings were fixed. The three entry-point-shape passes were
moved from Phase A to Phase C (after `validateAtomicOperations`,
before `floatNonUniformResourceIndex`) in the diagrams and tables,
with both phase tables renumbered. The Phase D table header was
corrected from `Pass / step` to the contract-required `Pass`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-emit.cpp:982-1001` runs only borrow/replace/bind in early region; `1952-1962` runs the three varying passes after specialization, before `floatNonUniformResourceIndex` (2033). | Moved `translateGlobalVaryingVar`/`resolveVaryingInputRef`/`fixEntryPointCallsites` out of Phase A (diagram+rows 6-8) into Phase C (new rows 3-5); renumbered Phase A (→17 rows) and Phase C (→34 rows) and updated the liveness row cross-reference. |
| F-002 | fixed | `docs/generated/design/_meta/prompts/_common.md:326-335` requires the column header `Pass`. | Renamed the Phase D table header column `Pass / step` to `Pass`. |
