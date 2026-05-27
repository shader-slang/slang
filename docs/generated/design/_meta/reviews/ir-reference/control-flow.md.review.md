---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ir-reference/control-flow.md
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
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for ir-reference/control-flow.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked block/param rows, terminator rows, other control-flow rows, source control-flow ranges, relative links, and front matter.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | lines 132-140 | The “Other control-flow opcodes” table omits concrete opcodes adjacent to included `RequirePrelude`, `StaticAssert`, and `Printf` entries. | `source/slang/slang-ir-insts.lua:1381-1389` defines `RequirePrelude`, `RequireTargetExtension`, `RequireComputeDerivative`, `StaticAssert`, `Printf`, `RequireMaximallyReconverges`, and `RequireQuadDerivatives`. | Add the four missing `Require*` rows or move the whole group to another page consistently. |
