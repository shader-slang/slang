---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:22:30Z
target_doc: pipeline/05-ir-passes.md
review_report: ../../reviews/pipeline/05-ir-passes.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/05-ir-passes.md

## Summary

All five findings were verified against the current source and fixed in the document. The document was also trimmed from 67,257 to about 62,000 bytes while adding the Purpose text, so it is under 64 KB.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/slang-emit.cpp:2653-2657 runs lowerCopyLogical for GLSL; slang-ir-spirv-legalize.cpp:2985-2994 for SPIR-V before 1.4 | Copy-logical row now names both call sites |
| F-002 | fixed | source/slang/slang-lower-to-ir.cpp:15846-15855 dumps a LOWER-TO-IR snapshot | Ordering section now says plain calls lack headers but appear in LOWER-TO-IR |
| F-003 | fixed | slang-ir-any-value-marshalling.cpp:78-89 and 1325-1336 key on type and size | Row now says one pair per (type, size) combination |
| F-004 | fixed | Prompt requires a Pass / File / Purpose table | Target-specific table renamed to Purpose with a behavior description per row, target kept as prefix |
| F-005 | fixed | Prompt limit is 64 KB | Condensed coverage, AnyValue inference, validate, global-values, CUDA builtin, defer-load and metadata cells |
