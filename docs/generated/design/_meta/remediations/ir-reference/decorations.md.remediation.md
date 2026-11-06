---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:16:10Z
target_doc: ir-reference/decorations.md
review_report: ../../reviews/ir-reference/decorations.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/decorations.md

## Summary

All six findings were verified against the current source and fixed in the document. The operand cells, AST-origin column, wrapper markers, hierarchy diagram and the missing autodiff callout were corrected; summaries were shortened. Producer claims were checked by grepping construction sites in `source/slang`; opcodes with only an `IRBuilder` helper and no caller are labelled `no producer at HEAD`. The sweep for unlinked source basenames (F-006) covered the cited rows only.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-insts.lua` gives only `min_operands` for these entries, so `(N unnamed)` is the contract form. | All 22 `(variadic, min=N)` cells became `(N unnamed)`. |
| F-002 | fixed | Producers confirmed in `source/slang/slang-lower-to-ir.cpp` (e.g. `visitTreatAsDifferentiableExpr` ~5385, `addLinkageDecoration` ~1431, `lowerMemberVarDecl` ~12843) and in the IR passes. | Replaced every bare `(synthesized)` / `(synthesized by autodiff)` cell with a named producer or `no producer at HEAD`; corrected import/export and call markers. |
| F-003 | fixed | `source/slang/slang-ir.h.lua` ~151 skips structs already defined in `slang-ir-insts.h`; 64 listed wrappers have a hand-written `struct`. | Marked the 64 wrappers with a double-dagger and stated the 64/137 split in `## Source`. |
| F-004 | fixed | `source/slang/slang-ir-insts.lua` nests `LinkageDecoration` (2252) and `DerivativeGroupDecoration` (2390) under `Decoration`. | Diagram now shows only Lua groups, adds those two with children, and a sentence separates page buckets from schema nodes. |
| F-005 | fixed | `Differentiable` / `BackwardDifferentiable` are not opcodes in `slang-ir-insts.lua`; conformance representation per `source/slang/slang-lower-to-ir.cpp` ~10821. | Added a callout under `## Notable opcodes` linking `differentiation.md` and the lowering page; no rows added. |
| F-006 | fixed | Multi-sentence summaries violate the one-clause rule. | Shortened the summaries for `BuiltinDecoration`, `requireCapabilityAtom`, `interpolationMode`, `format`, `streamOutputTypeDecoration`; removed backticked autodiff filenames and line numbers from `AutodiffParameterContextTypeDecoration`. |
