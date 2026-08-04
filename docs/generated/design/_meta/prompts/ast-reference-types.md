# Prompt: ast-reference/types.md

See [_common.md](_common.md) for the universal rules and the **AST
reference family contract**.

## Target

Produce `docs/generated/design/ast-reference/types.md` — the per-node
reference for the `Type` family declared in
[slang-ast-type.h](../../../../source/slang/slang-ast-type.h).

`Type` is internally a subhierarchy of `Val` (see
[slang-ast-base.h](../../../../source/slang/slang-ast-base.h) and
[slang-ast-val.h](../../../../source/slang/slang-ast-val.h)). The
`## Family hierarchy` section must make this relationship explicit:
`NodeBase -> Val -> Type -> <concrete types>`.

## Family-specific guidance

- The hierarchy diagram should group concrete types into useful
  buckets: `BasicExpressionType` (scalar / integer / float / bool /
  void / ...), arithmetic vector and matrix types, array and pointer
  types, resource and texture types, struct / class / enum type
  references (`DeclRefType`), function types, generic-parameter types
  (`GenericParamType`), associated-type and existential types
  (`ThisType`, `ExtractExistentialType`, `AndType`), `ErrorType`,
  pseudo-types used during checking.
- The `## Nodes` table must cover every concrete `Type` class in
  `slang-ast-type.h`. Where `Type` derives from a `Val` intermediate,
  show that intermediate as the `Parent` column value.
- Grammar column: types parsed directly (basic scalar/vector/matrix,
  pointer/array suffix, `DeclRefType` constructions) link to
  [../syntax-reference/grammar.md#types](../syntax-reference/grammar.md#types).
  Types that arise only from checking (`ErrorType`,
  `ExtractExistentialType`, `ThisType`) use `(none)` with a Summary
  note.

## Notable nodes

Cover at least the following:

- `BasicExpressionType` — what it represents and its relationship to
  scalar type tags.
- `VectorExpressionType` and `MatrixExpressionType` — their element
  type and shape parameters.
- `DeclRefType` — by far the most common type; explain that it wraps
  a decl-ref to a type declaration, including generic substitutions;
  see [../glossary.md](../glossary.md) for `decl-ref`.
- `ErrorType` — its role as the type of expressions that failed to
  check, and why having an explicit error type lets checking continue.
- `ThisType` — the "self" type of an interface or extension.
- `ExtractExistentialType` — its relationship to existential types in
  [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).
- `FuncType` — how function types compose parameter types and return
  type, including effects/qualifiers.
- `AndType` — its role in conjunction conformance.
- Resource/texture/sampler types — group note describing the family
  rather than each leaf, citing the relevant intrinsic definitions
  in the core module.

## Forbidden content

- IR types (`IRType` and friends) — see
  [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).
- Type-checking algorithms — see
  [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every concrete class in `slang-ast-type.h` appears in the table.
- [ ] Family hierarchy diagram includes the `Val -> Type` edge.
- [ ] Grammar links resolve.
