---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:17:14+00:00
target_doc: architecture/dependency-graph.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: f7a15e0c6c76a34adccaa06e7b5b78d535a56cd4684735b60b3fa360f894a2de
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for architecture/dependency-graph.md

## Summary
The page accurately captures the individual CMake target edges, but its diagram does not consistently honor the required subsystem granularity. Four nodes are build targets internal to `source/slang/`, not source subsystems or headings in the dependency `module-map.md`. Two smaller inaccuracies concern an overbroad target-ownership invariant and a stale root-CMake line citation.

## Items checked
- Reviewed the target, `_common.md`, its per-document prompt, `architecture/module-map.md`, and all 11 watched CMake files resolved by `regenerate.py show` at `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Verified all 27 solid diagram edges against the recorded `LINK_WITH_PRIVATE` and `LINK_WITH_PUBLIC` declarations, plus the dashed `SLANG_RECORD_REPLAY_SYSTEM` source-inclusion edge.
- Checked more than 10 additional factual claims, including the `standard-modules`, `slang-llvm`, mimalloc, `slang-rt`, embedded-core-module, tools-header, and `slang-common-objects` notes.
- Re-derived all three line-number citations at the recorded source commit; both references to `source/slang/CMakeLists.txt:164-167` are exact, while the root-CMake citation is stale.
- Resolved all 33 relative links (17 unique destinations) at the recorded commit and confirmed both generated peer pages are manifest entries.
- Verified the required front-matter keys, the 64-character digest against `regenerate.py digest`, the required sections, Mermaid syntax, and the 10,661-byte size against the 16-KB cap.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Edges (intra-project only)`, lines 34-87 | The diagram claims subsystem granularity but introduces `slang-capability-defs`, `slang-capability-lookup`, `slang-lookup-tables`, and `slang-fiddle-output` as standalone nodes. These are CMake targets defined inside `source/slang/`, not source-subsystem directories or headings in `module-map.md`, so the graph mixes target-level and subsystem-level abstraction and does not satisfy the prompt's node-coverage rule. | `docs/generated/design/_meta/prompts/architecture-dependency-graph.md:20-25,42-44` requires one node per logical unit group and says each node must correspond to a source directory or `module-map.md` heading; `source/slang/CMakeLists.txt:56-65,119-143,198-210` defines the four targets within the `source/slang/` subsystem. | Remove the four standalone generated-target nodes from the subsystem diagram, represent their target-level details in prose, and describe any resulting cross-subsystem relationship between `source/slang/` and `source/slang-core-module/` explicitly. |
| F-002 | minor | `## Notable invariants`, lines 184-188 | The statement that the main `slang` library is “the only target that pulls in the AST/IR/emit/check sources” is not true when `SLANG_EMBED_CORE_MODULE` is enabled: `slang-common-objects` compiles the directory sources, while both library targets use `NO_SOURCE` and link that object target. The later irregularities section partially acknowledges this, leaving the page internally inconsistent. | `source/slang/CMakeLists.txt:303-358` creates `slang-common-objects` from `.` and links it into `slang-without-embedded-core-module` and `slang`; the latter two declarations specify `NO_SOURCE`. | Replace “the only target” with subsystem-level wording, and state that the concrete source-owning target is `slang` in non-embedded builds or `slang-common-objects` in embedded builds. |
| F-003 | minor | `## Edges (intra-project only)`, lines 104-108 | The citation says `SLANG_SLANG_LLVM_FLAVOR` is in root `CMakeLists.txt` “around line 366,” but the option starts at line 385 and the identifier is on line 386, twenty lines later. | `CMakeLists.txt:385-401` declares the option and begins its handling; line 366 instead declares `SLANG_ENABLE_RELEASE_DEBUG_INFO`. | Change “around line 366” to “lines 385-401” (or omit the line number and cite the file only). |

## No-issues notes
- The wasm edge now includes `slang-lookup-tables` in both the diagram and citation table.
- The `slang-rt` note correctly distinguishes link independence from recompiling `source/core/` through `EXTRA_SOURCE_DIRS`.
- The optional embedded-core-module and generated cache relationships match their generator expressions and build-order declarations.
