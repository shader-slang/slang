---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:12:00Z
target_doc: name-resolution/lookup.md
review_report: ../../reviews/name-resolution/lookup.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/lookup.md

## Summary

Three of the four findings were verified and fixed: the `## Source` section
claimed four cited files were unwatched when the manifest now watches all of
them, the `AggTypeDeclBase` dispatch used an "i.e." list that excluded
`ClassDecl`, and the deduplication bullet conflated two duplicate paths to one
decl-ref with competing items for distinct declarations. The digest finding is
out of scope for remediation. Breakdown: 3 fixed, 0 rejected-bogus, 1
rejected-out-of-scope, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action                | Rationale                                                                                                                                                                                                                                        | Fix summary                                                                                                                                                                                           |
| ---------- | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md:97-100` reserves `watched_paths_digest` (with `generated_at` and `source_commit`) for the operator's `mark-fresh` run and forbids the remediator from editing it.                             | —                                                                                                                                                                                                     |
| F-002      | fixed                 | Confirmed: `docs/generated/design/_meta/manifest.yaml:494-497` lists `slang-check-impl.h`, `slang-check-expr.cpp`, `slang-check-overload.cpp`, and `slang-parser.cpp` in this page's `watched_paths`.                                            | `## Source`: "live outside this page's watched paths and are cited below because no watched file owns them" replaced with "round out the source inventory and are cited below".                       |
| F-003      | fixed                 | Confirmed: `source/slang/slang-ast-decl.h:360-386` makes `ExtensionDecl` and `AggTypeDecl` derive from `AggTypeDeclBase`, and `:428` declares `ClassDecl : AggTypeDecl`, so the list was not exhaustive.                                         | `### Unqualified lookup`, dispatch step: "i.e." changed to "for example" and `class` added to the list.                                                                                               |
| F-004      | fixed                 | Confirmed: `source/slang/slang-check-overload.cpp:1944-1958` treats the interface requirement and the concrete function as two distinct declarations, tested separately with `isInterfaceRequirement`; they are not one `DeclRef` reached twice. | `#### Deduplication`, second bullet: transparent-member case kept as the same-decl-ref example; interface requirement versus concrete member split out as competing items for different declarations. |
