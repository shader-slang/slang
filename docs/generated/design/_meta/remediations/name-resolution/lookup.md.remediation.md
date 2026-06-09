---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: name-resolution/lookup.md
review_report: ../../reviews/name-resolution/lookup.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/lookup.md

## Summary

The review reported two findings, both fixed by editing the target doc (manifest watched-path expansion is out of this stage's scope, so the alternative in-doc remedies the reviewer offered were applied). F-001: the namespace-collapse paragraph cited unwatched `slang-parser.cpp`; the citation was replaced with a link to `scopes.md`, where parser scope construction is in scope. F-002: the claim that `TryCheckOverloadCandidateVisibility` "drops duplicates" was removed because the function (`source/slang/slang-check-overload.cpp:265-287`) only tests visibility and returns false for invisible candidates, never deduplicating.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `slang-parser.cpp` is not in this doc's watched paths; editing the manifest is out of remediation scope, so the reviewer's alternative (link `scopes.md`) was used. | Replaced the `slang-parser.cpp` lines 4086-4180 citation with a prose link to `scopes.md` for parser namespace scope threading. |
| F-002 | fixed | `TryCheckOverloadCandidateVisibility` (`source/slang/slang-check-overload.cpp:275-286`) only calls `isDeclVisibleFromScope` and returns false when invisible; it does not compare decls or deduplicate. | Removed the "visibility filtering ... drops duplicates" clause; kept the accurate `CompareLookupResultItems` ranking note. |
