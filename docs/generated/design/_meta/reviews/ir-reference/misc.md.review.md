---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ir-reference/misc.md
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
finding_count: 3
severity_breakdown:
  critical: 0
  major: 3
  minor: 0
  nit: 0
---

# Review report for ir-reference/misc.md

## Summary
The page is structurally lint-clean, but review found 3 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked misc rows, catch-all coverage against unclaimed source entries, abstract-parent rows, source ranges, links, and front matter.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | lines 64-185 | The catch-all table omits many unclaimed concrete opcodes. | `source/slang/slang-ir-insts.lua:14-18`, `:1528-1576`, and `:3103-3140` define omitted concrete opcodes. | Add rows for remaining unclaimed opcodes after moving family-specific opcodes to their owning pages. |
| F-002 | major | lines 124-170 | Abstract/grouping parents are listed in opcode tables: `CastStorageToLogicalBase` and `LiveRangeMarker`. | `source/slang/slang-ir-insts.lua:2517-2522` and `:2701` define these as grouping parents. | Keep these parents in hierarchy/prose only and retain only concrete child rows in Opcodes. |
| F-003 | major | lines 145-158 | `DiffTypeInfo` is listed here although its source comment identifies differential-type-info semantics. | `source/slang/slang-ir-insts.lua:1006-1010` identifies `DiffTypeInfo` as differential type info. | Move `DiffTypeInfo` to `differentiation.md` or document why misc owns it. |
