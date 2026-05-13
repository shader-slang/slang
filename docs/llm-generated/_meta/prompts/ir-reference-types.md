# Prompt: ir-reference/types.md

See [_common.md](_common.md) for the universal rules and the
**IR-reference family contract**.

## Target

Produce `docs/llm-generated/ir-reference/types.md` — the per-opcode
reference for the `Type` family in
[../../../source/slang/slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua)
(the top-level `Type` entry starts around line 19 and runs to the
`}` that closes its inner list).

## Family-specific guidance

- This is by far the largest single family in the Lua file. Split
  `## Opcodes` into sub-tables, one per Lua intermediate group:
  - **Basic scalar types** (`BasicType`: `Void`, `Bool`, `Int8` ... `UIntPtr`).
  - **Storage-only floating-point** (`PackedFloatType`).
  - **Strings** (`StringTypeBase`).
  - **Raw and dynamic pointers** (`RawPointerTypeBase`, `DynamicType`,
    `AnyValueType`, `CapabilitySet`).
  - **Array and array-like** (`ArrayTypeBase`).
  - **Composite and parametric** (`Vec`, `Mat`, `Tuple`, `Optional`,
    `Result`, `Conditional`, `Enum`, `Conjunction`, `Attributed`,
    `RateQualified`, `Atomic`, `BasicBlock`, `Func`, ...).
  - **Pointer / address-space** (`PtrTypeBase` group).
  - **Resource types** (texture, sampler, buffer, structured-buffer,
    byte-address-buffer, acceleration-structure variants).
  - **Differentiation types** (`DifferentialPairTypeBase`,
    `TranslatedTypeBase`, the `*FuncType` family).
  - **Existential / interface types** (`BindExistentialsTypeBase`,
    `InterfaceType`).
  - **Rates and kinds** (`Rate`, `Kind`).
- The `Operands` column is the most important per-row data for type
  opcodes (e.g. `Vec` -> `elementType, count`). Use the Lua
  `operands` field; for entries without an explicit `operands`, write
  `(see Lua)` and consider whether the entry deserves a `## Notable
  opcodes` callout.
- The `AST origin` column for type opcodes typically points to the
  ASTNodes in [slang-ast-type.h](../../../source/slang/slang-ast-type.h)
  that lower into them. Trace through
  [slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp)
  `lowerType*` family of helpers. For built-in scalars, the origin
  is the corresponding `BasicExpressionType` tag. When in doubt,
  write `—`.
- The `Family hierarchy` diagram should show
  `IRInst -> Type` plus the immediate Lua subgroups listed above as
  children of `Type`.

## Notable opcodes

Cover at least the following in `## Notable opcodes`:

- `Vec` and `Mat` — their operand layout (element-type, element-count,
  row/col/layout).
- `Func` — variadic operand encoding (result type then parameter
  types).
- `Array` vs `UnsizedArray` — relation to fixed- vs unknown-extent
  arrays in the source language.
- `Enum` (parent) — how children encode the cases.
- `Ptr` and the address-space variants — what determines which
  variant is emitted.
- `AnyValueType` — its `size` operand and role in existential
  marshalling.
- `BindExistentialsType` / `BoundInterface` — link to the existentials
  page [generics-and-existentials.md](generics-and-existentials.md).
- `RateQualified` — its role in encoding compile-time vs runtime
  values; cite [../../design/ir.md](../../design/ir.md).

## Forbidden content

- AST-level type classes (`Type`, `BasicExpressionType`, `DeclRefType`,
  ...) — see [../ast-reference/types.md](../ast-reference/types.md).
- Type-system algorithms — see
  [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every Lua `Type`-family entry that names a concrete opcode
      appears in one of the sub-tables.
- [ ] Hierarchy diagram shows the `IRInst -> Type` edge and every
      immediate Lua subgroup.
- [ ] At least one row in each sub-table cites a `slang-lower-to-ir.cpp`
      visitor or `IRBuilder::getXType` helper that produces the opcode.
