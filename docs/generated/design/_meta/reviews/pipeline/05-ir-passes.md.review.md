---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:25+00:00
target_doc: pipeline/05-ir-passes.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: c7ceb8f7138b0d1f8d9559d2e33d3bfbcf92dade0e6bafe75a6e9dd8ace9f07f
source_commit: 05132edd86435f217f95634406f85184e58991f8
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
The pass catalog is factually consistent with the sampled source files and all links resolve, but review found one prompt-completeness issue. The required `## Adding a new pass` and `## Pass utilities` sections are present, but they appear in the reverse order from the required structure.

## Items checked
- Ran `regenerate.py show pipeline/05-ir-passes.md` and reviewed the manifest entry, prompt, resolved watched files, and dependency on `pipeline/04-ast-to-ir.md`.
- Verified front matter fields and resolved all 185 relative links.
- Checked `linkAndOptimizeIR` line anchors and call sites in `slang-emit.cpp`, the recorded count of `slang-ir-*.cpp` files, and representative pass files across linking, validation, cleanup, specialization, autodiff, legalization, entry-point, layout, target-specific, instrumentation, and utility categories.
- Verified cross-links to `04b`, `04c`, target pipelines, `06-emit`, design IR docs, tests, and the adding-a-pass checklist.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | document structure, `## Pass utilities` and `## Adding a new pass` | The prompt's required structure lists `## Adding a new pass` before `## Pass utilities`, but the document places `## Pass utilities` first and `## Adding a new pass` afterward. | `docs/generated/design/_meta/prompts/pipeline-05-ir-passes.md:55-60` requires `## Adding a new pass` followed by `## Pass utilities`; `docs/generated/design/pipeline/05-ir-passes.md` has `## Pass utilities` at line 316 and `## Adding a new pass` at line 330. | Move `## Adding a new pass` before `## Pass utilities`, preserving the existing content. |
