---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T18:30:00+00:00
target_doc: cross-cutting/diagnostics.md
review_report: ../../reviews/cross-cutting/diagnostics.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/diagnostics.md

## Summary

Both findings addressed by extending the manifest's `watched_paths`
for this page, following the runbook's recommended response to
manifest-gap findings. The document body itself was not changed:
the prose at the cited sections already accurately describes the
files now in scope.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The runbook (`docs/generated/design/_meta/regenerate.md`, "Manifest gaps") prescribes extending `watched_paths` when a doc cites a file that is not covered. The `SLANG_ASSERT` discussion is genuinely sourced from `source/core/slang-common.h` and is useful guidance for diagnostics consumers. | Added `source/core/slang-common.h` to the `cross-cutting/diagnostics.md` `watched_paths` in `_meta/manifest.yaml`. After `mark-fresh`, the doc's `watched_paths_digest` changes, so this doc will surface as `review-stale` in the next review cycle (by design — the next reviewer should re-validate the assert section against the now-watched header). |
| F-002 | fixed | Same manifest-gap pattern as F-001 for `slang-rich-diagnostics.h/.cpp`. | Added `source/slang/slang-rich-diagnostics.h` and `source/slang/slang-rich-diagnostics.cpp` to the same `watched_paths` block. |
