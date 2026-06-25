---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:14:52Z
target_doc: name-resolution/visibility.md
review_report: ../../reviews/name-resolution/visibility.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 3, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for name-resolution/visibility.md

## Summary

All three review findings were verified against the source at `eb9403ef595a99c2ff6def1d538dbd7a792d9371` and fixed. F-001 corrected the API surface for the `minLanguageVersion` default field; F-002 added the prompt-required `using`/re-export edge case, naming the actual source-backed rejection path; F-003 corrected the diagnostic attribution for duplicated visibility modifiers. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `include/slang.h:5574` declares `minLanguageVersion = SLANG_LANGUAGE_VERSION_2025` inside `struct SlangGlobalSessionDesc` (line 5565); `struct SessionDesc` (line 4244) has no such field. | Renamed the cited field to `SlangGlobalSessionDesc::minLanguageVersion` in the `SlangLanguageVersion` concept bullet. |
| F-002 | fixed | Prompt line 93 requires this edge case and the doc omitted it. Verified `visitUsingDecl` (slang-check-decl.cpp:16419) only accepts a namespace argument (`ExpectedANamespace` otherwise) and does not re-export individual members; the actual less-visible-alias rejection is `validatePublicCallableOperandVisibility`'s `FuncAliasDecl` branch (slang-check-decl.cpp:9012-9037) emitting `public-custom-derivative-uses-non-exported-import` (slang-diagnostics.lua:2652, code 31162). | Added a "Re-exporting a non-exported decl through an alias" edge-case bullet stating the absence of a `using`-specific path and citing the `FuncAliasDecl` rejection path and diagnostic. |
| F-003 | fixed | Confirmed: `InvalidVisibilityModifierOnTypeOfDecl` fires for `private`/`internal` on a `NamespaceDeclBase` (slang-check-modifier.cpp:1998,2018), not for `public public`. Duplicate visibility modifiers are caught by the `VisibilityModifier` conflict group (slang-check-modifier.cpp:1500) and emit `DuplicateModifier` (slang-check-modifier.cpp:2306, `duplicate-modifier` code 31202). | Replaced the `public public ...` example with the namespace `private`/`internal` case and noted that repeated modifiers emit `duplicate-modifier` from the conflict-group check. |
