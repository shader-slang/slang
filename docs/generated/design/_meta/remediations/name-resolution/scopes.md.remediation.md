---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T19:00:00+00:00
target_doc: name-resolution/scopes.md
review_report: ../../reviews/name-resolution/scopes.md.review.md
target_doc_source_commit_before: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/scopes.md

## Summary

Both findings addressed by expanding the manifest's `watched_paths`
for this page to cover every checker/lookup/session file the page
already cites. The document body itself was not modified: its
existing citations now line up with the watched set.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Runbook "Manifest gaps" prescribes extending `watched_paths` when a doc cites a file outside the watched set. | Added `source/slang/slang-check-decl.cpp`, `slang-check-stmt.cpp`, `slang-check-expr.cpp`, `slang-session.cpp`, and `slang-lookup.cpp` to the `name-resolution/scopes.md` `watched_paths` in `_meta/manifest.yaml`. |
| F-002 | fixed | Same manifest-gap pattern; the sibling-scope helper lives in `slang-check-expr.cpp`. | The F-001 manifest expansion already includes `slang-check-expr.cpp`, so this finding is closed by the same edit. |
