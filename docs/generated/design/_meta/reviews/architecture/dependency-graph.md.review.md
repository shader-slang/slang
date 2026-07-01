---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:26:46+00:00
target_doc: architecture/dependency-graph.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 02fa669b99e0a73ba4301f994b3212b5f9667ac9bffda7a1669c8780fdb96800
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
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

# Review report for architecture/dependency-graph.md

## Summary
The page is mostly aligned with the scoped CMake inputs, but the wasm dependency set is incomplete. `source/slang-wasm/CMakeLists.txt` links the wasm target with `slang-lookup-tables`, yet the diagram and edge-citation row omit that internal edge.

## Items checked
- Ran `regenerate.py show architecture/dependency-graph.md` and reviewed the target document, `_common.md`, `architecture-dependency-graph.md`, the dependency document `architecture/module-map.md`, and all 11 resolved `source/*/CMakeLists.txt` files.
- Checked front matter for all required keys, the recorded target source commit, the warning string, and a 64-character hex watched-path digest copied from the target document.
- Spot-checked the diagram and edge table against `LINK_WITH_PRIVATE` / `LINK_WITH_PUBLIC` clauses for `compiler-core`, `core`, `slang`, `slang-core-module`, `slang-glsl-module`, `slang-dispatcher`, `slang-glslang`, `slang-rt`, `slang-wasm`, `slangc`, and `standard-modules`.
- Verified the special-case notes for `standard-modules`, `slang-record-replay`, `slang-llvm`, `slang-common-objects`, and the optional embedded core-module targets against the scoped CMake inputs where those claims touch watched files.
- Checked the relative links used for source files and generated peer documents; no dangling relative links were found in the checked set.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Edges (intra-project only)` and `## Edge citations`, lines 80-85 and 123-125 | The wasm target's internal dependency list is incomplete. The document shows `slang-wasm` depending on `slang`, `core`, `compiler-core`, `slang-capability-defs`, `slang-capability-lookup`, and `slang-fiddle-output`, but omits the `slang-lookup-tables` edge. | `source/slang-wasm/CMakeLists.txt:12-21` lists `LINK_WITH_PRIVATE ... slang-lookup-tables` in the wasm target. | Add `slangWasm --> lookupTables` to the mermaid diagram and include `slang-lookup-tables` in the wasm edge-citation row. |

## No-issues notes
- The document's front matter is structurally valid and uses the target document's recorded source commit and digest.
- The `source/slang-record-replay/` dashed edge is correctly described as source inclusion through `SLANG_RECORD_REPLAY_SYSTEM`, not a `LINK_WITH_*` edge.
- The `standard-modules` note correctly reflects that the watched `source/standard-modules/CMakeLists.txt` configures a header and adds the `neural` subdirectory rather than declaring a top-level link target.
