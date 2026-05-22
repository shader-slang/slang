# Prompt: tests-agentic/ir-reference/resources-and-atomics/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at
`tests-agentic/ir-reference/resources-and-atomics/`, anchored to
[`docs/llm-generated/ir-reference/resources-and-atomics.md`](../../../docs/llm-generated/ir-reference/resources-and-atomics.md).

Audience: nightly CI. The bundle exercises the **per-opcode catalog**
of the IR resource/atomic family — `RWStructuredBuffer`,
`StructuredBuffer`, `ByteAddressBuffer` / `RWByteAddressBuffer`,
`AppendStructuredBuffer` / `ConsumeStructuredBuffer`,
`ParameterBlock`, `nonUniformResourceIndex`, and the atomic-operation
family (`atomicLoad` / `atomicStore` / `atomicExchange` /
`atomicCompareExchange` / `atomicAdd` / `atomicSub` / `atomicAnd` /
`atomicOr` / `atomicXor` / `atomicMin` / `atomicMax`).

The bundle is adjacent to:

- `ir-reference/values` — the value-opcode catalog (`var`/`load`/
  `store`/`get_field_addr`/`getElementPtr`). The resource-and-atomics
  bundle reuses those memory opcodes when accessing through a buffer
  pointer but does not assert anything about their behavior beyond
  what the resource-side claim requires.
- `ir-reference/types` — the resource type opcodes
  (`RWStructuredBuffer`, `ByteAddressBuffer`, etc.). This bundle
  observes the value opcodes that operate on those types; the type-
  opcode catalog is the sibling bundle's job.
- `pipeline/04-ast-to-ir` — the AST-to-IR mapping. This bundle is
  anchored at the IR-opcode side, not the AST-method side.

## The translation rule: claims to observations

`resources-and-atomics.md` lists every resource / atomic opcode in
tables grouped by family, each row giving an `Opcode`, `C++ wrapper`,
`Operands`, `Flags`, `AST origin`, and `Summary`. The testable
consequences are:

- **"Surface construct X produces IR opcode Y"** — compile to a text
  target with `-dump-ir` and FileCheck for the opcode name (and its
  operand shape) in the IR dump. This is the only mode used in this
  bundle.

### Observable claims (write tests for these)

The catalog row → claim mapping for opcodes that surface at the
LOWER-TO-IR stage from a portable shader surface:

- **`rwstructuredBufferGetElementPtr`** — `rwBuf[i] = ...` and
  `rwBuf[i].field = ...` lvalue paths surface this opcode (the
  cross-cutting `ir-instructions` bundle covers the scalar form;
  this bundle drills into the **nested-struct-field** form where
  the GetElementPtr is followed by `get_field_addr`).
- **`rwstructuredBufferLoad`** — `rwBuf.Load(idx)` rvalue method
  surfaces this opcode.
- **`structuredBufferLoad`** — `sBuf.Load(idx)` rvalue method on a
  read-only `StructuredBuffer`.
- **`byteAddressBufferLoad`** / **`byteAddressBufferStore`** —
  `ba.Load(offset)` and `rwba.Store(offset, val)` surface these
  opcodes inside the lib function bodies of
  `ByteAddressBuffer.Load` / `RWByteAddressBuffer.Store` that are
  pulled into the IR dump.
- **`StructuredBufferAppend`** / **`StructuredBufferConsume`** —
  `aBuf.Append(v)` and `cBuf.Consume()` surface these opcodes.
- **`nonUniformResourceIndex`** — `NonUniformResourceIndex(i)`
  intrinsic call surfaces this opcode.
- **`global_param`** — every module-scope `uniform` or shader-
  parameter declaration surfaces a `global_param` line in the IR
  dump preamble. `ParameterBlock<T> pb` is observable on the
  `ParameterBlock(%T)` type carried by the `global_param`.
- **Atomic opcodes** `atomicAdd` / `atomicSub` / `atomicAnd` /
  `atomicOr` / `atomicXor` / `atomicMin` / `atomicMax` /
  `atomicExchange` / `atomicCompareExchange` / `atomicLoad` /
  `atomicStore` — surface via the **`Atomic<T>`** type's methods
  (`a.add(v)`, `a.min(v)`, `a.compareExchange(e, d)`, `a.load()`,
  `a.store(v)`, ...) which lower directly to the corresponding
  opcode with a trailing memory-order operand
  (`0 : Int` for relaxed). The HLSL `Interlocked*` intrinsics also
  surface the atomic opcodes — but only after IR passes; at the
  initial LOWER-TO-IR dump they appear as `call %InterlockedAdd(...)`
  etc. Use `Atomic<T>` methods for the observation.

### Untested claims (record under the bundle's out-of-scope heading)

- **`imageSubscript`, `imageLoad`, `imageStore`, `sample`,
  `sampleGrad`** — the natural surface (`tex.Sample(samp, uv)`,
  `rwtex[uv]`, `rwtex[uv] = v`, `rwtex.Load(c)`) lowers to a
  `call specialize(...)` on a library `%Sample` / `%Load` /
  `%operatorx5Bx5Dx5Fget` / `%operatorx5Bx5Dx5Fset` function at the
  LOWER-TO-IR stage; the underlying `image*` / `sample*` opcodes
  appear inside those library functions' `GenericAsm` bodies, not
  in the user's `main`. The library bodies are visible in the IR
  dump but are not anchored at `func %main`.
- **`SubpassLoad`, `MetalCastToDepthTexture`, `IsTextureAccess`,
  `IsTextureScalarAccess`, `IsTextureArrayAccess`,
  `ExtractTextureFromTextureAccess`, `ExtractCoordFromTextureAccess`,
  `ExtractArrayCoordFromTextureAccess`** — synthesized or
  fragment-stage / backend-pass-only.
- **`makeCombinedTextureSampler`, `MakeCombinedTextureSamplerFromHandle`,
  `CombinedTextureSamplerGetTexture`, `CombinedTextureSamplerGetSampler`** —
  the doc lists `texture.combine(sampler)` but the surface is not
  reliably portable.
- **`structuredBufferLoadStatus`, `rwstructuredBufferLoadStatus`** —
  the `Load(idx, out status)` overload requires a capability that
  is unavailable in the `compute` stage on `spirv` without
  capability flags.
- **`rwstructuredBufferStore`** — the doc says `rwBuf[i] = val`
  lowers to `rwstructuredBufferStore`, but the observed lowering
  uses `rwstructuredBufferGetElementPtr` + `store`. Record as a
  doc gap.
- **`StructuredBufferGetDimensions`** — the doc lists this as the
  IR form of `sBuf.GetDimensions(cnt, str)`, but the observed
  lowering is a `call %StructuredBufferx5FGetDimensions(...)` —
  the opcode appears inside the lib function body, not on
  `main`. Record as a partial-doc-gap.
- **Synthesized helpers** `getNaturalStride`, `castDynamicResource`,
  `getEquivalentStructuredBuffer`, `getStructuredBufferPtr`,
  `getUntypedBufferPtr`, `getRegisterIndex`, `getRegisterSpace`,
  `LoadResourceDescriptorFromHeap`, `LoadSamplerDescriptorFromHeap`,
  `SPIRVLoadDescriptorFromHeap`, `SPIRVLoadTexelPointerFromHeap`,
  `SPIRVResourceHeap`, `SPIRVSamplerHeap` — all marked
  "(synthesized)" in the doc; no portable LOWER-TO-IR surface.
- **`GetWorkGroupSize`, `GetCurrentStage`** — "(synthesized;
  materialized during `translateGlobalVaryingVar`)" — produced
  during the layout / emit pipeline, not LOWER-TO-IR.
- **Mesh-shader outputs** `meshOutputRef`, `meshOutputSet`,
  `metalSetVertex`, `metalSetPrimitive`, `metalSetIndices` —
  require mesh stage; out-of-scope for this `-stage compute` bundle.
- **`GroupMemoryBarrierWithGroupSync`** — surface intrinsic
  appears as `call %GroupMemoryBarrierWithGroupSync()` at the
  LOWER-TO-IR stage rather than a direct opcode. Record as a
  doc gap.
- **`ControlBarrier`, `BeginFragmentShaderInterlock`,
  `EndFragmentShaderInterlock`** — likewise call-shaped at the
  LOWER-TO-IR stage.
- **Cooperative matrix / vector** opcodes (`CoopMatMulAdd`,
  `CoopVecMatMulAdd`, ...) — require core-module `CoopMat` /
  `CoopVec` types and capability flags not available portably.
- **Wave intrinsics** `waveGetActiveMask`, `waveMaskBallot`,
  `waveMaskMatch` — surface as `call %WaveGetActiveMask()` etc.
  at the LOWER-TO-IR stage.
- **Raytracing payload** opcodes — synthesized for OptiX / Vulkan
  raytracing backends; require raytracing stage.
- **Descriptor heaps** opcodes — bindless-only; no portable
  shader surface.
- **`MetalAtomicCast`, `IncrementCoverageCounter`** —
  "(synthesized)" or backend-specific.
- **`atomicSub`** — `Atomic<T>` does not expose a `.sub(v)`
  method directly; `a.add(-v)` lowers to `atomicAdd(%p, -%v)`,
  not `atomicSub`. Record as a doc gap.

If you find yourself thinking "this would verify the
deduplication-by-hash for `rwstructuredBufferGetElementPtr`", stop —
that's an internal probe.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for the
   unobservable-via-slangc items listed above (the heading is a
   convention shared with other bundles).
2. 15 to 25 `.slang` test files. Aim for one observable opcode (or
   one observable per-target lowering) per test. Group three or
   more closely-related opcodes (e.g. the four bitwise atomic
   opcodes) into one file rather than emitting four single-opcode
   files.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/ir-reference/resources-and-atomics.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off or the test specifically observes a
cross-bundle boundary):

- `docs/llm-generated/cross-cutting/ir-instructions.md`
- `docs/llm-generated/pipeline/04-ast-to-ir.md`
- `docs/llm-generated/ir-reference/types.md`
- `docs/llm-generated/ir-reference/values.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for verification only

You may look at these to confirm an opcode spelling. You may **not**
mine them for behavioral claims the doc does not make.

- `source/slang/slang-ir-insts.lua` — canonical opcode list.
- `source/slang/slang-ir-insts.h` — C++ wrapper structs.
- `source/slang/slang-lower-to-ir.cpp` — AST-to-IR lowering.

The opcode spellings in the IR dump are the lowercase Lua entry
keys for value opcodes (`atomicAdd`, `atomicMin`,
`atomicCompareExchange`, `atomicLoad`, `atomicStore`,
`rwstructuredBufferGetElementPtr`, `rwstructuredBufferLoad`,
`structuredBufferLoad`, `byteAddressBufferLoad`,
`byteAddressBufferStore`, `StructuredBufferAppend`,
`StructuredBufferConsume`, `nonUniformResourceIndex`,
`global_param`).

## Test directives

The standard form used here is:

```
//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
```

Per the universal `_common.md` rule: combine `-dump-ir` with
**`-target <text-target>`** AND **`-o /dev/null`** so the IR dump
goes to stdout uncontaminated by target text.

Anchor patterns at `func %main` (or a user-named helper) where the
observed opcode is in the user-body; for opcodes that appear inside
a lib function body (`byteAddressBufferLoad`, `byteAddressBufferStore`),
match the opcode globally and document this in `purpose`.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ir-reference/resources-and-atomics.md` (or one of the listed
      secondary docs).
- [ ] Every `-dump-ir` test uses `-target <text-target> -dump-ir
      -o /dev/null -stage compute -entry main`.
- [ ] Operands are non-constant (`uniform` globals or values
      derived from `SV_DispatchThreadID`) so constant folding does
      not collapse the operator.
- [ ] CHECK patterns target either `func %main` or the lib-function
      hosting the opcode — the IR-dump preamble must not match.
- [ ] No test depends on a GPU.
- [ ] No test asserts on `Interlocked*` intrinsic call as an
      atomic opcode — those are passes-introduced.
- [ ] README.md `## Doc gaps observed` is honest. If you wanted a
      test you could not anchor, write down which claim the doc
      would need to add.
