---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:33:09+00:00
target_doc: syntax-reference/grammar.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 962d2141d549a42c3603fbba721e8817eaf5daac65cba1a9fd6292074754985b
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 8
severity_breakdown:
  critical: 0
  major: 3
  minor: 5
  nit: 0
---

# Review report for syntax-reference/grammar.md

## Summary
The page has valid front matter, resolves its relative links, and broadly follows the requested reverse-engineered grammar shape, but several grammar productions do not match the parser. The most important issue is that the declaration and where-clause sections omit parser-supported forms, including registered declaration keywords and the type-coercion where constraint.

## Items checked
- Ran `regenerate.py show syntax-reference/grammar.md` and used only the reported prompt, watched paths, and dependency docs.
- Read the target document front matter/body, `_common.md`, `syntax-grammar.md`, `tokens.md`, and `keywords-and-builtins.md`.
- Verified HEAD is `c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8`, matching the requested review `source_commit` and the target doc's `source_commit`.
- Checked front matter for all required keys, the recorded target source commit, the warning string, and a 64-character hex watched-path digest.
- Resolved the target doc's source and generated-doc links sampled from the body, including links to `tokens.md`, `keywords-and-builtins.md`, parser/checker pipeline pages, `ast-reference/modifiers.md`, and the watched source files.
- Spot-checked source-backed claims against `source/slang/slang-parser.cpp`, `source/slang/slang-ast-expr.h`, and the dependency docs, including `parseFileReferenceDeclBase`, `parseModuleDeclarationDecl`, `parseNamespaceDecl`, `parseUsingDecl`, `g_parseSyntaxEntries[]`, `parseAssocType`, `parseOptionalGenericConstraints`, `maybeParseGenericConstraints`, `parseFuncDecl`, `parseTypeDef`, `parseAccessorDecl`, `ParseStatement`, `parseGenericApp`, `parseInfixExprWithPrecedence`, `parsePostfixExpr`, `parseAtomicExpr`, and the concrete `LiteralExpr` subclasses.
- Confirmed the page has no markdown source line-anchor citations requiring exhaustive line-number verification.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Declarations`, `CoreDecl`, lines 101-115 | The declaration alternatives are incomplete. `CoreDecl` omits parser-registered declaration keywords such as `__associatedfunc`, `type_param`, `__generic_value_param`, `__func_extension`, `semantic`, `__ignored_block`, and `__transparent_block`; one of those, `FuncExtensionDecl`, is later defined but is unreachable from `CoreDecl`. | `source/slang/slang-parser.cpp:10903` through `source/slang/slang-parser.cpp:10936` registers those forms in `g_parseSyntaxEntries[]`, including `_makeParseDecl("__associatedfunc", parseAssocFunc)`, `_makeParseDecl("__func_extension", parseFuncExtensionDecl)`, and `_makeParseDecl("semantic", parseSemanticDecl)`. | Add the missing declaration alternatives or explicitly group internal-only declarations, and make `FuncExtensionDecl` reachable from `CoreDecl`. |
| F-002 | minor | `## Top-level structure`, lines 72-86 | The module/import grammar is too narrow. It says module/implementing headers and import paths use only `IDENT`, but the parser accepts string literals for `module`, `import`, `__include`, and `implementing`; `module;` can also fall back to the current module name. | `source/slang/slang-parser.cpp:1320` through `source/slang/slang-parser.cpp:1349` accepts `StringLiteral` or dotted identifiers for file-reference declarations, and `source/slang/slang-parser.cpp:1379` through `source/slang/slang-parser.cpp:1402` accepts identifier, string literal, or no explicit module name. | Change `ModuleHeader` and `ImportPath` to include string-literal and optional-name cases, and distinguish `module` from file-reference declarations such as `implementing`. |
| F-003 | minor | `## Top-level structure`, lines 88-93 | The namespace and using productions do not match the parser. `NamespaceDecl` only shows one `IDENT`, but the parser accepts qualified namespace chains separated by `.` or `::`; `UsingDecl` is shown as a `QualifiedName`, but the parser accepts optional `namespace` and then an arbitrary expression. | `source/slang/slang-parser.cpp:4436` through `source/slang/slang-parser.cpp:4512` loops over `.` and `::` namespace separators, and `source/slang/slang-parser.cpp:4551` through `source/slang/slang-parser.cpp:4563` consumes optional `namespace` then calls `ParseExpression()`. | Expand `NamespaceDecl` to model qualified namespace chains, and change `UsingDecl` to `using namespace? Expr ';'` with a note that valid checked code is narrower than the parse grammar. |
| F-004 | major | `## Generics and where-clauses`, `WhereTerm`, lines 501-506 | The where-clause grammar omits a parser-supported type-coercion constraint form. After parsing a leading type, `maybeParseGenericConstraints` accepts `(` Type `)` and optional `implicit`, creating a `TypeCoercionConstraintDecl`, but the document lists only conformance, equality, nonempty, countof, and `__hasDiffTypeInfo`. | `source/slang/slang-parser.cpp:2036` through `source/slang/slang-parser.cpp:2048` creates `TypeCoercionConstraintDecl`, sets `toType` and `fromType`, and accepts the optional `implicit` keyword. | Add a `WhereTerm` alternative for `Type '(' Type ')' 'implicit'?` and describe it as the type-coercion constraint parsed by `maybeParseGenericConstraints`. |
| F-005 | minor | `## Statements`, `ContinueStmt`, line 310 | `ContinueStmt` incorrectly allows an optional label. The source parser reads only `continue` followed immediately by `;`, unlike `BreakStmt`, which does accept an optional identifier target. | `source/slang/slang-parser.cpp:7539` through `source/slang/slang-parser.cpp:7545` reads `continue` and then `Semicolon`; `source/slang/slang-parser.cpp:7526` through `source/slang/slang-parser.cpp:7535` shows the optional identifier only on `break`. | Change `ContinueStmt` to `continue ';'` and keep the optional label only on `BreakStmt`. |
| F-006 | minor | `## Statements`, `ThrowStmt`, line 314 | `ThrowStmt` includes a required trailing semicolon, but `ParseThrowStatement` does not consume one. This means the grammar asserts a token consumption behavior that is not present in the parser function it cites. | `source/slang/slang-parser.cpp:7568` through `source/slang/slang-parser.cpp:7574` reads `throw` and `ParseExpression()` and then returns without reading `TokenType::Semicolon`. | Either remove the semicolon from the grammar or add a note that a following semicolon is parsed separately as an empty statement if that is the intended surface behavior. |
| F-007 | major | `## Expressions`, precedence table and productions, lines 329-364 | The expression grammar omits the parser's `is` and `as` type-test/type-cast infix operators. They are special-cased inside the same precedence parser used for the listed binary operators, so leaving them out makes the expression grammar incomplete for parser-supported syntax. | `source/slang/slang-parser.cpp:7858` through `source/slang/slang-parser.cpp:7882` checks identifier operators `is` and `as`, parses a following type, and creates `IsTypeExpr` or `AsTypeExpr`. | Add `is` and `as` to the precedence ladder and production set, with a note that their right operand is parsed as a type rather than a general expression. |
| F-008 | minor | `## Declarations`, `AccessorName`, line 237 | `AccessorName` lists `modify`, but the parser only recognizes `get`, `set`, and `ref`; it diagnoses any other accessor name. | `source/slang/slang-parser.cpp:4692` through `source/slang/slang-parser.cpp:4708` branches on `get`, `set`, and `ref`, then calls `Unexpected(parser)` for other tokens. | Delete `modify` from `AccessorName`; if needed, add a separate note for any future accessor syntax only when the parser supports it. |

## No-issues notes
- The target document's required front matter is present and uses the requested target `source_commit` and watched-path digest values.
- The dependency links to `tokens.md` and `keywords-and-builtins.md` resolve and are consistent with the manifest `depends_on` list.
- The literal section correctly lists the concrete literal-expression subclasses present in `source/slang/slang-ast-expr.h`, including `BoolLiteralExpr`, `NullPtrLiteralExpr`, and `NoneLiteralExpr`.
- The generic-application `<` disambiguation note matches the parser's speculative parse plus follower-token strategy in `tryParseGenericApp`.
