---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: syntax-reference/grammar.md
review_report: ../../reviews/syntax-reference/grammar.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/grammar.md

## Summary

All four major findings were fixed. The `FuncDecl` production now
matches `parseFuncDecl`/`parseTraditionalFuncDecl` (`throws` then
`->`, no colon clause); the required sections were reordered so
Statements and Expressions precede Modifiers/Attributes/Generics;
expression productions now cite their parser functions; and
`DoCatchStmt` now reflects the optional parameter and statement
handler. No front-matter was touched.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-parser.cpp:4758-4765` reads optional `throws` then `->`; `parseTraditionalFuncDecl` (2163-2172) uses a leading return type then optional `throws`; neither uses a colon clause. | Changed both `FuncDecl` alternatives to `('throws' Type)? ('->' Type)?` (C-style: leading-Type return + `('throws' Type)?`), cited `parseTraditionalFuncDecl`, and removed the now-orphaned `ResultClause` definition. |
| F-002 | fixed | `docs/generated/design/_meta/prompts/syntax-grammar.md:41-73` mandates section order Statements, Expressions, Modifiers, Attributes, Generics. | Moved the Modifiers / Attributes / Generics block to after `## Expressions` (before the extra `## Types` section). |
| F-003 | fixed | Prompt checklist (`syntax-grammar.md:84-85`) requires each non-terminal to cite a parser function; `ParseExpression`/`parseInfixExprWithPrecedence`/`parsePrefixExpr`/`parsePostfixExpr`/`parseAtomicExpr` verified in `slang-parser.cpp`. | Added `--` parser-function citations to the `Expr`, precedence-ladder, `UnaryExpr`, `PostfixExpr`, and `AtomExpr` productions. |
| F-004 | fixed | `source/slang/slang-parser.cpp:7031` makes the catch parameter optional; line 7040 parses the handler via `ParseStatement()`. | Changed `DoCatchStmt` to `'do' Stmt 'catch' ('(' Param ')')? Stmt` and cited `ParseDoCatchStatement` (7018-7050). |
