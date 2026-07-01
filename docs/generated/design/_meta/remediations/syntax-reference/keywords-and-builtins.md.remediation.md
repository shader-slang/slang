---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:05:59Z
target_doc: syntax-reference/keywords-and-builtins.md
review_report: ../../reviews/syntax-reference/keywords-and-builtins.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/keywords-and-builtins.md

## Summary
One finding (F-001, major) was reviewed and rejected as bogus. The reviewer asserted the keyword inventory omits the hardcoded parser keywords `__target_switch`, `__stage_switch`, `__intrinsic_asm`, `__GPU_FOREACH`, `expand`, and `each`, but all six are already present in the target document with correct source-line citations that match the watched source at HEAD. No edit was made; the target doc source_commit is unchanged and mark-fresh should be skipped.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | All six keywords claimed missing are already documented. The statement table lists `__target_switch` (target doc line 81), `__stage_switch` (82), `__intrinsic_asm` (83), and `__GPU_FOREACH` (86), each marked compiler-internal, matching `source/slang/slang-parser.cpp:6941`, `6943`, `6945`, `6951`. `expand` and `each` are covered at target doc lines 156-163, citing `source/slang/slang-parser.cpp:3435`-`3441` and noting they are not in `g_parseSyntaxEntries[]`, matching the verified source. The reviewer appears to have read an earlier revision. | — |
