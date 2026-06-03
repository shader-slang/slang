---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: cross-cutting/diagnostics.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 81f47a822a99f93b8701131d76480cb13010b16d56d184d434ff385df559169e
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for cross-cutting/diagnostics.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked `Severity`, `DiagnosticInfo`, sink storage, warning-state tracker, diagnostic override declarations, internal-error macros, and diagnostic-guideline links.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Internal-compiler errors and assertions` | The page discusses `SLANG_ASSERT` environment behavior and release assert mechanics, but the watched paths do not include the assertion implementation; the prompt says to cite `source/core/slang-common.h` only if watched. | The diagnostics manifest entry excludes `source/core/slang-common.h`. | Limit this section to watched diagnostics macros such as `SLANG_INTERNAL_ERROR`, `SLANG_UNIMPLEMENTED`, and `SLANG_DIAGNOSE_UNEXPECTED`, or expand watched paths. |
| F-002 | minor | `## Diagnostic definitions in Lua` | The page cites `slang-rich-diagnostics.h/.cpp`, but those files are not in the manifest watched paths. | The diagnostics manifest entry includes `slang-diagnostics.*`, `slang-diagnostics.lua`, and `diagnostics/*.lua`, not rich-diagnostic implementation files. | Add those files to watched paths or anchor the description to watched Lua and `slang-diagnostics.h`. |
