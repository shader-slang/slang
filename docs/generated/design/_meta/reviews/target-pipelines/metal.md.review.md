---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T16:52:54+00:00
target_doc: target-pipelines/metal.md
target_doc_source_commit: 43e8ca0cef30f575bd1750589b2c7cd9f2b6e030
target_doc_watched_paths_digest: 751b986b2d853e8242f650fcb4a698ce747155b40fac3ebc58e2361363790674
source_commit: 76c9a59695365016093e84dd12b5fc57d1e751d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 3
  minor: 0
  nit: 0
---

# Review report for target-pipelines/metal.md

## Summary
The Metal page has the requested structure, valid front matter, and all checked relative links resolve. The rebuilt Phase B diagram is much closer to the contract because runtime gates are now rendered as diamonds, but Phase B still misses reachable minimal-optimization branch passes and misdraws one false branch. The Phase C table also has one ordered row pair that contradicts `linkAndOptimizeIR`.

## Items checked
- Read `regenerate.py show target-pipelines/metal.md`, `docs/generated/design/_meta/prompts/target-pipelines-metal.md`, `docs/generated/design/_meta/prompts/_common.md`, and the dependency docs listed by `show`.
- Confirmed the target document front matter has `source_commit` `43e8ca0cef30f575bd1750589b2c7cd9f2b6e030` and watched digest `751b986b2d853e8242f650fcb4a698ce747155b40fac3ebc58e2361363790674`.
- Verified the resolved watched source files have no diff from the target document's recorded `source_commit` to current `HEAD`.
- Resolved all 157 Markdown links in the page.
- Spot-checked the Phase A-D pass ordering, Phase B gate drawing, `legalizeIRForMetal`, `wrapCBufferElementsForMetal`, Metal parameter handling, Metal pointer lowering, `DescriptorHandle<T>` layout emission, downstream `MetalLib` handling, and loop claims against the watched source files.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Phase B: Specialization and type legalization` | The Phase B diagram and table omit reachable `SLANG_PASS` calls on the minimal-optimization branches. After `lowerSumVectorMatrixInsts`, the source can run `eliminateDeadCode` when `minimalOptimization` is true and generics are required; after `performForceInlining`, the source runs `applySparseConditionalConstantPropagation` and `eliminateDeadCode` when `minimalOptimization` is true. | `source/slang/slang-emit.cpp:1410-1417` contains the minimal-plus-generics `eliminateDeadCode` branch, and `source/slang/slang-emit.cpp:1516-1529` contains the minimal branch with `applySparseConditionalConstantPropagation` followed by `eliminateDeadCode`. | Add those conditional branch nodes to the Phase B diagram and add ordered table rows for each reachable `SLANG_PASS`, with gates matching the two source conditions. |
| F-002 | major | `## Phase B: Specialization and type legalization` | The `shouldLegalizeExistentialAndResourceTypes` diamond draws the false arm as falling through to `legalizeExistentialTypeLayout`, `validateStructuredBufferResourceTypes`, `legalizeResourceTypes`, and the Metal-arm `legalizeEmptyTypes`. In source, the false branch skips the existential/resource block and runs only the else-path `legalizeEmptyTypes` before continuing to matrix legalization. | `source/slang/slang-emit.cpp:1591-1684` contains the true branch with the existential/resource legalization sequence; `source/slang/slang-emit.cpp:1685-1690` shows the false branch is a separate `legalizeEmptyTypes` call. | Either treat this gate as always true for the actual Metal source-emission path and remove the diamond, or redraw the false arm so it goes to the else-path `legalizeEmptyTypes` and then rejoins before `legalizeMatrixTypes`. |
| F-003 | major | `## Phase C: Metal legalization, lowering, phi elimination` | The Phase C ordered table lists `legalizeImageSubscript` before `legalizeIRForMetal`, but the source runs `legalizeIRForMetal` first, then `floatNonUniformResourceIndex`, then `legalizeLogicalAndOr`, and only then `legalizeImageSubscript`. The Phase C diagram has the right order, so the companion table is inconsistent with both the diagram and source. | `source/slang/slang-emit.cpp:1993-1998` runs `legalizeIRForMetal`; `source/slang/slang-emit.cpp:2033-2040` runs the following non-SPIR-V and Metal logical-op passes; `source/slang/slang-emit.cpp:2046-2057` runs `legalizeImageSubscript` afterward. | Move the `legalizeImageSubscript` row after `legalizeLogicalAndOr`, renumber the Phase C table, and keep the diagram and table in the same order. |

## No-issues notes
- The target document front matter contains all required generated-document keys and the requested `source_commit` and digest values.
- The rebuilt Phase B diagram now uses diamond nodes with `true` and `false` arms for many runtime gates instead of omitting gates wholesale.
- The loops section is consistent with `source/slang/slang-ir-metal-legalize.cpp`: the Metal legalizer has ordinary traversal `for` loops but no iterative pipeline loop or fixed-point pass.
- The `DescriptorHandle<T>` parameter-binding description matches `source/slang/slang-emit-metal.cpp:135-198`, including the unwrap and system-semantic fallback.
