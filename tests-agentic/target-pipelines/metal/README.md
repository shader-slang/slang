---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T00:00:00Z
source_commit: 2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
watched_paths_digest: 346898dc4d064ddc3802d3a68b0195bea085b7a106cf552f607ca57a1f3da4ec
source_doc: docs/llm-generated/target-pipelines/metal.md
source_doc_digest: 4a7697c3667a9f1364743d589b96dbb7f0bbed4738f28a74cfefbb3153e47d62
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for target-pipelines/metal

## Intent

Tests verify the Metal target pipeline described in
[`docs/llm-generated/target-pipelines/metal.md`](../../../docs/llm-generated/target-pipelines/metal.md):
the ordered IR-pass + emit sequence executed when
`CodeGenTarget::Metal` is the target. The bundle exercises:

- the Metal text prelude (`#include <metal_stdlib>`,
  `#include <metal_math>`, `#include <metal_texture>`,
  `using namespace metal;`);
- the entry-point shape on Metal: `[[kernel]]` attribute, `main`
  renamed to `main_0`, the absence of an HLSL `[numthreads(...)]`
  attribute on the emitted function;
- the entry-point varying-param legalization done by
  `legalizeIRForMetal` /
  `legalizeEntryPointVaryingParamsForMetal`:
  `SV_DispatchThreadID` → `uint3 [[thread_position_in_grid]]`,
  `SV_GroupIndex` synthesized from
  `[[thread_position_in_threadgroup]]`;
- the positional `[[buffer(N)]]` / `[[texture(N)]]` /
  `[[sampler(N)]]` binding attributes (independent slot spaces,
  not driven by HLSL `register()` or Vulkan `vk::binding`);
- the Metal address-space annotations
  (`device` / `constant` / `threadgroup`) injected by
  `specializeAddressSpaceForMetal`;
- Phase-B `wrapCBufferElementsForMetal` (wrap `float4x4` in
  `_MatrixStorage_…natural` and wrap the outer cbuffer in
  `SLANG_ParameterGroup_…`);
- the `MetalParameterBlock` policy for
  `lowerBufferElementTypeToStorageType` on
  `ParameterBlock<Resources>` — resource-typed fields become
  raw `T device*` descriptor-handle pointer fields, and the
  block binds as a single `constant*` parameter;
- `lowerCombinedTextureSamplers` (HLSL/Metal/WGSL arm) splitting
  `Sampler2D` into `combined_texture_<N>` /
  `combined_sampler_<N>` with separate `[[texture]]` /
  `[[sampler]]` attributes;
- Phase-C IR-pass effects:
  `legalizeByteAddressBufferOps` with Metal-specific options
  (`as_type<T>` bitcast wrappers around pointer-indexed loads);
  `validateAtomicOperations` (`InterlockedAdd` →
  `atomic_fetch_add_explicit(..., memory_order_relaxed)`);
  `legalizeImageSubscript` (RWTexture2D `[ ]= ` →
  `.write(value, coord)` / `.read(coord)`);
  `legalizeLogicalAndOr` Metal arm (vector `&&` preserved);
  `lowerBitCast` (`asuint(f)` → `as_type<uint>(f)`);
  `moveGlobalVarInitializationToEntryPoints` /
  `introduceExplicitGlobalContext` via the Metal fallthrough
  (uniforms packed into `GlobalParams`, static-globals
  initialized inside the kernel);
  `eliminatePhis` with default options (if/else write-back to
  local for both scalars and structs); `lowerEnumType` (enum
  collapses to bare integer);
- the array-return survival contrast: Metal keeps
  `array<T, N> foo(...)` because `legalizeArrayReturnType` is
  filtered out for Metal — observed both as a Metal-only check
  and a side-by-side Metal-vs-HLSL emit;
- the Metal emit shape: `SourceWriter` writes `#line N "<file>"`
  directives by default; no SPIR-V-style identification comment
  exists in the prelude; `-target metal` stops at Metal text
  (no `.metallib` artifact, no Apple-toolchain invocation).

Coverage strategy: one positive test per concrete claim that can
be observed in `slangc -target metal` text. Default directive
is
`//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute`.
One cross-target test (`array-return-metal-vs-hlsl-rewrites.slang`)
adds a second `-target hlsl` directive to demonstrate the
filtered-out-pass contrast. `MetalLib` / `MetalLibAssembly`
require the Apple `metal` command-line tool and are out of
scope on the no-GPU runner.

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| Bare `-target metal` stops at Metal text; the Apple `metal` downstream compiler is invoked only for MetalLib/MetalLibAssembly, so the emitted output is plain MSL source (no .metallib bytes). | negative | [#downstream-apple-metal-compiler](../../../docs/llm-generated/target-pipelines/metal.md#downstream-apple-metal-compiler) | [`downstream-stops-at-text-no-metallib.slang`](downstream-stops-at-text-no-metallib.slang) |
| Conditional struct selection (non-vector composite select) lowers to an if/else block assigning a local of the struct type; the HLSL-only legalizeNonVectorCompositeSelect arm does not run for Metal but eliminatePhis already produces the same shape. | functional | [#eliminatephis-with-default-options](../../../docs/llm-generated/target-pipelines/metal.md#eliminatephis-with-default-options) | [`composite-select-via-eliminate-phis.slang`](composite-select-via-eliminate-phis.slang) |
| Stress: deeply nested if/else chains (>= 5 levels) reduce through default-option `eliminatePhis` to per-branch assignments to a single function-local variable. | stress | [#eliminatephis-with-default-options](../../../docs/llm-generated/target-pipelines/metal.md#eliminatephis-with-default-options) | [`deeply-nested-control-flow-stress.slang`](deeply-nested-control-flow-stress.slang) |
| `eliminatePhis` runs with default options on Metal: an if/else-merged value becomes a function-local variable assigned in each branch. | functional | [#eliminatephis-with-default-options](../../../docs/llm-generated/target-pipelines/metal.md#eliminatephis-with-default-options) | [`eliminate-phis-default-options.slang`](eliminate-phis-default-options.slang) |
| RWTexture2D<T> emits as texture2d<T, access::read_write> and legalizeImageSubscript rewrites subscript-store into .write() and subscript-load into .read(). | functional | [#legalizeimagesubscript](../../../docs/llm-generated/target-pipelines/metal.md#legalizeimagesubscript) | [`rwtexture2d-read-write-and-image-subscript.slang`](rwtexture2d-read-write-and-image-subscript.slang) |
| RWTexture3D<float4> emits as metal::texture3d<float, access::read_write> and legalizeImageSubscript rewrites subscript-store into .write() / subscript-load into .read(). | expansion | [#legalizeimagesubscript](../../../docs/llm-generated/target-pipelines/metal.md#legalizeimagesubscript) | [`rwtexture3d-emit.slang`](rwtexture3d-emit.slang) |
| SV_DispatchThreadID lowers to a uint3 entry-point parameter carrying the Metal [[thread_position_in_grid]] system-value attribute. | functional | [#legalizeirformetal](../../../docs/llm-generated/target-pipelines/metal.md#legalizeirformetal) | [`sv-dispatch-thread-id-attribute.slang`](sv-dispatch-thread-id-attribute.slang) |
| SV_GroupIndex is synthesized from `[[thread_position_in_threadgroup]]` because Metal does not provide a single builtin for the linear in-group index; Slang computes it in the kernel body. | functional | [#legalizeirformetal](../../../docs/llm-generated/target-pipelines/metal.md#legalizeirformetal) | [`sv-group-index-synthesized.slang`](sv-group-index-synthesized.slang) |
| ParameterBlock<T> uses the MetalParameterBlock lowering policy: resource-typed fields become descriptor handles (raw `T device*` pointer fields) and the block binds as one `constant*` parameter. | functional | [#metal-specific-lowerbufferelementtypetostoragetype](../../../docs/llm-generated/target-pipelines/metal.md#metal-specific-lowerbufferelementtypetostoragetype) | [`parameter-block-metal-policy.slang`](parameter-block-metal-policy.slang) |
| Boundary: an enum whose enumerator value is signed `int` MAX (2147483647) collapses through `lowerEnumType` to the integer literal in Metal text — the enumerator name (`MaxVal`) does not survive. | boundary | [#phase-a-link-and-entry-point-prep](../../../docs/llm-generated/target-pipelines/metal.md#phase-a-link-and-entry-point-prep) | [`enum-with-int-max-collapses-to-literal.slang`](enum-with-int-max-collapses-to-literal.slang) |
| lowerEnumType collapses an enum to its underlying integer; the emitted Metal text contains the integer literal (int(1)) rather than the enumerator name. | functional | [#phase-a-link-and-entry-point-prep](../../../docs/llm-generated/target-pipelines/metal.md#phase-a-link-and-entry-point-prep) | [`enum-lowers-to-integer-literal.slang`](enum-lowers-to-integer-literal.slang) |
| Negative (doc-gap pair): `AppendStructuredBuffer<T>` on the Metal compute stage is rejected by `checkEntryPointDecorations` with diagnostic E36107, so `lowerAppendConsumeStructuredBuffers` (which the doc says fires for Metal) never produces observable text in compute emit. | negative | [#phase-b-specialization-and-type-legalization](../../../docs/llm-generated/target-pipelines/metal.md#phase-b-specialization-and-type-legalization) | [`append-structured-buffer-rejected-on-metal.slang`](append-structured-buffer-rejected-on-metal.slang) |
| Sampler2D (combined texture+sampler) lowers to separate texture2d<T,sample>/sampler parameter pair with [[texture(N)]] and [[sampler(N)]]. | functional | [#phase-b-specialization-and-type-legalization](../../../docs/llm-generated/target-pipelines/metal.md#phase-b-specialization-and-type-legalization) | [`combined-sampler-splits-to-texture-and-sampler.slang`](combined-sampler-splits-to-texture-and-sampler.slang) |
| Boundary: asfloat(uint) renders as as_type<float>(...) on Metal (bitcast direction uint->float, dual of asuint). | boundary | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination) | [`bitcast-uint-to-float.slang`](bitcast-uint-to-float.slang) |
| Boundary: chained `asfloat(asuint(...))` round-trip emits nested `as_type<uint>(as_type<float>(...))` wrappers — Slang preserves both directions through Metal lowering. | boundary | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination) | [`asuint-float-bitcast-direction.slang`](asuint-float-bitcast-direction.slang) |
| ByteAddressBuffer.Load<T>/Store<T> lower under Metal's legalizeByteAddressBufferOps options (scalarizeVectorLoadStore=true, lowerBasicTypeOps=true) into pointer-indexed accesses with as_type<T> bitcasts. | functional | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination) | [`byte-address-buffer-load-store-lowering.slang`](byte-address-buffer-load-store-lowering.slang) |
| Cross-target negative observation: Metal keeps an array-return function as `array<T, N> foo(...)` (legalizeArrayReturnType skipped), while HLSL rewrites the function to `void` with an `out T[N]` parameter. | functional | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination) | [`array-return-metal-vs-hlsl-rewrites.slang`](array-return-metal-vs-hlsl-rewrites.slang) |
| InterlockedAdd lowers to Metal's atomic_fetch_add_explicit with memory_order_relaxed and an (atomic_uint device*) cast. | functional | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination) | [`atomic-fetch-add-explicit-lowering.slang`](atomic-fetch-add-explicit-lowering.slang) |
| `uniform` scalar/struct globals are packed into a GlobalParams struct that is passed as a `constant*` parameter via introduceExplicitGlobalContext (Metal fallthrough chain). | functional | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination) | [`uniform-globals-packed-into-globalparams.slang`](uniform-globals-packed-into-globalparams.slang) |
| legalizeArrayReturnType is filtered out for Metal (`!isMetalTarget && !isSPIRV`), so an array-return function survives as `array<T, N> foo(...)` rather than being rewritten to void with an out parameter. | functional | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination) | [`array-return-type-survives.slang`](array-return-type-survives.slang) |
| legalizeLogicalAndOr (Metal arm) preserves `&&` over vector operands as element-wise vector AND in the emitted MSL. | functional | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination) | [`vector-logical-and-survives.slang`](vector-logical-and-survives.slang) |
| lowerBitCast renders Slang's asuint(float) bitcast as Metal's as_type<uint>(...). | functional | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination) | [`bitcast-emits-as-type.slang`](bitcast-emits-as-type.slang) |
| Boundary: `1.0 / 0.0` is not constant-folded; the Metal emitter renders it as a literal division so the runtime can produce +infinity per IEEE 754. | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`float-positive-infinity-divide.slang`](float-positive-infinity-divide.slang) |
| Boundary: a second SamplerState in declaration order receives `[[sampler(1)]]`; the sampler slot space progresses independently of `[[texture(N)]]` and `[[buffer(N)]]`. | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`sampler-binding-index-one.slang`](sampler-binding-index-one.slang) |
| Boundary: even when both textures carry the same `[[vk::binding(0,0)]]` annotation, Metal still emits positional `[[texture(0)]]` and `[[texture(1)]]` indices in declaration order (Vulkan-style bindings do not drive Metal slots). | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`vk-binding-ignored-positional-wins.slang`](vk-binding-ignored-positional-wins.slang) |
| Boundary: half-precision values survive end-to-end: `half` typed buffer and uniform produce `half device*` and `half v_0` fields in the emitted Metal text. | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`half-precision-boundary-survives.slang`](half-precision-boundary-survives.slang) |
| Boundary: literal `uint(0xFFFFFFFF) + 1u` is constant-folded by the Metal pipeline to `0U` in the emitted text, demonstrating wrap-on-overflow at the unsigned MAX edge. | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`uint-max-plus-one-wraps-to-zero.slang`](uint-max-plus-one-wraps-to-zero.slang) |
| Boundary: maximum threadgroup size `[numthreads(1024,1,1)]` is accepted and emits standard `[[kernel]]` text without a `numthreads(...)` attribute. | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`numthreads-1024-one-one-emits-kernel.slang`](numthreads-1024-one-one-emits-kernel.slang) |
| Boundary: minimum threadgroup size `[numthreads(1,1,1)]` is accepted and still emits the standard `[[kernel]] void main_0` shape (no `numthreads` attribute survives in Metal text). | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`numthreads-one-one-one-still-emits-kernel.slang`](numthreads-one-one-one-still-emits-kernel.slang) |
| Boundary: signed `int` MAX literal (2147483647) survives constant-folding intact when combined with a runtime value and emits verbatim in the Metal text. | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`int-max-literal-emits-verbatim.slang`](int-max-literal-emits-verbatim.slang) |
| Boundary: signed `int` MIN (encoded as `int(0x80000000)`) emits as the negative decimal `-2147483648` in Metal text. | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`int-min-literal-emits-negative.slang`](int-min-literal-emits-negative.slang) |
| Boundary: two textures share one sampler — texture indices reach `[[texture(0)]]` and `[[texture(1)]]` while the sampler stays at `[[sampler(0)]]` in its independent slot space. | boundary | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`texture-and-sampler-paired-at-zero.slang`](texture-and-sampler-paired-at-zero.slang) |
| Metal compute entry points carry the [[kernel]] attribute (matched as bare 'kernel' due to FileCheck regex-variable rules). | functional | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`kernel-attribute-on-entry.slang`](kernel-attribute-on-entry.slang) |
| Metal emit begins with the standard <metal_stdlib>/<metal_math>/<metal_texture> includes and a `using namespace metal;` line. | functional | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`prelude-metal-stdlib-include.slang`](prelude-metal-stdlib-include.slang) |
| Metal expresses the threadgroup size at dispatch time, so `[numthreads(X,Y,Z)]` does not survive as a `numthreads(...)` attribute in the emitted MSL (contrast with HLSL). | negative | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`numthreads-not-an-emitted-attribute.slang`](numthreads-not-an-emitted-attribute.slang) |
| Multiple texture variants of different ranks (Texture1D / Texture3D / TextureCube / Texture2DArray) all share the positional [[texture(N)]] slot space and bind independently of buffers. | expansion | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`texture2darray-binding-multi.slang`](texture2darray-binding-multi.slang) |
| Negative: `[numthreads(0,1,1)]` is rejected with diagnostic E31102 ("expected a positive integer in 'numthreads' attribute, got '0'") before Metal lowering ever runs. | negative | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`numthreads-zero-x-rejected.slang`](numthreads-zero-x-rejected.slang) |
| RWStructuredBuffer<T> emits as a `T device*` pointer parameter with `[[buffer(0)]]` positional binding. | functional | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`buffer-binding-positional-zero.slang`](buffer-binding-positional-zero.slang) |
| Slang renames the user entry point 'main' to 'main_0' on Metal (contrast with HLSL, which preserves 'main'). | functional | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`entry-point-renamed-to-main-zero.slang`](entry-point-renamed-to-main-zero.slang) |
| SourceWriter emits `#line N "file"` directives by default in Metal output so downstream tools can map errors back to the original source. | functional | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`line-directive-emitted.slang`](line-directive-emitted.slang) |
| Stress: a function-local `int arr[256]` survives Phase B/C array-specialization and emits as `array<int, int(256)>` in the kernel. | stress | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`large-local-array-256-stress.slang`](large-local-array-256-stress.slang) |
| Stress: with 16 buffer parameters the Metal emitter still assigns sequential positional `[[buffer(N)]]` indices up through 15 (no Metal slot-budget compression). | stress | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`buffer-binding-index-fifteen.slang`](buffer-binding-index-fifteen.slang) |
| Stress: with 8+ buffer parameters the Metal emitter still assigns positional `[[buffer(N)]]` indices and produces a KernelContext struct holding all of them. | stress | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`buffer-binding-eight-buffer-pressure.slang`](buffer-binding-eight-buffer-pressure.slang) |
| Texture parameter bindings use Metal's `[[texture(N)]]` positional attribute, independent of any buffer slots. | functional | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`texture-binding-positional-zero.slang`](texture-binding-positional-zero.slang) |
| Texture1D<float> emits as metal::texture1d<float, access::sample> with a positional [[texture(N)]] binding. | expansion | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`texture1d-emit.slang`](texture1d-emit.slang) |
| Texture2D<T> and SamplerState declared separately survive as separate texture2d<T,sample> and sampler parameters on Metal. | functional | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`separate-texture2d-and-sampler-state.slang`](separate-texture2d-and-sampler-state.slang) |
| Texture2DArray<float4> emits as metal::texture2d_array<float, access::sample> with a positional [[texture(N)]] binding. | expansion | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`texture2darray-emit.slang`](texture2darray-emit.slang) |
| Texture3D SampleLevel lowers to the Metal `.sample(samp, coord, level(lod))` call shape. | expansion | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`texture3d-samplelevel-emit.slang`](texture3d-samplelevel-emit.slang) |
| Texture3D<float4> emits as metal::texture3d<float, access::sample> with a positional [[texture(N)]] binding. | expansion | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`texture3d-emit.slang`](texture3d-emit.slang) |
| TextureCube SampleGrad lowers to the Metal `.sample(samp, dir, gradientcube(dPdx, dPdy))` call shape. | expansion | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`texturecube-samplegrad-emit.slang`](texturecube-samplegrad-emit.slang) |
| TextureCube<float4> emits as metal::texturecube<float, access::sample> with a positional [[texture(N)]] binding. | expansion | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`texturecube-emit.slang`](texturecube-emit.slang) |
| The Metal source emitter does not begin with a `; SPIR-V` style identification comment; the prelude is purely the `#include` block followed by `using namespace metal;`. | negative | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`no-spirv-style-prelude-comment.slang`](no-spirv-style-prelude-comment.slang) |
| With multiple StructuredBuffer params Metal emits sequential positional [[buffer(N)]] indices (0, 1, 2 ...), not driven by HLSL register() or vk::binding(). | functional | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools) | [`buffer-binding-positional-multi.slang`](buffer-binding-positional-multi.slang) |
| Boundary: a groupshared array of vector type (`float4`) still emits with `threadgroup` address space on `array<float4, int(8)>`. | boundary | [#specializeaddressspaceformetal](../../../docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal) | [`groupshared-float4-vector-threadgroup.slang`](groupshared-float4-vector-threadgroup.slang) |
| Boundary: a scalar `groupshared int` element is annotated with the `threadgroup` address space (not just arrays of `groupshared`). | boundary | [#specializeaddressspaceformetal](../../../docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal) | [`groupshared-scalar-threadgroup-pointer.slang`](groupshared-scalar-threadgroup-pointer.slang) |
| Boundary: an array of a user struct stored as groupshared emits with the `threadgroup` address space on the array pointer — `specializeAddressSpaceForMetal` reaches user-defined element types. | boundary | [#specializeaddressspaceformetal](../../../docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal) | [`groupshared-struct-array-threadgroup.slang`](groupshared-struct-array-threadgroup.slang) |
| Boundary: declaring multiple `groupshared` variables of different shapes (scalar, array, vector) — each independently picks up the `threadgroup` address space annotation. | boundary | [#specializeaddressspaceformetal](../../../docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal) | [`multiple-groupshared-vars-threadgroup-each.slang`](multiple-groupshared-vars-threadgroup-each.slang) |
| Stress: a 256-element `groupshared` array still gets the `threadgroup` address space annotation and emits as `array<int, int(256)> threadgroup*`. | stress | [#specializeaddressspaceformetal](../../../docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal) | [`groupshared-array-256-stress.slang`](groupshared-array-256-stress.slang) |
| Structured-buffer storage pointers carry Metal's `device` address space; constant-buffer pointers carry `constant`. Both come from specializeAddressSpaceForMetal. | functional | [#specializeaddressspaceformetal](../../../docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal) | [`device-address-space-on-buffer-pointer.slang`](device-address-space-on-buffer-pointer.slang) |
| `groupshared T[N]` emits with Metal's `threadgroup` address space as `threadgroup array<T, int(N)>` thanks to specializeAddressSpaceForMetal. | functional | [#specializeaddressspaceformetal](../../../docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal) | [`groupshared-threadgroup-address-space.slang`](groupshared-threadgroup-address-space.slang) |
| moveGlobalVarInitializationToEntryPoints (Metal fallthrough) fires the initializer of a `static` module-scope variable inside the kernel; the global itself appears as a field of the kernel-context struct. | functional | [#undoparametercopy-transformparamstoconstref](../../../docs/llm-generated/target-pipelines/metal.md#undoparametercopy-transformparamstoconstref) | [`static-global-init-moves-to-entry-point.slang`](static-global-init-moves-to-entry-point.slang) |
| Boundary: a `column_major float4x4` inside a cbuffer triggers the `_MatrixStorage_float4x4_ColMajornatural_<N>` wrapper inside `SLANG_ParameterGroup_M_natural_<M>`. | boundary | [#wrapcbufferelementsformetal](../../../docs/llm-generated/target-pipelines/metal.md#wrapcbufferelementsformetal) | [`cbuffer-column-major-matrix-storage.slang`](cbuffer-column-major-matrix-storage.slang) |
| Boundary: a `row_major float4x4` inside a cbuffer is passed through `wrapCBufferElementsForMetal` and emits as a bare `matrix<float,int(4),int(4)>` field inside the `SLANG_ParameterGroup_M_0` wrapper (no MatrixStorage wrapper layer in this orientation path). | boundary | [#wrapcbufferelementsformetal](../../../docs/llm-generated/target-pipelines/metal.md#wrapcbufferelementsformetal) | [`cbuffer-row-major-matrix-storage.slang`](cbuffer-row-major-matrix-storage.slang) |
| Boundary: two `cbuffer` blocks each receive their own `SLANG_ParameterGroup_<name>_0` wrapper and sequential positional `[[buffer(N)]]` indices — cbuffer slots share the buffer slot space with RW buffers. | boundary | [#wrapcbufferelementsformetal](../../../docs/llm-generated/target-pipelines/metal.md#wrapcbufferelementsformetal) | [`multiple-cbuffers-positional-buffer-indices.slang`](multiple-cbuffers-positional-buffer-indices.slang) |
| ConstantBuffer<T> emits as a wrapper-struct pointer in Metal's `constant` address space with a positional [[buffer(N)]] attribute. | functional | [#wrapcbufferelementsformetal](../../../docs/llm-generated/target-pipelines/metal.md#wrapcbufferelementsformetal) | [`constant-buffer-in-constant-address-space.slang`](constant-buffer-in-constant-address-space.slang) |
| wrapCBufferElementsForMetal wraps a float4x4 cbuffer element in a _MatrixStorage_..._natural_<N> struct so the emitted MSL is valid. | functional | [#wrapcbufferelementsformetal](../../../docs/llm-generated/target-pipelines/metal.md#wrapcbufferelementsformetal) | [`cbuffer-matrix-storage-wrap.slang`](cbuffer-matrix-storage-wrap.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#target-pipelinesmetalmd](../../../docs/llm-generated/target-pipelines/metal.md#target-pipelinesmetalmd) | undocumented-behavior | `target-pipelines/metal.md` claims `lowerAppendConsumeStructuredBuffers` fires for Metal (`target != HLSL`), but in practice `checkEntryPointDecorations` rejects `AppendStructuredBuffer<T>` / `ConsumeStructuredBuffer<T>` on the Metal compute stage with diagnostic `E36107` ("uses features that are not available in 'compute' stage for 'metal' compilation target"). The pass cannot be observed in emitted text on a compute entry point. | The doc should either name the stage(s) where the pass output is observable or note the compute-stage rejection. |
| [#main](../../../docs/llm-generated/target-pipelines/metal.md#main) | undocumented-behavior | The doc does not state that Slang **renames the user entry point** `main` → `main_0` on Metal (and emits `warning E40100` about it). The Phase-D section should mention the rename so test authors do not match `void main(` and silently miss the entry-point signature. |  |
| [#kernel](../../../docs/llm-generated/target-pipelines/metal.md#kernel) | undocumented-behavior | The doc says Metal hits `[[kernel]]`, `[[buffer(N)]]`, etc., but does not call out that **threadgroup size is *not* an emitted attribute** — i.e. `[numthreads(X,Y,Z)]` is dropped in Metal text and the dimensions come from the dispatch call. Worth mentioning in Phase D. |  |
| [#wrapcbufferelementsformetal](../../../docs/llm-generated/target-pipelines/metal.md#wrapcbufferelementsformetal) | undocumented-behavior | The doc enumerates `wrapCBufferElementsForMetal` but does not describe the **specific wrapper-struct naming** that downstream tests would key on (`SLANG_ParameterGroup_<name>_natural_<N>` containing `_MatrixStorage_<spelling>natural_<M>`). A concrete emit example in `#wrapcbufferelementsformetal` would prevent test drift. |  |
| [#legalizebyteaddressbufferops](../../../docs/llm-generated/target-pipelines/metal.md#legalizebyteaddressbufferops) | undocumented-behavior | The doc names the Metal-arm `legalizeByteAddressBufferOps` options but does not show the **emitted form** (`as_type<uint>(buf[(off)>>2])`); a one-line example would let tests assert against a concrete pattern. |  |
| [#phase-d-](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-) | undocumented-behavior | The doc mentions `[[texture(N)]]` slot positionally but does not enumerate the **per-Slang-texture-variant → Metal `texture*<T, access::...>`** mapping (`Texture1D` → `texture1d`, `Texture3D` → `texture3d`, `TextureCube` → `texturecube`, `Texture2DArray` → `texture2d_array`, and `RWTexture3D` → `texture3d<..., access::read_write>`). A small table under `#phase-d-...` (or a Texture-types subsection) would let texture-variant tests anchor more precisely. The `#legalizeimagesubscript` paragraph only mentions `RWTexture2D`; it should generalize to RWTexture1D / RWTexture3D / RWTexture2DArray. |  |
| [#gradientcube](../../../docs/llm-generated/target-pipelines/metal.md#gradientcube) | undocumented-behavior | The doc does not describe the **`gradientcube(...)` / `gradient2d(...)` / `gradient3d(...)` selector wrappers** that the Metal emitter inserts around explicit-gradient SampleGrad arguments. Anchoring this would let SampleGrad-shape tests pin the gradient selector. |  |

## Untested coverable claims

| Anchor | Backend | Claim | Why untested |
| --- | --- | --- | --- |
| [#floatnonuniformresourceindex](../../../docs/llm-generated/target-pipelines/metal.md#floatnonuniformresourceindex) | gpu-bindless | **`floatNonUniformResourceIndex`.** Requires the `NonUniformResourceIndex(...)` intrinsic with bindless setup not present in the compute-only bundle. | Agent runtime has no GPU; CI / local machine does. |
| [#collectcooperativemetadata](../../../docs/llm-generated/target-pipelines/metal.md#collectcooperativemetadata) | gpu-cooperative | **`collectCooperativeMetadata`.** Requires the cooperative matrix or vector capability set. | Agent runtime has no GPU; CI / local machine does. |
| [#legalizeuniformbufferload](../../../docs/llm-generated/target-pipelines/metal.md#legalizeuniformbufferload) | gpu-cross-api-flag | **`legalizeUniformBufferLoad`, `invertYOfPositionOutput`, `rcpWOfPositionInput`.** All gated on Khronos / HLSL or cross-API options the Metal bundle does not engage. | Agent runtime has no GPU; CI / local machine does. |
| [#legalizemeshoutputtypes](../../../docs/llm-generated/target-pipelines/metal.md#legalizemeshoutputtypes) | gpu-mesh-shader | **`legalizeMeshOutputTypes`.** Requires a mesh-shader entry point; not part of the compute-only bundle. | Agent runtime has no GPU; CI / local machine does. |
| [#metal](../../../docs/llm-generated/target-pipelines/metal.md#metal) | gpu-metal-toolchain | **Apple `metal` compiler invocation**, `.metallib` bytecode emission, `.metallib` disassembly. | Requires the Xcode toolchain; only triggered by `MetalLib*` targets, which are out of scope on the no-GPU runner. |
| [#metallibassembly](../../../docs/llm-generated/target-pipelines/metal.md#metallibassembly) | gpu-metal-toolchain | **`MetalLibAssembly` skipping `wrapCBufferElementsForMetal`.** Observing the resulting MSL difference requires a `-target metallib-asm` build that needs the Apple toolchain. | Agent runtime has no GPU; CI / local machine does. |
| [#legalizesubpassinputsformetal](../../../docs/llm-generated/target-pipelines/metal.md#legalizesubpassinputsformetal) | gpu-non-compute | **`legalizeSubpassInputsForMetal` ([[color(N)]] fragment input).** Requires a fragment entry point with `SubpassInput<T>`; not part of the compute-only bundle. | Agent runtime has no GPU; CI / local machine does. |

## Out of scope

| Anchor | Reason | Claim | Why it's terminal |
| --- | --- | --- | --- |
| [#checkentrypointdecorations](../../../docs/llm-generated/target-pipelines/metal.md#checkentrypointdecorations) | (unclassified) | **`AppendStructuredBuffer<T>` / `ConsumeStructuredBuffer<T>` lowering on Metal.** Rejected by `checkEntryPointDecorations` on the compute stage; cannot be observed in compute-only emit. (Recorded as a doc gap.) | Not reachable via any allowed test directive. |
| (unspecified) | implementation-detail | **Pass ordering within Phase A/B/C.** Pass _existence_ is observable through its effect on emitted text; pass _ordering_ is an IR-level claim that requires `-dump-ir` cross-pass annotations the doc does not anchor to a specific marker. | Not reachable via any allowed test directive. |
| [#linkandoptimizeir](../../../docs/llm-generated/target-pipelines/metal.md#linkandoptimizeir) | implementation-detail | **Iterative-pass observation.** Metal has **no** iterative passes in `linkAndOptimizeIR`, so the absence of `simplifyIR` iteration cannot be directly tested through `slangc` text emit. | Not reachable via any allowed test directive. |
