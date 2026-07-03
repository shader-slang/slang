---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:04:48Z
target_doc: pipeline/02-parse-ast.md
review_report: ../../reviews/pipeline/02-parse-ast.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/02-parse-ast.md

## Summary

One minor finding was queued and no document edits were made this cycle. F-001 is rejected as bogus: the current target doc already attributes the base `Val` class to `slang-ast-base.h` and cites `slang-ast-val.h` only for concrete value subclasses, which is exactly the finding's own recommendation. The claim was verified against source. Front-matter is unchanged, so `target_doc_source_commit_after` equals `_before`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The current target doc at docs/generated/design/pipeline/02-parse-ast.md lines 176-181 already reads that the base `Val` class "is declared in slang-ast-base.h" "with concrete value subclasses in slang-ast-val.h". It never says `Val` is rooted in slang-ast-val.h as the finding quotes. Source confirms the existing attribution: class Val public NodeBase at source/slang/slang-ast-base.h line 380 (comment "Base class for compile-time values" at line 374), while source/slang/slang-ast-val.h holds only subclasses such as IntVal at line 144. The doc already matches the recommendation, so no edit applies. | — |
