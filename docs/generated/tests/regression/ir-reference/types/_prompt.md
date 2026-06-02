# Prompt: docs/generated/tests/regression/ir-reference/types/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/regression/ir-reference/types/`,
anchored to
[`docs/generated/design/ir-reference/types.md`](../../../design/ir-reference/types.md).

Audience: nightly CI. The bundle exercises the **per-opcode catalog**
of the IR `Type` family — that each documented `Type` opcode (scalars
`Bool`/`Int*`/`UInt*`/`Half`/`Float`/`Double`/`Char`, vectors `Vec`,
matrices `Mat`, arrays `Array`/`UnsizedArray`, functions `Func`,
pointers `Ptr` with optional address-space / access-qualifier
operands, structs `struct`, interfaces `interface`, witness-table
types `witness_table_t`, generic-parameter types `type_t`, optional
`Optional`, tuple `tuple_type`, and a handful of resource and
sampler types) appears in `-dump-ir` output for the obvious surface
construct that produces it, and that target-divergent type lowerings
(vector spelling, matrix wrapper, struct names) lower to the
documented per-target text on the text-emit backends.

This bundle is the **per-opcode reference** for type opcodes. It is
adjacent to:

- `cross-cutting/ir-instructions` — the category-level view (one
  test per family). That bundle already covers `Vec`/`Array`/`Ptr`
  at a sample level; the new bundle drills into the rest of the
  catalog without duplicating those three.
- `pipeline/04-ast-to-ir` — the AST-side mapping ("which AST node
  lowers to which IR opcode"). That bundle anchors at the AST side;
  this bundle anchors at the IR-type side.

Anchor each test at the IR-type opcode that the doc names; do not
write tests that observe IR passes, layout, or emit-only behaviors
unless the doc explicitly describes that behavior.

## The translation rule: claims to observations

`types.md` lists every IR `Type` opcode in tables grouped by family,
each row giving an `Opcode`, `C++ wrapper`, `Operands`, `Flags`, `AST
origin`, and `Summary`. The testable consequences are:

- **"AST construct X produces IR type opcode Y"** — compile to a
  text target with `-dump-ir` and FileCheck for the opcode name (and
  its operand shape) in the IR dump. This is the primary mode.
- **"Type Y lowers to text Z on target T"** — compile to target `T`
  and FileCheck the emitted code for an unambiguous substring. Use
  this when the per-target lowering of a type is unambiguous: vector
  spellings (`float3`/`vec3`/`vec3<f32>`/`Vector<float,3>`), matrix
  spellings (`float4x4`/`mat4x4`/`mat4x4<f32>`), pointer-like and
  buffer types.

### Observable claims (write tests for these)

The catalog row → claim mapping:

- **Basic scalars** (`Bool`, `Int8`/`Int16`/`Int`/`Int64`,
  `UInt8`/`UInt16`/`UInt`/`UInt64`, `Half`/`Float`/`Double`). A
  field of the named type appears in the IR dump as
  `field(%name, <Opcode>)`. Group all scalars into one or two tests
  rather than 16 single-claim files — the bundle is per-opcode but
  the operator-visible cost of 16 single-scalar tests outweighs the
  per-test signal.
- **`Void`**. Functions returning `void` show `Func(Void, ...)` in
  the IR.
- **`Vec(elementType, elementCount)`**. A `floatN` / `intN` value
  appears as `Vec(Float, N : Int)` etc. in the dump; emits as
  `floatN` (HLSL/Metal), `vecN` (GLSL), `vecN<f32>` (WGSL),
  `Vector<float, N>` (CPP) on text-emit targets.
- **`Mat(elementType, rowCount, columnCount, layout)`**. A
  `floatRxC` value appears as `Mat(Float, R : Int, C : Int, L : Int)`.
  Emits as `floatRxC` (HLSL), `matRxC` (GLSL), `matRxC<f32>` (WGSL).
- **`Array(elementType, elementCount)`** and **`UnsizedArray`** —
  a fixed array field appears as `Array(<T>, N : Int)`; an unsized
  array as `UnsizedArray(<T>)`.
- **`Func(resultType, paramTypes...)`** — every user function's IR
  declaration shows `func %<name> : Func(<R>, <P1>, <P2>, ...)`.
- **`Ptr(valueType, [accessQualifier?], [addressSpace?])`** — a
  local `var` produces a `Ptr<T>`-typed value; a `groupshared`
  module-scope variable's IR type is `RateQualified(GroupShared,
Ptr(<T>))`.
- **`struct`** / **`class`** — user struct lowers to a parent
  `struct %Name` followed by `field(%key, <T>)` rows. Note the
  container side is also documented in `structure.md`, but this
  bundle anchors at the "type" view (the struct type as an IR
  value).
- **`interface`** / **`witness_table_t`** / **`this_type`** /
  **`associated_type`** — an `interface IFoo` declaration emits an
  `interface_req_entry(...)` per method whose result type involves
  `this_type(%IFoo)`. A `struct S : IFoo { ... }` conformance emits
  `witness_table %... : witness_table_t(%IFoo)(%S)` with
  `witness_table_entry(...)` rows.
- **`Optional`** — `Optional<T>` parameter/return appears as
  `Optional(<T>)`.
- **`tuple_type`** — `makeTuple(a, b)` yields a value of type
  `tuple_type(<TA>, <TB>)`.
- **`type_t`** — a `GenericDecl`'s type parameter is typed
  `type_t` (the kind-of-a-type) in the IR dump.
- **`StructuredBuffer<T>`**, **`RWStructuredBuffer<T>`**,
  **`ConstantBuffer<T>`**, **`ByteAddressBuffer`**,
  **`RWByteAddressBuffer`** — a `uniform` of the corresponding
  surface type lowers to `global_param` with the same IR-type
  spelling (parameterized by element type and a layout marker for
  the typed-buffer family).
- **`SamplerState`** — a `uniform SamplerState samp` lowers to a
  `global_param` of IR type `SamplerState`.

### Notable opcode notes (`## Notable opcodes`)

The doc's `## Notable opcodes` section restates several catalog
rows with more prose:

- `Vec`/`Mat` hoistability → "same vector type appears as a single
  IR value across the module". This is hard to observe directly
  through `-dump-ir` (the dump shows the post-hoist result, not the
  decision). Record this as a doc-anchored but not testable claim;
  put it in `## Untested claims` if helpful.
- `Func` shape → first operand is result, rest are params. Test by
  checking `Func(Int, Int, Float)` for a Slang function
  `int helper(int v, float f)`.
- `Array` vs `UnsizedArray` → fixed vs runtime extent. Use a
  struct with both a fixed-size array field and an unsized-array
  field to exercise both.
- `Ptr` with access-qualifier / address-space operands → observe
  `RateQualified(GroupShared, Ptr(<T>))` for `groupshared int gs;`.
- `AnyValueType(size)` and the existential types (`BindExistentials`,
  `BoundInterface`, `DynamicType`) — these are produced by IR
  passes (existential-elimination), not by the lowering step
  observable in the first `-dump-ir` stage. They are doc gaps for
  this bundle and belong to `ir-reference/generics-and-existentials`.
- `RateQualified` — observable via `groupshared`. Other rate
  qualifiers (`ConstExpr`, `SpecConst`) are observable only via
  more advanced surface constructs and may be doc gaps.
- `TextureType` (9 operands) — observable through a
  `uniform Texture2D<float4> tex;`. The full operand list is too
  detailed for one test; one test that observes the opcode name is
  sufficient.
- `BackwardDiffIntermediateContextType` — only produced by autodiff;
  not testable without a `[Differentiable]` function. Record as a
  doc gap if you want it covered.

### Not testable through slangc (record under `## Untested claims`)

- **Hoistability flags** (`H` in the doc's tables) — the dump shows
  the post-hoist result, not the decision. Two textually identical
  `Vec(Float, 3 : Int)` types appearing once is observable; the
  _reason_ (hoisting) is not.
- The **C++ wrapper struct** identity (`VectorType`, `FuncType`,
  etc.) — internal API, not surface-visible.
- **`BackwardDiffIntermediateContextType`**, **`ForwardDiffFuncType`**
  and the other autodiff context types — produced by the autodiff
  pass when `[Differentiable]` is in play; this bundle does not
  cover autodiff.
- **`BindExistentialsType` / `BoundInterface` / `AnyValueType`** —
  produced by the existential-elimination pass, not by lowering;
  belong to `ir-reference/generics-and-existentials`.
- **Storage-only floats** (`FloatE4M3Type`, `FloatE5M2Type`,
  `BFloat16Type`) — surface support varies by target and core
  module; if these are not reachable from a portable .slang source
  on the no-GPU runner, record as out of scope.
- **`ComPtr` / `NativePtr` / `DescriptorHandle`** — host-side API
  types, not exercisable from a shader.
- **All buffer-layout markers** (`Std140Layout`, `Std430Layout`,
  `D3DConstantBufferLayout`, `MetalParameterBlockLayout`,
  `CUDALayout`, `LLVMLayout`, `CLayout`, `ScalarLayout`,
  `DefaultLayout`, `DefaultPushConstantLayout`) — surface visible
  only inside the typed-buffer family's IR encoding (e.g.
  `RWStructuredBuffer(Float, DefaultLayout, ...)`). One test that
  observes `DefaultLayout` inside the RWStructuredBuffer IR shape
  is enough; per-layout-marker tests are not warranted from the
  doc's claims.
- **`spirvLiteralType`** — emitted by `__intrinsic_asm` lowering;
  too internal for a surface test.
- **Set-theoretic types** (`UntaggedUnionType`, `ElementOfSetType`,
  `SetTagType`, `TaggedUnionType`, `OptionalNoneType`) — used by
  existential-elimination; out of scope here.
- The **kinds** (`Type`, `TypeParameterPack`, `Rate`, `Generic`)
  — the doc places `type_t` and `Type` (TypeKind) in the same
  section; `type_t` is observable as a generic param's type, the
  other kinds are deeper-internal.
- **Tensor and torch-tensor types** — Python-binding lowering,
  not reachable from .slang code in this runner.

If you find yourself thinking "this would verify the hoistability
flag for opcode X", stop — that's an internal probe.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for the
   unobservable-via-slangc items listed above (the heading is a
   convention shared with `cross-cutting/ir-instructions`).
2. 20 to 40 `.slang` test files. Aim for one observable opcode (or
   one observable per-target lowering) per test. The cap is 80
   files; quality over quantity. Group basic scalars into one or
   two tests rather than emitting 16 single-scalar files — that
   trades aesthetics for noise.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/ir-reference/types.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off or the test specifically observes
a cross-bundle boundary):

- `docs/generated/design/cross-cutting/ir-instructions.md`
- `docs/generated/design/pipeline/04-ast-to-ir.md`
- `docs/generated/design/ast-reference/types.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for verification only

You may look at these to confirm an opcode spelling (the exact name
as it appears in `-dump-ir` output). You may **not** mine them for
behavioral claims the doc does not make.

- `source/slang/slang-ir-insts.lua` — canonical opcode list.
- `source/slang/slang-ir-insts.h` — C++ wrapper structs.
- `source/slang/slang-ir.h`, `slang-ir.cpp` — `IRInst` + builder.
- `source/slang/slang-lower-to-ir.cpp` — AST-type to IR-type
  lowering.

The opcode spelling in the IR dump is generally the lowercase Lua
entry key for value/control-flow opcodes, but **type opcodes keep
their original capitalisation**: `Vec`, `Mat`, `Array`,
`UnsizedArray`, `Func`, `Ptr`, `Int`, `UInt`, `Half`, `Float`,
`Double`, `Bool`, `Optional`, `RWStructuredBuffer`, `StructuredBuffer`,
`ConstantBuffer`, `ByteAddressBuffer`, `RWByteAddressBuffer`,
`SamplerState`, `Texture2D` (via `TextureType`), `tuple_type`,
`type_t`, `witness_table_t`, `this_type`, `interface_req_entry`,
`witness_table_entry`, `RateQualified`, `GroupShared`,
`DefaultLayout`, `BackwardDiffIntermediateContextType`,
`OutParam`, `BorrowInOutParam`. (Lowercase exceptions: `struct`,
`class`, `interface`, `tuple_type`, `type_t`, `witness_table_t`,
`this_type`, `associated_type`.)

## Test directives

Type-opcode claims are best observed through one of two mechanisms.

1. **`-dump-ir` against the IR dump** (per-opcode appearance). The
   standard form used here is:

   ```
   //TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
   ```

   Per the universal `_common.md` rule: combine `-dump-ir` with
   **`-target <text-target>`** AND **`-o /dev/null`** so the IR dump
   goes to stdout uncontaminated by target text.

   Anchor patterns to a user-named function (`func %main`, a helper)
   or to a user struct (`struct %Bag`) to cut through the dump
   preamble. Use the `IR:` pattern prefix so the same file can also
   carry text-emit directives later.

2. **Multi-target emit** for type lowerings that are
   target-divergent. Vector and matrix spellings, struct-field
   rendering, and buffer-type wrappers diverge per backend; use one
   `//TEST:SIMPLE(filecheck=CHECK<target>):-target <T> ...` directive
   per feasible target, with a distinct CHECK prefix and per-target
   pattern. The bundle's source doc does not claim a uniform text-
   emit rendering across all targets; the test asserts the per-
   target text individually.

3. **`DIAGNOSTIC_TEST`** for the small set of negative claims (e.g.
   type-mismatch on assigning a `vector<int,3>` to a `vector<int,4>`,
   error code `E30019`). Use the suggested-annotations form per
   `_common.md`. Keep negative tests rare — most type-opcode claims
   are positive.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ir-reference/types.md` (or one of the listed secondary docs).
- [ ] Every `-dump-ir` test uses `-target <text-target> -dump-ir
-o /dev/null -stage compute -entry main` per CLAUDE.md.
- [ ] Outputs escape DCE: write to an `RWStructuredBuffer<T>` so
      the type observation survives to the dump's first stage.
- [ ] Scalar tests group several scalars into one file rather than
      16 micro-files.
- [ ] Per-target emit tests use distinct CHECK prefixes
      (`CHECKHLSL`, `CHECKGLSL`, `CHECKMETAL`, `CHECKWGSL`,
      `CHECKCPP`) and assert per-target text, not a single shared
      pattern. The doc does not claim cross-target uniformity for
      type spellings.
- [ ] For `Vec` observation in IR, the source uses a `floatN` /
      `intN` value — not a hand-built tuple.
- [ ] For `Ptr` observation in IR, use a local `var` of struct
      type (so the `var` survives — trivial scalar locals are
      collapsed during initial lowering).
- [ ] CHECK patterns anchored at `func %main`, a user struct, a
      user-named field, or a user-named `global_param` — the
      IR-dump preamble must not match.
- [ ] No test depends on a GPU.
- [ ] No test asserts a C++ wrapper-struct identity (`VectorType`,
      `FuncType`, etc.) — the dump shows the IR-type name.
- [ ] No test asserts on autodiff-produced types unless the doc
      describes a surface that triggers them.
- [ ] No test was written by inspecting an uncovered source line.
      If you find yourself thinking "this would cover the branch
      at `slang-lower-to-ir.cpp:NNNN`", stop and re-read the doc.
- [ ] README.md `## Doc gaps observed` is honest. If you wanted a
      test you could not anchor, write down which claim the doc
      would need to add.

## Lessons captured (apply to this bundle as well)

These are restated from `_common.md` because they bite hard in
type-observation tests:

- `-dump-ir` requires `-target <X>` (else compile stops early) and
  `-o /dev/null` (else target text mixes with IR on stdout).
- Trivial scalar locals are eliminated during lowering. A
  `var %p : Ptr(<T>) = var` line survives only if the local is a
  **struct** accessed by field address (or an **array** indexed
  elementwise).
- The IR dump prefixes user IR with a substantial preamble
  (Annotation, capability sets, differentiation witness tables,
  core-module symbols). Anchor at `func %main`, a user struct, or
  a user `global_param` to skip the preamble.
- The opcode spelling for **type** opcodes in the dump retains the
  capitalisation from the Lua entry key — `Vec`, not `vec`;
  `Float`, not `float`; `Optional`, not `optional`. Lowercase
  exceptions (`struct`, `class`, `interface`, `tuple_type`,
  `type_t`, `witness_table_t`, `this_type`, `associated_type`) are
  documented in the doc itself and in the Lua source.
- `Mat`'s fourth operand is a layout integer — `0` for column-major,
  the value the IR uses for HLSL's default. Match a wildcard
  (`{{[0-9]+}}`) rather than a specific layout literal.
