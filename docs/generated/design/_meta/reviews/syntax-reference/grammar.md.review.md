---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:44+00:00
target_doc: syntax-reference/grammar.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: fa970dcce70dd7f3374f9d33120c59e4a2f243c4f0bee9756c52610ae322f450
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 7
severity_breakdown:
  critical: 0
  major: 5
  minor: 2
  nit: 0
---

# Review report for syntax-reference/grammar.md

## Summary
The page is useful as an informal grammar, but several productions contradict the current parser. The most important issue is the `## Types` section: it documents `func` function types, tuple types, optional suffixes, and reference suffixes that the watched parser path does not accept.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show syntax-reference/grammar.md`; reviewed the listed watched files and dependency docs `syntax-reference/tokens.md` and `syntax-reference/keywords-and-builtins.md`.
- Verified the target front matter fields, source commit, digest shape, title, required sections, and relative links.
- Checked all explicit line-number citations in the target body, including statement dispatch and `ParseDoCatchStatement`.
- Spot-checked more than 20 grammar claims against `slang-parser.cpp`, `slang-parser.h`, `slang-lexer.cpp`, and `slang-ast-expr.h`, including module declarations, statement dispatch, parameter parsing, generic disambiguation, syntax-decl expression parsing, literal AST classes, modifier registration, type suffixes, and function type parsing.
- Compared the document against `docs/generated/design/_meta/prompts/syntax-grammar.md` and the shared `_common.md` contract.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Statements`, statement production comments | Several statement comments cite stale source ranges. For example, `IfStmt` says `around line 6457`, but line 6457 is `peekTypeName`; `DoCatchStmt` cites `slang-parser.cpp:7018-7050`, while the catch parser is now later in the file. | `source/slang/slang-parser.cpp:6534-6596` contains the current `ParseStatement` dispatch, `source/slang/slang-parser.cpp:6986-6999` contains `parseIfStatement`, and `source/slang/slang-parser.cpp:7095-7127` contains `ParseDoCatchStatement`. | Refresh the statement line citations to the current ranges, or remove approximate line numbers and cite the parser function names only. |
| F-002 | minor | `## Top-level structure`, `ModuleHeader` production | The production makes the semicolon optional with `';'?`, but both module-header parse paths require a semicolon. | `source/slang/slang-parser.cpp:1363-1371` routes `implementing` through `parseModuleDeclarationDecl`, and `source/slang/slang-parser.cpp:1397` calls `parser->ReadToken(TokenType::Semicolon)`. | Change `ModuleHeader` to require `';'`, or add a source-backed note if recovery can tolerate a missing semicolon diagnostically. |
| F-003 | major | `## Declarations`, `Param` production | The parameter production only documents type-first parameters, `ModifierList? Type IDENT`, but `parseModernParamDecl` accepts modern parameters of the form `IDENT ':' Type` and even bare identifier parameters in contexts where `_peekModernStyleVarDecl` succeeds. | `source/slang/slang-parser.cpp:4551-4571` detects modern-style declarations by an identifier followed by `:`, `,`, `)`, `}`, `]`, or `{`; `source/slang/slang-parser.cpp:4779-4797` chooses between modern and traditional parameter parsing; `source/slang/slang-parser.cpp:4713-4729` parses the modern `name ':' Type '=' Expr` shape. | Expand `Param` to cover both modern and traditional forms, such as an `IDENT` form with an optional colon type or a `Type IDENT` form, with notes for the parser's lookahead heuristic. |
| F-004 | major | `## Types`, `CoreType` production | The grammar says function types are written with `'func' '(' Type ... ')' '->' Type`, but the type parser accepts the keyword `functype`, not `func`, for function type expressions. | `source/slang/slang-parser.cpp:3190-3193` enters `parseFuncTypeExpr` only after `AdvanceIf(parser, "functype")`; `source/slang/slang-parser.cpp:2975-2988` parses the parenthesized parameter types and `->` result once that keyword has already been consumed. | Change the function-type production to use `'functype' '(' Type (',' Type)* ')' '->' Type`, or remove it if the page intentionally excludes this internal syntax. |
| F-005 | major | `## Types`, tuple type alternative | The grammar lists `(' Type (',' Type)+ ')'` as a tuple type, but the parser's tuple-type parser is compiled out and the branch in `_parseSimpleTypeSpec` is commented. | `source/slang/slang-parser.cpp:2958-2968` wraps `parseTupleTypeExpr` in `#if 0`, and `source/slang/slang-parser.cpp:3184-3188` shows the tuple-type branch commented out. | Remove the tuple-type alternative, or replace it with a note that tuple type parsing is currently disabled in the watched source. |
| F-006 | major | `## Types`, optional and reference suffixes | The grammar lists `Type '?'` and `Type '&'`, but the watched type parser's postfix suffix function only handles bracket array suffixes and `*` pointer suffixes. | `source/slang/slang-parser.cpp:2827-2857` implements `parsePostfixTypeSuffix` with only `[` `]` and `TokenType::OpMul`; no `TokenType::QuestionMark` or `TokenType::OpBitAnd` branch appears in that type-suffix path. | Remove the `?` and `&` suffix alternatives unless another watched parser path supports them, and point readers at `Optional<T>` or modifier-based reference syntax where appropriate. |
| F-007 | major | Productions throughout the page | The per-doc prompt requires every production to cite the parsing function that implements it, but many productions still have no function citation. Examples include `SourceFile`, `TopDecl`, `QualifiedName`, `Param`, `CoreType`, and `GenericArg`. | `docs/generated/design/_meta/prompts/syntax-grammar.md:75-78` requires each stated production to cite its implementing parser function. The target includes uncited productions in multiple code blocks. | Add parser-function citations to each production or split large production groups so every non-terminal names the implementation path that supports it. |

## No-issues notes
- The literal inventory matches the concrete `LiteralExpr` subclasses in `source/slang/slang-ast-expr.h`, including the fact that `CharLiteral` maps to `IntegerLiteralExpr`.
- The `<` disambiguation summary matches the `tryParseGenericApp` fallback and follow-token logic in `source/slang/slang-parser.cpp`.
- The associated-type constraint discussion matches the current `parseAssocType` and `parseOptionalGenericConstraints` comments and behavior.
