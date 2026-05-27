---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: syntax-reference/tokens.md
target_doc_source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
target_doc_watched_paths_digest: e2c6f1441dbe013ee44a514220f358519fb7666c14bf549fa51c11558ff1dd3e
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for syntax-reference/tokens.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked front matter, token enumerators, token flags, line continuation, comments, include-string, numeric suffix, source-location links, and raw string handling.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Token taxonomy` | The required heading/table contract is not met: the prompt requires `## Token-kind taxonomy` with columns `TokenKind`, `Lexer source range`, and `Notes`; the page uses different heading and columns. | `docs/generated/design/_meta/prompts/syntax-tokens.md` requires the exact taxonomy section shape. | Rename and reshape this section, or update the prompt if `TokenType` is the intended source name. |
| F-002 | major | `## Special-case lexing rules` | Raw strings are present in the watched lexer source but omitted from special-case lexing rules. | `source/compiler-core/slang-lexer.cpp:1025` and `source/compiler-core/slang-lexer.cpp:1427` contain raw string literal handling. | Add a raw-string bullet describing `R"delimiter(...)delimiter"` handling. |
