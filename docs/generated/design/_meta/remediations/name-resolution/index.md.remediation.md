---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:00:00Z
target_doc: name-resolution/index.md
review_report: ../../reviews/name-resolution/index.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/index.md

## Summary

Both minor findings were verified against the current source and fixed in the target document. The introduction now limits the skip-overload-resolution claim to fast-path builtin operators, and the flow diagram transition labels now name the owning pages. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-check-expr.cpp` lines 4991-5005 and 5232-5247 return null (fall back to normal resolution) for GLSL matrix/vector-equality and non-scalar logical and/or, so the unqualified claim was too broad. | Introduction: "builtin operators handled by the fast path skip overload resolution". |
| F-002 | fixed | The scope walk lives in `source/slang/slang-lookup.cpp` (lookup.md) and visibility filtering in `source/slang/slang-check-expr.cpp` line 1324 (visibility.md); prompt requires each transition to name its owning page. | Diagram: scopeWalk to raw labeled lookup.md; raw to visFilter labeled visibility.md; visFilter to narrow labeled "lookup.md and overload-resolution.md". |
