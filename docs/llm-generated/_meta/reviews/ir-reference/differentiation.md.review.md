---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ir-reference/differentiation.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for ir-reference/differentiation.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked differential-pair groups, `TranslateBase` children, checkpoint rows, misc overlap, autodiff source clusters, links, and front matter.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | lines 79-173 | Backward-autodiff temporary opcodes are omitted. | `source/slang/slang-ir-insts.lua:1110-1121` defines `LoadReverseGradient`, `ReverseGradientDiffPairRef`, `PrimalParamRef`, and `DiffParamRef`. | Add an “Autodiff temporaries” sub-table. |
| F-002 | major | lines 79-173 | `DiffTypeInfo` is not listed here even though it is differential metadata and the prompt calls it out. | `source/slang/slang-ir-insts.lua:1006-1010` says it holds witness tables for differential type info; it is currently listed in `misc.md`. | Move `DiffTypeInfo` to this page or make a clear ownership decision and update prompt/index. |
