---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T14:32:00Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 32a6f83c708fb280660629cff147cc6b41bd0816fc7f889340630eb73cb6b9f1
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
- [slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) holds
  the hand-written C++ wrapper structs (`struct IRFoo : IRInst`) and
  their inline accessors — but only for opcodes that need members the
  generator cannot derive. A FIDDLE template near the end of the file
  calls `getAllOtherInstStructsData()` and emits a wrapper for every
  opcode *not* explicitly defined there, complete with `isaImpl`, a
  `kOp` constant, and one accessor per named Lua operand. So
  `{ getNaturalStride = { operands = { { "type" } } } }` yields a
  `struct IRGetNaturalStride : IRInst` with a `getType()` accessor
  without anyone writing it, while `IRTypeAlignmentAttr` is written out
  by hand because its operands are declared as a `min_operands` count
  and its accessors have to interpret an optional trailing operand.
- [slang-ir-insts-enum.h](../../../../source/slang/slang-ir-insts-enum.h)
  declares `enum IROp : int32_t`, but its enumerators are not written
  by hand: the build-time tool `slang-fiddle` expands the
  `instEnums()` template in
  [slang-ir.h.lua](../../../../source/slang/slang-ir.h.lua) over the
  Lua table and emits `slang-ir-insts-enum.h.fiddle` (under
  `build/source/slang/fiddle/`), which that header includes. Its
  `instInfoEntries()` template likewise fills the `kIROps` table in
  [slang-ir-insts-info.cpp](../../../../source/slang/slang-ir-insts-info.cpp)
  with the `IROpInfo` record (mnemonic, fixed operand count, op flags)
  that `getIROpInfo` returns. At `source_commit` the enum holds 857
  concrete opcodes; `kIROpCount` is the sentinel that counts them and
  `kIROp_Invalid` aliases it.
- [slang-ir.h](../../../../source/slang/slang-ir.h) /
  [slang-ir.cpp](../../../../source/slang/slang-ir.cpp) define
  `IRInst`, `IRBuilder`, traversal helpers, and the hoistable / global-
  value deduplication infrastructure.

Several files this page has to cite are not in the manifest's
`watched_paths` for it, so changing them will not mark this page
stale: `slang-ir.h.lua` (which owns the Lua-key → `kIROpFlag_*`
mapping and the generated `IROpInfo` table), `slang-ir-insts-enum.h`,
[slang-ir-insts-stable-names.lua](../../../../source/slang/slang-ir-insts-stable-names.lua)
(which owns the serialized opcode identities described under *Module
versioning* below), and the two `extras/` checker scripts cited under
*Adding a new opcode*. They should be added to this document's
`watched_paths`.

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

Two defaults are worth knowing, both applied by the `process`
function at the bottom of the Lua file. When `struct_name` is
omitted it is derived from the entry name by `to_pascal_case`, so
`{ getNaturalStride = { ... } }` still yields `kIROp_GetNaturalStride`
and `IRGetNaturalStride`. When a leaf entry declares neither
`operands` nor `min_operands`, it gets `min_operands = 0`. An entry
that wants a variable-length tail without naming every operand uses
`min_operands = N` instead of an `operands` list — the `Attr` family
entries such as `{ TypeAlignment = { struct_name = "TypeAlignmentAttr",
min_operands = 1 } }` are written that way.

Entries are arranged into nested tables that establish an
inheritance hierarchy: a parent entry such as `BasicType` holds
children (`Void`, `Bool`, `Int`, ...), and their opcodes are
allocated as a contiguous range so that `as<IRBasicType>()` becomes a
single integer comparison. `process` also propagates flags down that
hierarchy, so writing `hoistable = true` once on `BasicType` makes
every scalar type opcode hoistable.

Four flag keys may appear on an entry. `hoistable = true` marks
instructions that the IR builder deduplicates and hoists to the
outermost scope where their operands are available — see
[../../../design/ir.md](../../../design/ir.md) for the semantics.
`parent = true` marks instructions that own children (functions,
blocks, modules). `global = true` marks instructions that always live
at module scope (`global_var`, `global_param`, `globalConstant`,
`key`, `interface`). `use_other = true` marks an opcode that stores
extra state in the "other" bits of its opcode word; `GLSLImageType`,
nested under `ResourceTypeBase`, is the only entry that sets it, and
the header comment in `slang-ir-insts-enum.h` names a resource type's
`TextureFlavor` as what those bits hold. The keys are translated to
the `kIROpFlag_*` values by the `flagMap` in `slang-ir.h.lua`.

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
| `MetalPackedVec` | `MetalPackedVectorType` | `elementType, elementCount` | Element-aligned, unpadded vector storage type for Metal device buffers; emitted as MSL `packed_T<N>`; hoistable. |
| `Array` | `ArrayType` | `elementType, elementCount` | Fixed-size array; hoistable. |
| `Ptr` | `PtrType` | `valueType, accessQualifierOperand?, addressSpaceOperand?, dataLayout?` | Pointer type; hoistable. |
| `TextureType` | — | `elementType, shape, isArray, isMS, sampleCount, accessOperand, isShadow, isCombined, format` | Texture types; hoistable. |
| `struct` / `class` / `interface` | `StructType` / `ClassType` / `InterfaceType` | parent of `field` / `key` / `interface_req_entry` | Parent containers; also documented in [../ir-reference/structure.md](../ir-reference/structure.md). |
| `DispatchNodeInputRecord`, `NodeOutput`, `EmptyNodeOutput`, ... | `DispatchNodeInputRecordType`, `NodeOutputType`, ... | `elementType` (nullary for the `Empty*` cases) | The ten-opcode `WorkGraphRecordTypeBase` subfamily for work-graph node input / output records; hoistable. |
| (...the `Type` family holds 170 opcodes at `source_commit`; see [../ir-reference/types.md](../ir-reference/types.md) for all of them) | | | |

### Value instructions

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `integer_constant`, `float_constant`, `string_constant`, ... | `IntLit`, `FloatLit`, `StringLit`, ... | (payload stored inline on the inst) | Literal constants; hoistable. |
| `add`, `sub`, `mul`, `div` | `Add`, `Sub`, `Mul`, `Div` | `left, right` | Arithmetic. |
| `cmpEQ`, `cmpLT`, ... | `Eql`, `Less`, ... | `left, right` | Comparisons. |
| `bitCast`, `intCast`, `floatCast`, ... | — | `val` | Conversion ops. |
| `constexprAdd` ... `constexprEnumCast` | — | 1-3 fixed operands; see Lua entries | Compile-time-folded arithmetic / cast variants; hoistable. |
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
| `RequirePrelude`, `RequireTargetExtension`, `Printf`, `Abort`, `StaticAssert`, ... | — | (variadic) | Other control-flow / backend-hint opcodes (`Abort` carries a `format` operand, like `Printf`). |
| (...see [../ir-reference/control-flow.md](../ir-reference/control-flow.md) for the full list) | | | |

### Function and module structure

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `module` | `ModuleInst` | (variadic) | Module root; parent of every top-level inst. |
| `func` | `IRFunc` | — | Function; children are blocks. |
| `generic` | `IRGeneric` | — | Type-level computation parent. Its body ends in `return_val`, not `yield`: `findGenericReturnVal` ([slang-ir.cpp](../../../../source/slang/slang-ir.cpp) line 9888) reads the last block's terminator as an `IRReturn`. A `yield` terminator does exist as a sibling of `return_val` ([slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) line 1453), but it closes an `expand` body, not a generic — its only producers are `visitExpandExpr` ([slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp) line 6592) and [slang-ir-lower-expand-type.cpp](../../../../source/slang/slang-ir-lower-expand-type.cpp) line 107. See [../ir-reference/control-flow.md](../ir-reference/control-flow.md). |
| `global_var`, `global_param`, `globalConstant` | `IRGlobalVar`, ... | (variadic) | Module-scope storage / parameters; `Global`. |
| `witness_table` / `witness_table_entry` | — | (variadic) / `requirementKey, satisfyingVal` | Witness table machinery; hoistable. |
| `key` / `builtinRequirementKey` | `StructKey` / `BuiltinRequirementKey` | — / `kindOperand` | Requirement keys: an ordinary `key` is a per-decl `global` symbol; `builtinRequirementKey` is `hoistable`, deduplicated from its `BuiltinRequirementKind` operand so a built-in requirement (e.g. an `IDifferentiable` member) resolves to one key inst. |
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
| `nameHint` | `NameHintDecoration` | `nameOperand` | Carries an identifier name for debug / link output. |
| `keepAlive` | `KeepAliveDecoration` | — | Forbids DCE on the host instruction. |
| `targetIntrinsic` | `TargetIntrinsicDecoration` | `target, definitionOperand` | Maps an IR op to a target intrinsic. |
| `entryPoint` | `EntryPointDecoration` | `profileInst, name, moduleName` | Marks a function as a pipeline entry point. |
| `BuiltinRequirementDecoration` | `IRBuiltinRequirementDecoration` | `kindOperand` | Tags an interface requirement key with its `BuiltinRequirementKind`, so consumers find the requirement by role rather than by entry order. |
| `glslFragDepthGreater` / `glslFragDepthLess` | `GLSLFragDepthGreaterDecoration` / `GLSLFragDepthLessDecoration` | — | Mark a fragment entry point whose `gl_FragDepth` is constrained to only increase / decrease (HLSL `SV_DepthGreaterEqual` / `SV_DepthLessEqual`); drives the GLSL `layout(depth_greater)` / `layout(depth_less)` redeclaration. |
| `nodeLaunch`, `nodeID`, `nodeDispatchGrid`, `maxRecords`, ... | `NodeLaunchDecoration`, `NodeIDDecoration`, ... | `mode` (an `IRStringLit`), `name, arrayIndex`, `x, y, z`, `count` | Work-graph node attributes; the launch mode is carried as a string literal so emission can re-spell it as a quoted HLSL attribute string rather than an integer. |
| (...the `Decoration` family holds 196 opcodes at `source_commit`; see [../ir-reference/decorations.md](../ir-reference/decorations.md) for all of them) | | | |

### Resource and shader-IO opcodes

| Opcode | `struct_name` | Operands | Notes |
| --- | --- | --- | --- |
| `imageLoad` / `imageStore` | — | `image, coord, ...` | Image read / write. |
| `ImageTexelPointer` | — | `image, coord, sample` | Forms a pointer to an image texel for atomic operations. |
| `structuredBufferLoad` / `rwstructuredBufferStore` | — | `base, index, val?` | Structured-buffer access. |
| `atomicLoad` / `atomicStore` / `atomicAdd` / ... | — | `ptr, val?` | `AtomicOperation` family. |
| `ControlBarrier` / `GroupMemoryBarrierWithGroupSync` / `BeginFragmentShaderInterlock` / `EndFragmentShaderInterlock` | — | — | Barriers and synchronization. |
| `waveGetActiveMask` / `waveMaskBallot` / ... | — | none / `mask, condition` | Wave intrinsics; operand shape varies by opcode. |
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

- `Parent` (Lua `parent`) — instruction owns a list of children (e.g.
  `Func`, `Block`, `Module`, `StructType`).
- `Hoistable` (Lua `hoistable`) — deduplicated; floats up to the
  outermost scope where its operands are defined. `IROpInfo` exposes
  it as `isHoistable()`.
- `Global` (Lua `global`) — always hoisted to module scope but,
  unlike `Hoistable`, never deduplicated; exposed as `isGlobal()`.
- `UseOther` (Lua `use_other`) — the opcode encoding stores extra
  information above the opcode field of the opcode word
  (`IROpMeta::kIROpMeta_OtherShift = 10`).

The opcode field is therefore ten bits wide: `kIROpMask_OpMask` is
`0x3ff`, and `getIROpInfo` masks with it before indexing the info
table, so the enum has room for 1024 opcodes against the 857 declared
at `source_commit`.

The semantics of these flags and the consequences for IR transformation
(use of `replaceOperand`, `replaceUsesWith`, traversal safety) are
covered in [../../../design/ir.md](../../../design/ir.md). Pass authors
**must** read that document before writing transformations that mutate
the IR.

`Hoistable` is the mechanism behind several "one canonical inst per
logical value" guarantees. For example, `builtinRequirementKey` is
hoistable so that `IRBuilder::getBuiltinRequirementKey(kind)` in
[slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) returns the same
key inst for a given `BuiltinRequirementKind` on every request within
one destination `IRModule` — making a witness lookup and the matching
witness-table entry agree by construction rather than by entry order.
The guarantee is module-scoped: `IRDeduplicationContext` is a member of
`IRModule`, so separately resident modules have their own arenas and
numbering maps, and an imported instruction is deduplicated once it
enters the destination module rather than by sharing pointer identity
across modules. The same flag is
why hoistable emitters are named `get*` (`getPoison`,
`getBuiltinRequirementKey`) rather than `emit*`: they may return an
existing deduplicated inst instead of creating a new one.

## Decorations

A number of opcodes are conceptually *decorations*: every entry in the
`Decoration` family in
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) (the
opcodes such as `nameHint` and `targetIntrinsic`, whose generated C++
wrappers are named `IRNameHintDecoration` and
`IRTargetIntrinsicDecoration`) is modeled as an ordinary `IRInst`,
wrapped by `IRDecoration` in
[slang-ir.h](../../../../source/slang/slang-ir.h). The older design
note in [../../../design/ir.md](../../../design/ir.md) still describes
decorations-as-instructions as a planned migration; at `source_commit`
that migration has already happened, so read that wording as history
rather than as an active roadmap. A decoration does
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

Note what this comment is *not* about. Inserting an entry in the
middle of the Lua table does renumber the `kIROp_*` values after it,
but that renumbering never reaches a `.slang-module` file, because an
opcode is not serialized as its enum value. Every entry also has a
*stable name* — a small integer assigned once and never reused, listed
in
[slang-ir-insts-stable-names.lua](../../../../source/slang/slang-ir-insts-stable-names.lua)
under the entry's dotted path (`Type.BasicType.Int`,
`Attr.TypeAlignment`, ...) — and the serializer converts through
`getOpcodeStableName` / `getStableNameOpcode`. At `source_commit` that
table holds 874 assignments with a maximum of 901; the difference from
the 857 live opcodes is retired instructions, whose identities stay in
the file so they can never be handed out again. See
[serialization.md](serialization.md) for the read/write path and how a
stable name the running compiler does not know becomes `Unrecognized`.

What the supported-version range does track is module *semantics*: a
module that uses a newly added instruction cannot be understood by an
older compiler. The declaration comment in
[slang-ir.h](../../../../source/slang/slang-ir.h) spells out the rule —
adding an instruction bumps only the maximum, whereas *removing* one
bumps the maximum and raises the minimum past the last version in
which that instruction could appear, and the number "represents the
version of module regarding semantics and doesn't have anything to do
with serialization format". `IRModule` holds the range as
`k_minSupportedModuleVersion` (4) and `k_maxSupportedModuleVersion`
(28), and a freshly built module records `m_version =
k_maxSupportedModuleVersion`. The design rationale is in
[../../../design/backwards-compat-for-ir-modules.md](../../../design/backwards-compat-for-ir-modules.md).

## Adding a new opcode

1. Add an entry to
   [slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua)
   inside the family it belongs to, not at the end of the file: the
   nesting is what keeps the family's opcodes in one contiguous range,
   which is what makes an `as<IRBasicType>()`-style check a single
   range comparison.
2. Give it a stable name. Running
   [extras/check-ir-stable-names.lua](../../../../extras/check-ir-stable-names.lua)
   with the `update` command appends the new dotted path to
   [slang-ir-insts-stable-names.lua](../../../../source/slang/slang-ir-insts-stable-names.lua)
   with the next unused id; the `check` command verifies that the two
   files are a bijection. CI enforces this through
   [extras/check-ir-stable-names-gh-actions.sh](../../../../extras/check-ir-stable-names-gh-actions.sh),
   which builds the vendored Lua, runs `check`, and fails the job with
   the diff `update` would have written.
3. Bump `k_maxSupportedModuleVersion` in
   [slang-ir.h](../../../../source/slang/slang-ir.h). CI reminds you
   via
   [extras/check-inst-version-changes.sh](../../../../extras/check-inst-version-changes.sh),
   which makes no GitHub API call itself: when a PR touches an IR
   instruction Lua file without also changing one of the two version
   constants, it writes `pr-number.txt` and `comment-body.txt` as an
   artifact, and a privileged `workflow_run` job posts the comment.
4. Decide whether it should be `hoistable`, `parent`, or `global`,
   remembering that the flag may already be inherited from the parent
   entry.
5. Nothing more is needed for a plain typed wrapper: naming the
   operands in the Lua entry is enough for the generator to emit
   `struct IRFoo` with matching accessors. Hand-write `struct IRFoo :
   IRInst` in
   [slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h),
   marked `FIDDLE()` / `FIDDLE(leafInst())`, only when the wrapper
   needs members the generator cannot derive.
6. Add an emitter to `IRBuilder` for ergonomic creation — declared in
   [slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) and
   defined in
   [slang-ir.cpp](../../../../source/slang/slang-ir.cpp), or written
   inline in the header when it is a one-liner. Name it `get*` rather
   than `emit*` when the opcode is hoistable, matching `getPoison`
   (defined in the `.cpp`) and `getBuiltinRequirementKey` (inline in
   the header).
7. Update lowering ([../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md))
   and any IR pass that needs to see or skip the new opcode.
8. If the opcode produces target-specific behaviour, extend the
   relevant emit backend
   ([../pipeline/06-emit.md](../pipeline/06-emit.md)).
9. Add tests under [tests/](../../../../tests).
