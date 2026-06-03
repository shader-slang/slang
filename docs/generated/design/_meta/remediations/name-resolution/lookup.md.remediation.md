---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T19:00:00+00:00
target_doc: name-resolution/lookup.md
review_report: ../../reviews/name-resolution/lookup.md.review.md
target_doc_source_commit_before: 5b3ff4ef03c6a7b8c4e1a98a16d18ae2b66b8c5e
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/lookup.md

## Summary

Both findings addressed: the manifest watched-paths gap is closed,
and the missing-information finding about lookup-level deduplication
is documented in a new sub-section under "Container-level overload
accumulation".

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The page documents `AddToLookupResult` behavior; the absence of any dedupe step is a load-bearing fact that callers (overload resolution, visibility filtering) rely on. | Added a new `#### Deduplication: there isn't any at the LookupResult level` block under `### Container-level overload accumulation`, with code citations to `AddToLookupResult` lines 95-125 and pointers to overload-resolution and visibility for the downstream handling. |
| F-002 | fixed | Runbook "Manifest gaps" pattern: the doc legitimately cites these files but they were absent from `watched_paths`. | Added `source/slang/slang-check-decl.cpp`, `slang-check-stmt.cpp`, `slang-check-expr.cpp`, and `include/slang.h` to the `name-resolution/lookup.md` `watched_paths` in `_meta/manifest.yaml`. |
