---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T19:00:00+00:00
target_doc: name-resolution/visibility.md
review_report: ../../reviews/name-resolution/visibility.md.review.md
target_doc_source_commit_before: a0f1b39ad14e7e4d4fdf3a07c98e3d76aae0ad1f
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/visibility.md

## Summary

All three findings addressed: the default-language-version statement
is clarified to distinguish `SLANG_LANGUAGE_VERSION_DEFAULT` (the
legacy enum value, still 2018) from `SessionDesc::minLanguageVersion`
(defaulted to 2025 in the API), and both manifest watched-paths
findings (F-001 and F-003 cover the same gap from different angles)
are closed by the same manifest edit.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Runbook "Manifest gaps" pattern: `slang-check-expr.cpp` holds the visibility filter the doc relies on, and it was not watched. | Added `source/slang/slang-check-decl.cpp`, `slang-check-modifier.cpp`, `slang-check-expr.cpp`, `slang-ast-support-types.h`, `slang-lookup.cpp`, `slang-diagnostics.lua`, and `include/slang.h` to the `name-resolution/visibility.md` `watched_paths` in `_meta/manifest.yaml`. |
| F-002 | fixed | The reviewer correctly flagged that `SLANG_LANGUAGE_VERSION_DEFAULT` is still 2018 in the public enum; what defaults to 2025 is `SessionDesc::minLanguageVersion`. | Rewrote the "Default language version" paragraph to make this distinction explicit, citing both `include/slang.h` and `slang-session.cpp`. |
| F-003 | fixed | Same manifest-gap pattern as F-001, listing the additional cited-but-unwatched files. | The F-001 manifest expansion already adds `slang-ast-support-types.h`, `slang-check-expr.cpp`, `slang-lookup.cpp`, `slang-diagnostics.lua`, and `include/slang.h`, so this finding is closed by the same edit. |
