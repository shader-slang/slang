---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: pipeline/05-ir-passes.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 8749b5a60327ef9aea96c0b02a10d643c2d39d04195e7cbd40904b69dabc7f6e
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for pipeline/05-ir-passes.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked the `linkAndOptimizeIR` anchor, representative pass files across categories, target-sensitive ordering caveat, pre-link and target-pipeline cross-links, and the adding-a-pass checklist.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | document structure | The prompt requires a top-level `## Pass utilities` section, but the document has only `### Shared utilities (not passes)` nested under `## Pass categories`. | `docs/llm-generated/_meta/prompts/pipeline-05-ir-passes.md` requires `## Pass utilities`. | Promote or rename the shared-utilities subsection to `## Pass utilities`. |
