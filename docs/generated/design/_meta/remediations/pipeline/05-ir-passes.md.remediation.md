---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:30:00Z
target_doc: pipeline/05-ir-passes.md
review_report: ../../reviews/pipeline/05-ir-passes.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/05-ir-passes.md

## Summary

All three findings (two major, one minor) were verified against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23` and all three were fixed. The stale watched-path note now mentions only the one path that is genuinely unwatched, and four pass-table purpose cells were rewritten to match the APIs their headers declare. Nothing was rejected, deferred, or escalated; the document was edited.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                                                                                                                                                          | Fix summary                                                                                                                                               |
| ---------- | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | `docs/generated/design/_meta/manifest.yaml` lines 196-207 list `source/slang/slang-emit-spirv.cpp` among this page's watched paths, and the `SPIR-V legalize` row is present in the target-specific table. `source/slang/slang-check-out-of-bound-access.cpp` is still absent from the manifest, so that half of the note remains actionable.                      | `## How the passes are ordered` note: dropped the SPIR-V clause, kept only the `slang-check-out-of-bound-access.cpp` gap.                                 |
| F-002      | fixed  | `source/slang/slang-ir-specialize-arrays.h:9-20` documents `specializeArrayParameters` as specializing calls to functions taking `struct` parameters with array fields; `source/slang/slang-ir-defunctionalization.h:1-16` is labelled "Aspirational filename" and declares only `specializeHigherOrderParameters`, which rewrites calls passing global functions. | `Specialize arrays` purpose rewritten; the `Defunctionalization` row renamed to `Specialize higher-order parameters` with the tagged-union claim removed. |
| F-003      | fixed  | `source/slang/slang-ir-late-require-capability.h:12-18` says the pass processes and eliminates `LateRequireCapability` insts and diagnoses missing capabilities under `-restrictive-capability-check`; `source/slang/slang-ir-string-hash.h:13-25` declares the hashed-string-literal pool helpers plus `checkGetStringHashInsts`.                                 | `Late require capability` and `String hash` purpose cells rewritten to the declared behavior.                                                             |
