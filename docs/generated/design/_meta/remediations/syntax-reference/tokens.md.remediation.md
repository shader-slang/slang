---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T20:00:00+00:00
target_doc: syntax-reference/tokens.md
review_report: ../../reviews/syntax-reference/tokens.md.review.md
target_doc_source_commit_before: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/tokens.md

## Summary

Restructured the taxonomy section to match the prompt's required
heading and column shape, and added a raw-string bullet to the
special-case lexing rules.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The prompt mandates `## Token-kind taxonomy` with columns `TokenKind`, `Lexer source range`, `Notes`; the doc used `## Token taxonomy` with `TokenType` / `Diagnostic name` (or `Spelling`) columns. | Renamed the section to `## Token-kind taxonomy`, replaced every sub-table's header with the three required columns (TokenKind / Lexer source range / Notes), and pointed the "Lexer source range" cell at the relevant lexer rule (e.g. "identifier rule in `slang-lexer.cpp`", "punctuation dispatch", "default-constructed `Token`"). Added a new "Preprocessor markers" sub-table to split out `Pound`, `PoundPound`, and `CompletionRequest`. Adjusted the introductory paragraph to describe the new five-group structure and explain that `TokenKind` rows are `TokenType::*` enumerators. |
| F-002 | fixed | Raw-string literals are handled by `slang-lexer.cpp` (around lines 1025 and 1427); the doc omitted them from `## Special-case lexing rules`. | Added a "Raw string literals" bullet describing the `R"delimiter(...)delimiter"` form and citing both lexer ranges. Also noted that nested block comments are not supported in the `BlockComment` row of the trivia table. |
