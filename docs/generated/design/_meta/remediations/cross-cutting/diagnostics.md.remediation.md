---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:59:02Z
target_doc: cross-cutting/diagnostics.md
review_report: ../../reviews/cross-cutting/diagnostics.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 4
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/diagnostics.md

## Summary

All 4 findings were re-verified against the current target document and
the watched source at HEAD. Every finding quotes prose that is not in the
current document and recommends a state the document already satisfies;
the review was produced against an earlier revision of the doc. All 4 are
`rejected-bogus`. No edits were applied this cycle, so
`target_doc_source_commit_after` equals `_before`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Doc no longer claims a JSON-emitting writer / JSON schema. Current `## Source locations and message rendering` (target doc lines 188-195) already describes `DiagnosticSink::Flag::MachineReadableDiagnostics` and the TSV record `E<code>\t<severity>\t...`, ending "(this is not a JSON schema)". Flag confirmed at `source/compiler-core/slang-diagnostic-sink.h:173`. | — |
| F-002 | rejected-bogus | Doc no longer says "Every diagnostic has a unique integer id". Current `## Error codes and the name field` (target doc lines 198-214) states ids "live in a single shared integer namespace ... managed centrally", cites `getDiagnosticById` (`slang-diagnostic-sink.h:468`, "possible for multiple diagnostics to have the same id") and `intentional_shared_code_list` (`slang-diagnostics-helpers.lua:72`), and recommends name/flag over id. | — |
| F-003 | rejected-bogus | Current `## Internal-compiler errors and assertions` (target doc lines 243-251) already covers `SLANG_ASSERT_FAILURE` via `handleAssert` and `SLANG_UNREACHABLE` via `handleSignal` with `SignalType::Unreachable`, citing the `slang-signal.h` include in `slang-common.h`. Verified at `source/core/slang-signal.h:31,33` and `source/core/slang-common.h:3`. | — |
| F-004 | rejected-bogus | Current `## Diagnostic definitions in Lua` (target doc lines 156-178) already states the primary span is optional and names `cannot-deduce-source-language` as locationless, using `err(name, code, message, primary_span, ...)`. Confirmed at `source/slang/slang-diagnostics.lua:87,158`. | — |
