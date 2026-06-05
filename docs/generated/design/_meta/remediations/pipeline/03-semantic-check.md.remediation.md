---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: pipeline/03-semantic-check.md
review_report: ../../reviews/pipeline/03-semantic-check.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/03-semantic-check.md

## Summary

Both findings were fixed. F-001 added the prompt-required
`slang-ast-decl-ref.cpp` citation to the `DeclRef` section. F-002
removed the `slang-check-out-of-bound-access.cpp` row, which the
source implements as an `InstPassBase` IR pass invoked via
`SLANG_PASS` from `slang-emit.cpp`, not as a `SemanticsVisitor`
checker (and which is not among the doc's watched paths).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Per-doc prompt `pipeline-03-semantic-check.md:38-41` requires the section to cite `slang-ast-decl-ref.cpp`; the file resolves at HEAD. | Added a sentence linking `source/slang/slang-ast-decl-ref.cpp` and naming the `DeclRefBase` operations it owns. |
| F-002 | fixed | `source/slang/slang-check-out-of-bound-access.cpp:12` declares `OutOfBoundAccessChecker : public InstPassBase`; `source/slang/slang-emit.cpp:1380` invokes it via `SLANG_PASS(checkForOutOfBoundAccess, sink)`; the file is not in the doc's watched paths. | Deleted the out-of-bound-access row from the file-responsibility table. |
