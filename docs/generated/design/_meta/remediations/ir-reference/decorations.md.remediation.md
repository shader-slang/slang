---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:10:40Z
target_doc: ir-reference/decorations.md
review_report: ../../reviews/ir-reference/decorations.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/decorations.md

## Summary

All eight findings were verified against source at the recorded commit
and all eight were fixed; none were rejected, deferred, or escalated.
Three operand cells were corrected to match the builders that
construct them, and two callouts had their backend-emission detail
replaced with a link to the emission page. The remaining four fixes
were single-sentence or single-cell corrections.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-insts.h:4708-4712` defines `addSimpleDecoration`; `source/slang/slang-ir.cpp:7390-7393` holds only the out-of-line `addLayoutDecoration`. | `## Source` sentence now splits op flags (`slang-ir.h`), inline builder helpers (`slang-ir-insts.h`), and out-of-line definitions (`slang-ir.cpp`). |
| F-002 | fixed | `source/slang/slang-lower-to-ir.cpp:5910-5943` and 14604-14615 create autodiff markers directly, so the "IR passes themselves" claim was too broad. | Qualified the sentence and named the two AST-lowered exceptions with line numbers. |
| F-003 | fixed | `source/slang/slang-ir-insts.h:4884-4903` builds either a two- or four-operand form; accessors at lines 86-100. | Operand cell gained `predicate?` and `typeScrutinee?`; callout explains the conditional four-operand form. See also F-008 (same row). |
| F-004 | fixed | `source/slang/slang-ir-insts.h:5172-5184` always passes three operands; caller at `source/slang/slang-lower-to-ir.cpp:13702-13706`. | Cell changed to `functionNameOperand: IRStringLit, fwdDiffFunc?, bwdDiffFunc?`. |
| F-005 | fixed | `source/slang/slang-lower-to-ir.cpp:14608-14615` always appends `sideEffectBehavior`, so the decoration is never nullary. | Cell changed from `—` to `sideEffectBehavior: IRIntLit`. |
| F-006 | fixed | `docs/generated/design/_meta/prompts/ir-reference-decorations.md:79-81` forbids backend-specific consumption. | `nodeLaunch`: dropped the HLSL/DXC emission passage, kept the named-constant representation. `shader64BitIndexing`: dropped the emitted-SPIR-V list, kept the entry-point-scoping reason. Both now link `../pipeline/06-emit.md`. |
| F-007 | fixed | `_common.md:65-66` requires coverage and audience in the first body paragraph. | Merged the audience sentence into the opening paragraph. |
| F-008 | fixed | `_common.md:74-79` forbids editorial commentary; the cell read "cite the glossary for `target intrinsic`". | Summary now links `[intrinsic](../glossary.md)` directly. See also F-003 (same row). |
