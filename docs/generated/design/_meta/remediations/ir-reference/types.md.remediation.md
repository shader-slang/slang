---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:34:00Z
target_doc: ir-reference/types.md
review_report: ../../reviews/ir-reference/types.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for ir-reference/types.md

## Summary

Four findings were fixed and one was deferred. All 170 `C++ wrapper` cells now
carry the `IR` prefix, and every resulting symbol was verified to exist as a
real `struct IR...` in the generated or hand-written headers. Eleven subtables
gained a row-level producing-visitor citation, the `Empty*` count was corrected,
and the intended-reader sentence was merged into the opening paragraph. The
operand-vocabulary finding was deferred because it needs a contract decision.
The document was edited, so `mark-fresh` is needed.

## Actions

| Finding ID | Action   | Rationale                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     | Fix summary                                                                                           |
| ---------- | -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| F-001      | fixed    | Confirmed: cells gave Lua `struct_name` values, but `docs/generated/design/_meta/prompts/_common.md:237` requires the `IRFoo` symbol, and the page's own prose at line 46 already says the wrapper is `IR` plus `struct_name`. Every prefixed name was cross-checked against `struct IR...` declarations in `build/source/slang/fiddle/slang-ir-insts.h.fiddle`, `source/slang/slang-ir-insts.h`, and `source/slang/slang-ir.h`; zero missing, including the sentinel `IRAfterBaseType`.                                                                                                                                                                                                                                                                      | Prefixed the wrapper cell in all 170 opcode rows with `IR`; the hand-written `‡` marker is unchanged. |
| F-002      | deferred | Valid contract deviation, but the fix is page-wide and needs a contract decision first. About 30 rows use the `†` marker, documented in the legend at lines 167-175, which names the real operand taken from the C++ accessor; `(see Lua)` per `ir-reference-types.md:37-39` would delete verified information, and 11 more rows use `name...` where `_common.md:238` wants `(variadic)`. The recommendation additionally asks that inferred layouts be relocated into notable-opcode prose. Follow-up: either amend `ir-reference-types.md:35-39` and `_common.md:238` to bless the `†`/`...` markers, or schedule the row rewrite as its own cycle.                                                                                                         | —                                                                                                     |
| F-003      | fixed    | The prompt checklist (`ir-reference-types.md:84-85`) requires one producing visitor or builder helper per subtable. Verified producers in `source/slang/slang-lower-to-ir.cpp`: `visitBasicExpressionType` at 2839, `visitArrayExpressionType` at 2861, `visitFuncType` at 2718, `lowerSimpleIntrinsicType` at 2879 (the `__intrinsic_type` path for the core-module scalar, string, raw-pointer, sampler, tensor, and work-graph subtables), `getWitnessTableType` at 2395, and `getRateQualifiedType` at 4556. Two subtables (`SPIR-V literals and kinds`, `Set-theoretic types`) remain uncited because every opcode in them is synthesized by IR passes and has no lowering producer at HEAD; the checklist item is inapplicable there rather than unmet. | Added a producer citation to one row in each of 11 subtables.                                         |
| F-004      | fixed    | Confirmed: `source/slang/slang-ir-insts.lua` has exactly three `Empty*` work-graph entries, at lines 252, 277, and 278, matching the three rows in the subtable.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              | Line 665: "four `Empty*` variants" changed to "three".                                                |
| F-005      | fixed    | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first body paragraph to state coverage and intended reader; the audience sentence sat in a separate paragraph.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            | Removed the paragraph break so the intended-reader sentence joins the opening paragraph.              |
