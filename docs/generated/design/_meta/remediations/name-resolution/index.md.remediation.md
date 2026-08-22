---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:05:00Z
target_doc: name-resolution/index.md
review_report: ../../reviews/name-resolution/index.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/index.md

## Summary

All three findings were verified at the recorded commit and all three were
fixed. The sibling-scope bullet wrongly attributed imported-module splicing
to `ScopesWired`; that work is done by the header visitor at
`SignatureChecked`. The deduplication bullet overstated what the narrowing
chain removes, and the opening paragraph did not name its reader. Breakdown:
3 fixed, 0 rejected-bogus, 0 rejected-out-of-scope, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                                                                                                  | Fix summary                                                                                                                                                                                                                |
| ---------- | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | Confirmed: `source/slang/slang-check-decl.cpp:17083` defines `visitImportDecl` on `SemanticsDeclHeaderVisitor`, and `:17864-17869` maps `ScopesWired` to `SemanticsDeclScopeWiringVisitor` while `SignatureChecked` maps to `SemanticsDeclHeaderVisitor`. The `importModuleIntoScope` call is at `:17115`. | `### Where the boundaries blur`, sibling-scope bullet: imported modules removed from the `ScopesWired` list and given their own clause naming `SemanticsDeclHeaderVisitor::visitImportDecl` at `SignatureChecked`.         |
| F-002      | fixed  | Confirmed: `source/slang/slang-check-expr.cpp:1436-1454` removes an existing item only when `CompareLookupResultItems` is negative and skips the new item only when it is positive, so items comparing equal are all retained.                                                                             | `### Where the boundaries blur`, deduplication bullet: "What collapses the rest" replaced with wording that the chain discards strictly worse paths and leaves equal items for overload resolution or ambiguity diagnosis. |
| F-003      | fixed  | Confirmed: `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first body paragraph to state both coverage and intended reader; the reader appeared only in the second paragraph.                                                                                                          | Opening paragraph, first sentence: added the clause "and is written for a compiler contributor working on or debugging those rules".                                                                                       |
