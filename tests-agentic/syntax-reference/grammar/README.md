---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:03:32+00:00
source_commit: ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
watched_paths_digest: 40df0e0f2fba874f2bb1d1a886aacac0f20100fdb9c27817c150f2b7ecea2322
source_doc: docs/llm-generated/syntax-reference/grammar.md
source_doc_digest: a7c2ca43400a92a62cec4ff463f7f9f3dfc1abe5a2093d10804cfd3e4b6d93ff
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for syntax-reference/grammar

## Intent

Tests verify the surface-grammar productions described in
[`docs/llm-generated/syntax-reference/grammar.md`](../../../docs/llm-generated/syntax-reference/grammar.md):
declaration / expression / statement / type / generic / modifier
productions, plus the precedence ladder and the `<` disambiguation
fork. Most tests are `pipeline_stage=parse` and use
`//TEST:INTERPRET` so the production shape is observed end-to-end on
the interpreter without a GPU. Two negative tests use
`//DIAGNOSTIC_TEST:SIMPLE` to assert that ill-formed inputs are
rejected at parse stage.

This bundle is intentionally disjoint from
[`syntax-reference/tokens`](../tokens/) (which is lexer-level) and
from `pipeline/02-parse-ast` (which is parser-stage internal
behavior). The angle here is **production-shape**: which strings the
grammar names, and in what shape they parse.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| `Attribute` parses as `'[' AttributeName ('(' ArgList? ')')? ']'` and attaches to the following decl. | functional | [#attributes-and-decorations](../../../docs/llm-generated/syntax-reference/grammar.md#attributes-and-decorations) | [`attribute-bracket-form.slang`](attribute-bracket-form.slang) |
| A token that cannot begin a `Decl` triggers a parse-stage diagnostic. | negative | [#declarations](../../../docs/llm-generated/syntax-reference/grammar.md#declarations) | [`decl-unexpected-token-rejected.slang`](decl-unexpected-token-rejected.slang) |
| `<` in a body context where the preceding expression does not resolve to a generic parses as the comparison operator. | functional | [#disambiguation](../../../docs/llm-generated/syntax-reference/grammar.md#disambiguation) | [`less-than-in-body-parses-as-comparison.slang`](less-than-in-body-parses-as-comparison.slang) |
| `<` is treated as a generic-argument list when followed by `>` and a "generic-follower" token like `(`. | functional | [#disambiguation](../../../docs/llm-generated/syntax-reference/grammar.md#disambiguation) | [`generic-call-followed-by-paren.slang`](generic-call-followed-by-paren.slang) |
| A parenthesized expression atom requires a matching `)`; a missing close-paren is rejected. | negative | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | [`expr-missing-close-paren-rejected.slang`](expr-missing-close-paren-rejected.slang) |
| Assignment `=` (level 14) is right-associative; `a = b = 5` parses as `a = (b = 5)`. | functional | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | [`expr-assign-right-associative.slang`](expr-assign-right-associative.slang) |
| Precedence: `&&` (level 11) binds tighter than `\|\|` (level 12). | functional | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | [`expr-logical-and-tighter-than-or.slang`](expr-logical-and-tighter-than-or.slang) |
| Precedence: `*` (level 3) binds tighter than `+` (level 4). | functional | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | [`expr-mul-binds-tighter-than-add.slang`](expr-mul-binds-tighter-than-add.slang) |
| Precedence: unary `-` (level 2) binds tighter than `*` (level 3). | functional | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | [`expr-unary-minus-tighter-than-mul.slang`](expr-unary-minus-tighter-than-mul.slang) |
| Ternary `?:` (level 13) is right-associative; `c1 ? a : c2 ? b : c` parses as `c1 ? a : (c2 ? b : c)`. | functional | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | [`expr-ternary-right-associative.slang`](expr-ternary-right-associative.slang) |
| `AtomExpr` accepts a parenthesized expression `'(' Expr ')'` and the parens override precedence. | functional | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | [`expr-paren-atom.slang`](expr-paren-atom.slang) |
| `KeywordExpr` accepts `'sizeof' '(' Type ')'`. | functional | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | [`expr-sizeof-keyword.slang`](expr-sizeof-keyword.slang) |
| `PostfixSuffix` accepts `'.' IDENT` member access, binding tighter than arithmetic. | functional | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | [`expr-postfix-member-access.slang`](expr-postfix-member-access.slang) |
| `Param` parses with an optional `'=' Expr` default-value clause. | functional | [#function-style-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#function-style-declarations) | [`decl-func-with-default-param.slang`](decl-func-with-default-param.slang) |
| `GenericParam` accepts the type-parameter form `IDENT ':' TypeList`. | functional | [#generics-and-where-clauses](../../../docs/llm-generated/syntax-reference/grammar.md#generics-and-where-clauses) | [`generic-type-param-with-constraint.slang`](generic-type-param-with-constraint.slang) |
| `WhereClause` parses as `'where' WhereTerm (',' WhereTerm)*` with `WhereTerm ::= Type ':' Type` for conformance. | functional | [#generics-and-where-clauses](../../../docs/llm-generated/syntax-reference/grammar.md#generics-and-where-clauses) | [`where-clause-conformance.slang`](where-clause-conformance.slang) |
| `ModifierKeyword` includes `inout`, which attaches to a `Param`. | functional | [#modifiers](../../../docs/llm-generated/syntax-reference/grammar.md#modifiers) | [`modifier-inout-param.slang`](modifier-inout-param.slang) |
| `ModifierList` accepts multiple `ModifierKeyword` tokens (`static`, `const`) in sequence before a `VarDecl`. | functional | [#modifiers](../../../docs/llm-generated/syntax-reference/grammar.md#modifiers) | [`modifier-static-const.slang`](modifier-static-const.slang) |
| `BreakStmt` and `ContinueStmt` parse as bare keyword forms (no label) terminating with `;`. | functional | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements) | [`stmt-break-continue.slang`](stmt-break-continue.slang) |
| `DoWhileStmt` parses as `'do' Stmt 'while' '(' Expr ')' ';'`. | functional | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements) | [`stmt-do-while.slang`](stmt-do-while.slang) |
| `ForStmt` parses with `DeclStmt` init, `Expr` condition, and `Expr` increment. | functional | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements) | [`stmt-for-loop.slang`](stmt-for-loop.slang) |
| `IfStmt` parses as `'if' '(' Expr ')' Stmt ('else' Stmt)?`. | functional | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements) | [`stmt-if-else.slang`](stmt-if-else.slang) |
| `ReturnStmt` parses as `'return' Expr? ';'` and yields the expression value. | functional | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements) | [`stmt-return-expr.slang`](stmt-return-expr.slang) |
| `SwitchStmt` parses with `SwitchCase` arms — `'case' Expr ':'` and `'default' ':'`. | functional | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements) | [`stmt-switch-case-default.slang`](stmt-switch-case-default.slang) |
| `WhileStmt` parses as `'while' '(' Expr ')' Stmt`. | functional | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements) | [`stmt-while-loop.slang`](stmt-while-loop.slang) |
| `NamespaceDecl` parses as `'namespace' IDENT '{' TopDecl* '}'` and qualifies names via `::`. | functional | [#top-level-structure](../../../docs/llm-generated/syntax-reference/grammar.md#top-level-structure) | [`decl-namespace.slang`](decl-namespace.slang) |
| `EnumDecl` parses `'enum' IDENT '{' EnumCase (',' EnumCase)* ','? '}'` with an optional trailing comma. | functional | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations) | [`decl-enum-with-cases.slang`](decl-enum-with-cases.slang) |
| `ExtensionDecl` parses as `'extension' Type '{' StructMember* '}'` and adds methods to an existing type. | functional | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations) | [`decl-extension.slang`](decl-extension.slang) |
| `InterfaceDecl` parses as `'interface' IDENT '{' InterfaceMember* '}'` and accepts a FuncDecl member. | functional | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations) | [`decl-interface-method.slang`](decl-interface-method.slang) |
| `StructDecl` parses as `'struct' IDENT '{' '}'` with an empty member list. | functional | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations) | [`decl-struct-empty.slang`](decl-struct-empty.slang) |
| `StructMember` accepts `VarDecl` members produced by `Type IDENT` C-style declarators. | functional | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations) | [`decl-struct-with-fields.slang`](decl-struct-with-fields.slang) |
| `TypeAliasDecl` parses as `'typealias' IDENT '=' Type ';'`. | functional | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations) | [`decl-typealias.slang`](decl-typealias.slang) |
| `TypedefDecl` parses as `'typedef' Type IDENT ';'` (C-style order: type before name). | functional | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations) | [`decl-typedef.slang`](decl-typedef.slang) |
| `CoreType` accepts the array form `Type '[' Expr ']'`. | functional | [#types](../../../docs/llm-generated/syntax-reference/grammar.md#types) | [`type-array-suffix.slang`](type-array-suffix.slang) |
| `LetDecl` parses as `'let' IDENT (':' Type)? '=' Expr ';'` with a mandatory initializer. | functional | [#variable-binding-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#variable-binding-declarations) | [`decl-let.slang`](decl-let.slang) |
| `VarDecl` accepts the C-style form `Type VarDeclarator (',' VarDeclarator)* ';'` with an `Initializer`. | functional | [#variable-binding-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#variable-binding-declarations) | [`decl-var-cstyle-with-init.slang`](decl-var-cstyle-with-init.slang) |
| `VarDeclarator` accepts an `ArraySuffix` of the form `'[' Expr ']'`. | functional | [#variable-binding-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#variable-binding-declarations) | [`decl-var-array-suffix.slang`](decl-var-array-suffix.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#generic-followers](../../../docs/llm-generated/syntax-reference/grammar.md#generic-followers) | undocumented-behavior | The `### `<` disambiguation` section lists the "generic-followers" punctuation set (`::`, `.`, `(`, `)`, `[`, `]`, `:`, `,`, `?`, `;`, `==`, `!=`, `>`, `>>`) but does not give a positive example for each follower individually. A claim-per-follower expansion would need either an enumerative example or a follower-specific sub-anchor in the doc. |  |
| [#lambda-direct-invocation-fails-under-interpret](../../../docs/llm-generated/syntax-reference/grammar.md#lambda-direct-invocation-fails-under-interpret) | undocumented-behavior | The doc lists `LambdaExpr ::= '(' ParamList? ')' '=>' Expr` and the single-parameter form `IDENT '=>' Expr`, but `_common.md` notes that "Lambda direct-invocation fails under INTERPRET", which precludes the simplest positive test. A separate IFunc-constrained idiom would be needed and is better anchored to the lambda feature doc, not this grammar bundle. |  |
| [#modifiers](../../../docs/llm-generated/syntax-reference/grammar.md#modifiers) | undocumented-behavior | The `## Modifiers` section lists ~50 modifier keywords as one alternative in `ModifierKeyword` without per-modifier syntactic notes; only the production "a modifier list precedes a decl" is a grammar claim. Individual modifier semantics belong to `ast-reference/modifiers.md` or the semantic-check bundle. |  |
| [#try](../../../docs/llm-generated/syntax-reference/grammar.md#try) | undocumented-behavior | The doc mentions `try` is an expression keyword but states there is no `try { } catch { }` statement, only `do ... catch`. We cover `DoCatchStmt` only indirectly via the statement list; a dedicated positive test would require exception machinery beyond pure-parse observation, which `_common.md` warns is fragile under INTERPRET. |  |
| [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions) | undocumented-behavior | The `## Expressions` section lists `AtomExpr ::= ... \| '(' Expr (',' Expr)+ ')' -- tuple` but in practice this surface is parsed as comma-operator expressions, not a tuple literal. The actual user-facing tuple construction uses `makeTuple(...)` or an explicit `Tuple<...>` type. | The doc should either narrow the production (e.g. "only after a tuple-typed binding context") or remove the standalone tuple-atom form. |
| [#types](../../../docs/llm-generated/syntax-reference/grammar.md#types) | undocumented-behavior | The `## Types` section lists pointer (`Type '*'`), optional (`Type '?'`), reference (`Type '&'`), and function-type spellings with the caveat "where supported", but does not say which Slang contexts permit each. Anchoring a positive test for those forms would require the doc to enumerate the supported positions. |  |

## Untested claims

(none)

