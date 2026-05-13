# Prompt: ir-reference/generics-and-existentials.md

See [_common.md](_common.md) for the universal rules and the
**IR-reference family contract**.

## Target

Produce `docs/llm-generated/ir-reference/generics-and-existentials.md`
— the per-opcode reference for the opcodes that bind generic
parameters, look up interface requirements through witness tables,
construct and destructure existentials, and carry runtime type
information.

Relevant Lua entries are scattered: `specialize`, `lookupWitness` /
`LookupWitnessMethod` (around line 932-933), `MakeExistential`,
`ExtractExistentialValue` / `Type` / `WitnessTable` (around lines
2477-...), `BindExistentialsType` / `BoundInterface` (handled as
*types* in [types.md](types.md) but cross-linked here), `RTTIObject`
/ `getSequentialID`, `bind_global_generic_param`, `thisTypeWitness`,
`TypeEqualityWitness`.

## Family-specific guidance

- Split `## Opcodes` into:
  - **Generic application** (`specialize`,
    `bind_global_generic_param`, `globalValueRef`).
  - **Witness lookup** (`lookupWitness` / `LookupWitnessMethod`).
  - **Existential construction** (`MakeExistential`,
    `MakeExistentialWithRTTI`, packing helpers).
  - **Existential destructuring** (`ExtractExistentialValue`,
    `ExtractExistentialType`, `ExtractExistentialWitnessTable`,
    `extractExistentialWitnessTable`).
  - **Runtime type info** (`RTTIObject`, `getSequentialID`,
    `GetRTTIHandleValue`, `RTTIType`).
  - **Witness facts** (`thisTypeWitness`, `TypeEqualityWitness`).
- The `AST origin` column for these opcodes typically cites
  expressions and decls in
  [slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp):
  `lowerGenericApp`, `lowerWitnessLookup`,
  `lowerExistentialMake*` / `lowerExistentialExtract*`,
  `lowerCastToInterface`. Several of these opcodes are
  `(synthesized)` — produced only by IR passes.
- Cross-link to [types.md](types.md) for `BindExistentialsType`,
  `BoundInterface`, `AnyValueType`, and the dynamic-dispatch type
  family.

## Notable opcodes

Cover at least:

- `specialize` — operands (`base`, `arg...`), hoistable so that two
  identical specializations dedup to one IR value.
- `lookupWitness` / `LookupWitnessMethod` — operands
  (`witnessTable`, `requirementKey`), how the pair encodes an
  interface dispatch.
- `MakeExistential` — operand triple (value, type, witness) and
  how it lowers to a `BindExistentialsType`-typed value.
- `ExtractExistentialValue` / `ExtractExistentialType` /
  `ExtractExistentialWitnessTable` — the three projections that
  reverse `MakeExistential`.
- `RTTIObject` — its role as a runtime-side artefact for existential
  dispatch; cite that it is materialized by the
  `slang-ir-rtti-object-impl.cpp` pass.
- `TypeEqualityWitness` — its operand pair and how it certifies
  type equality across generic argument substitution.

## Forbidden content

- Specialization pass details (the `slang-ir-specialize.cpp` pass) —
  see [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).
- AST-level decl-refs and generic substitutions — see
  [../ast-reference/values.md](../ast-reference/values.md) and
  [../glossary.md](../glossary.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every `Extract*` / `Make*Existential` / `RTTI*` opcode is in
      the tables.
- [ ] Hierarchy diagram groups the four categories above.
- [ ] At least one row cites
      [slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp).
