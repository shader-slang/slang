---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:57:39Z
target_doc: architecture/dependency-graph.md
review_report: ../../reviews/architecture/dependency-graph.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for architecture/dependency-graph.md

## Summary
One major finding reviewed and fixed. F-001 flagged the wasm target's `slang-lookup-tables` dependency as absent. Verified against `source/slang-wasm/CMakeLists.txt:21` (`LINK_WITH_PRIVATE ... slang-lookup-tables`). The mermaid diagram already contains `slangWasm --> lookupTables` (target doc line 86) and the edge-citation row's edge label already names `lookup-tables`; the sole real gap was the citation row's vague `Clause` cell, which I made concrete by spelling out the full `LINK_WITH_PRIVATE` list including `slang-lookup-tables`. No findings were rejected, deferred, or escalated.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed via `source/slang-wasm/CMakeLists.txt:21`; wasm links `slang-lookup-tables`. Diagram edge (target doc line 86) and citation-row edge label already covered it; only the `Clause` cell lacked explicit `slang-lookup-tables` verification. In-scope per the prompt's every-edge-justified-by-CMake contract. | Edge-citation row for `slang-wasm`: replaced generic `LINK_WITH_PRIVATE clause` with full list `miniz lz4_static slang core compiler-core slang-capability-defs slang-capability-lookup slang-fiddle-output slang-lookup-tables`. |
