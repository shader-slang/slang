# Prompt: ast-reference/declarations.md

See [_common.md](_common.md) for the universal rules and the **AST
reference family contract** that applies to this page.

## Target

Produce `docs/generated/design/ast-reference/declarations.md` — the
per-node reference for the `Decl` family declared in
[slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h).

## Family-specific guidance

- The family hierarchy diagram must show the abstract intermediate
  bases inside `Decl`: `DeclBase`, `Decl`, `ContainerDecl`,
  `AggTypeDeclBase`, `AggTypeDecl`, `StructDecl`-vs-`ClassDecl`,
  `VarDeclBase` (Variable / ParamDecl / GlobalGenericParamDecl / ...),
  `CallableDecl` (FuncDecl / ConstructorDecl / SubscriptDecl /
  AccessorDecl), `GenericDecl`. Cite the immediate parents from the
  header.
- The `## Nodes` table must cover every concrete `FIDDLE(...)`-declared
  class in `slang-ast-decl.h`. If a class is abstract (declared with
  `FIDDLE(abstract)`), exclude it from the table — it appears only in
  the family hierarchy diagram.
- Grammar column: link to [../syntax-reference/grammar.md](../syntax-reference/grammar.md)
  using the `#declarations` anchor by default. For declarations that
  correspond to a more specific section in the grammar doc (functions,
  generics, attributes, ...), link the closest applicable heading;
  otherwise use `#declarations`. For declarations that are never
  parsed (synthesized at type-checking time), write `(none)` and note
  in the Summary column why the declaration is synthesized.

## Notable nodes (the `## Notable nodes` callouts)

Cover at least the following nodes with a 2-5 sentence prose callout,
in this order:

- `GenericDecl` — the parser-level wrapping of a generic; explain that
  it owns parameter decls and an inner decl.
- `ExtensionDecl` — explain that it is parsed as a sibling of struct
  but attaches to a target type via lookup during checking.
- `AggTypeDecl` / `StructDecl` / `ClassDecl` distinction — what
  changes between them and why both exist.
- `InheritanceDecl` — its role in the conformance and inheritance
  graph.
- `SyntaxDecl` — its role in the syntax-as-declaration model; cite
  [../syntax-reference/keywords-and-builtins.md](../syntax-reference/keywords-and-builtins.md).
- `NamespaceDecl` and `ModuleDecl` — the relationship between the two
  and how the parser produces them.
- `EnumDecl` and `EnumCaseDecl` — how the parser pairs them.
- `AccessorDecl` family (`GetterDecl`, `SetterDecl`, `RefAccessorDecl`)
  — when the parser synthesizes them implicitly.
- `RequirementDecl` / `AssocTypeDecl` — interface requirements and
  associated types.

If a node from this list does not appear in the header at the current
commit, omit it rather than invent.

## Forbidden content (in addition to the universal forbidden list)

- IR-level declaration lowering (`IRFunc`, etc.) — that belongs in
  [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).
- Type-checking semantics — that belongs in
  [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every concrete class in `slang-ast-decl.h` appears in the table.
- [ ] Abstract classes appear in the hierarchy diagram but not in the
      table.
- [ ] Every Grammar link resolves to a heading in
      [../syntax-reference/grammar.md](../syntax-reference/grammar.md).
- [ ] `Notable nodes` callouts cite line ranges only when truly
      clarifying; otherwise just name the file.
