---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T19:00:00+00:00
target_doc: name-resolution/overload-resolution.md
review_report: ../../reviews/name-resolution/overload-resolution.md.review.md
target_doc_source_commit_before: 9be3b6cd58c80ba8c66ce5b6e7f5dadcde2c0c63
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for name-resolution/overload-resolution.md

## Summary

All three findings addressed: a missing finalize-phase step is now
listed in the algorithm, the conversion-cost table is rebuilt from
the source enum, and the "edge cases" entry that conflated per-
argument and per-candidate cost ceilings is corrected.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `TryCheckOverloadCandidateClassNewMatchUp` runs as part of finalize for class-new candidates and was missing from the step list. | Added a seventh step "Finalize / class-new match-up" to the algorithm section, citing `slang-check-overload.cpp` lines 2103-2130. |
| F-002 | fixed | The previous table omitted several `kConversionCost_*` constants and used out-of-order values; rebuild from source. | Rewrote the "Conversion costs" table to enumerate every `kConversionCost_*` constant in declaration order from `slang-check-overload.cpp`, with one-line descriptions and a citation to the enum block. |
| F-003 | fixed | The original prose suggested there was no ceiling at all; in fact `canConvertImplicitly` rejects any single argument whose cumulative conversion cost reaches `kConversionCost_GeneralConversion`, while `conversionCostSum` itself is unbounded. | Rewrote the "Argument that needs a chain of conversions" bullet to spell out the two-level structure: per-argument ceiling at 900 (via `canConvertImplicitly`), then unbounded summation into `conversionCostSum` for ranking. Added citations to `slang-check-conversion.cpp` and `slang-check-overload.cpp`. |
