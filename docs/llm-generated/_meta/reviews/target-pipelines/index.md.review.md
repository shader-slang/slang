---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: target-pipelines/index.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 562a0873aae2e59ee7743d5a1b0d436fb33c759dbcab165ff574e19fbf111219
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for target-pipelines/index.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is minor. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked front matter, all peer links, required index sections, cross-target table columns, and shared `linkAndOptimizeIR` references.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | lines 44-66 | The index includes detailed pass names and rough line ranges, even though the index contract forbids per-pass details. | `docs/llm-generated/_meta/prompts/_common.md` says the target-pipeline index is not a target page and must not document passes. | Keep the four-phase overview but remove individual pass names and line ranges. |
