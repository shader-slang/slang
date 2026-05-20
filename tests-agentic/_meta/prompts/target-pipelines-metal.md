# Prompt: tests-agentic/target-pipelines/metal/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/target-pipelines/metal/`,
anchored to
[`docs/llm-generated/target-pipelines/metal.md`](../../../docs/llm-generated/target-pipelines/metal.md).

Audience: nightly CI. The bundle exercises the **Metal target
pipeline**: the ordered IR-pass + emit sequence run by
`linkAndOptimizeIR` and `emitEntryPointsSourceFromIR` when
`CodeGenTarget::Metal` (or a downstream-Metal target such as
`MetalLib` / `MetalLibAssembly`) is requested. The doc enumerates
four phases (A: link and entry-point prep; B: specialization and
type legalization, including Metal-only `wrapCBufferElementsForMetal`,
the `MetalParameterBlock`-policy `lowerBufferElementTypeToStorageType`
invocation, and a Metal arm of `legalizeEmptyTypes`; C: Metal
legalization driver `legalizeIRForMetal`, `specializeAddressSpaceForMetal`,
phi elimination with default options; D: Metal emit by
`MetalSourceEmitter`, then optional Apple `metal` downstream
compilation for `MetalLib*`). The defining characteristic of this
bundle is that almost every claim is observable in **emitted Metal
text** — the Apple `metal` compiler is out of band and not invoked
by `-target metal`.

This bundle is therefore **single-target-by-design**: the default
test directive is

```
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute
```

Multi-backend probes are reserved for the "Metal does X but sibling
target does not" subset. The doc explicitly contrasts Metal with
HLSL (in some passes) and with SPIR-V/WGSL (in others); these
contrasts are observable as IR-pass side effects in emitted text.

## The translation rule: claims to observations

`target-pipelines/metal.md` describes:

- The ordered `SLANG_PASS` sequence for Metal in
  `linkAndOptimizeIR`, with `isMetalTarget(targetRequest)` as the
  central predicate.
- Metal-specific gates (Metal-only or Metal-included arms):
  `wrapCBufferElementsForMetal`, the `MetalParameterBlock`-policy
  `lowerBufferElementTypeToStorageType` (Phase B), the Metal arm of
  `legalizeEmptyTypes` (Phase B), `legalizeIRForMetal`,
  `legalizeImageSubscript` (Metal/GLSL/SPIR-V arm),
  `legalizeLogicalAndOr` (Metal arm), `undoParameterCopy` +
  `transformParamsToConstRef` (CPU/CUDA/Metal arm),
  `specializeAddressSpaceForMetal`,
  `moveGlobalVarInitializationToEntryPoints` (via fallthrough),
  `introduceExplicitGlobalContext` (via fallthrough),
  `lowerCombinedTextureSamplers` (HLSL/Metal/WGSL arm),
  `lowerAppendConsumeStructuredBuffers` (`target != HLSL`),
  `legalizeByteAddressBufferOps` with Metal-specific options.
- Metal-specific skips: `legalizeArrayReturnType` (`!isMetalTarget &&
  !isSPIRV` excludes Metal — array return types survive as
  `array<T,N>` return values), the HLSL-only
  `legalizeNonVectorCompositeSelect` and `wrapStructuredBuffersOfMatrices`
  arms, the SPIR-V-only `performIntrinsicFunctionInlining`,
  `synthesizeActiveMask` (CUDA only), `resolveTextureFormat`
  (GLSL/SPIR-V/WGSL only).
- Always-on Metal emit shapes: the `#include <metal_stdlib>` /
  `#include <metal_math>` / `#include <metal_texture>` /
  `using namespace metal;` prelude block, the `[[kernel]]`
  attribute on compute entry points (rendered as bare-substring
  `kernel` because `[[...]]` is a FileCheck regex-variable
  reference), the `[[thread_position_in_grid]]` /
  `[[thread_position_in_threadgroup]]` system-value attributes,
  the `[[buffer(N)]]` / `[[texture(N)]]` / `[[sampler(N)]]`
  positional binding attributes, the Metal `device` / `constant` /
  `threadgroup` / `thread` address-space qualifiers on pointer
  types, the `#line N "file"` directives the `SourceWriter`
  emits, the Apple-specific `texture2d<T, access::sample>` /
  `texture2d<T, access::read_write>` types, and the renaming of
  user entry points (`main` → `main_0` — Metal **does** rename).
- Downstream: bare `-target metal` stops at Metal text; Apple's
  `metal` compiler is required only for `MetalLib*` and is not
  invoked on the no-GPU runner.

### Observable claims (write tests for these)

- **Metal prelude starts with the standard includes and
  `using namespace metal;`** — every Metal compile emits these
  before any user code. (Anchor:
  `#phase-d-metal-emit-and-downstream-tools`.)
- **`[[kernel]]` attribute is emitted on the compute entry point**
  (matched as bare `kernel` because `[[…]]` is a FileCheck regex
  reference). (Anchor: `#phase-d-metal-emit-and-downstream-tools`.)
- **Slang renames the entry point** `main` → `main_0` on Metal
  (warning `E40100` confirms this); the emitted function name is
  `main_0`. (Anchor: `#phase-d-metal-emit-and-downstream-tools`.)
- **`[numthreads(X,Y,Z)]` does not survive as a Metal attribute**
  — Metal expresses the threadgroup size at dispatch time, not in
  source. The entry-point parameters carry positional binding
  attributes instead. (Anchor:
  `#phase-d-metal-emit-and-downstream-tools`; also relevant:
  `#legalizeirformetal` for varying-param legalization.)
- **`SV_DispatchThreadID` lowers to a `uint3` entry-point param
  with `[[thread_position_in_grid]]`.** (Anchor:
  `#legalizeirformetal`.)
- **`SV_GroupIndex` is synthesized from `[[thread_position_in_threadgroup]]`**
  (Slang computes the linear index in the kernel body since Metal
  does not provide a single builtin for it). (Anchor:
  `#legalizeirformetal`.)
- **`RWStructuredBuffer<T>` / `StructuredBuffer<T>` parameters
  emit as `T device*` pointers with `[[buffer(N)]]` positional
  attributes.** Indices are positional (0, 1, 2…) — not driven by
  `vk::binding` or `register(uN)`. (Anchor:
  `#phase-d-metal-emit-and-downstream-tools` + `#legalizeirformetal`.)
- **`ConstantBuffer<T>` / `cbuffer` declarations emit as
  `<wrapper-struct> constant*` pointer params with `[[buffer(N)]]`.**
  Constant buffers go in Metal's `constant` address space.
  (Anchor: `#wrapcbufferelementsformetal` +
  `#phase-d-metal-emit-and-downstream-tools`.)
- **`wrapCBufferElementsForMetal` wraps a `cbuffer` with a
  `float4x4` element in a `_MatrixStorage_…natural_<N>` struct**
  inside an `SLANG_ParameterGroup_<N>_natural_<M>` wrapper. The
  emitted Metal text references the storage struct, not a bare
  `float4x4`. (Anchor: `#wrapcbufferelementsformetal`.)
- **`Sampler2D` (combined texture+sampler) splits into separate
  `texture2d<T, access::sample>` and `sampler` params** with
  `[[texture(N)]]` and `[[sampler(N)]]`, named
  `combined_texture_<N>` / `combined_sampler_<N>`. (Anchor:
  `#phase-b-specialization-and-type-legalization` — Metal is in
  the `lowerCombinedTextureSamplers` arm.)
- **`Texture2D<T>` + `SamplerState` survive as separate
  parameters** — Metal natively has separate texture and sampler
  types. (Anchor: `#phase-d-metal-emit-and-downstream-tools`.)
- **`RWTexture2D<T>` emits as
  `texture2d<T, access::read_write>`** with `[[texture(N)]]` and
  the subscript-store lowers via `legalizeImageSubscript` into
  `.write(value, coord)`; subscript-load lowers into `.read(coord)`.
  (Anchor: `#legalizeimagesubscript`.)
- **`groupshared` arrays emit with the `threadgroup` address
  space** as `threadgroup array<T, N>` — `specializeAddressSpaceForMetal`
  is responsible. (Anchor: `#specializeaddressspaceformetal`.)
- **`InterlockedAdd` lowers to `atomic_fetch_add_explicit` with
  `memory_order_relaxed`** and a `(atomic_uint device*)` cast.
  (Anchor: `#phase-c-metal-legalization-lowering-phi-elimination`
  — `validateAtomicOperations` runs for Metal.)
- **`ByteAddressBuffer.Load<uint>(offset)` lowers to a Metal
  pointer-indexed load**: the Metal-options
  `scalarizeVectorLoadStore=true` + `lowerBasicTypeOps=true`
  produce `as_type<uint>(src[(offset)>>2])` form. (Anchor:
  `#phase-c-metal-legalization-lowering-phi-elimination` for the
  `legalizeByteAddressBufferOps` row.)
- **`enum` lowers to its underlying integer** on Metal: the
  enumerator name is erased and the emitted text contains a
  bare integer literal. (Anchor:
  `#phase-a-link-and-entry-point-prep`; `lowerEnumType`.)
- **An array-return function survives as `array<T, N> foo(...)`**
  on Metal because `legalizeArrayReturnType` is filtered out for
  Metal (`!isMetalTarget`). Contrast with HLSL where the function
  becomes `void` with an `out` parameter. (Anchor:
  `#phase-c-metal-legalization-lowering-phi-elimination` —
  filtered-out passes table.)
- **`if/else`-merged values lower to a function-local variable
  assigned in each branch.** Metal uses default
  `PhiEliminationOptions` (contrast with SPIR-V which uses
  register allocation). (Anchor:
  `#eliminatephis-with-default-options`.)
- **`static` module-scope variables have their initializer fire
  inside the entry point** via the
  `moveGlobalVarInitializationToEntryPoints` fallthrough.
  (Anchor:
  `#phase-c-metal-legalization-lowering-phi-elimination` +
  `#undoparametercopy-transformparamstoconstref`.)
- **`uniform` scalar/struct globals are packed into a
  `GlobalParams` struct** that is passed as a `constant*` pointer
  parameter to the kernel via
  `introduceExplicitGlobalContext`. (Anchor:
  `#phase-c-metal-legalization-lowering-phi-elimination` —
  fallthrough chain.)
- **`ParameterBlock<Resources>` is laid out with the
  `MetalParameterBlock` policy**: resource-typed fields inside
  the block become descriptor handles (raw `T device*` pointer
  fields) and the block itself binds as one `constant*` parameter.
  (Anchor: `#metal-specific-lowerbufferelementtypetostoragetype`.)
- **Element-wise vector `&&` survives** as a single `bool3(...) &&
  bool3(...)` expression on Metal (the `legalizeLogicalAndOr`
  Metal arm legalizes short-circuit forms while preserving the
  element-wise operator for vector operands). (Anchor:
  `#phase-c-metal-legalization-lowering-phi-elimination` —
  `legalizeLogicalAndOr`.)
- **`asuint(float)` (and similar bitcasts) emit as
  `as_type<uint>(...)`** on Metal — the Metal-specific bitcast
  rendering. (Anchor:
  `#phase-c-metal-legalization-lowering-phi-elimination` —
  `lowerBitCast`.)
- **Conditional struct selection (non-vector composite select)
  lowers to an `if/else` block assigning a local of the struct
  type** — Metal does **not** run the HLSL-only
  `legalizeNonVectorCompositeSelect` pass, but the general phi
  elimination already produces the same `if/else` shape.
  (Anchor: `#phase-c-metal-legalization-lowering-phi-elimination`
  — `eliminatePhis`.)
- **Source-position `#line N "file"` directives** appear in
  emitted Metal text by default and may reference
  `hlsl.meta.slang` / `core` for core-module-sourced items.
  (Anchor: `#phase-d-metal-emit-and-downstream-tools` —
  `SourceWriter` in `emitModule`.)
- **`-target metal` stops at Metal text.** No `.metallib` binary,
  no `.metalairlib`, no Apple `metal` invocation — that path is
  reserved for `MetalLib*` targets. (Anchor:
  `#downstream-apple-metal-compiler`.)
- **The Metal source emitter does not emit a target-language
  identification comment** for `-target metal` (no
  `// SPIR-V`-style header). The prelude is purely the `#include`
  block — verifying this is a structural negative-observation.
  (Anchor: `#phase-d-metal-emit-and-downstream-tools`.)
- **`isMetalTarget` is true at four sites** in `slang-emit.cpp`
  (line 1553, 1984, 2050, 2185). The combined consequence is
  observable as: a `MetalParameterBlock`-policy lowering for
  resource-typed fields inside parameter blocks; Metal arm of
  `legalizeLogicalAndOr`; `transformParamsToConstRef` for
  pass-by-const-ref of struct params;
  `specializeAddressSpaceForMetal` annotating pointer types with
  `device` / `constant` / `threadgroup` / `thread`.
  (Anchor: `#conditional-gates` — Metal-specific runtime
  predicates table.)

### Not testable through `slangc -target metal` (record under
`## Out of scope (no-GPU runner)`)

- **Apple `metal` compiler invocation**, `.metallib` bytecode
  emission, `.metallib` disassembly. Requires Xcode toolchain;
  not exercised on the no-GPU runner. (Anchor:
  `#downstream-apple-metal-compiler`.)
- **Pass ordering within Phase A/B/C.** Pass _existence_ is
  observable through its effect on emitted text; pass _ordering_
  is an IR-level claim that requires `-dump-ir` annotations
  cross-pass which the doc does not anchor to a specific marker.
- **`AppendStructuredBuffer<T>` / `ConsumeStructuredBuffer<T>`.**
  Although the doc claims `lowerAppendConsumeStructuredBuffers`
  fires for Metal (since `target != HLSL`), Slang's
  `checkEntryPointDecorations` actually rejects them on the Metal
  compute stage with `E36107` ("uses features that are not
  available in 'compute' stage for 'metal' compilation target"),
  so the lowering pass cannot be observed in emit. Record as a
  doc gap.
- **`legalizeMeshOutputTypes`.** Requires a mesh-shader entry
  point; not exercised on a compute-only no-GPU runner.
- **`legalizeSubpassInputsForMetal` ([[color(N)]] fragment input).**
  Requires a fragment entry point with `SubpassInput<T>`. The doc
  describes the frame-buffer-fetch mapping in detail; a fragment-
  stage probe is outside the compute-only scope of this bundle.
- **`MetalLibAssembly` skipping `wrapCBufferElementsForMetal`.**
  The doc names this as a minor inconsistency at line 1731-1735;
  observing the resulting MSL difference requires a side-by-side
  `-target metal` vs `-target metallib-asm` comparison and a
  cbuffer with a layout that the wrapper actually rewrites. The
  no-GPU runner cannot produce `metallib-asm` without the Apple
  toolchain.
- **`collectCooperativeMetadata`.** Requires the cooperative
  matrix or vector capability set.
- **`floatNonUniformResourceIndex`.** Requires the
  `NonUniformResourceIndex(...)` HLSL intrinsic with bindless
  resource heap setup that the compute-only bundle does not have.
- **`legalizeUniformBufferLoad`, `invertYOfPositionOutput`,
  `rcpWOfPositionInput`.** All gated on Khronos / HLSL or
  cross-API options the Metal bundle does not engage.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`.
2. 20 to 35 `.slang` test files. Metal is the focus, so the
   typical file carries one `//TEST:SIMPLE` directive targeting
   Metal. Cross-target probes are reserved for the "Metal does X
   but sibling skips" subset and should add a second directive
   with a distinct label.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/target-pipelines/metal.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/llm-generated/pipeline/05-ir-passes.md`
- `docs/llm-generated/pipeline/06-emit.md`
- `docs/llm-generated/target-pipelines/index.md`
- `docs/llm-generated/cross-cutting/targets.md`

If you would cite anything else, stop and instead record a
doc-gap finding in `BUNDLE.md`.

## Source files you may consult for _verification only_

Use these to confirm a specific Metal emit token. Do **not** mine
them for claims that the doc does not state.

- `source/slang/slang-emit.cpp`
- `source/slang/slang-emit-metal.cpp`
- `source/slang/slang-emit-metal-prelude.cpp`
- `source/slang/slang-emit-c-like.cpp`
- `source/slang/slang-ir-metal-legalize.cpp`
- `source/slang/slang-ir-specialize-address-space.cpp`
- `source/slang/slang-ir-legalize-binary-operator.cpp`
- `source/slang/slang-ir-byte-address-legalize.cpp`
- `source/slang/slang-ir-legalize-image-subscript.cpp`
- `source/slang/slang-ir-explicit-global-context.cpp`

## Test directives

Default for this bundle:

```
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute
```

For a Metal-fires-vs-sibling-skips claim, add a second directive
with a distinct label and per-target CHECK prefix:

```
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl  -entry main -stage compute
```

## Lessons captured for the Metal target pipeline

These are in addition to `_common.md`.

- **`[[kernel]]`, `[[buffer(N)]]`, `[[thread_position_in_grid]]`
  and similar Metal attributes are FileCheck regex-variable
  references.** Match as bare substrings (e.g. `kernel`,
  `thread_position_in_grid`) — never as the literal `[[...]]`
  form. FileCheck reports `undefined variable: kernel` if you
  write `[[kernel]]` as a pattern.
- **Multiple `[[...]]` markers on one emitted line** must be
  checked with `// CHECK:` then `// CHECK-SAME:` for the
  follow-on tokens.
- **`[[buffer(N)]]` indices are positional**, not driven by
  `vk::binding(...)` or HLSL `register(uN)`. Two struct fields
  bound via `vk::binding(7)` and `vk::binding(3)` may emit as
  `[[buffer(0)]]` and `[[buffer(1)]]`. Don't assert
  binding-driven indices on Metal.
- **Slang renames the entry point** `main` → `main_0` on Metal.
  Tests that match the entry function name must use `main_0`
  (with the trailing `_0`).
- **The Metal prelude block is fixed:** `#include <metal_stdlib>`,
  `#include <metal_math>`, `#include <metal_texture>`,
  `using namespace metal;`. It is the first non-blank emitted
  text on every Metal compile.
- **Address spaces appear on pointer types**: `T device*` for
  read-write buffer storage, `T constant*` for constant-buffer
  parameter pointers, `threadgroup array<T, N>` for groupshared
  arrays, `thread T` for stack/local objects. Use these
  qualifiers in CHECK patterns when verifying address-space
  effects of `specializeAddressSpaceForMetal`.
- **`uniform` scalar globals are packed into a `GlobalParams_0`
  struct** that is materialized as a `constant*` parameter; the
  emitted parameter list reads
  `GlobalParams_0 constant* globalParams_1 [[buffer(N)]]`.
- **`cbuffer X { … }` becomes
  `SLANG_ParameterGroup_X_natural_<N>` (or similar)** with a
  trailing `_<N>` suffix; use `{{.*}}` wildcards for the suffix.
- **`float4x4` in a `cbuffer` becomes
  `_MatrixStorage_float4x4_…natural_<N>`** with an
  `array<float4, int(4)> data_0` field; match the type name with
  a `_MatrixStorage_{{.*}}` wildcard.
- **`Sampler2D` splits to
  `combined_texture_<N>` / `combined_sampler_<N>`** with
  `[[texture(N)]]` and `[[sampler(N)]]` respectively; the texture
  type is `texture2d<T, access::sample>`.
- **`RWTexture2D<T>` emits as `texture2d<T, access::read_write>`**;
  subscript-store lowers to `.write(value, coord)`, subscript-load
  lowers to `.read(coord)`.
- **`groupshared T arr[N]` emits as `threadgroup array<T, int(N)>`**
  declared at function scope inside the kernel.
- **`InterlockedAdd(buf[i], v, prev)` emits as
  `atomic_fetch_add_explicit((atomic_uint device*)(buf+offset), v,
  memory_order_relaxed)`**.
- **`ByteAddressBuffer.Load<uint>(off)` emits as
  `as_type<uint>(buf[(off)>>2])`** under the Metal options for
  `legalizeByteAddressBufferOps`.
- **`Color.Green` enum lowers to a bare integer literal** —
  `int(1)` etc. The enumerator name does not survive.
- **`int[4] foo(...)` survives as
  `array<int, int(4)> foo_0(...)`** on Metal because Metal is
  filtered out of `legalizeArrayReturnType`.
- **`if/else`-merged value uses an explicit local temporary**
  declared with the struct/scalar type, then assigned in each
  branch — this is the default `eliminatePhis` shape.
- **`asuint(float)` (bitcast) emits as `as_type<uint>(...)`**;
  the IR pass `lowerBitCast` is responsible.
- **`ParameterBlock<Resources>` emits as a single
  `<block-struct> constant*` parameter with `[[buffer(N)]]`**;
  inside the struct, resource-typed fields become raw
  `<element> device*` pointer fields (descriptor handles),
  courtesy of the `MetalParameterBlock` lowering policy.
- **`#line N "file"` directives appear by default in Metal emit**
  and may reference `core` or `hlsl.meta.slang` for core-module
  items; use `{{.*}}` for the file name.
- **DCE strips locally-unused code.** Always write computed values
  to a buffer (or use them in a side-effecting call), or the test
  pattern will fail to find them in the emit.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `target-pipelines/metal.md` (or one of the listed secondary
      docs).
- [ ] Each `.slang` file declares
      `//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute`
      as its primary directive. Add a sibling-target directive
      only for "Metal does X but sibling skips" claims.
- [ ] CHECK patterns avoid raw `[[...]]` and use FileCheck
      wildcards for mangled identifiers (`tid_{{[0-9]+}}` etc.).
- [ ] The entry function is matched as `main_0` (Metal rename),
      not `main`.
- [ ] No test depends on a GPU. Compute-only entry points; no
      DXR; no graphics pipeline stages requiring rasterization;
      no `MetalLib*` target (downstream Apple toolchain is out of
      scope).
- [ ] `## Doc gaps observed` records claims that lack a checkable
      marker in the doc, or behaviors observed in slangc emit
      that the doc does not currently describe (e.g. the
      AppendStructuredBuffer Metal-compute rejection).
- [ ] `## Out of scope (no-GPU runner)` enumerates the
      Apple-toolchain / `MetalLib*` / DXR / mesh / subpass items.
