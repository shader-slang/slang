---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: pipeline/04b-pre-link-passes.md
review_report: ../../reviews/pipeline/04b-pre-link-passes.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04b-pre-link-passes.md

## Summary

Both findings were fixed. F-001 rewrote the early-inlining loop prose
to describe the real `changed`-flag flow: the
`performMandatoryEarlyInlining` result is overwritten by
`peepholeOptimizeGlobalScope` before the inner cluster OR-assigns and
the final break test runs. F-002 renamed the non-contract `Call`
column header to `Pass` in all four phase tables.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-lower-to-ir.cpp:14957` sets `changed` from inlining, `:14960` overwrites it from `peepholeOptimizeGlobalScope`, and `:14977-14978` breaks on the final value; the prose implied the inlining result was preserved. | Replaced the loop paragraph to state the overwrite, the OR-assigning inner cluster (and that `eliminateDeadCode` is not OR-ed), and the final break condition. |
| F-002 | fixed | `docs/generated/design/_meta/prompts/_common.md:444` requires the ordered table columns `# / Pass / File / Gate / Notes`. | Renamed the second column header from `Call` to `Pass` in all four phase tables. |
