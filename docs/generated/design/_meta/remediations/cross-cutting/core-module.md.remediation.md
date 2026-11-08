---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:35:20Z
target_doc: cross-cutting/core-module.md
review_report: ../../reviews/cross-cutting/core-module.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/core-module.md

## Summary

Both findings were verified against the current source and fixed in the document. The numerics artifact description now names two public entry points and two adapters, and the import-graph claim is replaced by the accurate dependency direction. The document was condensed from 28,821 to under 24,000 bytes. Breakdown: 2 fixed, 0 rejected, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: source/standard-modules/numerics/differentiable-wrappers.slang:11-14 and CMakeLists.txt:59-63 show differentiable is client-facing and public modules import the adapters. | Standard modules section: two public entry points plus two adapters; nesting does not imply privacy; adapters do not import the public modules. |
| F-002 | fixed | Confirmed: prompts/cross-cutting-core-module.md:53 requires under 24 KB; doc was 28,821 bytes. | Condensed build/cache prose and file inventories; all meta-Slang files, prelude headers and required sections retained. |
