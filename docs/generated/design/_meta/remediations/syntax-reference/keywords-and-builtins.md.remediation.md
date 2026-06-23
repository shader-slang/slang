---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:13:54Z
target_doc: syntax-reference/keywords-and-builtins.md
review_report: ../../reviews/syntax-reference/keywords-and-builtins.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/keywords-and-builtins.md

## Summary

The review contained one minor finding, which was fixed. F-001 reported
a stale source citation for the `new` expression keyword, and
verification against `slang-parser.cpp` at the target commit confirmed
it. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified at commit eb9403ef595a99c2ff6def1d538dbd7a792d9371: `parsePrefixExpr` is defined at `source/slang/slang-parser.cpp:9441` and its `AdvanceIf(parser, "new")` branch is at line 9449, while line 9370 is `parseSPIRVAsmExpr`. The cited lines 9372/9364 are stale by ~80 lines, as the reviewer reported. | Updated the `new` row to cite the `AdvanceIf(parser, "new")` branch at line 9449 and the `parsePrefixExpr` definition at line 9441. |
