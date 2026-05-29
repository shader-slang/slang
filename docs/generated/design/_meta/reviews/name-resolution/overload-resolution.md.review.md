---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: name-resolution/overload-resolution.md
target_doc_source_commit: 12bdd912949ee692a11a757b5829fe3ef819bebc
target_doc_watched_paths_digest: 8d205b22cafc669a98c4f58847addad46d1ed75557279ca4e4e51b8586320f26
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 2
  minor: 1
  nit: 0
---

# Review report for name-resolution/overload-resolution.md

## Summary
The page is structurally lint-clean, but review found 3 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked `OverloadCandidate` flavor/status/flags, `OverloadResolveContext`, filter-step functions, conversion-cost constants, comparator ordering, partial generic application, operator cache, and failure cases.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Algorithm` | Required pipeline step `TryCheckOverloadCandidateClassNewMatchUp` is not included in the main filter flow; it is only mentioned later in finalization. | `docs/generated/design/_meta/prompts/name-resolution-overload-resolution.md` requires it; `source/slang/slang-check-overload.cpp:1309-1330` calls it in candidate completion. | Include ClassNewMatchUp in the ordered algorithm, noting it runs in `CompleteOverloadCandidate`. |
| F-002 | major | `## Conversion costs` | The conversion-cost table is explicitly non-exhaustive and not in declaration order, but the prompt requires enum levels in source order. | `docs/generated/design/_meta/prompts/name-resolution-overload-resolution.md` requires all constants in order; `source/slang/slang-ast-support-types.h:89-192` declares them. | List all `kConversionCost_*` constants in source order. |
| F-003 | minor | `## Edge cases and failure modes` | The page says the sum across arguments is the only ceiling, but each argument conversion must first be allowed; only accepted costs are summed for ranking. | `source/slang/slang-check-overload.cpp:799-820` checks each argument; `source/slang/slang-check-conversion.cpp:3103-3108` implements conversion checking. | Say each argument conversion must be allowed, then accepted costs are summed for ranking. |
