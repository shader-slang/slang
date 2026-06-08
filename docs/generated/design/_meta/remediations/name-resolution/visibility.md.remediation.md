---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: name-resolution/visibility.md
review_report: ../../reviews/name-resolution/visibility.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for name-resolution/visibility.md

## Summary

The review reported one major and one minor finding. F-001 was fixed: the prompt (step 5) requires an `extern`/`export` interaction subsection that was missing; a `### Interaction with extern and export` subsection was added, grounded in `ExternModifier`/`ExtensionExternVarModifier`/`HLSLExportModifier`/`ExportedModifier` (`slang-ast-modifier.h:100,231,112,142`), `DeclPassesLookupMask` (`slang-lookup.cpp:41-54`), and `isModuleReachableViaExportedImports` (`slang-check-decl.cpp:8928-8954`). F-002 was deferred: the prompt explicitly requires this page to cite `slang-check-overload.cpp` for `TryCheckOverloadCandidateVisibility`, so the citation cannot be removed; closing the watched-paths gap requires a manifest expansion that is out of scope for this remediation cycle.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Prompt `name-resolution-visibility.md:69` (Rules step 5) requires `extern`/`export` coverage; modifiers and behaviors confirmed at `slang-ast-modifier.h:100,112,142,231`, `slang-lookup.cpp:41-54`, `slang-check-decl.cpp:8928-8954`. | Added a `### Interaction with extern and export` rules subsection between the container-level cap and generic-parameters subsections. |
| F-002 | deferred | Prompt `name-resolution-visibility.md:55-64` and checklist lines 120-122 require citing `slang-check-overload.cpp` here, so removing the citation would violate the contract; the only correct fix is adding `source/slang/slang-check-overload.cpp` to this doc's manifest `watched_paths`, a manifest change out of scope for this cycle. Follow-up: expand watched_paths in a later cycle. | — |
