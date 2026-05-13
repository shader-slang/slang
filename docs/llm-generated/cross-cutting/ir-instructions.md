---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-07T14:35:56+00:00
source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
watched_paths_digest: 2573e0c7179a260716d90aeee42b77115f5e2f23215d0751dba306d6cdebf933
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# IR Instruction Catalog

This document is a categorized reference for the Slang IR instruction
set, derived from
[slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua). The
intended reader is a developer writing or modifying an IR pass who
needs to look up an opcode.

The deep design rationale (uniformity of "everything is an
instruction", SSA via block parameters, structured control-flow
encoding, hoistable / global value deduplication) is in
[../../design/ir.md](../../design/ir.md) and
[../../design/ir-instruction-definition.md](../../design/ir-instruction-definition.md).
This document does not duplicate it.

## Source

- [slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua)
  is the canonical declaration of the instruction set. Each entry
  carries a name, optional `struct_name`, optional `operands`, and
  flags such as `hoistable` and `parent`.
- [slang-ir-insts.h](../../../source/slang/slang-ir-insts.h) declares
  the C++ wrapper structs (`struct IRFoo : IRInst`) and inline
  accessors that match the Lua entries.
- The build-time tool `slang-fiddle` reads the Lua file and produces
  `slang-ir-insts-enum.h.fiddle` (under `build/source/slang/fiddle/`),
  which defines the `IROp` enum used everywhere in the compiler.
- [slang-ir.h](../../../source/slang/slang-ir.h) /
  [slang-ir.cpp](../../../source/slang/slang-ir.cpp) define
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
[../../design/ir.md](../../design/ir.md) for the semantics.
`parent = true` marks instructions that own children (functions,
blocks, modules).

## Instruction families

The Lua file groups instructions into a small number of top-level
families. Counts here are approximate at `source_commit`; consult
[slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua) for
the authoritative list.

### Special / boundary

| Opcode | Notes |
| --- | --- |
| `nop` | Placeholder; no semantic effect |
| `Unrecognized` | Placeholder used during deserialization for opcodes unknown to this version of the compiler (per the comment at the top of the Lua file). Must not survive past deserialization |

### Type instructions

The largest family. Every type in the language is an instruction.

#### Basic scalar types

`BasicType` group: `Void`, `Bool`, `Int8`, `Int16`, `Int`, `Int64`,
`UInt8`, `UInt16`, `UInt`, `UInt64`, `Half`, `Float`, `Double`,
`Char`, `IntPtr`, `UIntPtr`. Each has a struct name like
`VoidType`, `BoolType`, ..., `IntType`. All are `hoistable`.

#### Floating-point storage-only types

`PackedFloatType` group: `FloatE4M3Type`, `FloatE5M2Type`,
`BFloat16Type`. Hoistable, used as restricted (storage-only) FP
formats.

#### Strings

`StringTypeBase`: `String`, `NativeString` (both hoistable).

#### Composite and parametric types

| Opcode | C++ wrapper | Operands |
| --- | --- | --- |
| `Vec` | `VectorType` | element type, element count |
| `Mat` | `MatrixType` | element type, row count, column count, layout |
| `Array` | `ArrayType` | element type, element count, optional stride |
| `UnsizedArray` | `UnsizedArrayType` | element type, optional stride |
| `Func` | `FuncType` | result type, variadic parameter types |
| `BasicBlock` | `BasicBlockType` | (none) |
| `Tuple` | `TupleType` | (variadic) field types |
| `Optional` | `OptionalType` | value type |
| `Result` | `ResultType` | value type, error type |
| `Conditional` | `ConditionalType` | value type, has-value flag |
| `Enum` | `EnumType` | tag type (parent inst — owns enum-case children) |
| `Atomic` | `AtomicType` | element type |
| `Conjunction` | `ConjunctionType` | (intersection of constraints) |
| `Attributed` | `AttributedType` | base type, attribute |
| `RateQualified` | `RateQualifiedType` | rate, value type |

(Each row is hoistable unless noted.)

#### Pointer / address-space types

`PtrTypeBase` group: `Ptr` (`PtrType`), plus address-space-tagged
pointer variants for the various memory spaces. The full list (uniform,
storage, group-shared, etc.) lives in the Lua file under
`PtrTypeBase`.

#### Resource types

GPU-specific types: `Texture*`, `RWTexture*`, `SamplerState`,
`SamplerComparisonState`, `Buffer`, `RWBuffer`, `StructuredBuffer`,
`RWStructuredBuffer`, `ByteAddressBuffer`,
`RWByteAddressBuffer`, `RaytracingAccelerationStructure`, etc. All
are hoistable.

#### Differentiation types

`DifferentialPairTypeBase`: `DiffPair` (`DifferentialPairType`),
`DiffRefPair` (`DifferentialPtrPairType`). Plus
`TranslatedTypeBase` containing the various
`BackwardDiff*ContextType` opcodes used by the autodiff passes
(see [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)).

Function-type variants for differentiation:
`ForwardDiffFuncType`, `BackwardDiffFuncType`,
`ApplyForBwdFuncType`, `BwdCallableFuncType`, `RematFuncType`. All
hoistable.

#### Existentials

`BindExistentialsTypeBase`: `BindExistentials`,
`BoundInterface`. Used to encode dynamic-dispatch interfaces; see
[../../design/existential-types.md](../../design/existential-types.md)
for the language-level model.

#### Rates and kinds

`Rate` group: `ConstExpr`, `SpecConst`, `GroupShared`,
`ActualGlobalRate`. `Kind` group: `Type`, `TypeParameterPack`,
`Rate`, `Generic` ("types of types").

#### Capability sets and other specialized types

`CapabilitySet` / `CapabilitySetType` (hoistable),
`AnyValueType` (size-parameterized), `RawPointerType`,
`RTTIPointerType`, `DynamicType`, `TensorViewType`,
`TorchTensorType`, `ArrayListType`, `BackwardDiffMinimalContextType`,
plus a long tail of target / specialization-specific types. The full
list is in the Lua file.

### Value-producing instructions

#### Constant literals

| Opcode | C++ wrapper | Notes |
| --- | --- | --- |
| `IntLit` | `IRIntLit` | Integer literal; value stored inline |
| `FloatLit` | `IRFloatLit` | Floating-point literal |
| `BoolLit` | `IRBoolLit` | `true` / `false` |
| `StringLit` | `IRStringLit` | String constant |
| `PtrLit` | `IRPtrLit` | Pointer constant (e.g. `nullptr`) |
| `VoidLit` | | Placeholder void value |
| `MakeWitnessPack` / similar | | Pack / structural literal construction |

Literal storage is the only place where `IRInst` carries
"semantically relevant data not captured by the operand list" (see
[../../design/ir.md](../../design/ir.md)).

#### Arithmetic and logic

`Add`, `Sub`, `Mul`, `Div`, `Mod`, `Neg`,
`And`, `Or`, `Not`, `Xor`,
`Lsh`, `Rsh`, `BitAnd`, `BitOr`, `BitXor`, `BitNot`,
`Eql`, `Neq`, `Less`, `LessEqual`, `Greater`, `GreaterEqual`,
`Select` (ternary). All take two or three operands; results have the
appropriate scalar / vector / matrix type.

#### Conversions

`Cast`, `BitCast`, `IntCast`, `FloatCast`, `IntToFloat`,
`FloatToInt`, `Reinterpret`, plus a number of lowering-helper
`Pack` / `Unpack` opcodes used by `slang-ir-any-value-marshalling`.

#### Memory

| Opcode | Notes |
| --- | --- |
| `Var` | Allocates a local variable; type is `Ptr<T>` |
| `GlobalVar` | Module-scope variable |
| `Load` / `Store` | Through any pointer |
| `FieldAddress` / `FieldExtract` | Member access (lvalue / rvalue) |
| `GetElementPtr` / `GetElement` | Array indexing (lvalue / rvalue) |
| `Swizzle*` | Vector / matrix swizzles |

### Control-flow instructions

Slang IR uses block parameters instead of phi nodes (see
[../../design/ir.md](../../design/ir.md)).

| Opcode | Notes |
| --- | --- |
| `Block` (parent) | Basic block; first N instructions are `Param` |
| `Param` | Block parameter (the SSA-without-phi encoding) |
| `unconditionalBranch` | Branch to a target with arguments |
| `conditionalBranch` | Two-way conditional branch (no arguments — critical edges are forbidden) |
| `loop` | Loop entry; encodes the join (`break`) target |
| `ifElse` | Structured `if`-`else`; encodes the join target as an explicit operand |
| `Switch` | N-way switch; per-case targets |
| `Return` | Function return |
| `Unreachable` | No reachable continuation |
| `Discard` | HLSL-style fragment discard |
| `Throw` | Error-handling throw |

Structured-control-flow join points are explicit operands on the
relevant terminator, not metadata — described in
[../../design/ir.md](../../design/ir.md).

### Function and module structure

| Opcode | C++ wrapper | Notes |
| --- | --- | --- |
| `Func` (parent) | `IRFunc` | Function; children are blocks |
| `Generic` (parent) | `IRGeneric` | Function-shaped instruction whose body computes type-level values |
| `Module` (parent) | `IRModule` | Top-level container |
| `StructType` (parent) | `IRStructType` | Struct; children are `StructField` and `StructKey` |
| `StructField` | `IRStructField` | Struct member declaration |
| `StructKey` | `IRStructKey` | Identity for a struct member |
| `InterfaceType` (parent) | `IRInterfaceType` | Interface; children are requirement keys |
| `InterfaceRequirementEntry` | | Interface method slot |
| `RTTIObject` | | Runtime type information for an existential |
| `WitnessTable` (parent) | `IRWitnessTable` | Maps interface requirements to concrete impls |
| `WitnessTableEntry` | | Single witness mapping |

### Specialization and existentials

| Opcode | Notes |
| --- | --- |
| `Specialize` | Apply generic arguments to a generic |
| `LookupWitnessMethod` | Resolve an interface method through a witness table |
| `ExtractExistentialValue` | Pull the underlying value out of an existential |
| `ExtractExistentialType` | Pull the type out of an existential |
| `ExtractExistentialWitnessTable` | Pull the witness table out of an existential |
| `MakeExistential` | Pack a value, type, and witness into an existential |
| `BindExistentialsType` | Construct a `BindExistentials<...>` type |
| `BoundInterface` | Specialized form for known-interface bind |

These opcodes are produced by the lowering step and consumed by the
specialization passes documented in
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).

### Decorations

Decorations attach metadata to other instructions. They are listed as
their own family in the Lua file with names ending in `Decoration`.
Examples (the full list is sizeable):

- `NameHintDecoration` — preserves a human-readable name across passes
- `LayoutDecoration` — attaches layout information to a type or var
- `EntryPointDecoration` — marks a function as an entry point
- `BindingDecoration` — register / binding assignment
- `RequireCapabilityAtomDecoration` — capability requirement
- `TargetIntrinsicDecoration` — declares the per-target spelling of
  an intrinsic
- `KeepAliveDecoration` — prevents DCE for module-public symbols
- `LoopUnrollDecoration` / `LoopUnrollMaxIterationsDecoration` —
  loop-unroll attributes

Some decorations are conceptually instructions today; the long-term
plan recorded in [../../design/ir.md](../../design/ir.md) is to
unify them more cleanly.

### Resource and shader-IO opcodes

Operations on resource handles: `ImageLoad`, `ImageStore`,
`ImageSubscript`, `SampleImplicit`, `SampleExplicit`,
`StructuredBufferLoad`, `StructuredBufferStore`,
`ByteAddressBufferLoad`, `ByteAddressBufferStore`,
`AppendStructuredBuffer`, `ConsumeStructuredBuffer`. Shader-IO
opcodes for entry-point parameters: `EntryPointParam`,
`GlobalParam`, `Vertex*` / `Fragment*` outputs, etc. The full list
is in the Lua file.

### Atomics and synchronization

`AtomicLoad`, `AtomicStore`, `AtomicAdd`, `AtomicMin`, `AtomicMax`,
`AtomicCAS`, plus barrier opcodes (`MemoryBarrier`, `GroupBarrier`,
...). Memory ordering is encoded with the
`IRMemoryOrder` enum from
[slang-ir.h](../../../source/slang/slang-ir.h):

```cpp
enum IRMemoryOrder
{
    kIRMemoryOrder_Relaxed = 0,
    kIRMemoryOrder_Acquire = 1,
    kIRMemoryOrder_Release = 2,
    kIRMemoryOrder_AcquireRelease = 3,
    kIRMemoryOrder_SeqCst = 4,
};
```

### Differentiation opcodes

The autodiff machinery
([../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md))
introduces dedicated opcodes used between the various stages of
forward / reverse / unzipping: `ForwardDifferentiate`,
`BackwardDifferentiate`, `MakeDifferentialPair`,
`DifferentialPairGetPrimal`, `DifferentialPairGetDifferential`, and
the `BackwardDiff*Context` family. The deeper user-level
documentation is in
[../../design/autodiff.md](../../design/autodiff.md).

## Hoistable / global / deduplicated values

The flag bits in
[slang-ir.h](../../../source/slang/slang-ir.h):

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
covered in [../../design/ir.md](../../design/ir.md). Pass authors
**must** read that document before writing transformations that mutate
the IR.

## Module versioning and opcode insertion

The comment at the top of
[slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua):

> Please make sure to update the supported module versions in
> Slang::IRModule accordingly when modifying this file.

Inserting a new opcode renumbers downstream entries, which breaks
deserialization of older `.slang-module` files unless the supported-
version range is bumped. The serialization rules are in
[../cross-cutting/serialization.md](../cross-cutting/serialization.md)
and
[../../design/backwards-compat-for-ir-modules.md](../../design/backwards-compat-for-ir-modules.md).

## Adding a new opcode

1. Add an entry to
   [slang-ir-insts.lua](../../../source/slang/slang-ir-insts.lua) at
   the appropriate point in its family (see the rule above about
   not inserting into the middle of an existing range without
   bumping the module version).
2. If the opcode wants a typed C++ wrapper, declare `struct IRFoo :
   IRInst` with operand accessors in
   [slang-ir-insts.h](../../../source/slang/slang-ir-insts.h) — the
   FIDDLE generator can supply boilerplate for many cases.
3. Decide whether it should be `hoistable` and/or `parent`. Choose
   the right base in the Lua hierarchy so the opcode lands in a
   contiguous range with its siblings.
4. Add an emitter to `IRBuilder` in
   [slang-ir.cpp](../../../source/slang/slang-ir.cpp) for ergonomic
   creation, especially if the opcode is hoistable (the builder is
   responsible for deduplication).
5. Update lowering ([../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md))
   and any IR pass that needs to see or skip the new opcode.
6. If the opcode produces target-specific behaviour, extend the
   relevant emit backend
   ([../pipeline/06-emit.md](../pipeline/06-emit.md)).
7. Add tests under [tests/](../../../tests/).
