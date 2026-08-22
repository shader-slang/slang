---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:25:00Z
target_doc: name-resolution/visibility.md
review_report: ../../reviews/name-resolution/visibility.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/visibility.md

## Summary

Five of the six findings were verified against the source at the recorded
commit and fixed with local wording corrections: the `languageVersion`
lifecycle, the `GenericTypeConstraintDecl` fall-through, the
`_getTypeVisibility` recursion shape, the parent-visibility cap, and the
`using` target kind. The digest finding is reserved for the operator.
Breakdown: 5 fixed, 0 rejected-bogus, 1 rejected-out-of-scope, 0 deferred, 0
escalated.

## Actions

| Finding ID | Action                | Rationale                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 | Fix summary                                                                                                                                                                                                                                        |
| ---------- | --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md:97-100` reserves `watched_paths_digest` for the operator's `mark-fresh` run and forbids the remediator from editing it.                                                                                                                                                                                                                                                                                                                | —                                                                                                                                                                                                                                                  |
| F-002      | fixed                 | Confirmed: `source/slang/slang-compile-request.cpp:324-325` reads `optionSet.getLanguageVersion()` and `:339` assigns it to `translationUnitSyntax->languageVersion`; `source/slang/slang-parser.cpp:1221-1227` defines `maybeUpgradeLanguageVersionFromLegacy` (legacy to 2025), called at `:1260`, `:1365`, `:1404`. No version is parsed from the `module` declaration. Follow-up for the operator: adding these two files to `watched_paths` is a manifest change outside this stage. | `## Concepts`, `languageVersion` bullet: "set from the `module` declaration's version (or the linkage default)" replaced with the option-set initialization plus the parser's legacy-to-2025 upgrade, flagged as living outside the watched paths. |
| F-003      | fixed                 | Confirmed: `source/slang/slang-check-decl.cpp:21260-21262` returns `DeclVisibility::Default` when `as<GenericDecl>(decl->parentDecl)` fails.                                                                                                                                                                                                                                                                                                                                              | `### Per-keyword semantics`, generic-parameter fall-through bullet: qualified to parents that are a `GenericDecl` and added the `Default` fall-back for other parents.                                                                             |
| F-004      | fixed                 | Confirmed: `source/slang/slang-check-expr.cpp:1117` guards the recursion with `as<DeclRefType>(arg)`, so non-`DeclRefType` generic arguments are skipped.                                                                                                                                                                                                                                                                                                                                 | `### Container-level cap`, first paragraph: "any generic type arguments" replaced with "its declaration-reference generic arguments" plus a clause naming the `DeclRefType` guard.                                                                 |
| F-005      | fixed                 | Confirmed: `source/slang/slang-check-modifier.cpp:2376-2381` initializes `parentDecl = decl` and breaks at the first `AggTypeDeclBase`, so an aggregate is compared with itself at `:2385`.                                                                                                                                                                                                                                                                                               | `### Container-level cap`, `checkVisibility` paragraph: parent cap restated as the nearest enclosing `AggTypeDeclBase` with a sentence noting the self-comparison for aggregates.                                                                  |
| F-006      | fixed                 | Confirmed: `source/slang/slang-check-decl.cpp:17332-17333` says "a namespace (or a module, since modules are namespace-like)" and `:17362`/`:17373` test `NamespaceDeclBase`.                                                                                                                                                                                                                                                                                                             | `## Edge cases and failure modes`, `using` bullet: "only brings a _namespace_ into scope" replaced with "only brings a _namespace-like_ container into scope — a namespace or a module".                                                           |
