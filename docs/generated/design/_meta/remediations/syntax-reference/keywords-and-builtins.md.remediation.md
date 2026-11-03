---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:00:00Z
target_doc: syntax-reference/keywords-and-builtins.md
review_report: ../../reviews/syntax-reference/keywords-and-builtins.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/keywords-and-builtins.md

## Summary

Both findings were verified against the current source and tests and both were real, so both were fixed with minimal edits to the "Parser-registered syntax keywords" section. The document grew by 117 bytes (31,430 bytes, under the 32,768 cap).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `tests/diagnostics/redeclaration-user-alias-modifier.slang:6` declares `syntax my_alias : UnscopedEnumCaseAliasModifier;` in user source, so the exclusivity claim was false. | Replaced "no user-level example exists" with "used mainly in core-module source; user source can also declare them", citing that test. |
| F-002 | fixed | `source/slang/slang-parser.cpp:5522-5529` copies `parseCallback` and `parseUserData`, and takes the existing syntax class only under `if (!syntaxClass)`. | Alias sentence now says callback and user data are copied and the class is inherited only when none was given. |
