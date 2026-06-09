---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-05T09:24:37Z
source_commit: 52339028a2aa703271533454c6b9528a534bac31
watched_paths_digest: 2e16e4101249030a9cd977caed2ab437622083f3808b536a0a0e81f0c47cf487
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# IR Instruction Catalog

This document is a categorized reference for the Slang IR instruction
set, derived from
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua). The
intended reader is a developer writing or modifying an IR pass who
needs to look up an opcode.

The deep design rationale (uniformity of "everything is an
instruction", SSA via block parameters, structured control-flow
encoding, hoistable / global value deduplication) is in
[../../../design/ir.md](../../../design/ir.md) and
[../../../design/ir-instruction-definition.md](../../../design/ir-instruction-definition.md).
This document does not duplicate it.

## Source

- [slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua)
  is the canonical declaration of the instruction set. Each entry
  carries a name, optional `struct_name`, optional `operands`, and
  flags such as `hoistable` and `parent`.
- [slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) declares
  the C++ wrapper structs (`struct IRFoo : IRInst`) and inline
  accessors that match the Lua entries.
- The build-time tool `slang-fiddle` reads the Lua file and produces
  `slang-ir-insts-enum.h.fiddle` (under `build/source/slang/fiddle/`),
  which defines the `IROp` enum used everywhere in the compiler.
- [slang-ir.h](../../../../source/slang/slang-ir.h) /
  [slang-ir.cpp](../../../../source/slang/slang-ir.cpp) define
  `IRInst`, `IRBuilder`, traversal helpers, and the hoistable / global-
  value deduplication infrastructure.

## Schema

A Lua entry has the form:

```lua
{
    Foo = {
        struct_name = "FooType",       -- optional; C++ wrapper name
        hoistable = true,              -- optional; deduplicated, hoisted to the enclosing scope
        operands = {
            { "name", "IRType" },      -- typed operand (IRType / IRInst / etc.)
            { "other", optional = true },
            { "rest", "IRInst", variadic = true },
        },
    },
},
```

Entries are arranged into nested tables that establish an
inheritance hierarchy: a parent entry such as `BasicType` holds
children (`Void`, `Bool`, `Int`, ...), and their opcodes are
allocated as a contiguous range so that `as<IRBasicType>()` becomes a
single integer comparison.

`hoistable = true` marks instructions that the IR builder
deduplicates and hoists to the outermost scope where their operands
are available — see
[../../../design/ir.md](../../../design/ir.md) for the semantics.
`parent = true` marks instructions that own children (functions,
blocks, modules).

## Instruction families

The per-opcode catalog lives in the
[../ir-reference/](../ir-reference) subtree. Each family page
tabulates every opcode in that family with its C++ wrapper, operand
shape, op-flags, AST origin, and a one-line summary; notable opcodes
have short callouts. Start at
[../ir-reference/index.md](../ir-reference/index.md) for the family
taxonomy and approximate per-family opcode counts. The family pages
are:

- [../ir-reference/types.md](../ir-reference/types.md) — `Type`
  family (basic scalar, packed FP, strings, composite/parametric,
  pointer/address-space, resource, differentiation, existential,
  rate/kind).
- [../ir-reference/values.md](../ir-reference/values.md) — constant
  literals, arithmetic/logic/comparison/bit ops, conversions,
  memory, aggregate constructors.
- [../ir-reference/control-flow.md](../ir-reference/control-flow.md)
  — `block`, `Param`, and the `TerminatorInst` family.
- [../ir-reference/structure.md](../ir-reference/structure.md) —
  module/function/generic/struct/interface/witness-table opcodes.
- [../ir-reference/generics-and-existentials.md](../ir-reference/generics-and-existentials.md)
  — `specialize`, `lookupWitness`, `MakeExistential` /
  `ExtractExistential*`, RTTI.
- [../ir-reference/resources-and-atomics.md](../ir-reference/resources-and-atomics.md)
  — image / buffer / sampler ops, shader IO, `AtomicOperation`
  family, barriers, wave intrinsics, raytracing.
- [../ir-reference/differentiation.md](../ir-reference/differentiation.md)
  — differential pairs, `ForwardDifferentiate`,
  `BackwardDifferentiate`, reverse-mode contexts.
- [../ir-reference/decorations.md](../ir-reference/decorations.md)
  — the `Decoration` family.
- [../ir-reference/metadata.md](../ir-reference/metadata.md) —
  `Layout`, `Attr`, `Debug*`, `SPIRVAsmOperand` families.
- [../ir-reference/misc.md](../ir-reference/misc.md) — pack /
  expansion helpers, type queries, size / alignment / count,
  liveness markers, descriptor heaps, kernel launch.

Each per-family summary table below is **representative, not
exhaustive**: it shows a handful of opcodes in each family so that a
reader can confirm they are on the right page before clicking
through. The full list of opcodes in each family lives in the
corresponding `ir-reference/*.md` page; the canonical opcode
declarations live in
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua).

### Type instructions

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `Int`, `Float`, `Bool`, ... | `IntType`, `FloatType`, `BoolType`, ... | — | Basic scalar types; see [../ir-reference/types.md](../ir-reference/types.md). |
| `Vec` | `VectorType` | `elementType, elementCount` | Vector types; hoistable. |
| `Mat` | `MatrixType` | `elementType, rowCount, columnCount, layout` | Matrix types; hoistable. |
| `Array` | `ArrayType` | `elementType, elementCount` | Fixed-size array; hoistable. |
| `Ptr` | `PtrType` | `valueType, accessQualifier?, addressSpace?, dataLayout?` | Pointer type; hoistable. |
| `Texture` | — | `elementType, shape, isArray, isMS, sampleCount, access, isShadow, isCombined, format` | Texture types; hoistable. |
| `struct` / `class` / `interface` | `StructType` / `ClassType` / `InterfaceType` | parent of `field` / `key` / `interface_req_entry` | Parent containers; also documented in [../ir-reference/structure.md](../ir-reference/structure.md). |
| (...plus ~150 more type opcodes; see [../ir-reference/types.md](../ir-reference/types.md) for the full list) | | | |

### Value instructions

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `IntLit`, `FloatLit`, `StringLit`, ... | `IntLit`, `FloatLit`, `StringLit`, ... | (payload stored inline on the inst) | Literal constants; hoistable. |
| `add`, `sub`, `mul`, `div` | `Add`, `Sub`, `Mul`, `Div` | `left, right` | Arithmetic. |
| `cmpEQ`, `cmpLT`, ... | `Eql`, `Less`, ... | `left, right` | Comparisons. |
| `bitCast`, `intCast`, `floatCast`, ... | — | `val` | Conversion ops. |
| `constexprAdd` ... `constexprEnumCast` | — | (variadic) | Compile-time-folded arithmetic / cast variants; hoistable. |
| (...see [../ir-reference/values.md](../ir-reference/values.md) for the full list) | | | |

### Memory instructions

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `var` | `IRVar` | — | Local variable allocation; result is `Ptr<T>`. |
| `alloca` | — | `allocSize` | Dynamically-sized stack allocation. |
| `load` / `store` | — | `ptr` / `ptr, val` | Pointer load and store. |
| `get_field` / `get_field_addr` | `FieldExtract` / `FieldAddress` | `base, key` | Struct member access (rvalue / lvalue). |
| `getElement` / `getElementPtr` | — | `base, index` | Indexed access. |
| (...see [../ir-reference/values.md](../ir-reference/values.md) ("Memory") for the full list) | | | |

### Control-flow instructions

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `block` | `IRBlock` | parent of `Param`s and instructions | Basic block; first N children are `Param`s. |
| `param` | `IRParam` | (variadic) | Block or function parameter; replaces SSA `phi`. |
| `unconditionalBranch` / `conditionalBranch` / `ifElse` / `switch` / `loop` | — | (terminator-specific) | Terminators in the `TerminatorInst` family. |
| `return_val` / `unreachable` / `discard` | — | (terminator-specific) | Return and exit terminators. |
| `RequirePrelude`, `RequireTargetExtension`, `Printf`, `StaticAssert`, ... | — | (variadic) | Other control-flow / backend-hint opcodes. |
| (...see [../ir-reference/control-flow.md](../ir-reference/control-flow.md) for the full list) | | | |

### Function and module structure

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `module` | `ModuleInst` | (variadic) | Module root; parent of every top-level inst. |
| `func` | `IRFunc` | (variadic) | Function; children are blocks. |
| `generic` | `IRGeneric` | (variadic) | Type-level computation parent ending in `yield`. |
| `global_var`, `global_param`, `globalConstant` | `IRGlobalVar`, ... | (variadic) | Module-scope storage / parameters; `Global`. |
| `witness_table` / `witness_table_entry` | — | (variadic) / `requirementKey, satisfyingVal` | Witness table machinery; hoistable. |
| (...see [../ir-reference/structure.md](../ir-reference/structure.md) for the full list) | | | |

### Specialization and existentials

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `specialize` | — | `base, args...` | Applies generic arguments; hoistable. |
| `lookupWitness` | `LookupWitnessMethod` | `witnessTable, requirementKey` | Resolves an interface requirement; hoistable. |
| `makeExistential` | `MakeExistential` | `value, witness` | Packs a value plus its witness. |
| `extractExistentialValue` / `extractExistentialType` / `extractExistentialWitnessTable` | — | `existential` | Existential projections. |
| `TypeSet`, `FuncSet`, `WitnessTableSet`, `GenericSet`, `GetDispatcher`, ... | — | (variadic) | Type-flow specialization. |
| (...see [../ir-reference/generics-and-existentials.md](../ir-reference/generics-and-existentials.md) for the full list) | | | |

### Decorations

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `NameHintDecoration` | `IRNameHintDecoration` | `name` | Carries an identifier name for debug / link output. |
| `KeepAliveDecoration` | `IRKeepAliveDecoration` | — | Forbids DCE on the host instruction. |
| `TargetIntrinsicDecoration` | `IRTargetIntrinsicDecoration` | `targetTokens, definition` | Maps an IR op to a target intrinsic. |
| `EntryPointDecoration` | `IREntryPointDecoration` | `profile, name, moduleName` | Marks a function as a pipeline entry point. |
| (...see [../ir-reference/decorations.md](../ir-reference/decorations.md) for the full list of ~180 decorations) | | | |

### Resource and shader-IO opcodes

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `imageLoad` / `imageStore` | — | `image, coord, ...` | Image read / write. |
| `structuredBufferLoad` / `rwstructuredBufferStore` | — | `base, index, val?` | Structured-buffer access. |
| `atomicLoad` / `atomicStore` / `atomicAdd` / ... | — | `ptr, val?` | `AtomicOperation` family. |
| `ControlBarrier` / `GroupMemoryBarrierWithGroupSync` / `BeginFragmentShaderInterlock` / `EndFragmentShaderInterlock` | — | — | Barriers and synchronization. |
| `waveGetActiveMask` / `waveMaskBallot` / ... | — | (variadic) | Wave intrinsics. |
| (...see [../ir-reference/resources-and-atomics.md](../ir-reference/resources-and-atomics.md) for the full list) | | | |

This document keeps the *conventions* — schema, op-flag bits,
hoistable / global deduplication, module versioning, the workflow
for adding a new opcode — that every family page assumes you have
read.

## Hoistable / global / deduplicated values

The flag bits in
[slang-ir.h](../../../../source/slang/slang-ir.h):

```cpp
enum : IROpFlags
{
    kIROpFlags_None = 0,
    kIROpFlag_Parent = 1 << 0,
    kIROpFlag_UseOther = 1 << 1,
    kIROpFlag_Hoistable = 1 << 2,
    kIROpFlag_Global = 1 << 3,
};
```

- `Parent` — instruction owns a list of children (e.g. `Func`,
  `Block`, `Module`, `StructType`).
- `Hoistable` — deduplicated; floats up to the outermost scope where
  its operands are defined.
- `Global` — like `Hoistable` but always at the module level.
- `UseOther` — the opcode encoding stores extra information in the
  high bits of the opcode word (`IROpMeta::kIROpMeta_OtherShift = 10`).

The semantics of these flags and the consequences for IR transformation
(use of `replaceOperand`, `replaceUsesWith`, traversal safety) are
covered in [../../../design/ir.md](../../../design/ir.md). Pass authors
**must** read that document before writing transformations that mutate
the IR.

## Decorations

A number of opcodes are conceptually *decorations*: every entry in the
`Decoration` family in
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) (the
`*Decoration` opcodes such as `NameHintDecoration` and
`TargetIntrinsicDecoration`) is modeled as an ordinary `IRInst`,
wrapped by `IRDecoration` in
[slang-ir.h](../../../../source/slang/slang-ir.h). A decoration does
not sit in a block's instruction stream; it is attached to a host
instruction's decoration list and reached via
`IRInst::getFirstDecoration`, annotating the host with metadata
(names, linkage, layout, target-intrinsic spellings) without
producing a value. The full per-opcode catalog is in
[../ir-reference/decorations.md](../ir-reference/decorations.md).

## Module versioning and opcode insertion

The comment at the top of
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua):

> Please make sure to update the supported module versions in
> Slang::IRModule accordingly when modifying this file.

Inserting a new opcode renumbers downstream entries, which breaks
deserialization of older `.slang-module` files unless the supported-
version range is bumped. The serialization rules are in
[../cross-cutting/serialization.md](serialization.md)
and
[../../../design/backwards-compat-for-ir-modules.md](../../../design/backwards-compat-for-ir-modules.md).

## Adding a new opcode

1. Add an entry to
   [slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) at
   the appropriate point in its family (see the rule above about
   not inserting into the middle of an existing range without
   bumping the module version).
2. If the opcode wants a typed C++ wrapper, declare `struct IRFoo :
   IRInst` with operand accessors in
   [slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) — the
   FIDDLE generator can supply boilerplate for many cases.
3. Decide whether it should be `hoistable` and/or `parent`. Choose
   the right base in the Lua hierarchy so the opcode lands in a
   contiguous range with its siblings.
4. Add an emitter to `IRBuilder` in
   [slang-ir.cpp](../../../../source/slang/slang-ir.cpp) for ergonomic
   creation, especially if the opcode is hoistable (the builder is
   responsible for deduplication).
5. Update lowering ([../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md))
   and any IR pass that needs to see or skip the new opcode.
6. If the opcode produces target-specific behaviour, extend the
   relevant emit backend
   ([../pipeline/06-emit.md](../pipeline/06-emit.md)).
7. Add tests under [tests/](../../../../tests).
