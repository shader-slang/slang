---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:06:11Z
target_doc: syntax-reference/tokens.md
review_report: ../../reviews/syntax-reference/tokens.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/tokens.md

## Summary
One finding was reviewed; no edits were applied this cycle. F-001 claims the `CompletionRequest` row states the kind is "synthesized by the language-server pipeline" and cites a non-existent `slang-completion-token.cpp`. That text is not present in the target document at the reviewed commit — the row already cites the `#?` arm of the `#` branch in `_lexTokenImpl` (around line 2137) returning `TokenType::CompletionRequest`, exactly what the recommendation requests. The finding describes a prior revision; it is rejected as bogus. The target document is unchanged.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Target doc `docs/generated/design/syntax-reference/tokens.md:82` reads: `CompletionRequest` from the "`#?` arm of the `#` branch in `_lexTokenImpl` (`slang-lexer.cpp` around line 2137)" with no "language-server pipeline" claim and no `slang-completion-token.cpp` citation. Source confirms `source/compiler-core/slang-lexer.cpp:2137`-`2139` handles `#` then `?` and returns `TokenType::CompletionRequest`. The recommended state already holds. | — |
