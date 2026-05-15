---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T20:00:00+00:00
target_doc: syntax-reference/grammar.md
review_report: ../../reviews/syntax-reference/grammar.md.review.md
target_doc_source_commit_before: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/grammar.md

## Summary

All three findings addressed: missing top-level sections added,
`TryStmt` replaced with the real `do ... catch` statement, and the
`IDENT` / `INT_LIT` / ... terminal names cross-referenced to the
`TokenType` enumerators.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The prompt requires `## Attributes and decorations` and `## Generics and where-clauses` as top-level sections; the doc folded both into other headings. | Split `## Modifiers and attributes` into `## Modifiers` and `## Attributes and decorations`. Moved the `### Generic declarations` block out of `## Declarations` and promoted it to `## Generics and where-clauses`, with the `WhereClause` / `WhereTerm` productions hoisted up from `## Declarations` and a paragraph explaining the body-deferred interaction. |
| F-002 | fixed | The parser does not accept `try Block CatchClause+`; `try` is an expression, and the statement-level handler is `do ... catch`. | Removed the bogus `TryStmt ::= 'try' Block CatchClause+` and `CatchClause` productions. Added `DoCatchStmt ::= 'do' Stmt 'catch' '(' Param ')' Block` (citing `slang-parser.cpp` lines 6919-6967). Added a paragraph at the top of `## Statements` explaining that `try` is an expression keyword and `do ... catch` is the statement form. |
| F-003 | fixed | The `IDENT` / `INT_LIT` / `FLOAT_LIT` / `STRING_LIT` / `CHAR_LIT` names are not the source-level token-kind names; the reviewer wants explicit aliases. | Rewrote the second paragraph of `## Notation` to list the alias-to-`TokenType` mapping in a small table and explain that the aliases are used purely for table readability. |
