---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: target-pipelines/hlsl.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 37fe61fef33084d2ef7676b9897a7ef8dc2cd50f8e3b6afdd3b425f68b07b571
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for target-pipelines/hlsl.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked required target-pipeline sections, Phase B/C target arms, target-specific legalizer/emit entry points, downstream tool path, and relative links.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | lines 320-405 | The Phase C diagram/table omit liveness marker passes reachable when liveness tracking is enabled. | `source/slang/slang-emit.cpp:2307-2325` runs `LivenessUtil::addVariableRangeStarts` and `LivenessUtil::addRangeEnds` around `eliminatePhis`. | Add both liveness passes to Phase C with the `codeGenContext->shouldTrackLiveness()` gate. |
