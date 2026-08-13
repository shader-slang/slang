---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:32+00:00
target_doc: ir-reference/decorations.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 6ca22e11b1ae848bc68390906f1d20589efa4eb3e3366532aa60f8ccaecd4b6c
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 8
severity_breakdown:
  critical: 0
  major: 0
  minor: 6
  nit: 2
---

# Review report for ir-reference/decorations.md

## Summary

The page exhaustively lists all 196 concrete decoration opcodes, and all links, line citations, and front-matter values check out at the recorded commit. Eight non-blocking findings remain: several operand shapes and producer statements omit source-supported cases, one infrastructure helper is assigned to the wrong file, and a few passages miss the prompt's content/style rules. The most important correction is to document the optional predicate and type-scrutinee operands of `targetIntrinsic`.

## Items checked

- Ran `regenerate.py show ir-reference/decorations.md`; reviewed the common and per-doc prompts, all three dependency pages, and all six resolved watched files at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Compared the table's opcode set to the stable-name entries: 196 unique table rows exactly match the 196 concrete `Decoration` leaves.
- Verified all 13 line-number mentions, including the `1752-2702` Lua range and every cited entry point in `slang-lower-to-ir.cpp`.
- Spot-checked more than 10 factual claims, including `nameHint`, `layout`, `targetIntrinsic`, `intrinsicOp`, `KeepAliveDecoration`, `entryPoint`, work-graph decorations, `shader64BitIndexing`, `synthesizedParameterGroup`, constrained fragment depth, `BuiltinRequirementDecoration`, and the autodiff markers.
- Resolved all 30 markdown link occurrences (19 unique targets) and confirmed generated peer targets are present in the manifest.
- Recomputed the watched-path digest, confirmed all mandatory front-matter keys, and measured 50,687 bytes against the 98,304-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Source`, lines 37-41 | The text places `addSimpleDecoration<T>` and the per-decoration `add*Decoration` helpers in `slang-ir.h` / `slang-ir.cpp`, but `addSimpleDecoration<T>` and most inline helpers are declared in `slang-ir-insts.h`. | `source/slang/slang-ir-insts.h:4708-4712` defines `IRBuilder::addSimpleDecoration`; `source/slang/slang-ir.cpp:7390-7393` only defines the out-of-line `addLayoutDecoration` example. | Add `slang-ir-insts.h` to the infrastructure sentence and state that `slang-ir.cpp` contains the out-of-line helper definitions. |
| F-002 | minor | `## Source`, lines 55-58 | The blanket claim that “The autodiff markers ... are introduced by the IR passes themselves” is too broad. AST lowering directly creates call differentiability markers and the checkpoint/recompute decorations. | `source/slang/slang-lower-to-ir.cpp:5890-5943` emits `TreatCallAsDifferentiableDecoration` / `DifferentiableCallDecoration`; lines 14604-14615 emit `PreferCheckpointDecoration` / `PreferRecomputeDecoration`. | Qualify the sentence: primal/differential transcription markers come from autodiff passes, while the named call and checkpoint hints can originate in AST lowering. |
| F-003 | minor | `targetIntrinsic` row and `### targetIntrinsic / TargetIntrinsicDecoration` | The page says `targetIntrinsic` has exactly two operands, but predicate-bearing target intrinsics have four: target capabilities, definition, predicate string, and type scrutinee. | `source/slang/slang-ir-insts.h:86-100` exposes the predicate/scrutinee accessors; lines 4884-4902 construct either the two- or four-operand form. | Change the operand cell to include optional `predicate` and `typeScrutinee`, and mention the conditional four-operand form in the notable-opcode callout. |
| F-004 | minor | `AutoPyBindCUDA` row | The row lists only `functionNameOperand`, although lowering creates a three-operand decoration carrying optional forward- and backward-derivative functions. | `source/slang/slang-ir-insts.h:443-449` exposes operands 1 and 2; lines 5172-5183 construct all three operands. | Change the operand cell to `functionNameOperand: IRStringLit, fwdDiffFunc?, bwdDiffFunc?`. |
| F-005 | minor | `PreferRecomputeDecoration` row | The operand cell is `—`, but lowering always appends the attribute's `sideEffectBehavior` as an integer operand. | `source/slang/slang-lower-to-ir.cpp:14608-14615` passes an `IRIntLit` containing `attr->sideEffectBehavior` to the decoration. | Replace `—` with `sideEffectBehavior: IRIntLit`. |
| F-006 | minor | `### nodeLaunch and the work-graph node decorations`; `### shader64BitIndexing` | These callouts give backend-specific HLSL and SPIR-V consumption details even though the per-doc prompt explicitly assigns backend consumption to the emission page. | `docs/generated/design/_meta/prompts/ir-reference-decorations.md:75-81` forbids backend-specific consumption; the target describes exact HLSL text at lines 450-456 and SPIR-V emission at lines 491-494. | Keep the IR representation and producer facts, then replace backend emission details with a link to `../pipeline/06-emit.md`. |
| F-007 | nit | Opening paragraphs, lines 12-23 | The first paragraph says what the page covers, but the intended reader appears in a separate second paragraph; the common contract requires both in the first paragraph. | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first body paragraph to state coverage and audience. | Merge the audience sentence into the opening paragraph. |
| F-008 | nit | `targetIntrinsic` row, line 150 | The summary ends with the editorial instruction `cite the glossary for target intrinsic` instead of an actual cross-reference. | `docs/generated/design/_meta/prompts/_common.md:74-79` forbids editorial commentary; the glossary link already exists later in the page. | Remove the instruction or replace `target intrinsic` with a direct relative link to `../glossary.md`. |

## No-issues notes

- The family hierarchy reflects the Lua nesting and uses valid Mermaid node identifiers.
- Every table has the six required columns in the required order, with exactly one row per concrete opcode.
- The recorded digest is valid hexadecimal and exactly matches `regenerate.py digest ir-reference/decorations.md`.
