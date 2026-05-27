---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ir-reference/values.md
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

# Review report for ir-reference/values.md

## Summary
The page is structurally lint-clean, but review found 3 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked literal, undefined, arithmetic, conversion, memory, aggregate, optional/result tables, duplicates, source value clusters, links, and front matter.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | lines 96-103 | `Undefined` is listed as an opcode row despite being a grouping parent. | `source/slang/slang-ir-insts.lua:857-894` defines `Undefined` as the parent for `LoadFromUninitializedMemory` and `Poison`. | Remove the `Undefined` row from `## Opcodes` and leave it in hierarchy/prose only. |
| F-002 | major | lines 78-236 | Several concrete value-like opcodes are omitted, including `allocObj`, `CUDA_LDG`, `getNativeStr`, `makeString`, `getNativePtr`, managed-pointer helpers, and managed-pointer attach/detach operations. | `source/slang/slang-ir-insts.lua:946`, `:1067`, and `:1139-1149` define these opcodes. | Add these to the values or misc table, with cross-links if ownership is intentionally elsewhere. |
| F-003 | major | lines 105-169 | The compile-time arithmetic/cast opcode family is missing. | `source/slang/slang-ir-insts.lua:3145-3174` defines `constexprAdd` through `constexprEnumCast`. | Add a constexpr arithmetic/cast sub-table or explicitly move these to `misc.md`. |
