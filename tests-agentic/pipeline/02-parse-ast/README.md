---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T15:00:00+00:00
source_commit: 74db89b9f77cdced9c4d0c47f377b38fffb9180b
watched_paths_digest: 4b0540420709071d4ae8e6fb49464229e1ed4854c95ebaf2ab3f36c22ad3b022
source_doc: docs/llm-generated/pipeline/02-parse-ast.md
source_doc_digest: 977f6e495e28bc2ba271733427de33d6aa497d32ec2fd71b39151f0476b5887b
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for pipeline/02-parse-ast

## Intent

Tests verify the parser-stage behaviors described in
[`docs/llm-generated/pipeline/02-parse-ast.md`](../../../docs/llm-generated/pipeline/02-parse-ast.md):
two-stage parsing (decl-stage vs body-stage), syntax-as-declaration
via the `SyntaxParseInfo` table, error recovery, the major AST node
families (`Decl`, `Expr`, `Stmt`, `Type`, `Modifier`), the
`<` disambiguation strategy at the heart of `## Generics ambiguity`,
modifier and attribute parsing, and the `## Failure modes` parse-time
diagnostics. The bundle deliberately does not duplicate
`syntax-reference/tokens` (lex-surface) or
`syntax-reference/keywords-and-builtins` (keyword spellings); claims
here all hinge on something the parser does after token assembly.

Coverage strategy: at least one functional test for every named
sub-section in the source doc that yields a slangc/slangi-observable
claim. Most tests use `//TEST:INTERPRET` because parser claims are
target-independent — multi-backend repetition would only add runtime
cost. Failure-mode and error-recovery claims use
`//DIAGNOSTIC_TEST:SIMPLE` with the `non-exhaustive` argument so the
test pins the diagnostic the doc names without overcommitting to
neighbouring messages the doc does not promise.

## Claims enumerated

| Claim ID | Anchor                                                                                                                          | Claim (one line)                                                                                                                                       | Tests                                              |
| -------- | ------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------- |
| C-01     | [#two-stage-parsing](../../../docs/llm-generated/pipeline/02-parse-ast.md#two-stage-parsing)                                    | Function bodies are captured as `UnparsedStmt` at decl-stage and re-parsed in body mode after the outer scope is established.                          | [`function-body-deferred-to-check-stage.slang`](function-body-deferred-to-check-stage.slang)      |
| C-02     | [#two-stage-parsing](../../../docs/llm-generated/pipeline/02-parse-ast.md#two-stage-parsing)                                    | Because body-parse runs after decl-stage, a body may forward-reference an outer declaration that appears later in the file.                            | [`body-forward-references-outer-decl.slang`](body-forward-references-outer-decl.slang)         |
| C-03     | [#syntax-as-declaration](../../../docs/llm-generated/pipeline/02-parse-ast.md#syntax-as-declaration)                            | Modifier keywords are bound to syntax via the `SyntaxParseInfo` table rather than being hard-coded; the parser dispatches by identifier lookup.        | [`modifier-keyword-from-syntax-table.slang`](modifier-keyword-from-syntax-table.slang)         |
| C-04     | [#error-recovery](../../../docs/llm-generated/pipeline/02-parse-ast.md#error-recovery)                                          | On an unexpected token the parser emits a diagnostic and uses skip-to-synchronization-point heuristics to continue, so later errors are also reported. | [`error-recovery-continues-after-syntax-error.slang`](error-recovery-continues-after-syntax-error.slang)|
| C-05     | [#the-major-node-families](../../../docs/llm-generated/pipeline/02-parse-ast.md#the-major-node-families)                        | `Decl` family: `struct` parses into a Decl with member sub-Decls.                                                                                      | [`decl-struct-with-fields.slang`](decl-struct-with-fields.slang)                    |
| C-06     | [#the-major-node-families](../../../docs/llm-generated/pipeline/02-parse-ast.md#the-major-node-families)                        | `Decl` family: `extension <Type> { ... }` parses into an ExtensionDecl whose members become callable on the target.                                    | [`decl-extension-adds-method.slang`](decl-extension-adds-method.slang)                 |
| C-07     | [#the-major-node-families](../../../docs/llm-generated/pipeline/02-parse-ast.md#the-major-node-families)                        | `Stmt` family: a `{ ... }` block holds an ordered sequence of sub-statements that execute in source order.                                             | [`stmt-block-sequence.slang`](stmt-block-sequence.slang)                        |
| C-08     | [#the-major-node-families](../../../docs/llm-generated/pipeline/02-parse-ast.md#the-major-node-families)                        | `Stmt` family: `for (init; cond; update) { body }` parses with three sub-expressions and a body block.                                                 | [`stmt-for-loop.slang`](stmt-for-loop.slang)                              |
| C-09     | [#the-major-node-families](../../../docs/llm-generated/pipeline/02-parse-ast.md#the-major-node-families)                        | `Expr` family: precedence is encoded in the Expr tree shape; `*` binds tighter than `+`.                                                               | [`expr-precedence-mul-over-add.slang`](expr-precedence-mul-over-add.slang)               |
| C-10     | [#the-major-node-families](../../../docs/llm-generated/pipeline/02-parse-ast.md#the-major-node-families)                        | `Expr` family: assignment is right-associative.                                                                                                        | [`expr-assign-right-associative.slang`](expr-assign-right-associative.slang)              |
| C-11     | [#the-major-node-families](../../../docs/llm-generated/pipeline/02-parse-ast.md#the-major-node-families)                        | `Expr` family: postfix `.` member access binds tightly and left-associatively.                                                                         | [`expr-postfix-binds-tight.slang`](expr-postfix-binds-tight.slang)                   |
| C-12     | [#the-major-node-families](../../../docs/llm-generated/pipeline/02-parse-ast.md#the-major-node-families)                        | `Expr` family: at parse time function calls and type constructions share the InvokeExpr representation (expr-or-type unification).                     | [`expr-invoke-call-and-type-construction.slang`](expr-invoke-call-and-type-construction.slang)     |
| C-13     | [#types](../../../docs/llm-generated/syntax-reference/grammar.md#types)                                                         | `Type` family: the Type production includes the array form `T '[' Expr? ']'`.                                                                          | [`type-array-spelling.slang`](type-array-spelling.slang)                        |
| C-14     | [#generics-ambiguity](../../../docs/llm-generated/pipeline/02-parse-ast.md#generics-ambiguity)                                  | `<` after an identifier in body position commits as a generic-application when the trial parse succeeds.                                               | [`generic-application-in-body.slang`](generic-application-in-body.slang)                |
| C-15     | [#generics-ambiguity](../../../docs/llm-generated/pipeline/02-parse-ast.md#generics-ambiguity)                                  | When the generic-application trial does not commit cleanly the parser rolls back and parses `<` as the less-than operator.                             | [`less-than-in-body-is-comparison.slang`](less-than-in-body-is-comparison.slang)            |
| C-16     | [#generics-ambiguity](../../../docs/llm-generated/pipeline/02-parse-ast.md#generics-ambiguity)                                  | A generic declaration after a decl keyword is unambiguous; `<` can only begin a parameter list there.                                                  | [`generic-decl-after-keyword.slang`](generic-decl-after-keyword.slang)                 |
| C-17     | [#modifier-parsing](../../../docs/llm-generated/pipeline/02-parse-ast.md#modifier-parsing)                                      | Modifier tokens collected before a decl keyword attach as Modifier nodes to the resulting Decl.                                                        | [`modifier-static-const-on-decl.slang`](modifier-static-const-on-decl.slang)              |
| C-18     | [#modifier-parsing](../../../docs/llm-generated/pipeline/02-parse-ast.md#modifier-parsing)                                      | Attribute parsing is modifier parsing in disguise — `[name(args)]` tokens become an AttributeBase modifier on the following decl.                      | [`attribute-bracket-form.slang`](attribute-bracket-form.slang)                     |
| C-19     | [#modifier-parsing](../../../docs/llm-generated/pipeline/02-parse-ast.md#modifier-parsing)                                      | Attribute syntax attaches modifiers to statements as well as to declarations (`[unroll]` on a for-loop).                                               | [`attribute-on-stmt.slang`](attribute-on-stmt.slang)                          |
| C-20     | [#failure-modes](../../../docs/llm-generated/pipeline/02-parse-ast.md#failure-modes)                                            | Token-level errors not previously reported by the lexer (e.g. an unrecognized punctuator inside a decl) surface as parse-time diagnostics.             | [`failure-mode-unrecognized-punctuator.slang`](failure-mode-unrecognized-punctuator.slang)       |

## Tests in this bundle

| File                                                | Intent     | Doc anchor                  |
| --------------------------------------------------- | ---------- | --------------------------- |
| [`function-body-deferred-to-check-stage.slang`](function-body-deferred-to-check-stage.slang)       | functional | `#two-stage-parsing`        |
| [`body-forward-references-outer-decl.slang`](body-forward-references-outer-decl.slang)          | functional | `#two-stage-parsing`        |
| [`modifier-keyword-from-syntax-table.slang`](modifier-keyword-from-syntax-table.slang)          | functional | `#syntax-as-declaration`    |
| [`error-recovery-continues-after-syntax-error.slang`](error-recovery-continues-after-syntax-error.slang) | negative   | `#error-recovery`           |
| [`decl-struct-with-fields.slang`](decl-struct-with-fields.slang)                     | functional | `#the-major-node-families`  |
| [`decl-extension-adds-method.slang`](decl-extension-adds-method.slang)                  | functional | `#the-major-node-families`  |
| [`stmt-block-sequence.slang`](stmt-block-sequence.slang)                         | functional | `#the-major-node-families`  |
| [`stmt-for-loop.slang`](stmt-for-loop.slang)                               | functional | `#the-major-node-families`  |
| [`expr-precedence-mul-over-add.slang`](expr-precedence-mul-over-add.slang)                | functional | `#the-major-node-families`  |
| [`expr-assign-right-associative.slang`](expr-assign-right-associative.slang)               | functional | `#the-major-node-families`  |
| [`expr-postfix-binds-tight.slang`](expr-postfix-binds-tight.slang)                    | functional | `#the-major-node-families`  |
| [`expr-invoke-call-and-type-construction.slang`](expr-invoke-call-and-type-construction.slang)      | functional | `#the-major-node-families`  |
| [`type-array-spelling.slang`](type-array-spelling.slang)                         | functional | `#types`                    |
| [`generic-application-in-body.slang`](generic-application-in-body.slang)                 | functional | `#generics-ambiguity`       |
| [`less-than-in-body-is-comparison.slang`](less-than-in-body-is-comparison.slang)             | functional | `#generics-ambiguity`       |
| [`generic-decl-after-keyword.slang`](generic-decl-after-keyword.slang)                  | functional | `#generics-ambiguity`       |
| [`modifier-static-const-on-decl.slang`](modifier-static-const-on-decl.slang)               | functional | `#modifier-parsing`         |
| [`attribute-bracket-form.slang`](attribute-bracket-form.slang)                      | functional | `#modifier-parsing`         |
| [`attribute-on-stmt.slang`](attribute-on-stmt.slang)                           | functional | `#modifier-parsing`         |
| [`failure-mode-unrecognized-punctuator.slang`](failure-mode-unrecognized-punctuator.slang)        | negative   | `#failure-modes`            |

## Doc gaps observed

- The `## Generics ambiguity` section describes the disambiguation
  strategy in narrative form ("try a generic-application parse and
  roll back if it does not commit cleanly") but does not enumerate
  the **set of follower tokens** that decides commit-vs-rollback.
  Grammar.md's `### \`<\` disambiguation` lists them explicitly
  (`::`, `.`, `(`, `)`, `[`, `]`, `:`, `,`, `?`, `;`, `==`, `!=`,
  `>`, `>>`). The parse-AST doc should either copy that table or
  unambiguously hand off to it; right now a reader could not write a
  rollback test from the parse-AST doc alone.
- The `## AST data model` family list mentions `Val` as a sixth
  family ("compile-time values used by generics") but gives no
  parse-stage observable that distinguishes a `Val` from a `Type`.
  No test anchored against `Val` here; it would need a
  semantics-stage handoff to make a verifiable claim.
- `## Two-stage parsing` mentions the body parser carries a
  back-pointer to `SemanticsVisitor` and asks the checker whether
  the token before `<` resolves to a generic; the doc does not give
  an *observable* failure case when the heuristic is *wrong*. The
  recovery from a wrong guess ("parser prefers to produce *some* AST
  and let the checker emit a more specific error") is named in
  `## Failure modes` but there is no concrete example the doc
  promises to the user. A small example pinning the recovery
  behavior would let this bundle add a regression test.
- `## Modifier parsing` does not enumerate which contexts accept
  which attributes (e.g. which attributes are valid on a `Stmt`
  vs a `Decl`). The bundle tests one attribute on a stmt
  (`[unroll]`) and one on a decl (`[ForceInline]`) but cannot make a
  general claim about attribute applicability without further doc
  detail. Could be filed against the modifier-parsing section.
- `## Error recovery` says the parser uses "simple
  skip-to-synchronization-point heuristics (looking for `;`, `}`,
  or the next declaration keyword)" but does not promise that any
  specific input produces any specific number of diagnostics. The
  bundle's error-recovery test relies on the observable fact that
  two adjacent function bodies each report their own error; if the
  recovery heuristic changes to merge or suppress one of them the
  test will need to be updated even though the doc claim hasn't
  changed. Recorded as fragility; a tighter doc commitment would
  help.
- `## AST data model` describes `ASTBuilder` ownership and hash-
  consing of types, but neither has a slangc/slangi-observable
  surface (hash-consing identity is hidden by the IR / emit
  pipelines). Recorded as architecture-only, not testable here.
- The pure-decl-stage observable that the doc calls out — "Function
  and method bodies are still unparsed at this stage" — is hard to
  pin without IR-dump inspection that the agentic suite explicitly
  excludes (and the doc does not promise any user-visible diagnostic
  for it). The bundle's `function-body-deferred-to-check-stage.slang`
  and `body-forward-references-outer-decl.slang` cover the
  consequence; the cleanest direct test would need a doc-promised
  observable for "this body has not been parsed yet".

## Out of scope (no-GPU runner)

None for this bundle. All parse-stage claims are fully observable
through `slangi` or via diagnostic-only tests.
