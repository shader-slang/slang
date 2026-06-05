---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: cross-cutting/core-module.md
review_report: ../../reviews/cross-cutting/core-module.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/core-module.md

## Summary

The review reported one minor finding, which was fixed. The `## Core module` provided-types sentence listed `Result`, but a search of the watched `core.meta.slang`, `hlsl.meta.slang`, `diff.meta.slang`, and `glsl.meta.slang` files at the source commit found `Result` only in comments, never as a declared type. The sentence now names only `Optional` and `Tuple`, both confirmed declared in `source/slang/core.meta.slang` (lines 1805 and 1921).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `Result` not declared in any watched meta-slang file (only in comments at `hlsl.meta.slang:12902` etc.); `Optional`/`Tuple` confirmed at `core.meta.slang:1805,1921`. | Dropped `Result` from the provided-types sentence; now reads `Optional` and `Tuple`. |
