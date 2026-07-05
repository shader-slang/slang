---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:04:47Z
target_doc: name-resolution/visibility.md
review_report: ../../reviews/name-resolution/visibility.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 2
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/visibility.md

## Summary

Both findings were verified against the watched sources and rejected as bogus: the prose each finding quotes as the document's "current" text is not present in the target document at `source_commit` c21ead2, and the existing text already states the source-backed position the reviewer recommends. The reviewer appears to have evaluated an earlier revision. No edits were made, so `target_doc_source_commit_after` equals `_before`. The action counts (2 rejected-bogus) sum to the review's finding_count of 2.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The quoted claim ("New sessions therefore reject modules whose declared version is older than 2025 by default") is absent (verified by grep for "reject" near the language-version section). Lines 72-77 already state `SlangGlobalSessionDesc::minLanguageVersion` defaults to `SLANG_LANGUAGE_VERSION_2025` and "records a preferred floor and is not consulted by the visibility-classification path in the watched sources" — the source-backed framing F-001 recommends. Consistent with `include/slang.h:5654-5655` (comment: "oldest Slang language version that any sessions will use"). The cited counter-evidence files `slang-compiler.cpp`/`slang-preprocessor.cpp` are also outside this page's `watched_paths`. | — |
| F-002 | rejected-bogus | The quoted framing ("constructed without an explicit visibility modifier and inherit the parent's default") is absent (verified by grep). Lines 254-278 already state synthesized members are "assigned a visibility at their synthesis site rather than relying on the module default," citing `addVisibilityModifier(synthesized, Math::Min(parentVisibility, requirementVisibility))` at `source/slang/slang-check-decl.cpp` 7442-7451, 8523-8531, 8902-8910, and `addVisibilityModifier(decl, getDeclVisibility(parent))` at lines 3873-3878, 3933-3937, plus `slang-check-expr.cpp` line 817. All cited sites verified accurate against source — this is exactly the rewrite F-002 asks for. | — |
