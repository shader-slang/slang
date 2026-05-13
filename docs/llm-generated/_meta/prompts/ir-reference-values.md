# Prompt: ir-reference/values.md

See [_common.md](_common.md) for the universal rules and the
**IR-reference family contract**.

## Target

Produce `docs/llm-generated/ir-reference/values.md` — the per-opcode
reference for value-producing opcodes that are not types, not
control-flow, not structural, not GPU resource ops, and not
autodiff-specific. In practice this covers the `Constant` family
(literals), arithmetic / logic / comparison / bit / shift, conversions
(`Cast`, `BitCast`, `IntCast`, `FloatCast`, `IntToFloat`,
`FloatToInt`, `Reinterpret`, `Pack*` / `Unpack*`), memory ops (`Var`,
`GlobalVar`, `Load`, `Store`, `FieldAddress`, `FieldExtract`,
`GetElementPtr`, `GetElement`, `Swizzle*`), and aggregate constructors
(`makeVector`, `makeMatrix`, `makeArray`, `makeStruct`, `makeTuple`,
`makeCoopVector`, ...).

The relevant Lua ranges start with the `Constant` group around line
838 and continue with the `make*` cluster.

## Family-specific guidance

- Split `## Opcodes` into the following sub-sections (each its own
  table):
  - **Literals** (`boolConst`, `integer_constant`, `float_constant`,
    `ptr_constant`, `void_constant`, `string_constant`,
    `blob_constant`).
  - **Undefined and default** (`Undefined`, `LoadFromUninitializedMemory`,
    `Poison`, `defaultConstruct`).
  - **Arithmetic and logic** (`Add`, `Sub`, `Mul`, `Div`, `Mod`,
    `Neg`, `And`, `Or`, `Not`, `Xor`, `Lsh`, `Rsh`, `BitAnd`,
    `BitOr`, `BitXor`, `BitNot`, `Select`, `frem`, `logicalAnd`,
    `logicalOr`).
  - **Comparison** (`Eql`, `Neq`, `Less`, `LessEqual`, `Greater`,
    `GreaterEqual`, `cmpLE`, `cmpNE`, ...).
  - **Conversions** (`Cast`, `BitCast`, `IntCast`, `FloatCast`,
    `IntToFloat`, `FloatToInt`, `Reinterpret`, `Pack*`, `Unpack*`).
  - **Memory** (`Var`, `GlobalVar`, `globalConstant`, `Load`, `Store`,
    `FieldAddress`, `FieldExtract`, `GetElementPtr`, `GetElement`,
    `Swizzle`, `SwizzleSet`, `SwizzledStore`).
  - **Aggregate constructors** (`makeVector`, `makeMatrix`,
    `makeMatrixFromScalar`, `makeArray`, `makeArrayFromElement`,
    `makeStruct`, `makeTuple`, `makeCoopVector`,
    `makeCoopMatrixFromScalar`, `makeCombinedTextureSampler`,
    `makeUInt64`, ...).
  - **Reshape and pack helpers** (`matrixReshape`, `vectorReshape`,
    `getTupleElement`, `getTargetTupleElement`).
- The `AST origin` column should cite the `slang-lower-to-ir.cpp`
  visitor that produces the opcode. The arithmetic / comparison /
  bit-ops are usually emitted by `tryLowerIntrinsic` or the
  `visit*Expr` family. Memory ops typically come from
  `visitVarDecl`, `visitMemberExpr`, `visitIndexExpr`, etc.
- `defaultConstruct`, `Poison`, and `LoadFromUninitializedMemory`
  are typically `(synthesized)` (introduced by IR passes).

## Notable opcodes

Cover at least:

- `Var` — its `Ptr<T>` result type and relationship to `Load` / `Store`.
- `FieldAddress` vs `FieldExtract` — lvalue vs rvalue paths.
- `GetElementPtr` vs `GetElement` — same distinction for arrays.
- `Swizzle` family — operand encoding (vector plus index list).
- `defaultConstruct` — semantics and where IR passes introduce it.
- `Poison` and `LoadFromUninitializedMemory` — link to LLVM-ish
  semantics noted in
  [../../design/ir.md](../../design/ir.md).
- `Select` — three-operand ternary; result type matches operand types.

## Forbidden content

- IR-pass behaviour (DCE, SSA reconstruction, ...) — see
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).
- Resource-typed loads / stores (`imageLoad`, `structuredBufferLoad`,
  ...) — see
  [resources-and-atomics.md](resources-and-atomics.md).

## Quality checklist (in addition to the universal one and the family contract)

- [ ] Every concrete opcode in the listed Lua ranges is in one of the
      sub-tables.
- [ ] Each sub-table has at least one `slang-lower-to-ir.cpp` citation
      in either a row or the family-level prose.
- [ ] The literal opcodes' `Notes` mention that their "extra payload"
      (integer/float value, string bytes) is stored on the `IRInst`
      itself, not as operands.
