---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T18:30:00+00:00
target_doc: cross-cutting/serialization.md
review_report: ../../reviews/cross-cutting/serialization.md.review.md
target_doc_source_commit_before: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/serialization.md

## Summary

F-001 addressed by expanding the manifest's `watched_paths` to
include the IR-type and source-loc serialize files cited in the
body. F-002 addressed by removing the unsupported alternative-
workflow claim from `## Round-trip and repro files`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The runbook ("Manifest gaps") prescribes extending `watched_paths`. The body legitimately discusses what those serialize-helper files do, so bringing them in scope is the cleanest fix. | Added `source/slang/slang-serialize-ir-types.{h,cpp}` and `source/slang/slang-serialize-source-loc.{h,cpp}` to the page's `watched_paths` in `_meta/manifest.yaml`. After `mark-fresh`, the doc's `watched_paths_digest` changes, so this doc will surface as `review-stale` next cycle. |
| F-002 | fixed | The prompt limits this section to historical (deprecated) repro handling; the "`-target slang` plus the test-server framework is the supported path" claim was beyond that scope and was not anchored in any watched file. | Reworded the second sentence of `## Round-trip and repro files` to say "any newer alternative workflow is out of scope for this page" instead of naming a specific (unsubstantiated) workflow. |
