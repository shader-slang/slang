# Prompt: ast-reference/expressions.md

See [_common.md](_common.md) for the universal rules and the **AST
reference family contract**.

## Target

Produce `docs/llm-generated/ast-reference/expressions.md` — the
per-node reference for the `Expr` family declared in
[slang-ast-expr.h](../../../source/slang/slang-ast-expr.h).

## Family-specific guidance

- The family hierarchy diagram should show the most-used abstract
  intermediates: `Expr`, `OperatorExpr`, `InfixExpr` / `PrefixExpr` /
  `PostfixExpr`, `InvokeExpr`, `MemberExpr`-family, `LiteralExpr`-family,
  `OverloadedExpr`-family, `MaterializeExpr`-family.
- The `## Nodes` table must cover every concrete class in
  `slang-ast-expr.h`. Abstract `FIDDLE(abstract)` classes appear only
  in the hierarchy diagram.
- Grammar column: link to the `Expressions` section anchor
  ([../syntax-reference/grammar.md#expressions](../syntax-reference/grammar.md#expressions))
  by default. Use a more specific anchor when one exists for the
  applicable production (literal, member access, call, ...).
  For expressions that the parser does not produce directly — i.e.
  those synthesized by semantic checking — write `(none)`.

## Notable nodes (the `## Notable nodes` callouts)

Cover at least the following nodes with a 2-5 sentence callout:

- `InvokeExpr` — its dual role as the result of explicit calls and as
  the entry point for overload resolution.
- `OverloadedExpr` / `OverloadedExpr2` — when they appear and how the
  checker collapses them.
- `MemberExpr` family (`MemberExpr`, `StaticMemberExpr`,
  `DerefMemberExpr`) — the distinction and when each is produced.
- `VarExpr` and `DeclRefExpr` — the relationship; when the parser
  emits one vs the other.
- `*MaterializeExpr` family — what materialization means in this
  context and why these nodes exist.
- `PartiallyAppliedGenericExpr` — its role in two-stage parsing.
- `LiteralExpr` family — the kinds of literals (int, float, bool,
  string, null) and how the lexer feeds them.
- `LambdaExpr` — its role; cite the relevant `slang-parser.cpp`
  function.
- `AsTypeExpr` / `IsTypeExpr` — explicit type tests; how they relate
  to existential types in
  [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).

If any of these classes does not exist at the current commit, omit it
without comment.

## Forbidden content (in addition to the universal forbidden list)

- Operator precedence — that lives in
  [../syntax-reference/grammar.md](../syntax-reference/grammar.md).
- IR lowering of expressions — that lives in
  [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every concrete class in `slang-ast-expr.h` appears in the table.
- [ ] Every grammar link resolves.
- [ ] At least 6 nodes have a `Notable nodes` callout.
