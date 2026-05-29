# Prompt: ast-reference/values.md

See [_common.md](_common.md) for the universal rules and the **AST
reference family contract**.

## Target

Produce `docs/generated/design/ast-reference/values.md` — the per-node
reference for the **non-Type** `Val` subclasses declared in
[slang-ast-val.h](../../../../source/slang/slang-ast-val.h).

`Type` is also a `Val`, but it has its own page at
[types.md](types.md); do not duplicate type nodes here. The `## Source`
paragraph must explain this split and cross-link `types.md`.

## Family-specific guidance

- The hierarchy diagram should show `Val` at the root and group its
  non-type descendants: `IntVal` family (concrete `ConstantIntVal`,
  `GenericParamIntVal`, `WitnessLookupIntVal`, `PolynomialIntVal`,
  `FuncCallIntVal`, ...), `Witness` family (`SubtypeWitness`,
  `TransitiveSubtypeWitness`, `ConjunctionSubtypeWitness`,
  `TypeEqualityWitness`, ...), and miscellaneous `Val`s such as
  `Substitutions` and `DifferentiateVal` (only if present).
- The `## Nodes` table must cover every concrete non-Type class in
  `slang-ast-val.h`. Add a Note column to the standard table or fold
  the discussion into the Summary column, whichever keeps each row
  to a single line.
- Grammar column: most Vals are not surface-parsable. Default to
  `(none)`; use a grammar link only for Vals that have a textual
  syntax (rare).

## Notable nodes

Cover at least the following:

- The `IntVal` family — what compile-time integer values represent and
  why they are first-class `Val`s (used in array sizes, generic
  arguments, etc.).
- `GenericParamIntVal` — how it represents an unsubstituted integer
  generic parameter.
- `Witness` and its role: link to
  [../glossary.md](../glossary.md) entries `witness table` and
  `existential type`.
- `SubtypeWitness` and `TransitiveSubtypeWitness` — how conformance
  evidence flows through inheritance chains.
- `TypeEqualityWitness` — when the checker constructs one.
- Hash-consing of `Val`s — explain that `Val`s are deduplicated by
  the `ASTBuilder`, cross-link to the entry in
  [../glossary.md](../glossary.md).

## Forbidden content

- The `Type` subhierarchy — see [types.md](types.md).
- IR-level witness tables — see
  [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every non-Type concrete class in `slang-ast-val.h` appears in
      the table.
- [ ] No `Type` subclass appears in this page.
- [ ] The relationship `Type : Val` is mentioned in `## Source` with
      a link to [types.md](types.md).
