---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:05:49+00:00
target_doc: target-pipelines/cuda.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 14e144c55f95a3a6bcf4a07633067a3feb34968de49ae572e8b9c5be07287d5b
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 1
  major: 2
  minor: 2
  nit: 0
---

# Review report for target-pipelines/cuda.md

## Summary
The page has one critical target-filtering error: CUDA artifacts are not CPU-like according to the predicate used by `linkAndOptimizeIR`, so three Phase-B pass decisions are reversed. It also omits an internal iterative CUDA legalization loop and two reachable DCE call sites. Links, front matter, and almost all line citations are sound.

## Items checked
- Reviewed the target, `_common.md`, the per-document prompt, all resolved watched files, and all five `depends_on` documents at source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Spot-checked more than 30 factual claims across the CUDA dispatch, pass gates, OptiX handling, emitter construction, immutable-load lowering, PyTorch-adjacent handling, capabilities, and downstream PTX path.
- Verified every line-number citation in the body, appearing on 108 document lines, against the cited source at the recorded commit.
- Resolved relative links and peer generated-document references; `regenerate.py lint target-pipelines/cuda.md` completed without errors.
- Compared every CUDA-reachable `SLANG_PASS` call in `linkAndOptimizeIR` with the phase diagrams and tables.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Phase B and Conditional gates; lines 212-215, 293-306, 381-393, 762, 800 | The page classifies CUDA as CPU-like, marks `performTypeInlining` and `checkGetStringHashInsts` skipped, and says `lowerCombinedTextureSamplers` runs through the CPU-like fallthrough. A CUDA source/header artifact has payload `CUDA`, while `isCpuLikeTarget` accepts source payloads only `C` or `Cpp`; therefore both former passes can run and the combined-sampler switch breaks before its pass. | `source/compiler-core/slang-artifact-desc-util.cpp:306-315` maps CUDA source/header to payload `CUDA`; `source/compiler-core/slang-artifact-desc-util.cpp:602-620` defines the predicate; `source/slang/slang-emit.cpp:1633-1651` gates the two inlining/hash passes on its negation; `source/slang/slang-emit.cpp:1758-1771` uses it for combined samplers. | Remove `lowerCombinedTextureSamplers` from the CUDA sequence, add `performTypeInlining` and conditionally `checkGetStringHashInsts`, and correct the related prose/gate rows. The per-document prompt's contrary CPU-like instruction also needs an out-of-band correction. |
| F-002 | major | `## Loops in the pipeline`; lines 817-832 | The page says CUDA has no iterative pass and calls terminate-reaching inlining a single traversal. `legalizeEntryPointVaryingParamsForCUDA` invokes a changed-driven inlining loop that repeatedly flattens terminate-reaching calls, with `maxIterations = reachable.getCount() + 1` and an assertion on the bound. | `source/slang/slang-ir-legalize-varying-params.cpp:1937-1958` contains the loop and bound; `source/slang/slang-ir-legalize-varying-params.cpp:2570-2577` invokes it from the CUDA legalization entry point. | Replace the single-traversal claim with the loop's condition, body, convergence argument, and bound; note that the orchestrator invokes the enclosing pass once. |
| F-003 | major | Phase B diagram/table; lines 250-409 | Two reachable `SLANG_PASS(eliminateDeadCode, ...)` calls have no distinct diagram node or table row: the post-sum-reduction call under minimum optimization plus `requiredLoweringPassSet.generics`, and the post-type-legalization alternative to `simplifyIR`. Mentioning the latter only in another row's Notes cell does not satisfy the one-node/one-row coverage rule. | `source/slang/slang-emit.cpp:1589-1596` and `source/slang/slang-emit.cpp:1938-1941`; target-pipeline contract in `docs/generated/design/_meta/prompts/_common.md:314-338,364-368`. | Add both DCE call sites as gated nodes and ordered rows, with separate order labels, and draw their mutually exclusive branches against the corresponding `simplifyIR` calls. |
| F-004 | minor | Phase B diagram; lines 310 and 313 | Two embedded line annotations are stale: `inlineGlobalConstantsForLegalization` says line 1624 and the CUDA `legalizeEmptyTypes` branch says line 1728. | The calls are at `source/slang/slang-emit.cpp:1791-1795` and `source/slang/slang-emit.cpp:1907-1911`. | Change the diagram labels to line 1791 and line 1911. |
| F-005 | minor | Intro; lines 12-32 | The first body paragraph explains the subject but never identifies the intended reader, despite the universal contract and per-document prompt naming compiler developers as the audience. | `docs/generated/design/_meta/prompts/_common.md:65-66`; `docs/generated/design/_meta/prompts/target-pipelines-cuda.md:12-15`. | Add a short intended-reader clause to the first paragraph, focused on compiler developers locating CUDA pass order, gates, and OptiX handling. |

## No-issues notes
- The ordinary final-PTX path is now correctly described as `PTX` to `CUDASource` source emission followed by NVRTC.
- CUDA-specific gates for OptiX uniform collection, active-mask synthesis, varying-parameter legalization, and immutable loads match the source.
- The remaining line-number citations, including the CUDA emitter and artifact-wrapping locations, are accurate.
