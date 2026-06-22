---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:16:32Z
target_doc: syntax-reference/grammar.md
review_report: ../../reviews/syntax-reference/grammar.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/grammar.md

## Summary

All seven review findings were verified against `source/slang/slang-parser.cpp` at the doc's source commit and all seven were fixed. Two were minor (stale statement line citations; the optional module-header semicolon) and five were major (the `Param`, function-type, tuple-type, and optional/reference-suffix productions contradicting the parser, plus missing parser-function citations on several productions). Every edit was confined to the target doc body.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed line 6457 is `peekTypeName` (slang-parser.cpp:6458), `parseIfStatement` is at 6986, and `ParseDoCatchStatement` is at 7095 (not 7018-7050); the stale ranges are wrong. | Replaced the `around line NNNN` and `:7018-7050` citations in the Statements block with the actual parser-function names (`parseIfStatement`, `ParseForStatement`, `ParseDoStatement`, etc.). |
| F-002 | fixed | Both header paths require the semicolon: `parseModuleDeclarationDecl` calls `ReadToken(TokenType::Semicolon)` (slang-parser.cpp:1397) and `parseImplementingDecl` -> `parseFileReferenceDeclBase` does the same (slang-parser.cpp:1343). | Changed `ModuleHeader` from `';'?` to `';'` and noted both paths read the trailing `;`. |
| F-003 | fixed | `parseModernParamDecl` (slang-parser.cpp:4769) accepts modern `IDENT (':' Type)? ...` params via `_peekModernStyleVarDecl` (4551) / `parseModernVarDeclBaseCommon` (4713, colon-type optional); `parseModernParamList` is used by `func`-style decls. | Added a modern name-first `Param` alternative with parser citations, and labeled the existing type-first form as traditional. |
| F-004 | fixed | The type parser enters `parseFuncTypeExpr` only after `AdvanceIf(parser, "functype")` (slang-parser.cpp:3190); the keyword is `functype`, not `func`. | Changed the function-type production from `'func'` to `'functype'` and cited `parseFuncTypeExpr`. |
| F-005 | fixed | `parseTupleTypeExpr` is wrapped in `#if 0` (slang-parser.cpp:2958) and its `_parseSimpleTypeSpec` branch is commented out (3184-3188); the tuple type is not parsed. | Removed the tuple-type alternative from `CoreType`. |
| F-006 | fixed | `parsePostfixTypeSuffix` (slang-parser.cpp:2828) handles only `[` `]` arrays and `OpMul` pointers; the `QuestionMark` at line 2764 is in the generic-app FOLLOW set, not a type suffix, and no `OpBitAnd` suffix exists in the type path. | Removed the `Type '?'` and `Type '&'` suffix alternatives from `CoreType`. |
| F-007 | fixed | The per-doc prompt (syntax-grammar.md:75-78, 84-85) requires every production to cite its parser function; the named examples (`SourceFile`, `TopDecl`, `QualifiedName`, `Param`, `CoreType`, `GenericArg`) had none. | Added parser-function citations to each named production (`parseSourceFile`/`parseDecls`, `ParseDecl`, `ParseExpression`->`parseStaticMemberType`, `ParseType`/`_parseSimpleTypeSpec`, `parseGenericApp`/`_parseGenericArg`). |
