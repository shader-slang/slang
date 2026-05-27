---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T19:30:00+00:00
target_doc: pipeline/overview.md
review_report: ../../reviews/pipeline/overview.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/overview.md

## Summary

All three findings addressed: the inflated pass-file count is
corrected, the `EndToEndCompileRequest` declaration reference is
fixed, and `slang-compile-request.cpp` orchestration is now covered
with a description of `checkAllTranslationUnits` /
`checkTranslationUnit`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The watched glob resolves to ~161 files, not ~300; the inflated count would be a misleading guide to reviewers. | Updated the "IR passes" paragraph to read "roughly 160 `slang-ir-*.cpp` files". |
| F-002 | fixed | The class is declared in `slang-end-to-end-request.h`; the `.cpp` only contains the implementation. | Split the "Driver entry points" bullet so it cites the header for the declaration and the `.cpp` for the implementation. |
| F-003 | fixed | The prompt requires orchestration coverage from `slang-compile-request.cpp`; the section linked only the header. | Expanded the first "Driver entry points" bullet to cite `slang-compile-request.cpp`, naming the `checkAllTranslationUnits` / `checkTranslationUnit` orchestration loop and pointing at line 513. |
