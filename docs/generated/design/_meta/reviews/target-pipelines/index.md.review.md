---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T13:46:37+00:00
target_doc: target-pipelines/index.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 79d77df3a1037f04643bcb85b77033ae0519e608f7d87b8700499b1d91026561
source_commit: 05132edd86435f217f95634406f85184e58991f8
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
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
The index has the required navigation sections, the peer-page list, and the comparison table, and all checked links resolve. One small contract issue remains: a paragraph after the table explains individual pass behavior even though the index prompt says not to document per-pass details.

## Items checked
- Read `regenerate.py show target-pipelines/index.md`, the index prompt, `_common.md`, and all five peer target-pipeline docs.
- Checked front matter, required index sections, peer page coverage, comparison-table columns, and the shared `linkAndOptimizeIR` reference.
- Resolved all 24 relative links in the page at the recorded source commit.
- Spot-checked the peer target names, Phase C entries, Phase D emitters, downstream tool names, and loop summaries against the peer docs and `source/slang/slang-emit.cpp`.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Cross-target comparison` | The paragraph after the comparison table documents pass-level behavior, including `eliminatePhis` register-allocation settings and address-space propagation pass placement. The index contract says this page is a navigation hub and forbids per-pass details. | `docs/generated/design/_meta/prompts/_common.md` under "Target-pipeline index contract" says the index "does not document any pass" and forbids "per-pass details"; the target doc currently names `eliminatePhis`, `specializeAddressSpaceForMetal`, and `specializeAddressSpaceForWGSL` immediately after the table. | Keep the comparison table, but delete or generalize the extra paragraph so detailed pass behavior remains in the per-target pages. |
