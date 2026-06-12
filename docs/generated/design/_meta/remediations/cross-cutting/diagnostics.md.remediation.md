---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: cross-cutting/diagnostics.md
review_report: ../../reviews/cross-cutting/diagnostics.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/diagnostics.md

## Summary

The review reported one minor source-alignment finding, which was fixed. The assertion paragraph claimed a release-assert "typically calls into the sink before terminating," but `SLANG_RELEASE_ASSERT` at `source/core/slang-common.h:375-380` expands to `::Slang::handleAssert` with no `DiagnosticSink` argument. The sentence was rewritten to state the assert macros route through `handleAssert` and to point at the genuine sink-based internal-error macros instead.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `SLANG_RELEASE_ASSERT` (`source/core/slang-common.h:375`) calls `::Slang::handleAssert` (line 379) with no sink; sink path is the `SLANG_INTERNAL_ERROR` family per `source/slang/slang-diagnostics.h:43`. | Replaced the "calls into the sink before terminating" clause with a `handleAssert`-routing statement and a pointer to the sink-based internal-error macros. |
