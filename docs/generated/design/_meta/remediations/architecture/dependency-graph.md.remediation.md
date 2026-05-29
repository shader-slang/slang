---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T21:00:00+00:00
target_doc: architecture/dependency-graph.md
review_report: ../../reviews/architecture/dependency-graph.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for architecture/dependency-graph.md

## Summary

Both findings addressed: the graph now has nodes for
`standard-modules`, `slang-record-replay`, and `slang-llvm` with
explicit "no observed link edge" annotations for the two that lack a
CMake link target, and a new `## Edge citations` table maps every
solid edge to its `CMakeLists.txt` clause.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The prompt requires one node per logical unit group in `module-map.md`; `standard-modules`, `slang-llvm`, and `slang-record-replay` were missing. | Added three nodes: `standardModules` (labelled "no observed link edge"), `slangLlvm` (same), and `slangRecordReplay` (folded into `slang` sources, with a dashed source-include edge to `slang`). Added a prose paragraph beneath the diagram explaining why each subsystem lacks an ordinary `LINK_WITH_*` edge, citing the relevant `CMakeLists.txt` files. |
| F-002 | fixed | The prompt requires each edge to be justified by a `CMakeLists.txt` citation; the doc only cited a few build files in prose. | Added a new `## Edge citations` table that maps every solid edge to the `LINK_WITH_PUBLIC` / `LINK_WITH_PRIVATE` clause in the cited `CMakeLists.txt`, with one row per edge group. The dashed `slang -.-> slang-record-replay` edge is documented separately as a source-list inclusion at `source/slang/CMakeLists.txt` lines 164-167. |
