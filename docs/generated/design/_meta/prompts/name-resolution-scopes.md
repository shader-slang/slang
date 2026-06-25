# Prompt: name-resolution/scopes.md

See [_common.md](_common.md) for the universal rules and the
**Name-resolution family contract** that applies to this page.

## Target

Produce `docs/generated/design/name-resolution/scopes.md` — the page that
documents the `Scope` data structure, how the parser builds the scope
chain, and which AST nodes introduce new scopes.

Audience: a developer modifying scope construction (in the parser),
adding a new scope-bearing node, or trying to understand why a given
identifier is in scope at a particular source location.

## Family-specific guidance

This page uses the `## Rules` heading (not `## Algorithm`). The
"algorithm" is parser-driven: the parser threads `Scope` instances
through the AST as it descends.

The `## Concepts` section must cover, at minimum:

- `Scope` — declared in
  [slang-ast-base.h](../../../../source/slang/slang-ast-base.h). Cite the
  `containerDecl`, `parent`, `nextSibling` fields and the line range.
- `ContainerDecl` — the `Decl` subclass that owns a scope (it has an
  `ownedScope` member). Declared in
  [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h).
- `ScopeDecl` — the synthetic `Decl` subclass used to attach a scope
  to a `BlockStmt`. Declared in `slang-ast-decl.h`.
- `ScopeStmt` — the abstract base of statements that introduce a
  scope. Declared in
  [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h). Cite
  its `scopeDecl` field.
- `BlockStmt` — concrete scope-bearing statement; cite the field that
  links it to a `ScopeDecl`.

The `## Rules` section must explain:

1. **Scope kinds**. Enumerate the AST nodes that introduce a fresh
   scope: `ModuleDecl`, `NamespaceDecl`, `AggTypeDecl` (and subclasses
   `StructDecl` / `ClassDecl` / `InterfaceDecl` / `EnumDecl`),
   `ExtensionDecl`, `GenericDecl`, `CallableDecl` (function /
   constructor / subscript / accessor), `BlockStmt`, `ForStmt`,
   `IfStmt` (when it introduces a let-binding), and any other concrete
   class in the watched headers that carries an `ownedScope` or a
   `ScopeStmt::scopeDecl`. For each, cite the C++ symbol that proves
   it owns a scope.
2. **Parser scope construction**. Show how the parser threads scopes.
   Cite the parser entry points in
   [slang-parser.cpp](../../../../source/slang/slang-parser.cpp) that
   push/pop scopes (search for `pushScope`, `popScope`,
   `currentScope`, `parseScopedStmt`, or whatever the actual helpers
   are named at `source_commit`). Include a short mermaid diagram of
   the scope chain for a representative nested structure:

   ```
   module -> namespace -> struct -> generic-params -> func -> block
   ```

3. **Sibling scopes**. Cover
   `addSiblingScopeForContainerDecl` (or whatever the helper is
   actually called at `source_commit`) declared in
   [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h). Explain
   the reason siblings exist: extension / partial-class / multiple
   `namespace` declarations of the same logical container.
4. **Implicit scopes**. Generic parameter lists, extension bodies, and
   interface requirements introduce implicit scopes between their
   "outside" and "inside" decls; explain which class owns each.
5. **Scope walking order during lookup**. State the order in plain
   prose (current scope -> sibling scopes -> parent -> ... -> module
   -> imported modules). The detailed algorithm lives in
   [lookup.md](lookup.md); this page just states the order and
   defers.

The `## Edge cases and failure modes` section must cover:

- A `BlockStmt` body that contains no declarations still has a fresh
  `ScopeDecl` — why this matters for shadowing.
- Generic parameter list scopes are visible to the inner decl but not
  to siblings of the outer decl.
- `ExtensionDecl` scopes do not contain the target type's members
  directly — those are reached through lookup, not through the scope
  chain.
- Multiple `namespace Foo {}` declarations in the same module produce
  multiple `NamespaceDecl`s linked as sibling scopes; lookup must walk
  every sibling.
- `using`-decl injection (if `UsingDecl` is present in the watched
  header) and how it appears in the scope.

## Forbidden content (in addition to the universal forbidden list and the family contract)

- Per-`Decl`-subclass field documentation. Link
  [../ast-reference/declarations.md](../ast-reference/declarations.md)
  and [../ast-reference/statements.md](../ast-reference/statements.md)
  instead.
- The lookup algorithm itself — that belongs in
  [lookup.md](lookup.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every scope-bearing class is named with its declaring header.
- [ ] Mermaid scope-chain diagram is consistent with the actual parser
      construction sites cited.
- [ ] No claims about sibling-scope behavior that are not grounded in
      a cited helper function.
