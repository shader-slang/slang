---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ir-reference/index.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 44cc076396b4503f18997a6be579c3163209ab7a24f87f3aae489b26a3963cbd
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 1
  major: 0
  minor: 0
  nit: 0
---

# Review report for ir-reference/index.md

## Summary
The page is structurally lint-clean, but review found 1 finding; the most significant severity is critical. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked front matter, all relative links, family-page table, opcode-row union across family docs, and sampled source ranges in `slang-ir-insts.lua`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | intro | The intro claims every opcode appears in exactly one family page, but the set is neither complete nor unique. | `source/slang/slang-ir-insts.lua:1112-1121` and `source/slang/slang-ir-insts.lua:2893-3174` define missing opcodes; duplicate rows include `global_var`, `param`, `witness_table`, and `DebugFunction`. | Fix family pages first, then update the index wording and counts to match exact coverage. |
