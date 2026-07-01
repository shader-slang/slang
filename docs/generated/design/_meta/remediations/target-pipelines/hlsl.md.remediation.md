---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:06:52Z
target_doc: target-pipelines/hlsl.md
review_report: ../../reviews/target-pipelines/hlsl.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 2
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for target-pipelines/hlsl.md

## Summary
Three findings, no target-doc edits this cycle. F-001 and F-003 are rejected as bogus: the current document already contains the table row and the line numbers the review claims are missing or stale (the review appears to have been written against an earlier draft). F-002 is deferred: its Source-section concern is already satisfied in the current doc, and the only actionable remainder is a manifest `watched_paths` expansion, which is outside the target-doc-only edit scope. Front-matter is unchanged; `target_doc_source_commit_after` equals `_before`.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | `checkStaticAssert` is already present in the Phase B ordered table as row 66 (target doc line 307: gate `(always)`, note "Direct call (not `SLANG_PASS`) at line ~1795"), matching `source/slang/slang-emit.cpp:1795`. Diagram node `cSA` (doc line 235 chain `...sAP --> cSA --> wSBoM`) and the table row agree; no mismatch. | — |
| F-002 | deferred | The Source section already lists `slang-ir-wrap-structured-buffers.cpp` (target doc lines 47-48), so the documented half of the finding does not apply. The remaining valid part — adding the file to the page manifest `watched_paths` (confirmed absent via `regenerate.py show`) — is a manifest change outside the target-doc-only edit scope; defer to a manifest-update cycle so future changes to that HLSL-only pass stale the page. | — |
| F-003 | rejected-bogus | The current doc already cites the live source lines, not the stale ~2616/~2752 the review reports. Doc line 481/517 say "line ~2630" and line 522 says "line ~2766", matching `source/slang/slang-emit.cpp:2630` (`new HLSLSourceEmitter`) and `:2766` (`createArtifactForCompileTarget`). | — |
