---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T06:46:49Z
target_doc: name-resolution/overload-resolution.md
review_report: ../../reviews/name-resolution/overload-resolution.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for name-resolution/overload-resolution.md

## Summary

Each finding was checked against the current source. Four findings were real and were fixed in the page (F-001 to F-004). F-005 is valid but needs a manifest change, which this cycle may not make, so it is deferred.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/slang-check-overload.cpp:897 builds the QualType from the parameter's l-value flag; source/slang/slang-check-conversion.cpp:3017-3024 adds the surcharge only in the initializer branch. | LValueCast row rewritten; ordinary f(v) example and exact-match claim removed |
| F-002 | fixed | ThisParameterMode and Breadcrumb::Kind::This do not exist; slang-ast-support-types.h:1388-1426 has ThisExpr/ThisType; slang-check-expr.cpp:5394-5399 passes the callee as base. | receiver paragraph rewritten around explicit base flow and ThisExpr/ThisType |
| F-003 | fixed | slang-check-overload.cpp:2024-2149 contains the constructor exception, generic-extension, aggregate, subscript and synthesized rules. | step 3 extended with the missing comparator rules in source order |
| F-004 | fixed | canConvertImplicitly is defined at slang-check-conversion.cpp:3464-3470; line 3328 is cast-expression construction. | citation 3328 -> 3464-3470 |
| F-005 | deferred | Valid, but the fix is a watched_paths expansion in docs/generated/design/_meta/manifest.yaml, which remediation may not edit; the operator must add the five paths and note it in the Source section in a later cycle. | — |
