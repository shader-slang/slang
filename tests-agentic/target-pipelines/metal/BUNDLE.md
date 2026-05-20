---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:50:00Z
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

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                                | Claim (one line)                                                                                                                                            | Tests                                                          |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| C-01     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | Metal emit begins with the `<metal_stdlib>` / `<metal_math>` / `<metal_texture>` includes and `using namespace metal;`.                                     | `prelude-metal-stdlib-include.slang`                           |
| C-02     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | Metal compute entry points carry the `[[kernel]]` attribute (matched as bare `kernel`).                                                                     | `kernel-attribute-on-entry.slang`                              |
| C-03     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | Slang renames the user entry point `main` → `main_0` on Metal.                                                                                              | `entry-point-renamed-to-main-zero.slang`                       |
| C-04     | [#legalizeirformetal](../../../docs/llm-generated/target-pipelines/metal.md#legalizeirformetal)                                                                                       | `SV_DispatchThreadID` lowers to a `uint3` entry-point param with `[[thread_position_in_grid]]`.                                                             | `sv-dispatch-thread-id-attribute.slang`                        |
| C-05     | [#legalizeirformetal](../../../docs/llm-generated/target-pipelines/metal.md#legalizeirformetal)                                                                                       | `SV_GroupIndex` is synthesized from `[[thread_position_in_threadgroup]]`.                                                                                   | `sv-group-index-synthesized.slang`                             |
| C-06     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | RWStructuredBuffer params emit as `T device*` pointers with `[[buffer(0)]]` (single-buffer base case).                                                      | `buffer-binding-positional-zero.slang`                         |
| C-07     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | `[[buffer(N)]]` indices are positional and assigned in declaration order, not driven by `register()` / `vk::binding`.                                       | `buffer-binding-positional-multi.slang`                        |
| C-08     | [#wrapcbufferelementsformetal](../../../docs/llm-generated/target-pipelines/metal.md#wrapcbufferelementsformetal)                                                                     | `ConstantBuffer<T>` emits as a wrapper-struct pointer in the `constant` address space with a positional `[[buffer(N)]]` attribute.                          | `constant-buffer-in-constant-address-space.slang`              |
| C-09     | [#wrapcbufferelementsformetal](../../../docs/llm-generated/target-pipelines/metal.md#wrapcbufferelementsformetal)                                                                     | `wrapCBufferElementsForMetal` wraps a `float4x4` cbuffer element in `_MatrixStorage_…natural_<N>` inside a `SLANG_ParameterGroup_<name>_natural_<M>` outer. | `cbuffer-matrix-storage-wrap.slang`                            |
| C-10     | [#phase-b-specialization-and-type-legalization](../../../docs/llm-generated/target-pipelines/metal.md#phase-b-specialization-and-type-legalization)                                   | `Sampler2D` splits into `combined_texture_<N>` / `combined_sampler_<N>` with separate `[[texture]]` / `[[sampler]]`.                                        | `combined-sampler-splits-to-texture-and-sampler.slang`         |
| C-11     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | `Texture2D<T>` + `SamplerState` survive as separate `texture2d<T, access::sample>` and `sampler` params (Metal-native separation).                          | `separate-texture2d-and-sampler-state.slang`                   |
| C-12     | [#legalizeimagesubscript](../../../docs/llm-generated/target-pipelines/metal.md#legalizeimagesubscript)                                                                               | RWTexture2D emits as `texture2d<T, access::read_write>` and `legalizeImageSubscript` rewrites subscript into `.write(...)` / `.read(...)`.                  | `rwtexture2d-read-write-and-image-subscript.slang`             |
| C-13     | [#specializeaddressspaceformetal](../../../docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal)                                                               | `groupshared T[N]` emits with the `threadgroup` address space (`threadgroup array<T, int(N)>`).                                                             | `groupshared-threadgroup-address-space.slang`                  |
| C-14     | [#specializeaddressspaceformetal](../../../docs/llm-generated/target-pipelines/metal.md#specializeaddressspaceformetal)                                                               | Buffer-storage pointers carry the `device` address space; constant-buffer pointers carry `constant`.                                                        | `device-address-space-on-buffer-pointer.slang`                 |
| C-15     | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination)                     | `uniform` globals are packed into a `GlobalParams` struct passed as a `constant*` parameter via `introduceExplicitGlobalContext`.                           | `uniform-globals-packed-into-globalparams.slang`               |
| C-16     | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination)                     | `InterlockedAdd` lowers to `atomic_fetch_add_explicit(..., memory_order_relaxed)` with an `(atomic_uint device*)` cast.                                     | `atomic-fetch-add-explicit-lowering.slang`                     |
| C-17     | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination)                     | `ByteAddressBuffer.Load<T>` / `Store<T>` lower with Metal's options to `as_type<T>(buf[(off)>>2])` pointer-indexed accesses.                                | `byte-address-buffer-load-store-lowering.slang`                |
| C-18     | [#phase-a-link-and-entry-point-prep](../../../docs/llm-generated/target-pipelines/metal.md#phase-a-link-and-entry-point-prep)                                                         | `lowerEnumType` collapses an enum to its underlying integer literal; the enumerator name does not survive in the emit.                                      | `enum-lowers-to-integer-literal.slang`                         |
| C-19     | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination)                     | Array-return functions survive as `array<T, N> foo(...)` on Metal (`legalizeArrayReturnType` is filtered out).                                              | `array-return-type-survives.slang`                             |
| C-20     | [#eliminatephis-with-default-options](../../../docs/llm-generated/target-pipelines/metal.md#eliminatephis-with-default-options)                                                       | `eliminatePhis` runs with default options on Metal: an if/else-merged value becomes a function-local variable assigned in each branch.                      | `eliminate-phis-default-options.slang`                         |
| C-21     | [#eliminatephis-with-default-options](../../../docs/llm-generated/target-pipelines/metal.md#eliminatephis-with-default-options)                                                       | Conditional struct selection lowers to the same if/else write-back to a local of the struct type (no Metal-specific composite-select pass).                 | `composite-select-via-eliminate-phis.slang`                    |
| C-22     | [#undoparametercopy-transformparamstoconstref](../../../docs/llm-generated/target-pipelines/metal.md#undoparametercopy-transformparamstoconstref)                                     | `moveGlobalVarInitializationToEntryPoints` (via Metal fallthrough) fires a `static` module-scope variable's initializer inside the kernel.                  | `static-global-init-moves-to-entry-point.slang`                |
| C-23     | [#metal-specific-lowerbufferelementtypetostoragetype](../../../docs/llm-generated/target-pipelines/metal.md#metal-specific-lowerbufferelementtypetostoragetype)                       | `ParameterBlock<Resources>` uses the `MetalParameterBlock` policy: resource-typed fields become raw descriptor-handle pointer fields; the block is a single `constant*` parameter. | `parameter-block-metal-policy.slang`                          |
| C-24     | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination)                     | `legalizeLogicalAndOr` Metal arm preserves vector `&&` as element-wise `bool3(...) && bool3(...)`.                                                          | `vector-logical-and-survives.slang`                            |
| C-25     | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination)                     | `lowerBitCast` renders `asuint(f)` as `as_type<uint>(...)`.                                                                                                 | `bitcast-emits-as-type.slang`                                  |
| C-26     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | `SourceWriter` emits `#line N "<file>"` directives in Metal output by default.                                                                              | `line-directive-emitted.slang`                                 |
| C-27     | [#phase-c-metal-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination)                     | Cross-target negative observation: Metal keeps an array-return function as `array<T, N> foo(...)` while HLSL rewrites it to `void` with an `out` parameter. | `array-return-metal-vs-hlsl-rewrites.slang`                    |
| C-28     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | The Metal source emitter does not emit a SPIR-V-style identification comment in its prelude; the first emitted text is the `#include` block.                | `no-spirv-style-prelude-comment.slang`                         |
| C-29     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | Texture parameter binding uses an independent `[[texture(N)]]` slot space (not shared with `[[buffer]]` / `[[sampler]]`).                                   | `texture-binding-positional-zero.slang`                        |
| C-30     | [#phase-d-metal-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools)                                             | `[numthreads(X,Y,Z)]` is HLSL-specific and does not survive as a `numthreads(...)` attribute on Metal — threadgroup size is a dispatch-time concern.        | `numthreads-not-an-emitted-attribute.slang`                    |
| C-31     | [#downstream-apple-metal-compiler](../../../docs/llm-generated/target-pipelines/metal.md#downstream-apple-metal-compiler)                                                             | Bare `-target metal` stops at Metal text; no `.metallib` is produced and the entry function appears as `main_0` (not an HLSL-style `main`).                 | `downstream-stops-at-text-no-metallib.slang`                   |

## Tests in this bundle

| File                                                       | Intent     | Doc anchor                                                |
| ---------------------------------------------------------- | ---------- | --------------------------------------------------------- |
| `prelude-metal-stdlib-include.slang`                       | functional | `#phase-d-metal-emit-and-downstream-tools`                |
| `kernel-attribute-on-entry.slang`                          | functional | `#phase-d-metal-emit-and-downstream-tools`                |
| `entry-point-renamed-to-main-zero.slang`                   | functional | `#phase-d-metal-emit-and-downstream-tools`                |
| `sv-dispatch-thread-id-attribute.slang`                    | functional | `#legalizeirformetal`                                     |
| `sv-group-index-synthesized.slang`                         | functional | `#legalizeirformetal`                                     |
| `buffer-binding-positional-zero.slang`                     | functional | `#phase-d-metal-emit-and-downstream-tools`                |
| `buffer-binding-positional-multi.slang`                    | functional | `#phase-d-metal-emit-and-downstream-tools`                |
| `constant-buffer-in-constant-address-space.slang`          | functional | `#wrapcbufferelementsformetal`                            |
| `cbuffer-matrix-storage-wrap.slang`                        | functional | `#wrapcbufferelementsformetal`                            |
| `combined-sampler-splits-to-texture-and-sampler.slang`     | functional | `#phase-b-specialization-and-type-legalization`           |
| `separate-texture2d-and-sampler-state.slang`               | functional | `#phase-d-metal-emit-and-downstream-tools`                |
| `rwtexture2d-read-write-and-image-subscript.slang`         | functional | `#legalizeimagesubscript`                                 |
| `groupshared-threadgroup-address-space.slang`              | functional | `#specializeaddressspaceformetal`                         |
| `device-address-space-on-buffer-pointer.slang`             | functional | `#specializeaddressspaceformetal`                         |
| `uniform-globals-packed-into-globalparams.slang`           | functional | `#phase-c-metal-legalization-lowering-phi-elimination`    |
| `atomic-fetch-add-explicit-lowering.slang`                 | functional | `#phase-c-metal-legalization-lowering-phi-elimination`    |
| `byte-address-buffer-load-store-lowering.slang`            | functional | `#phase-c-metal-legalization-lowering-phi-elimination`    |
| `enum-lowers-to-integer-literal.slang`                     | functional | `#phase-a-link-and-entry-point-prep`                      |
| `array-return-type-survives.slang`                         | functional | `#phase-c-metal-legalization-lowering-phi-elimination`    |
| `eliminate-phis-default-options.slang`                     | functional | `#eliminatephis-with-default-options`                     |
| `composite-select-via-eliminate-phis.slang`                | functional | `#eliminatephis-with-default-options`                     |
| `static-global-init-moves-to-entry-point.slang`            | functional | `#undoparametercopy-transformparamstoconstref`            |
| `parameter-block-metal-policy.slang`                       | functional | `#metal-specific-lowerbufferelementtypetostoragetype`     |
| `vector-logical-and-survives.slang`                        | functional | `#phase-c-metal-legalization-lowering-phi-elimination`    |
| `bitcast-emits-as-type.slang`                              | functional | `#phase-c-metal-legalization-lowering-phi-elimination`    |
| `line-directive-emitted.slang`                             | functional | `#phase-d-metal-emit-and-downstream-tools`                |
| `array-return-metal-vs-hlsl-rewrites.slang`                | functional | `#phase-c-metal-legalization-lowering-phi-elimination`    |
| `no-spirv-style-prelude-comment.slang`                     | negative   | `#phase-d-metal-emit-and-downstream-tools`                |
| `texture-binding-positional-zero.slang`                    | functional | `#phase-d-metal-emit-and-downstream-tools`                |
| `numthreads-not-an-emitted-attribute.slang`                | negative   | `#phase-d-metal-emit-and-downstream-tools`                |
| `downstream-stops-at-text-no-metallib.slang`               | negative   | `#downstream-apple-metal-compiler`                        |
| `uint-max-plus-one-wraps-to-zero.slang`                    | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `int-max-literal-emits-verbatim.slang`                     | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `int-min-literal-emits-negative.slang`                     | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `float-positive-infinity-divide.slang`                     | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `half-precision-boundary-survives.slang`                   | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `bitcast-uint-to-float.slang`                              | boundary   | `#phase-c-metal-legalization-lowering-phi-elimination`    |
| `asuint-float-bitcast-direction.slang`                     | boundary   | `#phase-c-metal-legalization-lowering-phi-elimination`    |
| `buffer-binding-index-fifteen.slang`                       | stress     | `#phase-d-metal-emit-and-downstream-tools`                |
| `buffer-binding-eight-buffer-pressure.slang`               | stress     | `#phase-d-metal-emit-and-downstream-tools`                |
| `texture-and-sampler-paired-at-zero.slang`                 | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `sampler-binding-index-one.slang`                          | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `vk-binding-ignored-positional-wins.slang`                 | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `groupshared-scalar-threadgroup-pointer.slang`             | boundary   | `#specializeaddressspaceformetal`                         |
| `groupshared-struct-array-threadgroup.slang`               | boundary   | `#specializeaddressspaceformetal`                         |
| `groupshared-float4-vector-threadgroup.slang`              | boundary   | `#specializeaddressspaceformetal`                         |
| `multiple-groupshared-vars-threadgroup-each.slang`         | boundary   | `#specializeaddressspaceformetal`                         |
| `groupshared-array-256-stress.slang`                       | stress     | `#specializeaddressspaceformetal`                         |
| `numthreads-one-one-one-still-emits-kernel.slang`          | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `numthreads-1024-one-one-emits-kernel.slang`               | boundary   | `#phase-d-metal-emit-and-downstream-tools`                |
| `numthreads-zero-x-rejected.slang`                         | negative   | `#phase-d-metal-emit-and-downstream-tools`                |
| `cbuffer-row-major-matrix-storage.slang`                   | boundary   | `#wrapcbufferelementsformetal`                            |
| `cbuffer-column-major-matrix-storage.slang`                | boundary   | `#wrapcbufferelementsformetal`                            |
| `multiple-cbuffers-positional-buffer-indices.slang`        | boundary   | `#wrapcbufferelementsformetal`                            |
| `large-local-array-256-stress.slang`                       | stress     | `#phase-d-metal-emit-and-downstream-tools`                |
| `deeply-nested-control-flow-stress.slang`                  | stress     | `#eliminatephis-with-default-options`                     |
| `enum-with-int-max-collapses-to-literal.slang`             | boundary   | `#phase-a-link-and-entry-point-prep`                      |
| `append-structured-buffer-rejected-on-metal.slang`         | negative   | `#phase-b-specialization-and-type-legalization`           |

## Doc gaps observed

- `target-pipelines/metal.md` claims `lowerAppendConsumeStructuredBuffers` fires for Metal (`target != HLSL`), but in practice `checkEntryPointDecorations` rejects `AppendStructuredBuffer<T>` / `ConsumeStructuredBuffer<T>` on the Metal compute stage with diagnostic `E36107` ("uses features that are not available in 'compute' stage for 'metal' compilation target"). The pass cannot be observed in emitted text on a compute entry point. The doc should either name the stage(s) where the pass output is observable or note the compute-stage rejection.
- The doc does not state that Slang **renames the user entry point** `main` → `main_0` on Metal (and emits `warning E40100` about it). The Phase-D section should mention the rename so test authors do not match `void main(` and silently miss the entry-point signature.
- The doc says Metal hits `[[kernel]]`, `[[buffer(N)]]`, etc., but does not call out that **threadgroup size is *not* an emitted attribute** — i.e. `[numthreads(X,Y,Z)]` is dropped in Metal text and the dimensions come from the dispatch call. Worth mentioning in Phase D.
- The doc enumerates `wrapCBufferElementsForMetal` but does not describe the **specific wrapper-struct naming** that downstream tests would key on (`SLANG_ParameterGroup_<name>_natural_<N>` containing `_MatrixStorage_<spelling>natural_<M>`). A concrete emit example in `#wrapcbufferelementsformetal` would prevent test drift.
- The doc names the Metal-arm `legalizeByteAddressBufferOps` options but does not show the **emitted form** (`as_type<uint>(buf[(off)>>2])`); a one-line example would let tests assert against a concrete pattern.

## Out of scope (no-GPU runner)

- **Apple `metal` compiler invocation**, `.metallib` bytecode emission, `.metallib` disassembly. Requires the Xcode toolchain; only triggered by `MetalLib*` targets, which are out of scope on the no-GPU runner.
- **Pass ordering within Phase A/B/C.** Pass _existence_ is observable through its effect on emitted text; pass _ordering_ is an IR-level claim that requires `-dump-ir` cross-pass annotations the doc does not anchor to a specific marker.
- **`AppendStructuredBuffer<T>` / `ConsumeStructuredBuffer<T>` lowering on Metal.** Rejected by `checkEntryPointDecorations` on the compute stage; cannot be observed in compute-only emit. (Recorded as a doc gap.)
- **`legalizeMeshOutputTypes`.** Requires a mesh-shader entry point; not part of the compute-only bundle.
- **`legalizeSubpassInputsForMetal` ([[color(N)]] fragment input).** Requires a fragment entry point with `SubpassInput<T>`; not part of the compute-only bundle.
- **`MetalLibAssembly` skipping `wrapCBufferElementsForMetal`.** Observing the resulting MSL difference requires a `-target metallib-asm` build that needs the Apple toolchain.
- **`collectCooperativeMetadata`.** Requires the cooperative matrix or vector capability set.
- **`floatNonUniformResourceIndex`.** Requires the `NonUniformResourceIndex(...)` intrinsic with bindless setup not present in the compute-only bundle.
- **`legalizeUniformBufferLoad`, `invertYOfPositionOutput`, `rcpWOfPositionInput`.** All gated on Khronos / HLSL or cross-API options the Metal bundle does not engage.
- **Iterative-pass observation.** Metal has **no** iterative passes in `linkAndOptimizeIR`, so the absence of `simplifyIR` iteration cannot be directly tested through `slangc` text emit.
