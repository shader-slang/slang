# Prompt: docs/generated/tests/target-pipelines/wgsl/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/target-pipelines/wgsl/`,
anchored to
[`docs/generated/design/target-pipelines/wgsl.md`](../../../design/target-pipelines/wgsl.md).

Audience: nightly CI. The bundle exercises the **WGSL target
pipeline**: the ordered IR-pass + emit sequence run by
`linkAndOptimizeIR` and `emitEntryPointsSourceFromIR` when
`CodeGenTarget::WGSL` (or the downstream `WGSLSPIRV` /
`WGSLSPIRVAssembly` reductions) is requested. The doc enumerates
four phases (A: link and entry-point prep; B: specialization and
type legalization; C: WGSL legalization, lowering, phi
elimination; D: WGSL emit + Tint downstream). The defining
characteristic of this bundle is that almost every claim is
observable in **emitted WGSL text** — Tint is out of band and not
invoked by `-target wgsl`.

This bundle is therefore **single-target-heavy by design**: the
default test directive is

```
//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute
```

The bundle stays compute-only because the no-GPU CI runner has no
way to exercise rasterization, ray tracing, or runtime WGSL
execution (Tint / Dawn).

## The translation rule: claims to observations

`target-pipelines/wgsl.md` describes:

- The ordered `SLANG_PASS` sequence for WGSL in
  `linkAndOptimizeIR`.
- WGSL-specific gates: `legalizeIRForWGSL`,
  `specializeAddressSpaceForWGSL`,
  `lowerBufferElementTypeToStorageType` with policy `WGSL`,
  `legalizeByteAddressBufferOps` with the WGSL flag set
  (`scalarizeVectorLoadStore`, `useBitCastFromUInt`, etc.),
  `legalizeLogicalAndOr` (WGSL is in the
  `isD3DTarget || isKhronosTarget || isWGPUTarget || isMetalTarget`
  arm), `floatNonUniformResourceIndex`,
  `eliminatePhis` with **default options** (contrast with SPIR-V
  which sets non-default options).
- WGSL-specific skips: `lowerAppendConsumeStructuredBuffers` runs
  (the type is not in core WGSL but the runtime rejects the
  shader before emit, so the bundle does not test this), the
  HLSL-only `legalizeNonVectorCompositeSelect` /
  `wrapStructuredBuffersOfMatrices` arms, Metal's
  `legalizeEmptyTypes` Metal-only branch, the SPIR-V
  `legalizeIRForSPIRV` driver.
- Always-on emit shapes: `@compute`, `@workgroup_size(X, Y, Z)`,
  `@binding(N) @group(N)`, `var<storage, read_write>`,
  `var<storage, read>`, `var<uniform>`, `var<workgroup>`,
  `var<private>`, `fn main(@builtin(...) ... )`, the per-field
  `@align(N)` annotations on `std140`/`std430`-shaped structs, the
  `_MatrixStorage_<spelling>_ColMajorstd430` wrapper for matrices
  inside a structured buffer.
- **No `#line` directives.** WGSL emit sets
  `LineDirectiveMode::None` because WGSL has no `#line` directive.
- Downstream: WGSL text is the artifact endpoint for
  `CodeGenTarget::WGSL`; Tint is invoked only for
  `CodeGenTarget::WGSLSPIRV` / `WGSLSPIRVAssembly` and is not
  exercised by `-target wgsl`.

### Observable claims (write tests for these)

- **`[numthreads(X,Y,Z)]` becomes `@workgroup_size(X, Y, Z)`** with
  a leading `@compute` attribute on the entry-point `fn main(...)`.
  (Anchor: `#phase-d-wgsl-emit-and-downstream-tools`.)
- **`@binding(N) @group(N)` annotations** appear on every global
  resource: `RWStructuredBuffer` (`var<storage, read_write>`),
  `StructuredBuffer` (`var<storage, read>`), `ConstantBuffer`
  (`var<uniform>`), `Texture2D` (`var ... : texture_2d<...>`),
  `SamplerState` (`var ... : sampler`). (Anchor:
  `#phase-d-wgsl-emit-and-downstream-tools` and `#legalizeirforwgsl`.)
- **`groupshared T x[N]` becomes `var<workgroup> x_<N> : array<T, N>`**
  — the `workgroup` address-space marker is selected by
  `specializeAddressSpaceForWGSL`. (Anchor:
  `#specializeaddressspaceforwgsl`.)
- **`static int x = E;` at module scope becomes
  `var<private> x_<N> : T`** _and_ the initializer fires at entry
  to `main`. (`moveGlobalVarInitializationToEntryPoints` plus the
  `private` address-space selection.)
  (Anchor: `#phase-c-wgsl-legalization-lowering-phi-elimination`
  and `#specializeaddressspaceforwgsl`.)
- **`legalizeLogicalAndOr` rewrites `a && b` over vector
  operands** into a WGSL `select(...)` call (or per-element
  selects), because WGSL only allows `&&` on scalar booleans.
  (Anchor: `#legalizelogicalandor`.)
- **Scalar `a && b` becomes a `var ... : bool` with explicit
  `if/else` assignment** because `eliminatePhis` (with default
  options) introduces a function-local temporary in place of the
  phi. (Anchor: `#eliminatephis-with-default-options`.)
- **`SV_DispatchThreadID` becomes
  `@builtin(global_invocation_id) <name> : vec3<u32>`** on the
  entry function — this is `legalizeEntryPointVaryingParamsForWGSL`
  inside `legalizeIRForWGSL`. (Anchor: `#legalizeirforwgsl`.)
- **`SV_GroupThreadID` becomes
  `@builtin(local_invocation_id)`** as a sibling entry parameter.
  (Anchor: `#legalizeirforwgsl`.)
- **`enum` is lowered to its underlying integer literal** —
  `Color.Green` emits as `i32(1)` (or `u32(1)`), not the
  enumerator name. (Anchor:
  `#phase-a-link-and-entry-point-prep`.)
- **`int[N] foo()` is rewritten to take an
  `out` parameter (`ptr<function, array<T, N>>`)** because WGSL
  forbids array return values. `legalizeArrayReturnType` fires for
  WGSL. (Anchor:
  `#phase-c-wgsl-legalization-lowering-phi-elimination`.)
- **`asuint(f)` / `asfloat(u)` emit as
  `bitcast<u32>(...)` / `bitcast<f32>(...)`** — the WGSL spelling
  for a reinterpret. (Anchor:
  `#phase-c-wgsl-legalization-lowering-phi-elimination` —
  `lowerBitCast`.)
- **`RWStructuredBuffer<float4x4>` wraps the matrix element as a
  `_MatrixStorage_float4x4_ColMajorstd430_<N>` struct** with a
  `data_<N> : array<vec4<f32>, i32(4)>` field. This is
  `lowerBufferElementTypeToStorageType` with WGSL policy.
  (Anchor:
  `#phase-c-wgsl-legalization-lowering-phi-elimination`.)
- **`ByteAddressBuffer.Load<uint>(...)` lowers to an
  `array<u32>` index expression divided by 4** — WGSL has no
  byte-address buffer, so the WGSL options drive a
  `[(offset)/4]` shape, with bit-cast spellings where needed.
  (Anchor: `#legalizebyteaddressbufferops-with-wgsl-options`.)
- **`ConstantBuffer<S>` emits as `var<uniform> cb_<N> :
  S_std140_<N>`** — a `std140`-shaped struct with `@align(...)`
  on every field. (Anchor:
  `#phase-d-wgsl-emit-and-downstream-tools`.)
- **`Sampler2D` splits into a separate texture + sampler pair**
  named `<base>_texture_<N>` and `<base>_sampler_<N>` —
  `lowerCombinedTextureSamplers` fires for WGSL. (Anchor:
  `#phase-b-specialization-and-type-legalization`.)
- **`Atomic<uint>` becomes `atomic<u32>`** inside the buffer
  element type; `.add(...)` lowers to `atomicAdd(&(buf[i]), v)`.
  (Anchor: `#phase-d-wgsl-emit-and-downstream-tools`.)
- **No `#line` directives in WGSL emit** — WGSL has no `#line`
  syntax, so `emitEntryPointsSourceFromIR` selects
  `LineDirectiveMode::None`. (Anchor:
  `#phase-d-wgsl-emit-and-downstream-tools`.)
- **WGSL entry function is named `fn main(...)`** — Slang does not
  rename the entry point on WGSL. (Anchor:
  `#phase-d-wgsl-emit-and-downstream-tools`.)
- **`-target wgsl` stops at WGSL text** — no Tint invocation, no
  SPIR-V byte stream. (Anchor: `#downstream-tint`.)
- **WGSL has no iterative passes in `linkAndOptimizeIR`** — the
  consequence is observable indirectly: the pipeline is "run once"
  and the emit is deterministic. We record this in the
  "Out of scope" section rather than write a `slangc`-text test.
  (Anchor: `#loops-in-the-pipeline`.)

### Not testable through `slangc -target wgsl` (record under
`## Untested claims`)

- **Tint invocation / WGSLSPIRV path.** `-target wgsl` stops at
  WGSL text. The downstream Tint binary is exercised only by
  `-target wgsl-spirv` and is not on the no-GPU runner.
- **Pass-ordering claims inside Phase A/B/C** beyond what is
  visible in emitted text. Pass _existence_ is observable;
  intra-phase _ordering_ requires `-dump-ir` annotations that the
  doc does not anchor to a specific marker.
- **`AppendStructuredBuffer<T>` / `ConsumeStructuredBuffer<T>`.**
  The frontend rejects these types in a WGSL compute entry point
  with `E36107: unavailable features in entry point`. The doc
  notes `lowerAppendConsumeStructuredBuffers` runs for WGSL, but
  the no-front-end-rejected path is not reachable from a `.slang`
  source.
- **`InterlockedAdd` and HLSL-style atomic intrinsics.** Slang
  rejects these for WGSL — use `Atomic<T>` instead.
- **`legalizeUniformBufferLoad`, `invertYOfPositionOutput`,
  `rcpWOfPositionInput`.** Khronos / HLSL only.
- **`legalizeEntryPointsForGLSL`, `legalizeImageSubscript`,
  `legalizeConstantBufferLoadForGLSL`, etc.** GLSL/SPIR-V only.
- **`collectCooperativeMetadata`.** Requires the cooperative
  matrix or vector capability set.
- **WGSL has no iterative passes (zero loops).** A textual claim,
  not directly testable.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. 15 to 25 `.slang` test files. Bundle is WGSL-target-focused,
   so the typical file carries one `//TEST:SIMPLE` directive
   targeting WGSL.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/target-pipelines/wgsl.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/generated/design/pipeline/05-ir-passes.md`
- `docs/generated/design/pipeline/06-emit.md`
- `docs/generated/design/target-pipelines/index.md`
- `docs/generated/design/cross-cutting/targets.md`

If you would cite anything else, stop and instead record a
doc-gap finding in `README.md`.

## Source files you may consult for _verification only_

Use these to confirm a specific WGSL emit token. Do **not** mine
them for claims that the doc does not state.

- `source/slang/slang-emit.cpp`
- `source/slang/slang-emit-wgsl.cpp`
- `source/slang/slang-emit-c-like.cpp`
- `source/slang/slang-ir-wgsl-legalize.cpp`
- `source/slang/slang-ir-legalize-varying-params.cpp`
- `source/slang/slang-ir-byte-address-legalize.cpp`
- `source/slang/slang-ir-legalize-binary-operator.cpp`
- `source/slang/slang-ir-specialize-address-space.cpp`

## Test directives

Default for this bundle:

```
//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute
```

## Lessons captured for the WGSL target pipeline

These are in addition to `_common.md`:

- **`@workgroup_size(N, M, K)` always emits with spaces** between
  arguments (`@workgroup_size(8, 4, 2)`, not `8,4,2`).
- **Resource binding indices are sequential from 0** and follow
  declaration order; `@binding(0) @group(0)`, `@binding(1)
  @group(0)`, etc. Do not assert a specific binding index unless
  the resource is the only one declared.
- **Slang renames every user identifier with a `_<N>` suffix** on
  WGSL emit (e.g. `outBuf` → `outBuf_0`, `tid` → `tid_0`,
  `i` → `i_0`). Use FileCheck wildcards: `outBuf_{{[0-9]+}}`.
- **`int` constants emit as `i32(N)`, `uint` as `u32(N)`,
  `float` as `<value>f`.** The constructor-style spelling is
  uniform across the WGSL output.
- **`vec3<u32>` is the type spelling for `uint3`**; `vec2<f32>`
  for `float2`; `vec4<f32>` for `float4`. WGSL has no `floatN` /
  `uintN` shorthand.
- **`array<T, i32(N)>` (or `array<T, N>` depending on context) is
  the WGSL array-type spelling.** Treat the size literal as
  variable: match `array<f32, {{.*}}>`.
- **`bitcast<T>(expr)` is the WGSL spelling for reinterpret.**
  `asuint(f)` and `asfloat(u)` both lower through this opcode.
- **`select(false_value, true_value, cond)` is WGSL's vector
  ternary.** `a && b` on `vec3<bool>` lowers to `select(...)`
  with the false value first, true value second, condition third.
- **WGSL does not emit `#line` directives** — `LineDirectiveMode`
  is `None`. There is no `#line` token in the output.
- **`Atomic<T>` becomes `atomic<u32>` / `atomic<i32>`** inside the
  buffer element. Member calls (`.add(v)`) become
  `atomicAdd(&(buf[i]), v)`.
- **DCE strips locally-unused code** before WGSL emit. Always
  write a computed value to a `RWStructuredBuffer` parameter to
  observe it in the output.
- **`AppendStructuredBuffer` / `ConsumeStructuredBuffer` /
  HLSL-style `InterlockedAdd` are front-end-rejected on WGSL.**
  Don't write tests that use them.
- **WGSL's `&&` over scalars** lowers to an `if/else` with a
  function-local `var : bool` — that's `eliminatePhis` with
  default options.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `target-pipelines/wgsl.md` (or one of the listed secondary
      docs).
- [ ] Each `.slang` file declares
      `//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute`
      as its primary directive.
- [ ] CHECK patterns avoid raw `[[...]]` and use FileCheck
      wildcards for mangled identifiers.
- [ ] No test depends on a GPU. Compute-only entry points; no
      DXR; no graphics pipeline stages requiring rasterization.
- [ ] `## Doc gaps observed` records claims that lack a checkable
      marker in the doc, or behaviors observed in slangc emit
      that the doc does not currently describe.
- [ ] `## Untested claims` enumerates the Tint /
      WGSLSPIRV / append-consume-buffer / interlocked-add items.
