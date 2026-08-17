---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:28+00:00
target_doc: syntax-reference/grammar.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: da0d1f10369218c4e091cd72414d8d541941c541ee85497b27e089007588104f
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 12
severity_breakdown:
  critical: 0
  major: 5
  minor: 7
  nit: 0
---

# Review report for syntax-reference/grammar.md

## Summary
The page has valid front matter, all links resolve, and many detailed claims match the recorded source, including literals, precedence, generic-application disambiguation, attributes, and associated-type constraints. It is nevertheless incomplete as a parser grammar: several declaration variants, postfix forms, and type forms disagree with `slang-parser.cpp`, and numerous productions lack the parser-function citation required by the generation contract. The most consequential factual gap is the postfix grammar, which omits `::` and `->` and misstates subscript argument cardinality.

## Items checked
- Verified all four resolved watched files exactly match target commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, which also equals review-time HEAD.
- Read `_common.md`, `syntax-grammar.md`, both dependency pages, and the full target page.
- Verified all 20 relative-link occurrences (8 unique targets) exist at the target commit and that generated peers are present in the manifest.
- Confirmed there are no numeric line citations in the target body, so there were no line-number citations to re-derive.
- Recomputed the watched-path digest as `da0d1f10369218c4e091cd72414d8d541941c541ee85497b27e089007588104f` and checked all mandatory front-matter fields.
- Spot-checked more than 10 factual claims, including module/import names, namespace and using declarations, syntax dispatch, struct/class/enum parsing, associated-type constraints, generic parameters and where terms, declarators and properties, accessors, statement dispatch, precedence and associativity, `is`/`as`, postfix expressions, all concrete `LiteralExpr` subclasses, adjacent strings and character literals, `<` disambiguation, attributes, type parsing, and deferred bodies.
- Swept whole source filenames in the page; all three named source files exist.
- Checked the page size (48,566 UTF-8 bytes) against the 49,152-byte manifest cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Productions throughout, especially lines 72-107, 128-215, 328-389, and 539-544 | The page does not meet the prompt's requirement that every non-terminal cite an implementing parser function. Many productions have no citation, and `Operator`, `RegToken`, `KeywordExprHead`, and `GenericSpecialization` are used as non-terminals without definitions; `InterfaceConstraintDecl` is also named even though no such source symbol exists. | `docs/generated/design/_meta/prompts/syntax-grammar.md:75-85` requires a parser-function citation for every production. The watched source instead defines `parseInterfaceConstraintDecl` producing `GenericTypeConstraintDecl` at `source/slang/slang-parser.cpp:4335-4359`. | Define or inline every referenced helper non-terminal, replace `InterfaceConstraintDecl` with the actual parser function/node terminology, and add an implementing parser-function citation to each production. |
| F-002 | minor | `## Top-level structure`, lines 72-78 | The `ModuleName` production incorrectly permits dotted identifier names as well as string literals after `module`, and `SourceFile ::= ModuleHeader? TopDecl*` implies a special one-time leading header position that the parser does not enforce. | `parseModuleDeclarationDecl` consumes only one identifier, one string literal, or no name at `source/slang/slang-parser.cpp:1376-1402`; `parseSourceFile` simply calls `parseDecls` at line 6355, and `module` is registered as an ordinary declaration at line 10724. | Give `module` its own single-name production and model its parser dispatch as a top-level declaration, with any intended placement restriction described separately as a nesting/semantic rule. |
| F-003 | major | `## Declarations` and `## Generics and where-clauses`, lines 173-201 and 709-729 | Generic-capable declaration shapes are inconsistent with the parser. `ExtensionDecl` omits inline generic parameters and a where clause; generic `InterfaceDecl` and `TypeAliasDecl` omit their where clauses; meanwhile the prose claims `ClassDecl` accepts inline generics even though `ParseClass` does not. | `parseExtensionDecl` uses `parseOptGenericDecl` and then `maybeParseGenericConstraints` at `source/slang/slang-parser.cpp:4220-4231`; interface does the same at lines 4409-4425 and type aliases at lines 5162-5183. `Parser::ParseClass` has no generic path at lines 6433-6446. | Add the actual generic/where slots to extension, interface, and type-alias productions, and remove `ClassDecl` from the list of inline-generic forms. |
| F-004 | major | `## Declarations`, lines 184-200 and 272-347 | The member and declarator grammar is substantially narrower than the parser. `PropertyDecl` shows only `property name : Type`, but the parser also accepts a traditional `property Type Declarator`; C-style `VarDecl` uses the full recursive declarator machinery, not the page's `IDENT ArraySuffix?` form. | `parsePropertyDecl` selects modern or traditional syntax at `source/slang/slang-parser.cpp:4847-4901`. C-style declarations call `parseInitDeclarator` and `UnwrapDeclarator` at lines 3669-3702 and repeat that path for comma-separated declarations at lines 3729-3772. | Add the traditional property alternative and reuse the defined `Declarator` non-terminal in C-style variable productions instead of the narrower `VarDeclarator`. |
| F-005 | minor | `## Declarations`, lines 289-331 | Initializer productions do not match token consumption. Modern `var` and `let` use `ParseInitExpr`, so a top-level comma is not part of the initializer; `let`'s initializer is parser-optional; and C-style declarations do not accept a bare `{...}` initializer without a preceding `=`. | `parseModernVarDeclBaseCommon` makes `=` optional and calls `ParseInitExpr` at `source/slang/slang-parser.cpp:4986-5003`; both `parseLetDecl` and `parseVarDecl` use it at lines 5012-5023. `parseInitDeclarator` has only the `=` branch at lines 2726-2733. | Use `ArgExpr`/`ParseInitExpr` for parameter and modern-binding defaults, make the parser-level `let` initializer optional, and express braced initialization as the expression after `=` rather than a bare initializer alternative. |
| F-006 | minor | `EnumDecl`, lines 189-192 | `EnumDecl` requires at least one `EnumCase`, but the parser accepts an empty body because it tests for the closing brace before parsing a case. | The loop at `source/slang/slang-parser.cpp:6553-6565` exits immediately when `}` is next. | Change the body to an optional case list, for example `(EnumCase (',' EnumCase)* ','?)?`. |
| F-007 | minor | `DoCatchStmt`, line 429 | The production permits exactly one `catch`, but `ParseDoCatchStatement` accepts a chain of catch clauses. | `source/slang/slang-parser.cpp:7482-7513` loops until the next token is not `catch`, nesting each additional `CatchStmt`. | Change the suffix to one-or-more catch clauses and note that the AST nests chained catches. |
| F-008 | major | `PostfixSuffix`, lines 539-544 | The postfix grammar omits static-member access `::` and pointer-member access `->`. It also requires exactly one expression in `[...]`, while the parser accepts zero or multiple comma-separated `ArgExpr` operands. | `parsePostfixExpr` handles zero-or-more index arguments at `source/slang/slang-parser.cpp:9118-9141`, `::` at lines 9172-9190, and both `.` and `->` at lines 9192-9213. | Add `::` and `->` member suffixes and change subscript syntax to `'[' ArgList? ']'`. |
| F-009 | minor | `KeywordExpr`, lines 558-560 | The productions `'try' Expr` and `'no_diff' Expr` give these keywords an entire expression operand, but both parsers consume only a leaf/prefix expression. This changes grouping around following infix operators. | `parseTryExpr` calls `ParseLeafExpression` at `source/slang/slang-parser.cpp:8177-8183`; `parseTreatAsDifferentiableExpr` does the same at lines 8186-8192; `ParseLeafExpression` delegates to `parsePrefixExpr` at lines 9888-9891. | Change both operands to `UnaryExpr` (or a named leaf-expression non-terminal) so their binding matches the parser. |
| F-010 | minor | `KeywordExpr`, line 570 | `__dispatch_kernel` is described as accepting a generic nonempty `ArgList`, but its parser requires exactly three comma-separated arguments. | `parseDispatchKernel` reads `baseFunction`, `dispatchSize`, and `threadGroupSize` with two mandatory commas at `source/slang/slang-parser.cpp:3209-3222`. | Replace `ArgList` with exactly three `ArgExpr` operands and name their roles in the adjacent comment. |
| F-011 | major | `## Types`, lines 765-783 | The type grammar omits parser-supported interface conjunctions (`T & U`) and dotted member-type suffixes. It also redundantly puts generic arguments after `QualifiedName` even though the implementation interleaves generic, `::`, and `.` suffixes. | `_parseInfixTypeExprSuffix` builds `AndTypeExpr` for `&` at `source/slang/slang-parser.cpp:7684-7721`; `_parseSimpleTypeSpec` loops over generic application, `::`, and `.` at lines 3493-3515. | Introduce atomic/postfix/infix type levels that model interleaved generic/member suffixes and the `&` conjunction operator. |
| F-012 | minor | `SyntaxDecl`, lines 370-375 | The note says at least one of the `:` or `=` clauses is required, but the parser currently accepts neither: the supposed requirement is only a TODO and no diagnostic is emitted. | `parseSyntaxDecl` makes both clauses optional and leaves the missing-clause diagnostic as a TODO at `source/slang/slang-parser.cpp:5197-5265`. | Remove the parser-acceptance claim, or explicitly label it as an intended validity rule that the current parser does not enforce. |

## No-issues notes
- The precedence table, including `is`/`as` and assignment associativity, agrees with `GetOpLevel` and `parseInfixExprWithPrecedence`.
- The literal inventory covers every concrete `LiteralExpr` subclass in `source/slang/slang-ast-expr.h`, and the character/adjacent-string notes match `parseAtomicExpr`.
- The generic-application follower set and body-stage semantic lookup description match `tryParseGenericApp`.
- Attribute-name flattening and optional commas match `parseAttributeName` and `ParseSquareBracketAttributes`.
