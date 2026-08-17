# Prompt: ast-reference/base.md

See [_common.md](_common.md) for universal rules and for the **AST
reference family contract** that applies to every page under
`docs/generated/design/ast-reference/`.

## Target

Produce `docs/generated/design/ast-reference/base.md` — the entry point of
the AST reference subtree. Unlike the per-family pages, this page
describes the **abstract roots** that every family page assumes the
reader already knows.

Audience: a developer who has read [../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md)
and now wants the per-class facts before drilling into a family page.

## Required structure

Override the standard family contract for the `## Nodes` section: this
page lists *abstract* roots, not concrete leaves. Use this structure:

1. `# AST Base Reference` title.
2. `## Source` linking [slang-ast-base.h](../../../../source/slang/slang-ast-base.h),
   [slang-ast-forward-declarations.h](../../../../source/slang/slang-ast-forward-declarations.h),
   and [slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h).
3. `## Root hierarchy` — a mermaid diagram of the abstract roots
   declared in `slang-ast-base.h`:
   `NodeBase`, `SyntaxNode`, `DeclBase`, `Decl`, `ModifiableSyntaxNode`,
   `Stmt`, `Expr`, `Modifier`, `Val`, `Type`.
   The diagram must reflect the actual `: public X` lines in the
   header. Verify each edge before drawing it.
4. `## Roots` — one subsection per abstract root, in this order:
   `NodeBase`, `SyntaxNode`, `DeclBase`, `Decl`, `ModifiableSyntaxNode`,
   `Stmt`, `Expr`, `Modifier`, `Val`, `Type`. For each root:
   - Bold header with the class name and its immediate parent in
     parentheses, e.g. `### Decl (DeclBase)`.
   - 2-4 sentences describing the role of the class.
   - A short bullet list of the most semantically important fields
     declared at this level (not inherited). 1-4 fields, in `name: Type`
     form. If the root declares no fields beyond what the parent has,
     write "(no additional state)".
   - A `Family page:` line linking the per-family reference page for
     this root, when one exists (e.g. `Decl` -> `declarations.md`,
     `Expr` -> `expressions.md`, ...). For roots without a dedicated
     family page (`NodeBase`, `SyntaxNode`, `DeclBase`,
     `ModifiableSyntaxNode`), write "(no dedicated family page)".
5. `## Support types` — short table covering the most-used
   non-node helpers from
   [slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h):
   `DeclRef`, `LookupResult`, `NameLoc`, `DiagnosticInfo`-related
   helpers, `Scope`. Columns: Name, Header, Purpose. One row per type.
6. `## See also` — bullets linking the per-family reference pages and
   [../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md).

## Quality checklist (in addition to the universal one)

- [ ] Every class named here is declared in one of the three watched
      headers. Do not invent classes.
- [ ] Every parent relationship in the diagram and in the `### Root`
      subsection headers matches the `: public X` line in the source.
- [ ] No concrete-leaf classes (e.g. `IfStmt`, `IntLitExpr`) appear in
      this page — they live in the per-family pages.
