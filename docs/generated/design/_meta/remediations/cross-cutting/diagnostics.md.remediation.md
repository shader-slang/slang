---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:33:22Z
target_doc: cross-cutting/diagnostics.md
review_report: ../../reviews/cross-cutting/diagnostics.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for cross-cutting/diagnostics.md

## Summary

Two findings were verified against current source and fixed in the document: the unknown-exception output text (F-001) and the attribution of the final processing call (F-002). The size finding (F-003) is valid but needs a broad condensation rewrite, so it was deferred. Breakdown: 2 fixed, 0 rejected, 1 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/compiler-core/slang-diagnostic-sink.cpp:985` passes the bare string; `diagnoseRaw` (lines 800-828) writes it verbatim and uses `Fatal` only to abort. | Unknown-exception route now says output is the bare `An unknown exception occurred`, no `fatal error:` prefix. |
| F-002 | fixed | `source/slang/slang-diagnostics.lua:6750-6756` makes the call, checks errors, returns the list; the helpers file only exports `process_diagnostics` (line 1135). | Step 2 attributes the call, validation check and return to `slang-diagnostics.lua`. |
| F-003 | deferred | Valid (prompt line 54 says under 24 KB; doc is about 31 KB, under the 40,960-byte manifest cap). Meeting it needs a multi-section condensation, beyond a minimal edit. | — |
