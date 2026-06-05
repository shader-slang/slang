---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:58:02+00:00
target_doc: syntax-reference/grammar.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 1084d6ac21281bc1db256e51bd36ad00a4bce0602ec9747f43e80e7a14436e98
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 4
  minor: 0
  nit: 0
---

# Review report for syntax-reference/grammar.md

## Summary
The grammar page has useful broad coverage, but it misses several mandatory prompt-contract details and has two source mismatches. The most important factual issues are the function declaration production, which documents a colon return clause, and the `do ... catch` production, which makes the catch parameter and handler block more restrictive than the parser.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show syntax-reference/grammar.md` and used the listed watched files at `52339028a2aa703271533454c6b9528a534bac31`.
- Read the per-document prompt, `_common.md`, and dependency docs `syntax-reference/tokens.md` and `syntax-reference/keywords-and-builtins.md`.
- Resolved all relative markdown links and anchors in the target document.
- Verified required front matter keys and checked that the target digest is a 64-character hex value.
- Spot-checked at least 15 source-alignment claims against parser and AST sources, including module headers, import declarations, struct and enum parsing, `parseFuncDecl`, `parseFuncExtensionDecl`, `parseAttributeSyntaxDecl`, statement dispatch, `do ... catch`, expression keyword registration, `new`, literal subclasses, `<` disambiguation, token aliases, and type-name disambiguation.
- Compared the document heading order and non-terminal citation style against `docs/generated/design/_meta/prompts/syntax-grammar.md`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Declarations`, `FuncDecl` production | The production says `func` declarations use `(':' ResultClause)?`, but `parseFuncDecl` accepts optional `throws` followed by optional `->` return type. | `source/slang/slang-parser.cpp:4758-4764` reads `throws` into `errorType` and then reads `TokenType::RightArrow` before `returnType`; there is no colon return branch in that parser path. | Change `FuncDecl` to use `('throws' Type)? ('->' Type)?` after the parameter list, and update `ResultClause` so it does not imply colon syntax for `func`. |
| F-002 | major | Section order around `## Modifiers`, `## Statements`, and `## Expressions` | The grammar prompt requires `## Statements` before `## Expressions`, followed by `## Modifiers`, `## Attributes and decorations`, and `## Generics and where-clauses`; the document places modifiers, attributes, and generics before statements and expressions. | `docs/generated/design/_meta/prompts/syntax-grammar.md:41-73` specifies the required section order, while the target headings put `## Modifiers` before `## Statements`. | Reorder the required sections to match the prompt contract, keeping any extra `## Types` or checker-notes material after the required sections if it remains useful and source-backed. |
| F-003 | major | Productions throughout `## Expressions` and related grammar blocks | The prompt requires every production to cite the parsing function that implements it, but many non-terminals list no implementing function or only broad prose. | `docs/generated/design/_meta/prompts/syntax-grammar.md:75-78` requires parser-function citations; for example the target `Expr`, `AssignExpr`, `TernaryExpr`, and `PostfixExpr` productions do not name their parser functions. | Add parenthetical parser-function citations to each production, such as `ParseExpression`, `parsePrefixExpr`, `parsePostfixExpr`, and the relevant statement or declaration parser functions. |
| F-004 | major | `## Statements`, `DoCatchStmt` production | The production requires `catch` to have a parenthesized parameter and a block handler, but the parser makes the error parameter optional and parses the handler as any statement. | `source/slang/slang-parser.cpp:7031-7036` wraps the parameter parse in `AdvanceIf(TokenType::LParent)`, and `source/slang/slang-parser.cpp:7039-7040` assigns `handleBody = ParseStatement()`. | Change the production to allow an optional parenthesized parameter and use `Stmt` for the handler body, for example `DoCatchStmt ::= 'do' Stmt 'catch' ('(' Param ')')? Stmt`. |
