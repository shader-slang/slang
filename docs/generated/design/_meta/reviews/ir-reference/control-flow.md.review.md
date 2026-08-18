---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:19:17+00:00
target_doc: ir-reference/control-flow.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 64be22b621bde4e26ac349ba999894219b13a0f0d103c6e61d02970a8258d1bc
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: fail
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 1
  major: 1
  minor: 1
  nit: 0
---

# Review report for ir-reference/control-flow.md

## Summary

The opcode catalog and its links are largely accurate, but the side-effect-cache guidance is unsafe: a stale cached `false` can let DCE remove a call whose associated callee has become effectful. The opcode tables also miss the prompt's required per-subtable visitor citations, and one source line range excludes an emitter it claims to cover.

## Items checked

- Verified all 48 explicit line-number or line-range citations against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Verified more than 30 factual claims covering Lua opcode membership, wrapper types, operand layouts, flags, AST origins, block-parameter ordering, builder behavior, and side-effect analysis.
- Confirmed all 32 relative-link occurrences resolve at the recorded source commit and all generated peer links name manifest pages.
- Checked every concrete `TerminatorInst` child plus `block`, `param`, `discard`, the backend-hint group, and `gpuForeach` against `slang-ir-insts.lua`.
- Ran identifier and whole-filename sweeps; generated wrapper/range identifiers absent from checked-in source were confirmed as FIDDLE-generated.
- Confirmed the front matter, watched-path digest, required sections, table columns, dependencies, and structural lint.

## Findings

| ID    | Severity | Location                                                           | Description                                                                                                                                                                                                                                                                                                        | Evidence                                                                                                                                                                                                                                                                           | Recommendation                                                                                                                                                                                                                                                                                              |
| ----- | -------- | ------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | critical | `### Moving and deleting control-flow instructions`, lines 407-417 | The claim that a stale `calleeSideEffectCache` entry “is conservative” is unsafe. `doesCalleeHaveSideEffect` may cache `false`; adding an `IRAnnotation` that associates an effectful callee changes the correct answer to `true`, so reusing the stale value can make DCE eliminate a call with effects.          | `source/slang/slang-ir-insts.h:3620-3623` explicitly says adding an annotation changes the result and “must not happen while a callee-side-effect cache is live.” `source/slang/slang-ir-util.cpp:1670-1708` shows both the associated-callee scan and unconditional cache lookup. | Replace the stale-entry claim with the actual lifetime rule: share the cache only while annotations and purity facts are unchanged, and clear or discard it before such mutations. Mention that `simplifyIR` clears it each outer iteration (`source/slang/slang-ir-ssa-simplification.cpp:60-80`).         |
| F-002 | major    | `## Opcodes`, lines 126-199                                        | None of the nine opcode subtables has an AST-origin cell that names a `visit*` visitor, despite the per-document quality checklist requiring at least one `slang-lower-to-ir.cpp` visitor citation in every subtable. Several cells cite the filename only as backticked prose rather than a Markdown source link. | `docs/generated/design/_meta/prompts/ir-reference-control-flow.md:63-67` states the per-subtable requirement; `docs/generated/design/_meta/prompts/_common.md:43-45` requires source-file citations to be Markdown links.                                                          | Regroup into the four requested subtable categories, or update every retained subtable with a direct AST producer to name and link at least one exact visitor (for example, `ReturnStmt` via `visitReturnStmt`). Merge an all-synthesized subtable into a compatible group rather than inventing a visitor. |
| F-003 | minor    | `## Source`, lines 55-58                                           | The text includes `emitBlock` among emitters “defined in `slang-ir.cpp` at lines 6331-6560,” but `IRBuilder::emitBlock` is at line 5447; the cited range begins with `emitReturn`.                                                                                                                                 | `source/slang/slang-ir.cpp:5447-5452` defines `IRBuilder::emitBlock`; `source/slang/slang-ir.cpp:6331-6560` covers return/yield and branch/loop/if/switch emitters instead.                                                                                                        | Give `emitBlock` its own line citation at 5447, or remove it from the list attributed to the 6331-6560 range.                                                                                                                                                                                               |

## No-issues notes

- Every concrete opcode under the Lua `TerminatorInst` group appears in the tables.
- The mandatory generated-document front matter is complete and the digest matches the driver.
- All source, dependency, peer, pipeline, glossary, and design-document links resolve.
