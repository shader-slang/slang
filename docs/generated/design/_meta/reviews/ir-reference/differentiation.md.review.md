---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:07+00:00
target_doc: ir-reference/differentiation.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 50a5584b2851342292d4b982e8c4767f3127bd44d5e4d4de95333b7b3e0e7fa5
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 1
  major: 0
  minor: 0
  nit: 0
---

# Review report for ir-reference/differentiation.md

## Summary
The page covers the required differential-pair base groups, `TranslateBase` leaves, checkpointing rows, and related notable opcodes. One source-alignment issue is actively misleading: it says `__bwd_diff`/`BackwardDifferentiateExpr` lowers to `BackwardDifferentiate`, but the lowering visitor rejects that expression as unexpected.

## Items checked
- Ran `regenerate.py show ir-reference/differentiation.md` and used its prompt path, watched files, and dependencies.
- Read `_common.md`, `ir-reference-differentiation.md`, the full target document, `cross-cutting/ir-instructions.md`, `pipeline/04-ast-to-ir.md`, `ir-reference/types.md`, and `ir-reference/values.md`.
- Verified front matter keys, target source commit, watched-path digest shape, required IR-reference sections, table columns, and all relative links via source inspection plus pending lint.
- Checked opcode coverage against `source/slang/slang-ir-insts.lua` for `MakeDifferentialPairBase`, `DifferentialPairGetDifferentialBase`, `DifferentialPairGetPrimalBase`, `TranslateBase`, `checkpointObj`, `loopExitValue`, `ReportCheckpointStore`, `DiffTypeInfo`, `BuiltinRequirementKey`, and `detachDerivative`.
- Spot-checked more than 10 factual claims: differential-pair operands, projection operands, `ForwardDifferentiate` AST lowering, `BackwardDifferentiate` AST-origin claim, hoistable flags on `TranslateBase`, legacy bridge operand minima, `FunctionCopy`, synthesized witness-table rows, checkpoint store operands, `detachDerivative` lowering, and built-in requirement key decoration behavior.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Source` and `### Reverse-mode` | The page says user-facing `__bwd_diff` lowering produces `BackwardDifferentiate` and the row lists `BackwardDifferentiateExpr` as the AST origin, but the lowering visitor for `BackwardDifferentiateExpr` calls `SLANG_UNEXPECTED` instead of emitting the opcode. | `source/slang/slang-lower-to-ir.cpp:5675` defines `visitBackwardDifferentiateExpr`; `source/slang/slang-lower-to-ir.cpp:5678` contains `SLANG_UNEXPECTED("BackwardDifferentiateExpr present during IR lowered")`. | Replace the AST-origin text for `BackwardDifferentiate` with the actual source path that creates it, such as the synthesized/Val lowering path, and remove the claim that `__bwd_diff` expression lowering directly emits this opcode. |
