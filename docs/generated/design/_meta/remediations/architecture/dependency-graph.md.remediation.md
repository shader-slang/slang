---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:00:00Z
target_doc: architecture/dependency-graph.md
review_report: ../../reviews/architecture/dependency-graph.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for architecture/dependency-graph.md

## Summary

Both findings were verified against the current source and fixed with minimal edits to the target document. F-001 corrected the description of `prelude` from an include-only dependency to an object library of generated embeddings. F-002 removed the unsupported `CLAUDE.md` attribution and cited the public include directory in the build file.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `prelude/CMakeLists.txt` lines 9-29 generate `.cpp` via slang-embed and declare an `OBJECT` target; `source/slang/CMakeLists.txt` line 306 lists `prelude` under `LINK_WITH_PRIVATE`. | Edge-citations row for `slang`: now describes `prelude` as a privately linked object library of generated prelude embeddings, citing `prelude/CMakeLists.txt`. |
| F-002 | fixed | `CLAUDE.md` lines 456-459 only require ABI and source compatibility; `source/slang/CMakeLists.txt` line 315 sets `INCLUDE_DIRECTORIES_PUBLIC ${slang_SOURCE_DIR}/include`. | Notable invariants bullet: dropped the `CLAUDE.md` attribution, cited line 315 as the observed include boundary, and noted the build does not enforce the rule. |
