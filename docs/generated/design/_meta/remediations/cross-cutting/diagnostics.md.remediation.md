---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T13:37:00Z
target_doc: cross-cutting/diagnostics.md
review_report: ../../reviews/cross-cutting/diagnostics.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for cross-cutting/diagnostics.md

## Summary

Four of the five findings were verified against source at
`53b76e6d3009b8e6434d41573524c7ce5c499d23` and fixed: the critical
`extra`-group advice in the add-diagnostic checklist, the two stale "not in
this page's watched paths" statements, the over-broad debug-note claim for
`SLANG_DIAGNOSE_UNEXPECTED`, and the two assertion-macro inaccuracies. The
remaining finding asks for a manifest `watched_paths` expansion, which
remediation may not perform, so it is deferred with the follow-up recorded.
Nothing was rejected.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `source/compiler-core/slang-diagnostic-sink.h` initializes `m_enabledWarningLevels` to `uint32_t(1) << uint32_t(WarningLevel::Extra)`, and its adjacent comment names `pedantic` as the group for a warning that should stay silent until the user opts in. Listing `extra` as an off-by-default option was therefore wrong. | `## Adding a new diagnostic` step 5: dropped `extra` from the off-by-default list, kept `all` / `pedantic`, and added one clause explaining that `Extra` is on by default. |
| F-002 | fixed | Confirmed: `regenerate.py show cross-cutting/diagnostics.md` resolves `include/slang.h`, `source/slang/slang-options.cpp`, and `source/slang/slang-compiler-options.cpp`, so both "not watched" claims were false. The digest half of the finding is the operator's `mark-fresh` responsibility per `_remediate.md` lines 97-100 and was not hand-edited. | `## Warning groups` and `## What is not in this document`: deleted both stale "not watched" statements and turned the bare filenames into resolving links. |
| F-003 | fixed | Confirmed: `source/slang/slang-diagnostics.h` defines `SLANG_INTERNAL_ERROR` and `SLANG_UNIMPLEMENTED` inside `#ifdef _DEBUG` with a `diagnoseRaw` note, and defines `SLANG_DIAGNOSE_UNEXPECTED` after the `#endif` with no note. | `## Internal-compiler errors`: named the two note-emitting macros and excluded `SLANG_DIAGNOSE_UNEXPECTED` explicitly. |
| F-004 | fixed | Confirmed: `source/core/slang-common.h` expands `SLANG_ASSERT` to a `handleAssert` call only under `_DEBUG` and to `SLANG_ASSUME(VALUE)` otherwise, while `SLANG_RELEASE_ASSERT` calls `handleAssert` unconditionally; `SLANG_ASSERT_FAILURE` is defined independently in `source/core/slang-signal.h` and is not what an assert expands to. | `## Internal-compiler errors`: rewrote the assertion sentence to separate debug from release `SLANG_ASSERT`, state that `SLANG_RELEASE_ASSERT` always calls `handleAssert`, and describe `SLANG_ASSERT_FAILURE` as a separate macro. |
| F-005 | deferred | Confirmed: the TSV record is built by `renderDiagnosticMachineReadable` in `source/compiler-core/slang-rich-diagnostics-render.cpp`, which `regenerate.py show` does not resolve for this page. The only correct fix is to add `source/compiler-core/slang-rich-diagnostics-render.{h,cpp}` to the manifest entry and then link the implementation; `_remediate.md` lines 92-94 forbid remediation from editing the manifest, and `_remediate.md` lines 72-78 name a watched-paths expansion as a deferral case. Follow-up: operator adds the two paths, then a regeneration cycle links the renderer where the TSV schema is stated. | — |
