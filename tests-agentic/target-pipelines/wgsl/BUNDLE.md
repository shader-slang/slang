---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:52:31Z
source_commit: 2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
watched_paths_digest: 3a231855d2500716d08acc7404223e6d69655007cc529e406477b87c9bf1a697
source_doc: docs/llm-generated/target-pipelines/wgsl.md
source_doc_digest: d16c48a1b04e16432afea2867f729fb871e9db503b94181cca95f2b816a5f61d
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for target-pipelines/wgsl

## Intent

This bundle exercises the **WGSL target pipeline** documented at
`docs/llm-generated/target-pipelines/wgsl.md`. The doc enumerates
four phases (Link / Specialization / WGSL legalization / Emit +
Tint) and several WGSL-specific gates and emit shapes. The
strategy is: one functional test per observable claim, using the
compute stage and FileCheck on the WGSL emit text. All tests run
with `-target wgsl -entry main -stage compute` against the
`slangc` text emitter; no Tint, no runtime, no GPU.

## Claims enumerated

| Claim ID | Anchor                                                         | Claim (one line)                                                                                | Tests                                                |
| -------- | -------------------------------------------------------------- | ----------------------------------------------------------------------------------------------- | ---------------------------------------------------- |
| C-01     | #phase-d-wgsl-emit-and-downstream-tools                        | `[numthreads(X,Y,Z)]` becomes `@compute` + `@workgroup_size(X, Y, Z)`.                          | `numthreads-becomes-workgroup-size.slang`            |
| C-02     | #phase-d-wgsl-emit-and-downstream-tools                        | `RWStructuredBuffer<T>` emits `var<storage, read_write>` with `@binding`/`@group`.              | `rw-structured-buffer-storage-binding.slang`         |
| C-03     | #phase-d-wgsl-emit-and-downstream-tools                        | `StructuredBuffer<T>` is `var<storage, read>` (read-only access mode).                          | `structured-buffer-storage-read.slang`               |
| C-04     | #phase-d-wgsl-emit-and-downstream-tools                        | `Texture2D<float4>` becomes a `var ... : texture_2d<f32>` global with a binding pair.           | `texture2d-binding.slang`                            |
| C-05     | #specializeaddressspaceforwgsl                                 | `groupshared` becomes `var<workgroup> ... : array<T, N>`.                                       | `groupshared-becomes-workgroup-address-space.slang`  |
| C-06     | #specializeaddressspaceforwgsl                                 | Module-scope `static` becomes `var<private>`; initializer moved into entry-point body.          | `static-module-global-becomes-private.slang`         |
| C-07     | #legalizelogicalandor                                          | `legalizeLogicalAndOr` rewrites vector `&&` into a WGSL `select(...)` expression.               | `vector-logical-and-becomes-select.slang`            |
| C-08     | #eliminatephis-with-default-options                            | `eliminatePhis` (default options) introduces a `var` assigned in each `if`/`else` branch.       | `eliminate-phis-default-options.slang`               |
| C-09     | #legalizeirforwgsl                                             | `SV_DispatchThreadID` maps to `@builtin(global_invocation_id)`.                                 | `dispatch-thread-id-builtin.slang`                   |
| C-10     | #legalizeirforwgsl                                             | `SV_GroupThreadID` maps to `@builtin(local_invocation_id)`.                                     | `group-thread-id-builtin.slang`                      |
| C-11     | #phase-a-link-and-entry-point-prep                             | `lowerEnumType` lowers enumerator references to the underlying integer literal.                 | `enum-lowering-to-integer.slang`                     |
| C-12     | #phase-c-wgsl-legalization-lowering-phi-elimination            | `legalizeArrayReturnType` rewrites `T[N] foo()` to take a `ptr<function, array<T, N>>` out param. | `array-return-rewritten-to-out-pointer.slang`        |
| C-13     | #phase-c-wgsl-legalization-lowering-phi-elimination            | `lowerBitCast` emits WGSL `bitcast<T>(...)` for reinterpret casts (`asuint`).                   | `bitcast-spelling.slang`                             |
| C-14     | #phase-c-wgsl-legalization-lowering-phi-elimination            | `lowerBufferElementTypeToStorageType` (WGSL policy) wraps matrix elements in a `_MatrixStorage_` struct. | `structured-buffer-of-matrix-wraps-storage.slang`    |
| C-15     | #legalizebyteaddressbufferops-with-wgsl-options                | `ByteAddressBuffer.Load<uint>(off)` lowers to an `array<u32>[off/4]` indexing expression.       | `byte-address-buffer-load-divides-by-four.slang`     |
| C-16     | #phase-d-wgsl-emit-and-downstream-tools                        | `ConstantBuffer<S>` emits as `var<uniform>` over a `std140`-shaped struct with `@align(...)`.   | `constant-buffer-uniform-std140.slang`               |
| C-17     | #phase-b-specialization-and-type-legalization                  | `lowerCombinedTextureSamplers` splits `Sampler2D` into a `<name>_texture_*` + `<name>_sampler_*` pair. | `combined-texture-sampler-split.slang`              |
| C-18     | #phase-d-wgsl-emit-and-downstream-tools                        | `Atomic<uint>` emits as `atomic<u32>` and `.add(v)` lowers to `atomicAdd(&(buf[i]), v)`.        | `atomic-add-buffer.slang`                            |
| C-19     | #phase-d-wgsl-emit-and-downstream-tools                        | WGSL emit selects `LineDirectiveMode::None` — no `#line` directives in the output.              | `no-line-directives.slang`                           |
| C-20     | #phase-d-wgsl-emit-and-downstream-tools                        | The entry-point name `main` is preserved — `fn main(...)` in the WGSL emit.                    | `entry-point-name-main-preserved.slang`              |
| C-21     | #phase-d-wgsl-emit-and-downstream-tools                        | Multiple resources at module scope receive distinct sequential `@binding` indices.              | `multiple-resources-distinct-bindings.slang`         |
| C-22     | #phase-d-wgsl-emit-and-downstream-tools                        | Slang integer constants emit as `i32(N)` / `u32(N)` (constructor-style spelling).               | `integer-literal-spelling.slang`                     |
| C-23     | #phase-d-wgsl-emit-and-downstream-tools                        | Slang vector types spell out as `vec<rank><elem>` (no `uintN`/`floatN` shorthand).              | `uint3-becomes-vec3-u32.slang`                       |

## Tests in this bundle

| File                                                  | Intent     | Doc anchor                                                       |
| ----------------------------------------------------- | ---------- | ---------------------------------------------------------------- |
| `array-return-rewritten-to-out-pointer.slang`         | functional | `#phase-c-wgsl-legalization-lowering-phi-elimination`            |
| `atomic-add-buffer.slang`                             | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `bitcast-spelling.slang`                              | functional | `#phase-c-wgsl-legalization-lowering-phi-elimination`            |
| `byte-address-buffer-load-divides-by-four.slang`      | functional | `#legalizebyteaddressbufferops-with-wgsl-options`                |
| `combined-texture-sampler-split.slang`                | functional | `#phase-b-specialization-and-type-legalization`                  |
| `constant-buffer-uniform-std140.slang`                | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `dispatch-thread-id-builtin.slang`                    | functional | `#legalizeirforwgsl`                                             |
| `eliminate-phis-default-options.slang`                | functional | `#eliminatephis-with-default-options`                            |
| `entry-point-name-main-preserved.slang`               | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `enum-lowering-to-integer.slang`                      | functional | `#phase-a-link-and-entry-point-prep`                             |
| `group-thread-id-builtin.slang`                       | functional | `#legalizeirforwgsl`                                             |
| `groupshared-becomes-workgroup-address-space.slang`   | functional | `#specializeaddressspaceforwgsl`                                 |
| `integer-literal-spelling.slang`                      | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `multiple-resources-distinct-bindings.slang`          | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `no-line-directives.slang`                            | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `numthreads-becomes-workgroup-size.slang`             | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `rw-structured-buffer-storage-binding.slang`          | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `static-module-global-becomes-private.slang`          | functional | `#specializeaddressspaceforwgsl`                                 |
| `structured-buffer-of-matrix-wraps-storage.slang`     | functional | `#phase-c-wgsl-legalization-lowering-phi-elimination`            |
| `structured-buffer-storage-read.slang`                | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `texture2d-binding.slang`                             | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `uint3-becomes-vec3-u32.slang`                        | functional | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `vector-logical-and-becomes-select.slang`             | functional | `#legalizelogicalandor`                                          |
| `append-structured-buffer-rejected.slang`             | negative   | `#phase-b-specialization-and-type-legalization`                  |
| `array-index-out-of-bounds-rejected.slang`            | negative   | `#phase-b-specialization-and-type-legalization`                  |
| `atomic-int-add.slang`                                | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `atomic-uint-add-max-value.slang`                     | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `binding-15-group-3-high.slang`                       | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `binding-zero-group-zero.slang`                       | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `byte-address-buffer-load-offset-zero.slang`          | boundary   | `#legalizebyteaddressbufferops-with-wgsl-options`                |
| `constant-buffer-matrix-std140-wrapper.slang`         | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `float-nan-via-helper.slang`                          | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `float-positive-and-negative-zero.slang`              | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `float-vector-with-infinity.slang`                    | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `groupshared-array-256-elements.slang`                | boundary   | `#specializeaddressspaceforwgsl`                                 |
| `integer-literal-int-max.slang`                       | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `integer-literal-int-min.slang`                       | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `integer-literal-uint-max.slang`                      | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `integer-literal-uint-zero.slang`                     | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `interlocked-add-rejected.slang`                      | negative   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `large-array-1024-elements.slang`                     | stress     | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `matrix-storage-rectangular-3x4.slang`                | boundary   | `#phase-c-wgsl-legalization-lowering-phi-elimination`            |
| `matrix-storage-square-2x2.slang`                     | boundary   | `#phase-c-wgsl-legalization-lowering-phi-elimination`            |
| `multi-combined-texture-sampler-pairs.slang`          | boundary   | `#phase-b-specialization-and-type-legalization`                  |
| `multi-resources-many-bindings-stress.slang`          | stress     | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `nested-branches-five-deep-phi.slang`                 | stress     | `#eliminatephis-with-default-options`                            |
| `numthreads-256-1-1-webgpu-max.slang`                 | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `numthreads-3d-product-256.slang`                     | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `numthreads-one-one-one-minimum.slang`                | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `numthreads-zero-rejected.slang`                      | negative   | `#phase-d-wgsl-emit-and-downstream-tools`                        |
| `storage-uniform-explicit-address-space.slang`        | boundary   | `#specializeaddressspaceforwgsl`                                 |
| `vec3-padding-trailing-field.slang`                   | boundary   | `#phase-d-wgsl-emit-and-downstream-tools`                        |

## Doc gaps observed

- The doc names the `i32(N)` / `u32(N)` / `<value>f` literal
  spellings only obliquely (in passing inside the address-space
  discussion). A short subsection under "Phase D" that pins down
  the literal-emit shapes (and the `vec<rank><elem>` rule for
  WGSL vector types) would let tests anchor directly to that
  subsection instead of to the broad `#phase-d-...` anchor.
- The doc does not describe the `_MatrixStorage_<spelling>_ColMajorstd430_*`
  wrapper struct shape produced when a structured buffer holds a
  matrix element. The `lowerBufferElementTypeToStorageType`
  paragraph mentions the policy but not the generated struct
  name. Add an anchor (e.g. `#matrix-storage-wrapper`) under
  Phase C with the canonical wrapper name.
- The doc mentions `std140`-shaped struct layouts for constant
  buffers but does not document the per-field `@align(N)`
  annotations on the wrapper struct. A subsection describing the
  `_std140_*` wrapper struct shape and its `@align` decorations
  would let `constant-buffer-uniform-std140.slang` anchor more
  precisely.
- The doc states `lowerCombinedTextureSamplers` fires for WGSL
  (since WGSL is in the same arm as HLSL/Metal/WGSL) but does
  not document the naming convention for the split pair
  (`<name>_texture_*` / `<name>_sampler_*`). The naming is an
  emit-observable that lacks a marker in the doc.
- `eliminatePhis` is described in terms of `PhiEliminationOptions`
  defaults but the doc does not say what the WGSL emit shape of
  an `if`/`else` merged value looks like (the `var ... : T` +
  branch assignment pattern). A small example of the emit shape
  would strengthen the anchor for the eliminate-phis test.

## Out of scope (no-GPU runner)

- **Tint downstream invocation.** `-target wgsl-spirv` invokes
  Tint to translate WGSL to SPIR-V; this bundle stays at
  `-target wgsl` (text artifact) and does not exercise Tint or
  any GPU runtime.
- **Pass-ordering inside Phase A/B/C.** Pass existence is
  observable from emitted text; intra-phase ordering needs
  `-dump-ir` anchors that the doc does not pin.
- **`AppendStructuredBuffer<T>` / `ConsumeStructuredBuffer<T>`.**
  The Slang front-end rejects these in a WGSL compute entry
  point with `E36107`, so the `lowerAppendConsumeStructuredBuffers`
  pass that runs for WGSL is not reachable from a `.slang`
  source.
- **HLSL-style `InterlockedAdd` / `InterlockedExchange`.** Slang
  rejects these for WGSL; use `Atomic<T>` instead (covered by
  `atomic-add-buffer.slang`).
- **DXR / mesh / ray-tracing / graphics-stage entry points.**
  WGSL through Slang is compute-targeted in this bundle; the
  runner is no-GPU.
- **`collectCooperativeMetadata`.** Requires the cooperative
  matrix or vector capability set.
- **`legalizeUniformBufferLoad`, `invertYOfPositionOutput`,
  `rcpWOfPositionInput`.** Khronos / HLSL only.
- **`legalizeEntryPointsForGLSL`, `legalizeImageSubscript`,
  `legalizeConstantBufferLoadForGLSL`.** GLSL/SPIR-V only.
- **WGSL has no iterative passes (zero loops in
  `linkAndOptimizeIR`).** A textual claim about the absence of a
  while loop; not directly observable in emitted text.
