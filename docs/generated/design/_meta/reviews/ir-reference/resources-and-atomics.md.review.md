---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ir-reference/resources-and-atomics.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for ir-reference/resources-and-atomics.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked atomics children, buffer/image/sampler rows, shader IO, barrier/wave/raytracing/descriptor rows, source clusters, links, and front matter.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | lines 86-245 | Several resource, synchronization, cooperative, and Torch helper opcodes are omitted. | `source/slang/slang-ir-insts.lua:1503-1576` and `source/slang/slang-ir-insts.lua:2709-2711` define omitted helper and interlock opcodes. | Add rows in resource/cooperative/sync sub-tables or move non-resource helpers to `misc.md`. |
| F-002 | major | lines 239-245 | `BindingQuery` is listed as an opcode row even though it is only the grouping parent for `getRegisterIndex` and `getRegisterSpace`. | `source/slang/slang-ir-insts.lua:1578-1591` defines `BindingQuery` as a parent entry. | Remove `BindingQuery` from the Opcodes table and keep it in hierarchy/prose only. |
