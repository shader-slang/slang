---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T17:30:00+00:00
target_doc: ir-reference/resources-and-atomics.md
review_report: ../../reviews/ir-reference/resources-and-atomics.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/resources-and-atomics.md

## Summary

Two major findings addressed: added a new cooperative-matrix/vector
sub-table and the fragment-shader-interlock sync opcodes, and
removed the `BindingQuery` grouping-parent row. Non-resource helper
opcodes from the cited Lua range (texture-access introspection,
SPIR-V global-param helpers, Torch / tensor-view helpers,
`allocateOpaqueHandle`) are picked up by the `misc.md` remediation
in this same cycle.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-insts.lua:2709-2711` defines `BeginFragmentShaderInterlock` and `EndFragmentShaderInterlock` as synchronization opcodes; `:1533-1574` defines a cooperative matrix/vector cluster (`CoopMatMulAdd`, `CoopVecMatMulAdd`, etc.) that fits the resources/cooperative scope. | Added the two interlock rows to `### Barriers and synchronization` and a new `### Cooperative matrix and vector` sub-table with `CoopMatMapElementIFunc`, `CoopMatMulAdd`, `CoopVecMatMulAdd`, `CoopVecOuterProductAccumulate`, `CoopVecReduceSumAccumulate`. |
| F-002 | fixed | `source/slang/slang-ir-insts.lua:1578-1591` defines `BindingQuery` as a grouping parent with `getRegisterIndex` and `getRegisterSpace` as its only concrete children. | Removed the `BindingQuery` row; replaced with a one-paragraph note above the table identifying it as the grouping parent of the remaining two concrete rows. |
