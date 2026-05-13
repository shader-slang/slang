# Prompt: ast-reference/statements.md

See [_common.md](_common.md) for the universal rules and the **AST
reference family contract**.

## Target

Produce `docs/llm-generated/ast-reference/statements.md` — the
per-node reference for the `Stmt` family declared in
[slang-ast-stmt.h](../../../source/slang/slang-ast-stmt.h).

## Family-specific guidance

- The family hierarchy diagram should show the abstract intermediates
  that group related concrete statements: `Stmt`, `ScopeStmt`,
  `ChildStmt`, control-flow groupings (e.g. `LoopStmt`-like
  intermediates if present, branching statements grouping).
- The `## Nodes` table must cover every concrete class in
  `slang-ast-stmt.h`.
- Grammar column: link to
  [../syntax-reference/grammar.md#statements](../syntax-reference/grammar.md#statements).
  For statements that have a more specific production (for-loop,
  while-loop, switch, defer, return), name that production in the
  Summary column.

## Notable nodes (the `## Notable nodes` callouts)

Cover at least the following nodes:

- `BlockStmt` and `SeqStmt` — the distinction between a scoped block
  and a flat sequence of statements, and when each is produced.
- `ScopeStmt` — its role as a parent for block-scoped declarations.
- `IfStmt`, `SwitchStmt`, `CaseStmt`, `DefaultStmt` — the case/default
  parenting model.
- `ForStmt`, `WhileStmt`, `DoWhileStmt` — the parameter sets they
  share (initial / condition / step / body).
- `ReturnStmt` — its role both as control flow and as the carrier of
  the function's return expression.
- `DeferStmt` — what it produces in the AST, deferring lowering to
  the IR pipeline; cite
  [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) for
  semantics.
- `LabelStmt`, `BreakStmt`, `ContinueStmt`, `DiscardStmt` — the
  labeled-control-flow model.
- `ExpressionStmt`, `DeclStmt`, `EmptyStmt` — boilerplate but worth
  one short note each on when the parser emits them.

If a class is absent at the current commit, omit it.

## Forbidden content

- IR-level statement lowering — see
  [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md).
- Reachability / control-flow analysis — see
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every concrete class in `slang-ast-stmt.h` appears in the table.
- [ ] Grammar links resolve.
