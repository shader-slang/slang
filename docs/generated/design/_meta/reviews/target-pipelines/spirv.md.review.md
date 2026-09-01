---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:03:14+00:00
target_doc: target-pipelines/spirv.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 68a85e13aad997a240500c6924c43cbfb5c7a2705b13eee149bc97d9ad794aeb
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: fail
finding_count: 6
severity_breakdown:
  critical: 1
  major: 2
  minor: 3
  nit: 0
---

# Review report for target-pipelines/spirv.md

## Summary
The page is detailed and mostly tracks the SPIR-V direct-emission source correctly, but it needs remediation in pipeline gating, contract coverage, citations, and front matter. The most important error is Phase D row 20: active downstream compilation is gated by the presence of the downstream compiler, not solely by `needsOptimization`.

## Items checked
- Inspected the target, `_common.md`, the per-document prompt, all eight resolved watched files, and all five dependency documents from `regenerate.py show`.
- Confirmed the watched source files match the target's recorded `source_commit`, then verified every line-number citation; three citation-bearing statements are stale and are reported below.
- Spot-checked more than 20 factual claims, including dispatch, required-pass scans, Phase A-C ordering, SPIR-V legalization, both fixed-point loops, abort lowering, descriptor-heap handling, debug emission, capability selection, and the downstream link/validate/compile chain.
- Ran the document linter and resolved the relative source and generated-peer links; no dangling links were reported.
- Checked every required target-pipeline section, phase diagram/table pairing, conditional-gate grouping, loop coverage, front-matter field, and severity/count invariant.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase D: IR-to-SPIR-V emit, simplification loop, downstream tools`, table row 20 | The row says downstream `compiler->compile` is gated by `needsOptimization`. In source the call runs whenever `compiler` was loaded; `needsOptimization` is only one contributor to `needsDownstreamCompiler`, alongside linking, validation, and separate debug info. This misstates when the final downstream compile executes. | `source/slang/slang-emit.cpp:3387-3404` computes `needsDownstreamCompiler` and loads `compiler`; `source/slang/slang-emit.cpp:3473` calls `compiler->compile` without a surrounding `needsOptimization` test. | Change the Gate cell to `compiler != nullptr` / `needsDownstreamCompiler`, and state that `needsOptimization` is one reason the compiler may be loaded rather than the direct call gate. |
| F-002 | major | Front matter, lines 1-7 | `watched_paths_digest` is stale. The document records `68a85e...`, but the resolved watched files are clean at the recorded `source_commit` and `regenerate.py digest target-pipelines/spirv.md` computes `acce7b597096fbbe5e6eff92e308e01122e6271397aec68a4f310b729f497200`. | `docs/generated/design/target-pipelines/spirv.md:5-6`; the manifest entry's watched-path set is unchanged, and `git diff --quiet 53b76e6d... -- <resolved watched files>` succeeds. | Replace the front-matter digest with `acce7b597096fbbe5e6eff92e308e01122e6271397aec68a4f310b729f497200` during remediation and refresh it through the normal freshness workflow. |
| F-003 | major | `## Conditional gates`, lines 825-927 | The required consolidated table does not cover every gate drawn in the phase diagrams. In particular, Phase C's `target != PyTorchCppBinding && targetCaps imply descriptor_handle` gate and Phase D's `compiler loaded?` / `needsDownstreamCompiler` gate have no rows; the explicitly required simplification-mode group is also folded into option toggles instead of appearing in prompt order. | `source/slang/slang-emit.cpp:2726-2734` contains the descriptor-handle gate; `source/slang/slang-emit.cpp:3393-3404` contains the downstream-compiler gate. The contract is `docs/generated/design/_meta/prompts/_common.md:340-348`, with group order refined by `docs/generated/design/_meta/prompts/target-pipelines-spirv.md:78-117`. | Add rows for both missing gates and split simplification-mode predicates into their required group between context predicates and SPIR-V runtime predicates. |
| F-004 | minor | Phase tables, lines 417-484, 639-686, and 790-819 | The companion tables do not consistently use the required one-row-per-node, 1-based order. Phase B combines the separate `finalizeAutoDiffPass` and `stripAutoDiffDecorations` nodes into row 9; Phase C labels consecutive rows `44`, `44`, `46`; Phase D uses `5a` through `5g`. | `source/slang/slang-emit.cpp:1446-1452` has two distinct `SLANG_PASS` calls. The table contract at `docs/generated/design/_meta/prompts/_common.md:329-338` requires one row per pass node and a 1-based order. | Split Phase B row 9 and renumber each affected table with consecutive integer row numbers while retaining loop membership in Notes. |
| F-005 | minor | Lines 205, 217, and 643 | Three line citations are materially stale: the metadata object is created at line 1019, not `~944`; the non-Khronos `glslSSBO` branch is at line 1057, not 983; and `translateGlobalVaryingVar` is invoked at line 2188, not `~1996`. | `source/slang/slang-emit.cpp:1015-1020`, `source/slang/slang-emit.cpp:1057-1058`, and `source/slang/slang-emit.cpp:2188`. | Refresh those three citations to the recorded source commit; retain the other verified line citations unchanged. |
| F-006 | minor | `## Source`, lines 112-115 | The `slang-target-program.h` bullet says that header declares both `TargetProgram::shouldEmitSPIRVDirectly` and the `OptionSet` accessors. It only declares the forwarding `TargetProgram` method; the cited option accessors are in `slang-compiler-options.h`. | `source/slang/slang-target-program.h:113-116`; `source/slang/slang-compiler-options.h:340` and `source/slang/slang-compiler-options.h:380`. | Limit the existing bullet to `TargetProgram::shouldEmitSPIRVDirectly` and add or reference a `slang-compiler-options.h` source bullet for the `OptionSet` accessors. |

## No-issues notes
- The direct-emit dispatch and SPIR-V Assembly indirection match `slang-code-gen.cpp`.
- The page correctly records that the nominal 8/16 simplification bounds are unenforced because neither counter is incremented.
- The active spirv-link and spirv-val conditions, disabled in-source `optimizeSPIRV`, and validation of the freshly emitted buffer match source.
