---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:06:54+00:00
target_doc: target-pipelines/metal.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 3bfc164e382505a7acce894d60950a1812eb10280d5da247c705758df95dccb7
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: partial
finding_count: 7
severity_breakdown:
  critical: 0
  major: 2
  minor: 5
  nit: 0
---

# Review report for target-pipelines/metal.md

## Summary
The page is unusually thorough and its main Metal ordering, legalization, emitter, and downstream-chain descriptions agree with the source. It nevertheless misses a reachable `SLANG_PASS` despite the target-pipeline coverage rule, and its recorded watched-path digest does not match the watched files at the recorded source commit. Five smaller contract and factual issues remain in the phase table, opcode count, introduction, gate labeling, and See-also list.

## Items checked
- Ran `regenerate.py show target-pipelines/metal.md`; inspected the target, `_common.md`, the per-document prompt, all nine resolved watched files, and all five dependency documents.
- Verified every line-number citation against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, including all cited regions in `slang-emit.cpp`, the Metal emitter/legalizers, downstream codegen, diagnostics, core-module intrinsics, and IR-builder factories.
- Spot-checked more than 10 factual claims: target transitions, required-pass scans, coverage-width capping, Metal byte-address options, three buffer-element policies, parameter fallthrough, subpass and varying-output legalization, address-space specialization, descriptor-handle bindings, `printf`, `precise`, literal suffixes, Metal opcode producers, and Apple-tool flags.
- Ran the generated-doc linter, resolved every markdown link, checked peer generated pages against the manifest, and recomputed the watched-path digest.
- Compared all four phase diagrams and tables with reachable Metal call sites and checked the required section order, loop statement, Mermaid conventions, size cap, warning string, and front-matter keys.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Front matter, line 6 | The recorded `watched_paths_digest` is `3bfc164e...dccb7`, but recomputing the SHA-256 over the resolved watched files at the recorded source commit produces `efe89af9...5af1`. Because `source_commit` equals HEAD and the watched source files are clean, this is not worktree drift. | `docs/generated/design/_meta/regenerate.py:441-457` defines the digest over resolved watched-file paths, sizes, and contents; `regenerate.py digest target-pipelines/metal.md` returned `efe89af9797e1b4513603c9cb5e2d6d902ddb4a58cb88ad67f7d706be8145af1`. | Replace the front-matter digest with `efe89af9797e1b4513603c9cb5e2d6d902ddb4a58cb88ad67f7d706be8145af1` when remediating the generated page, then use the normal freshness workflow to record the same value. |
| F-002 | major | Phase B, lines 507-512; Option-set toggles, line 850 | The page explicitly excludes `addUserTypeHintDecorations` from the ordered Phase B diagram/table because it has no Metal-specific behavior, even though the call is reachable on Metal whenever `VulkanEmitReflection` is set. The contract requires every reachable `SLANG_PASS` call to appear in exactly one phase table. | `source/slang/slang-emit.cpp:1774-1777` calls `SLANG_PASS(addUserTypeHintDecorations)` based only on the option; `docs/generated/design/_meta/prompts/_common.md:364-368` requires every reachable call in a phase table. | Add an option-gated `addUserTypeHintDecorations` node and Phase B row after `lowerCombinedTextureSamplers`, and keep its option in the consolidated gate table. Delete the rationale for omitting it. |
| F-003 | minor | Phase B diagram and table, lines 345-348, 382-384, 399-403, 429, 461-463, 482 | The companion table is not one row per diagram pass node: it combines `finalizeAutoDiffPass` with `stripAutoDiffDecorations`, omits the diagram's `dce4` / direct `eliminateDeadCode`, and combines the final alternative `simplifyIR` and `eliminateDeadCode` nodes. | `source/slang/slang-emit.cpp:1446-1453`, `1654`, and `1938-1941` contain the distinct calls; `docs/generated/design/_meta/prompts/_common.md:329-338` requires one table row per pass node. | Split rows 9 and 62 into one row per alternative pass, add the missing `eliminateDeadCode` row between `checkGetStringHashInsts` and `lowerTuples`, and renumber Phase B. |
| F-004 | minor | Phase C table row 22, line 629 | The Gate cell says `isMetalTarget` selects `lowerBufferElementTypeToStorageType`, but this invocation is unconditional; `isMetalTarget` only selects the `Metal` policy configuration. | `source/slang/slang-emit.cpp:2463-2479` selects a policy at lines 2470-2472 and calls the pass unconditionally at lines 2476-2479. | Change the Gate cell to `(always)` and move `isMetalTarget` / `BufferElementTypeLoweringPolicyKind::Metal` into Notes. |
| F-005 | minor | `### Where the Metal-specific opcodes come from`, lines 1114-1146 | The text says `Four opcodes in the IR are Metal-only`, then names five: three `metalSet*` ops, `MetalCastToDepthTexture`, and `MetalAtomicCast`. Its later producer analysis correctly distinguishes four reachable opcodes from the unproduced fifth. | `source/slang/slang-ir-insts.lua:1379-1382` declares the first four, and line 1679 declares `MetalAtomicCast`. | Say that the subsection covers five Metal-specific value/resource opcodes: four have producers at this commit, while `MetalAtomicCast` has no producer. |
| F-006 | minor | Introduction, lines 12-40 | The required one-paragraph introduction is split across multiple paragraphs and never identifies the intended reader, despite the per-document prompt naming that audience. | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first paragraph to state coverage and intended reader; `docs/generated/design/_meta/prompts/target-pipelines-metal.md:12-14` defines the audience; the page's first paragraph at lines 12-33 states only scope and behavior. | Consolidate the target values and the `05-ir-passes.md` relationship into one opening paragraph and add that it is for compiler developers locating Metal pass order, gates, and emitter cooperation. |
| F-007 | minor | `## See also`, lines 1185-1199 | The See-also list omits the existing user-facing Metal target page, which the target-pipeline contract requires when one exists. | `docs/user-guide/a2-02-metal-target-specific.md` exists; `docs/generated/design/_meta/prompts/_common.md:355-362` requires the relevant user-facing target documentation. | Add a relative link to `../../../user-guide/a2-02-metal-target-specific.md` with a one-clause description. |

## No-issues notes
- All source and generated-peer links resolve, all cited peer pages are manifest entries, and the generated-doc linter reports no errors.
- The public `MetalLibAssembly` route is now described correctly as Metal source to `MetalLib` followed by `metal-objdump --disassemble`.
- The source-backed descriptions of Metal legalization, pointer lowering, emitter extension tracking, coverage capping, and downstream language-version/logging flags are accurate.
