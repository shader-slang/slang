---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T12:00:00Z
target_doc: name-resolution/lookup.md
review_report: ../../reviews/name-resolution/lookup.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/lookup.md

## Summary

All seven findings were verified against the current source and found valid; all were fixed with minimal edits to the document. Front-matter `generated_at` and `watched_paths_digest` were refreshed.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-lookup.cpp` 420-429: `IgnoreInheritance` is an `else if` after the interface test. | Option description and facet-filter bullet now say interface facets are governed only by `IgnoreBaseInterfaces`. |
| F-002 | fixed | `slang-lookup.cpp` 299-301 can add `Completion`; 197-212 lacks the `declToExclude` test present at 230-231. | Entry point "initially supplies None"; `declToExclude` statement limited to named lookup. |
| F-003 | fixed | `LookupRequest` in `slang-ast-support-types.h` 1585-1605 has no `name` field. | `request.name` replaced with "the `name` argument". |
| F-004 | fixed | `slang-check-expr.cpp` 1428-1435 diagnoses before the language-server return. | Both modes diagnose; language-server returns the unfiltered result afterwards. |
| F-005 | fixed | `slang-lookup.cpp` 831 loops until `scope` is null; module scope can have an outer parent. | "Terminates at the module root" replaced with parent-chain-reaches-null wording. |
| F-006 | fixed | `slang-compile-request.cpp` is outside the watched paths (`_common.md` Scope rule). | Removed the citation and dialect-resolution claim. |
| F-007 | fixed | Parser dialect mechanics and pairwise ranking are excluded by the lookup prompt and family contract. | Dropped GLSL dialect paragraph and example; reduced ranking text to the handoff and overload-resolution link. |
