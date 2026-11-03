---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:22:45Z
target_doc: pipeline/03-semantic-check.md
review_report: ../../reviews/pipeline/03-semantic-check.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/03-semantic-check.md

## Summary

Both findings were verified against the current source and fixed. The document stayed under its size cap by dropping a redundant sentence in the file-responsibilities intro.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-check-decl.cpp` lines 17207 and 17240: the forward helper builds `IForwardDifferentiable`; the backward helper builds `IBackwardDifferentiable` with a `kIROp_FunctionCopy` `bwd_diff` and context structs. | Split the sentence: forward helper keeps the original description, backward helper described separately. |
| F-002 | fixed | `source/slang/slang-check-shader.cpp` lines 150-200: scalars get count 1 and only counts are compared, so `float` matches `float1`. | Replaced "both scalar, or both vectors" with equal element count, scalar counting as one. |
