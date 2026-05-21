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

## Claims enumerated

| Claim ID | Anchor                                                                                                                                  | Claim (one line)                                                            | Tests                                                                            |
| -------- | --------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| C-01     | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations)                       | `StructDecl` parses with an empty body.                                     | `decl-struct-empty.slang`                                                        |
| C-02     | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations)                       | `StructMember` accepts C-style `Type IDENT;` field declarations.            | `decl-struct-with-fields.slang`                                                  |
| C-03     | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations)                       | `EnumDecl` parses comma-separated `EnumCase`s with optional trailing comma. | `decl-enum-with-cases.slang`                                                     |
| C-04     | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations)                       | `TypeAliasDecl` parses as `'typealias' IDENT '=' Type ';'`.                 | `decl-typealias.slang`                                                           |
| C-05     | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations)                       | `TypedefDecl` parses as `'typedef' Type IDENT ';'`.                         | `decl-typedef.slang`                                                             |
| C-06     | [#top-level-structure](../../../docs/llm-generated/syntax-reference/grammar.md#top-level-structure)                                     | `NamespaceDecl` and `QualifiedName` with `::`.                              | `decl-namespace.slang`                                                           |
| C-07     | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations)                       | `InterfaceDecl` parses with a `FuncDecl` member.                            | `decl-interface-method.slang`                                                    |
| C-08     | [#type-defining-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations)                       | `ExtensionDecl` parses and adds methods to an existing type.                | `decl-extension.slang`                                                           |
| C-09     | [#function-style-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#function-style-declarations)                     | `Param` accepts an `'=' Expr` default-value clause.                         | `decl-func-with-default-param.slang`                                             |
| C-10     | [#variable-binding-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#variable-binding-declarations)                 | `LetDecl` parses with an initializer and no type annotation.                | `decl-let.slang`                                                                 |
| C-11     | [#variable-binding-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#variable-binding-declarations)                 | `VarDecl` C-style with multiple declarators in one statement.               | `decl-var-cstyle-with-init.slang`                                                |
| C-12     | [#variable-binding-declarations](../../../docs/llm-generated/syntax-reference/grammar.md#variable-binding-declarations)                 | `VarDeclarator` accepts `ArraySuffix` `'[' Expr ']'`.                       | `decl-var-array-suffix.slang`                                                    |
| C-13     | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions)                                                     | Precedence: `*` binds tighter than `+`.                                     | `expr-mul-binds-tighter-than-add.slang`                                          |
| C-14     | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions)                                                     | Precedence: unary `-` binds tighter than `*`.                               | `expr-unary-minus-tighter-than-mul.slang`                                        |
| C-15     | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions)                                                     | Assignment `=` is right-associative.                                        | `expr-assign-right-associative.slang`                                            |
| C-16     | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions)                                                     | Ternary `?:` is right-associative.                                          | `expr-ternary-right-associative.slang`                                           |
| C-17     | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions)                                                     | `&&` binds tighter than `\|\|`.                                             | `expr-logical-and-tighter-than-or.slang`                                         |
| C-18     | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions)                                                     | `PostfixSuffix` `.member` binds tighter than arithmetic.                    | `expr-postfix-member-access.slang`                                               |
| C-19     | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions)                                                     | `AtomExpr` accepts a parenthesized expression `'(' Expr ')'`.               | `expr-paren-atom.slang`                                                          |
| C-20     | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions)                                                     | `KeywordExpr` accepts `'sizeof' '(' Type ')'`.                              | `expr-sizeof-keyword.slang`                                                      |
| C-21     | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements)                                                       | `IfStmt` parses with optional `else` arm.                                   | `stmt-if-else.slang`                                                             |
| C-22     | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements)                                                       | `ForStmt` parses with decl-init, condition, increment.                      | `stmt-for-loop.slang`                                                            |
| C-23     | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements)                                                       | `WhileStmt` parses.                                                         | `stmt-while-loop.slang`                                                          |
| C-24     | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements)                                                       | `DoWhileStmt` parses; body runs once before condition.                      | `stmt-do-while.slang`                                                            |
| C-25     | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements)                                                       | `SwitchStmt` parses `case` and `default` arms.                              | `stmt-switch-case-default.slang`                                                 |
| C-26     | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements)                                                       | `ReturnStmt` parses with an `Expr` operand.                                 | `stmt-return-expr.slang`                                                         |
| C-27     | [#statements](../../../docs/llm-generated/syntax-reference/grammar.md#statements)                                                       | `BreakStmt` / `ContinueStmt` parse in their bare forms.                     | `stmt-break-continue.slang`                                                      |
| C-28     | [#types](../../../docs/llm-generated/syntax-reference/grammar.md#types)                                                                 | `CoreType` accepts the array suffix `Type '[' Expr ']'`.                    | `type-array-suffix.slang`                                                        |
| C-29     | [#disambiguation](../../../docs/llm-generated/syntax-reference/grammar.md#disambiguation)                                               | `<` followed by `>(` resolves as a generic-application.                     | `generic-call-followed-by-paren.slang`                                           |
| C-30     | [#disambiguation](../../../docs/llm-generated/syntax-reference/grammar.md#disambiguation)                                               | `<` in body context with non-generic LHS is a comparison.                   | `less-than-in-body-parses-as-comparison.slang`                                   |
| C-31     | [#generics-and-where-clauses](../../../docs/llm-generated/syntax-reference/grammar.md#generics-and-where-clauses)                       | `GenericParam` accepts `IDENT ':' TypeList` constraint.                     | `generic-type-param-with-constraint.slang`                                       |
| C-32     | [#generics-and-where-clauses](../../../docs/llm-generated/syntax-reference/grammar.md#generics-and-where-clauses)                       | `WhereClause` parses conformance constraints `Type ':' Type`.               | `where-clause-conformance.slang`                                                 |
| C-33     | [#attributes-and-decorations](../../../docs/llm-generated/syntax-reference/grammar.md#attributes-and-decorations)                       | Bracket attribute `'[' Name '(' ArgList ')' ']'` parses and attaches.       | `attribute-bracket-form.slang`                                                   |
| C-34     | [#modifiers](../../../docs/llm-generated/syntax-reference/grammar.md#modifiers)                                                         | `ModifierList` collects `static` `const` before a `VarDecl`.                | `modifier-static-const.slang`                                                    |
| C-35     | [#modifiers](../../../docs/llm-generated/syntax-reference/grammar.md#modifiers)                                                         | `inout` modifier attaches to a `Param`.                                     | `modifier-inout-param.slang`                                                     |
| C-36     | [#declarations](../../../docs/llm-generated/syntax-reference/grammar.md#declarations)                                                   | A token that cannot begin a `Decl` triggers a parse-stage diagnostic.       | `decl-unexpected-token-rejected.slang`                                           |
| C-37     | [#expressions](../../../docs/llm-generated/syntax-reference/grammar.md#expressions)                                                     | Missing close-paren in a parenthesized expression is rejected.              | `expr-missing-close-paren-rejected.slang`                                        |

## Tests in this bundle

| File                                            | Intent     | Doc anchor                       |
| ----------------------------------------------- | ---------- | -------------------------------- |
| `decl-struct-empty.slang`                       | functional | `#type-defining-declarations`    |
| `decl-struct-with-fields.slang`                 | functional | `#type-defining-declarations`    |
| `decl-enum-with-cases.slang`                    | functional | `#type-defining-declarations`    |
| `decl-typealias.slang`                          | functional | `#type-defining-declarations`    |
| `decl-typedef.slang`                            | functional | `#type-defining-declarations`    |
| `decl-namespace.slang`                          | functional | `#top-level-structure`           |
| `decl-interface-method.slang`                   | functional | `#type-defining-declarations`    |
| `decl-extension.slang`                          | functional | `#type-defining-declarations`    |
| `decl-func-with-default-param.slang`            | functional | `#function-style-declarations`   |
| `decl-let.slang`                                | functional | `#variable-binding-declarations` |
| `decl-var-cstyle-with-init.slang`               | functional | `#variable-binding-declarations` |
| `decl-var-array-suffix.slang`                   | functional | `#variable-binding-declarations` |
| `expr-mul-binds-tighter-than-add.slang`         | functional | `#expressions`                   |
| `expr-unary-minus-tighter-than-mul.slang`       | functional | `#expressions`                   |
| `expr-assign-right-associative.slang`           | functional | `#expressions`                   |
| `expr-ternary-right-associative.slang`          | functional | `#expressions`                   |
| `expr-logical-and-tighter-than-or.slang`        | functional | `#expressions`                   |
| `expr-postfix-member-access.slang`              | functional | `#expressions`                   |
| `expr-paren-atom.slang`                         | functional | `#expressions`                   |
| `expr-sizeof-keyword.slang`                     | functional | `#expressions`                   |
| `stmt-if-else.slang`                            | functional | `#statements`                    |
| `stmt-for-loop.slang`                           | functional | `#statements`                    |
| `stmt-while-loop.slang`                         | functional | `#statements`                    |
| `stmt-do-while.slang`                           | functional | `#statements`                    |
| `stmt-switch-case-default.slang`                | functional | `#statements`                    |
| `stmt-return-expr.slang`                        | functional | `#statements`                    |
| `stmt-break-continue.slang`                     | functional | `#statements`                    |
| `type-array-suffix.slang`                       | functional | `#types`                         |
| `generic-call-followed-by-paren.slang`          | functional | `#disambiguation`                |
| `less-than-in-body-parses-as-comparison.slang`  | functional | `#disambiguation`                |
| `generic-type-param-with-constraint.slang`      | functional | `#generics-and-where-clauses`    |
| `where-clause-conformance.slang`                | functional | `#generics-and-where-clauses`    |
| `attribute-bracket-form.slang`                  | functional | `#attributes-and-decorations`    |
| `modifier-static-const.slang`                   | functional | `#modifiers`                     |
| `modifier-inout-param.slang`                    | functional | `#modifiers`                     |
| `decl-unexpected-token-rejected.slang`          | negative   | `#declarations`                  |
| `expr-missing-close-paren-rejected.slang`       | negative   | `#expressions`                   |

## Doc gaps observed

- The `### `<` disambiguation` section lists the "generic-followers"
  punctuation set (`::`, `.`, `(`, `)`, `[`, `]`, `:`, `,`, `?`,
  `;`, `==`, `!=`, `>`, `>>`) but does not give a positive example
  for each follower individually. A claim-per-follower expansion
  would need either an enumerative example or a follower-specific
  sub-anchor in the doc.
- The doc lists `LambdaExpr ::= '(' ParamList? ')' '=>' Expr` and
  the single-parameter form `IDENT '=>' Expr`, but `_common.md`
  notes that "Lambda direct-invocation fails under INTERPRET",
  which precludes the simplest positive test. A separate
  IFunc-constrained idiom would be needed and is better anchored
  to the lambda feature doc, not this grammar bundle.
- The `## Modifiers` section lists ~50 modifier keywords as one
  alternative in `ModifierKeyword` without per-modifier syntactic
  notes; only the production "a modifier list precedes a decl" is a
  grammar claim. Individual modifier semantics belong to
  `ast-reference/modifiers.md` or the semantic-check bundle.
- The doc mentions `try` is an expression keyword but states there
  is no `try { } catch { }` statement, only `do ... catch`. We
  cover `DoCatchStmt` only indirectly via the statement list; a
  dedicated positive test would require exception machinery beyond
  pure-parse observation, which `_common.md` warns is fragile under
  INTERPRET.
- The `## Expressions` section lists `AtomExpr ::= ... | '(' Expr (',' Expr)+ ')' -- tuple` but in practice this surface is parsed as comma-operator expressions, not a tuple literal. The actual user-facing tuple construction uses `makeTuple(...)` or an explicit `Tuple<...>` type. The doc should either narrow the production (e.g. "only after a tuple-typed binding context") or remove the standalone tuple-atom form.
- The `## Types` section lists pointer (`Type '*'`), optional
  (`Type '?'`), reference (`Type '&'`), and function-type spellings
  with the caveat "where supported", but does not say which Slang
  contexts permit each. Anchoring a positive test for those forms
  would require the doc to enumerate the supported positions.

## Out of scope (no-GPU runner)

None: grammar-stage claims are observed via parse + INTERPRET, no
GPU runtime required. `attribute-bracket-form.slang` uses
`-target hlsl` text emit only to keep the entry-point attribute in
the source where the parser must consume it; no GPU is involved.
