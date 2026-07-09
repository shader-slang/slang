---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:25:38+00:00
target_doc: ir-reference/differentiation.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: e27926ca78614bca20d3b57a5268d5884f642e04074ed66afbbed157eadbfdd7
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for ir-reference/differentiation.md

## Summary
The page is mostly source-aligned for the opcode declarations I checked, but it does not fully satisfy its per-document prompt. The main issue is that the required notable-opcode coverage for pair projections and reverse-mode context is missing or left to peer pages without an explicit explanation.

## Items checked
- Ran `regenerate.py show ir-reference/differentiation.md` and reviewed the target document, common contract, per-document prompt, dependency docs, and the resolved watched files.
- Checked front matter for all required generated-doc keys, the recorded source commit, warning string, and 64-character hex watched-path digest.
- Resolved the document's relative links to generated peer pages, watched source files, and `docs/design/autodiff.md`.
- Verified the required IR-reference sections: Source, Family hierarchy, Opcodes, Notable opcodes, and See also.
- Spot-checked source claims for `MakeDifferentialPairBase`, `MakeDiffPair`, `MakeDiffRefPair`, `GetDifferential`, `GetDifferentialPtr`, `GetPrimal`, `GetPrimalRef`, `BuiltinRequirementKey`, `BuiltinRequirementDecoration`, `TranslateBase`, `ForwardDifferentiate`, `BackwardDifferentiate`, `checkpointObj`, `loopExitValue`, `ReportCheckpointStore`, `DiffTypeInfo`, `detachDerivative`, `visitForwardDifferentiateExpr`, `visitBackwardDifferentiateVal`, and `visitBackwardDifferentiateExpr`.
- Checked the prose line-number claims around the Lua ranges for the differential-pair, checkpointing, decoration, built-in requirement key, and `TranslateBase` entries.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Notable opcodes`, lines 225-306 | The per-document prompt says the notable section must cover at least `GetDifferential` / `GetPrimal` and `BackwardDiffPropagateContext`, but the section has no callout for the projection opcodes and no reverse-mode context callout. The page also does not explain why the current source uses nearby names such as `BackwardDiffIntermediateContextType` / `BackwardDiffMinimalContextType` instead of the prompt's `BackwardDiffPropagateContext` spelling. | `docs/generated/design/_meta/prompts/ir-reference-differentiation.md:49` requires those notable entries; `source/slang/slang-ir-insts.lua:172` and `source/slang/slang-ir-insts.lua:189` define the current reverse-mode context-type opcodes. | Add short notable callouts for `GetDifferential` and `GetPrimal`. Add the reverse-mode context callout using the current source opcode names, or explicitly state that the prompt's exact `BackwardDiffPropagateContext` spelling was not found and that the context opcodes are documented in `types.md`. |
| F-002 | minor | `## Opcodes` / Checkpointing and rematerialization table, line 223 | The `detachDerivative` row gives the AST origin as `DetachDerivativeExpr`, but the source visitor and AST class use `DetachExpr`; there is no watched-source evidence for a `DetachDerivativeExpr` visitor. | `source/slang/slang-lower-to-ir.cpp:5730` defines `visitDetachExpr(DetachExpr* expr)` and emits `emitDetachDerivative(...)`; `source/slang/slang-ir-insts.lua:1066` defines the `detachDerivative` opcode. | Change the AST-origin cell from `DetachDerivativeExpr` to `DetachExpr` / `visitDetachExpr`, keeping the `detach(...)` user-facing spelling if desired. |
