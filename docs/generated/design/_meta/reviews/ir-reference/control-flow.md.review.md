---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:05:06Z
target_doc: ir-reference/control-flow.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 06367290cedb9969bf17dfd68997c182e530e14c2cf4a25816d819c1b11385bb
source_commit: 8341c7e537a3f5cb3a60cd062066ffd630da40d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 6
severity_breakdown:
  critical: 1
  major: 2
  minor: 3
  nit: 0
---

# Review report for ir-reference/control-flow.md

## Summary

The document covers all 26 catalogued opcodes, and its source citations are generally accurate. The most important correction is that CFG simplification before the first IR dump is conditional, so the document overstates which block shapes readers will observe. The opcode tables also need several contract-required metadata corrections.

## Items checked

- Reviewed against the **current working tree** as requested; verified that `git diff 2518a942d59b1228fc8cc70a37739baab82b6002 -- source/` is empty and HEAD matches the supplied review commit.
- Read `_review.md`, `_common.md`, the document-specific prompt, and the resolved watched paths from `regenerate.py show ir-reference/control-flow.md`.
- Checked every body line-number citation, covering 82 distinct cited positions, including range endpoints and their surrounding implementations.
- Spot-checked more than 20 behavioral claims: parameter placement, branch arguments, loop labels, convenience branch emitters, switch accessors, bool-switch legalization, error handling, defer lowering, missing-return diagnostics, unreachable-block handling, discard classification, and side-effect caching.
- Compared all 26 opcode rows with the Lua declarations and C++ wrappers; checked the required headings, columns, producer attribution, and notable-opcode coverage.
- Resolved all 39 Markdown links, representing 23 distinct targets, in the working tree and at the document’s recorded commit. Verified all eight linked generated pages have manifest entries; consulted dependency documents only for cross-reference checks.
- Checked required front-matter fields, Mermaid syntax, Liquid safety, and the size limit: 32,619 bytes against 65,536. No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### What a lowering dump shows`, lines 214–221; `### ifElse`, lines 305–311 | The document calls the pre-dump sequence a `fixed block of mandatory passes` and says `the mandatory simplifyCFG` folds empty arms. This incorrectly makes the resulting dump shape unconditional. | `source/slang/slang-lower-to-ir.cpp:15641` computes `minimumOptimizations`; lines 15642–15647 place `simplifyCFG` inside `if (!minimumOptimizations)`. The early-inlining loop’s simplification is similarly gated at line 15701. | State that `simplifyCFG` runs before the dump when minimum optimizations are disabled. Describe empty-arm folding as a possible result under that configuration, rather than a guaranteed dump shape. |
| F-002 | minor | `### missingReturn and unreachable`, lines 430–433 | The explanation attributes the missing-return source location to the declaration-location scope at line 14379. That scope belongs only to optional function-coverage instrumentation and ends before body lowering and `emitMissingReturn`. | `source/slang/slang-lower-to-ir.cpp:14370` gates coverage instrumentation; the RAII scope at line 14379 ends at line 14383. `lowerDecl` establishes the declaration source-location scope at line 14834, while `lowerStmt` temporarily overrides it at line 9719. | Replace the line-14379 explanation with the enclosing declaration-location scope in `lowerDecl`, noting that statement-local source-location scopes restore the previous location before the implicit terminator is emitted. |
| F-003 | major | `## Opcodes`, lines 132–208 | Required table metadata is incomplete. Hand-written wrappers have no explicit marker or count, and unnamed Lua operands use forms such as `target, args... (min=1)` without the required accessor-derived marker and legend. | `docs/generated/design/_meta/prompts/_common.md:271–272` requires hand-written-wrapper identification/count and either `(N unnamed)` or accessor-derived names marked `†`. Examples of unnamed declarations occur in `source/slang/slang-ir-insts.lua:1462–1470`; their accessors are in `source/slang/slang-ir-insts.h:2105–2137`. | Mark the 20 hand-written wrappers and state that count; retain the six generated-wrapper labels. Normalize unnamed operand cells to `(N unnamed)` or mark accessor-derived names with `†` and explain the legend. Use the prescribed variadic notation where applicable. |
| F-004 | major | `## Opcodes`, especially lines 138–139 and 162 | The `conditionalBranch` origin remains `(synthesized)`, which the contract explicitly retires. The `param` row names `ParamDecl` without its visitor, and the `block` row identifies its structural role without naming a construction helper. | `docs/generated/design/_meta/prompts/_common.md:274` requires actual producers. `source/slang/slang-ir.cpp:6521–6529` constructs `conditionalBranch` in the three-argument `IRBuilder::emitBranch`; `emitBlock` constructs/inserts blocks at line 5488. | Name the verified three-argument `IRBuilder::emitBranch` producer for `conditionalBranch`; substantiate any additional pass attribution with a caller. Add the actual parameter-lowering visitor for `ParamDecl`, and name `createBlock`/`emitBlock` while retaining the explanation that `block` has no single AST origin. |
| F-005 | minor | `## Opcodes`, subsection organization | The tables cover the required opcodes but do not use the requested grouping into Block and parameters, Branches, Function exits, and Other terminators. | `docs/generated/design/_meta/prompts/ir-reference-control-flow.md:16–24` explicitly prescribes those four groups. | Reorganize the existing rows under those four groups. Preserve the explicit explanation that `discard` is not an IR terminator and retain a separate group for additional non-terminator markers. |
| F-006 | minor | `### Moving and deleting control-flow instructions`, lines 517–542 | The detailed call-side-effect cache analysis and simplification-iteration discussion exceeds the per-family opcode scope and the required short notable-opcode callout format. | `docs/generated/design/_meta/prompts/_common.md:276–286` specifies short opcode-focused callouts; lines 303–309 exclude pass-by-pass behavior descriptions. The extended material describes `source/slang/slang-ir-util.cpp:2031–2104` and `source/slang/slang-ir-ssa-simplification.cpp:60–110`, rather than the listed control-flow opcodes. | Keep the verified statement that terminators, `discard`, `Printf`, and `Abort` conservatively report side effects. Replace the cache implementation and iteration discussion with a brief link to the pass documentation or relevant source. |