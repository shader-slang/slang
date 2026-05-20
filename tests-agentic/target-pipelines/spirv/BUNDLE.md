---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:30:00+00:00
source_commit: 330c9a8d807b9f9352e4754f466d1244ae681cff
watched_paths_digest: bd75ad021965ba68fbd7d335d6359ad1cf49b78a95a9be48be867759452b78f1
source_doc: docs/llm-generated/target-pipelines/spirv.md
source_doc_digest: b1daf02c9f85dda22a3cac37e70daa316be6cde3c511da1abf8a80d483e1dba1
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for target-pipelines/spirv

## Intent

Tests verify the SPIR-V direct-emit target pipeline described in
[`docs/llm-generated/target-pipelines/spirv.md`](../../../docs/llm-generated/target-pipelines/spirv.md):
the ordered IR-pass + emit sequence executed when
`CodeGenTarget::SPIRV` / `CodeGenTarget::SPIRVAssembly` is the
target and `shouldEmitSPIRVDirectly() == true`. The bundle
exercises:

- the SPIR-V text prelude (`OpCapability Shader`,
  `OpExtension SPV_KHR_storage_buffer_storage_class`,
  `OpMemoryModel Logical GLSL450`, `OpSource Slang 1`);
- the entry-point shape (`OpEntryPoint GLCompute %func "main"`,
  `OpExecutionMode %func LocalSize X Y Z` from
  `[numthreads(X,Y,Z)]`);
- the entry-point varying lowering done by
  `legalizeEntryPointsForGLSL` (`gl_GlobalInvocationID` /
  `gl_LocalInvocationIndex` with `BuiltIn` decorations);
- the storage-class assignments (`StorageBuffer` for
  `StructuredBuffer`, `Uniform` for `cbuffer` /
  `ConstantBuffer<T>`, `Workgroup` for `groupshared`,
  `UniformConstant` for textures/samplers, `Input` for
  varying inputs);
- the resource decorations (`Block`, `Offset 0`, `Binding`,
  `DescriptorSet 0`, `NonWritable`, `ArrayStride`);
- Phase C / D IR-pass effects:
  `legalizeByteAddressBufferOps` with
  `translateToStructuredBufferOps=true`;
  `legalizeLogicalAndOr` (`OpLogicalAnd` on `v3bool`);
  `legalizeImageSubscript` (`OpImageWrite` / `OpImageRead`);
  `validateAtomicOperations` (`OpAtomicIAdd`);
  `moveGlobalVarInitializationToEntryPoints`;
  `performForceInlining` + `performIntrinsicFunctionInlining`
  (the SPIR-V-only second inlining pass);
  `eliminatePhis` with `useRegisterAllocation = true`;
  Sampler2D as `OpTypeSampledImage` (combined types are
  supported natively in SPIR-V, so
  `lowerCombinedTextureSamplers` is **skipped**, unlike HLSL);
- the SPIR-V word-emission shape (`OpFunction` / `OpReturn` /
  `OpFunctionEnd`, `OpAccessChain` for buffer indexing,
  `OpBitcast` for `uint`→`int` casts, `OpStore` / `OpLoad`
  for register-allocated phi locals);
- debug-info opt-in (`DebugLine` ExtInst instructions emit
  only with `-g`).

Coverage strategy: one test per concrete claim that can be
observed in `slangc -target spirv-asm` text. Default directive
is
`//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute`.
The bundle is SPIR-V-only by design; cross-target probes are
not part of this bundle. spirv-link / spirv-val / spirv-opt
are downstream tools that don't change the emitted assembly
text and are out of scope.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                                                                                                       | Claim (one line)                                                                                                                              | Tests                                            |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------ |
| C-01     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | SPIR-V emit begins with the `; SPIR-V` text prelude and a `; Version: 1.X` line.                                                              | `prelude-spirv-header.slang`                     |
| C-02     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | The first declaration is `OpCapability Shader`.                                                                                               | `capability-shader.slang`                        |
| C-03     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | `OpMemoryModel Logical GLSL450` follows the capability declarations.                                                                          | `memory-model-logical-glsl450.slang`             |
| C-04     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | `SPV_KHR_storage_buffer_storage_class` extension is declared for `StructuredBuffer`.                                                          | `storage-buffer-extension.slang`                 |
| C-05     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | A compute entry point emits `OpEntryPoint GLCompute %func "main" ...`.                                                                        | `entry-point-glcompute.slang`                    |
| C-06     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | `[numthreads(X,Y,Z)]` lowers to `OpExecutionMode %func LocalSize X Y Z`.                                                                      | `execution-mode-local-size.slang`                |
| C-07     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | The source-language marker `OpSource Slang 1` appears in every SPIR-V emit.                                                                   | `opsource-slang.slang`                           |
| C-08     | [#legalizeentrypointsforglsl-despite-the-name](../../../docs/llm-generated/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name)                                                                                                             | `SV_DispatchThreadID` lowers to `gl_GlobalInvocationID` with `BuiltIn GlobalInvocationId`.                                                    | `sv-dispatch-thread-id-builtin.slang`            |
| C-09     | [#legalizeentrypointsforglsl-despite-the-name](../../../docs/llm-generated/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name)                                                                                                             | `SV_GroupIndex` lowers to `gl_LocalInvocationIndex` with `BuiltIn LocalInvocationIndex`.                                                      | `sv-group-index-builtin.slang`                   |
| C-10     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | `RWStructuredBuffer<T>` lowers to a `Block`-decorated struct in `StorageBuffer` storage class with `Binding` / `DescriptorSet` decorations.   | `rw-structured-buffer-storage-buffer.slang`      |
| C-11     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | `StructuredBuffer<T>` (read-only) carries the `NonWritable` decoration.                                                                       | `read-only-buffer-nonwritable.slang`             |
| C-12     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | Multiple resources get sequential `Binding 0`, `Binding 1`, ... values.                                                                       | `multiple-resources-sequential-bindings.slang`   |
| C-13     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | The runtime array inside the `StructuredBuffer` block carries an `ArrayStride` decoration matching the element size.                          | `runtime-array-stride.slang`                     |
| C-14     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | `groupshared` lowers to the `Workgroup` storage class.                                                                                        | `groupshared-workgroup-storage-class.slang`      |
| C-15     | [#phase-c-spir-v-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination)                                                                                           | `cbuffer { ... }` lowers to a `*_std140` `Block`-decorated struct in `Uniform` storage class.                                                 | `cbuffer-uniform-std140.slang`                   |
| C-16     | [#phase-c-spir-v-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination)                                                                                           | `ConstantBuffer<T>` lowers identically to `cbuffer { ... }` (`*_std140` `Block` `Uniform`).                                                   | `constant-buffer-uniform-std140.slang`           |
| C-17     | [#phase-c-spir-v-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination)                                                                                           | `Sampler2D` lowers to an `OpTypeSampledImage` (combined texture+sampler — SPIR-V supports it natively, so the lowering pass is skipped).      | `sampler2d-sampled-image.slang`                  |
| C-18     | [#phase-c-spir-v-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination)                                                                                           | `RWTexture2D` writes through subscript lower to `OpImageWrite` and reads to `OpImageRead` (`legalizeImageSubscript`).                         | `rw-texture-image-subscript.slang`               |
| C-19     | [#phase-c-spir-v-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination)                                                                                           | `InterlockedAdd` lowers to `OpAtomicIAdd` (atomic operations survive Phase C SPIR-V legalization).                                            | `atomic-operation-opatomic.slang`                |
| C-20     | [#phase-c-spir-v-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination)                                                                                           | Vector `&&` lowers to a single `OpLogicalAnd` on `v3bool` (`legalizeLogicalAndOr` Khronos arm).                                               | `vector-logical-and-or.slang`                    |
| C-21     | [#phase-c-spir-v-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination)                                                                                           | `ByteAddressBuffer.Load<T>(offset)` lowers to an `OpAccessChain` through the underlying runtime array (no SPIR-V template call).              | `byte-address-buffer-access-chain.slang`         |
| C-22     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | Buffer-element indexing emits `OpAccessChain %_ptr_StorageBuffer_<T> %buf %int_0 %idx`.                                                       | `buffer-index-access-chain.slang`                |
| C-23     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | A `uint`→`int` cast emits an `OpBitcast` (the cast is preserved through legalization).                                                        | `bitcast-uint-to-int.slang`                      |
| C-24     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | Entry-point function body is wrapped in `OpFunction %void None %<type> ... OpReturn OpFunctionEnd`.                                           | `entry-point-function-shape.slang`               |
| C-25     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | A function named `myKernel` keeps the `%myKernel` symbol but the OpEntryPoint string operand is the canonical `"main"`.                       | `entry-point-name-symbol-vs-string.slang`        |
| C-26     | [#eliminatephis-with-spir-v-specific-options](../../../docs/llm-generated/target-pipelines/spirv.md#eliminatephis-with-spir-v-specific-options)                                                                                                               | `eliminatePhis` in register-allocation mode lowers an `if/else` phi to a `Function`-storage `OpVariable` plus `OpStore` (no `OpPhi`).         | `eliminate-phis-register-allocation.slang`       |
| C-27     | [#phase-c-spir-v-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination)                                                                                           | `performIntrinsicFunctionInlining` (SPIR-V-only, gated on `emitSpirvDirectly`) folds a small helper into the caller — no separate OpFunction. | `intrinsic-function-inlining.slang`              |
| C-28     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | A `for` loop survives as SPIR-V structured control flow (`OpLoopMerge` / `OpBranchConditional`).                                              | `for-loop-structured-control-flow.slang`         |
| C-29     | [#option-set-toggles](../../../docs/llm-generated/target-pipelines/spirv.md#option-set-toggles)                                                                                                                                                               | Without `-g`, no `DebugLine` `OpExtInst` calls appear in the emit; the only source marker is `OpSource Slang 1`.                              | `debug-info-opt-in-default-off.slang`            |
| C-30     | [#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools](../../../docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools)                                                                       | `float4` lowers to `OpTypeVector %float 4` and `int` / `uint` to `OpTypeInt 32 (1|0)`.                                                        | `primitive-type-emission.slang`                  |

## Tests in this bundle

| File                                              | Intent     | Doc anchor                                                                |
| ------------------------------------------------- | ---------- | ------------------------------------------------------------------------- |
| `prelude-spirv-header.slang`                      | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `capability-shader.slang`                         | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `memory-model-logical-glsl450.slang`              | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `storage-buffer-extension.slang`                  | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `entry-point-glcompute.slang`                     | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `execution-mode-local-size.slang`                 | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `opsource-slang.slang`                            | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `sv-dispatch-thread-id-builtin.slang`             | functional | `#legalizeentrypointsforglsl-despite-the-name`                            |
| `sv-group-index-builtin.slang`                    | functional | `#legalizeentrypointsforglsl-despite-the-name`                            |
| `rw-structured-buffer-storage-buffer.slang`       | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `read-only-buffer-nonwritable.slang`              | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `multiple-resources-sequential-bindings.slang`    | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `runtime-array-stride.slang`                      | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `groupshared-workgroup-storage-class.slang`       | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `cbuffer-uniform-std140.slang`                    | functional | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `constant-buffer-uniform-std140.slang`            | functional | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `sampler2d-sampled-image.slang`                   | functional | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `rw-texture-image-subscript.slang`                | functional | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `atomic-operation-opatomic.slang`                 | functional | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `vector-logical-and-or.slang`                     | functional | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `byte-address-buffer-access-chain.slang`          | functional | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `buffer-index-access-chain.slang`                 | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `bitcast-uint-to-int.slang`                       | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `entry-point-function-shape.slang`                | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `entry-point-name-symbol-vs-string.slang`         | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `eliminate-phis-register-allocation.slang`        | functional | `#eliminatephis-with-spir-v-specific-options`                             |
| `intrinsic-function-inlining.slang`               | functional | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `for-loop-structured-control-flow.slang`          | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `debug-info-opt-in-default-off.slang`             | functional | `#option-set-toggles`                                                     |
| `primitive-type-emission.slang`                   | functional | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `add-uint32-max-overflow.slang`                   | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `int32-min-literal.slang`                         | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `uint32-max-literal.slang`                        | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `float-negative-zero-constant.slang`              | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `float-divide-by-zero-runtime.slang`              | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `float-inf-plus-inf-runtime.slang`                | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `uint16-storage-buffer-16bit-capability.slang`    | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `int64-storage-buffer-int64-capability.slang`     | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `double-storage-buffer-float64-capability.slang`  | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `half-storage-buffer-float16-capability.slang`    | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `binding-explicit-vk-binding.slang`               | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `binding-zero-descriptor-set-zero.slang`          | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `struct-offset-zero-first-member.slang`           | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `struct-offset-large-third-member.slang`          | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `array-stride-int8-element.slang`                 | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `array-stride-struct-element-large.slang`         | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `numthreads-localsize-1024-x.slang`               | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `numthreads-localsize-asymmetric-xyz.slang`       | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `matrix-4x4-emit.slang`                           | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `vector-swizzle-wxyz-shuffle.slang`               | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `switch-many-cases-ten-plus-default.slang`        | stress     | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `switch-one-case-plus-default.slang`              | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `entry-point-fragment-origin-upper-left.slang`    | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `entry-point-vertex-position.slang`               | boundary   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `eliminate-phis-deeply-nested-branches.slang`     | stress     | `#eliminatephis-with-spir-v-specific-options`                             |
| `nested-loops-three-deep.slang`                   | stress     | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |
| `image-subscript-at-origin-zero-zero.slang`       | boundary   | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `constant-buffer-scalar-uniform.slang`            | boundary   | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `uniform-constant-texture-sampler-pair.slang`     | boundary   | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `atomic-on-workgroup-storage.slang`               | boundary   | `#phase-c-spir-v-legalization-lowering-phi-elimination`                   |
| `negative-out-of-bound-static-array-index.slang`  | negative   | `#phase-b-specialization-and-type-legalization`                           |
| `negative-undefined-identifier.slang`             | negative   | `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`         |

## Doc gaps observed

- The doc enumerates the Phase D word-emission steps
  (`emitDebug`, `emitParams`, `emitEPs`, `emitFront`,
  `emitPhys`) but does not name the canonical SPIR-V text
  prelude markers (`; SPIR-V`, `; Version: 1.X`,
  `OpCapability Shader`, `OpMemoryModel Logical GLSL450`,
  `SPV_KHR_storage_buffer_storage_class`,
  `OpSource Slang 1`). A one-line statement of these
  prelude facts would let a test anchor them precisely;
  the bundle anchors them to the general
  `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`
  section.
- The doc does not name the storage-class assignments
  (`StorageBuffer` for `StructuredBuffer`, `Uniform` for
  `cbuffer`, `Workgroup` for `groupshared`,
  `UniformConstant` for textures/samplers, `Input` for
  varying inputs). These are core to the SPIR-V emit but
  are inferred via cross-referencing `legalizeEntryPointsForGLSL`
  and the per-target storage-class table. A doc table
  enumerating storage classes per Slang resource type
  would be useful.
- The doc names `legalizeConstantBufferLoadForGLSL` and
  `lowerBufferElementTypeToStorageType` but does not state
  that the emitted SPIR-V uses the `*_std140`-suffixed
  wrapper-struct convention (visible in
  `OpName %SLANG_ParameterGroup_<name>_std140 ...`). A
  one-line statement of this would let a test pin the
  exact spelling rather than a regex; the bundle uses
  regex wildcards.
- The doc's `## Source` table cites line numbers (`linkAndOptimizeIR`
  at line ~893, `emitSPIRVForEntryPointsDirectly` at line ~3122,
  etc.). These are navigation aids and not user-observable;
  no test.
- The doc states `simplifyIRForSpirvLegalization` settles in
  2-3 outer iterations in practice but has no documented
  observable consequence; no test.
- The doc lists `addUserTypeHintDecorations` as gated on
  `VulkanEmitReflection`. The doc does not name an emit
  marker; this is a reflection-only feature, not a SPIR-V
  text observable. No test.
- The doc's `## Forward-declared pointer fixup` describes the
  loop but does not give a user-source pattern that triggers
  it. Slang's `checkForRecursiveTypes` rejects recursive
  structs, so the loop is unreachable from compute-stage
  user code. No test.
- The doc says `OpEntryPoint` operands include all variables
  the entry point references but does not say the entry-point
  string operand is always `"main"` regardless of the Slang
  entry-point function name. We pin this via experiment in
  the test `entry-point-name-symbol-vs-string.slang`; a doc
  statement would be cleaner.
- The doc says `validateAtomicOperations` is called inside
  `legalizeIRForSPIRV` but does not state which SPIR-V opcode
  `InterlockedAdd` lowers to. We pin `OpAtomicIAdd` from
  emit-experiment.
- The doc says `legalizeLogicalAndOr` runs for SPIR-V (Khronos
  arm) but does not state the SPIR-V opcode the vector `&&`
  produces. We pin `OpLogicalAnd` from emit-experiment.

## Out of scope (no-GPU runner)

- **spirv-link** (`#downstream-spirv-link-spirv-val-spirv-opt-chain`).
  Only invoked when `spirvFiles.getCount() > 1`, i.e. when
  there is an `IREmbeddedDownstreamIR` of `CodeGenTarget::SPIRV`
  to merge with the freshly emitted module. The compute-stage
  bundle does not set up `-embed-downstream-ir`.
- **spirv-val**. Gated on `SLANG_RUN_SPIRV_VALIDATION` env
  var or `-validate-spirv` flag; doesn't change the emitted
  assembly text. The CI runner may set the env var
  separately; the bundle's tests don't depend on validation.
- **spirv-opt**. Invoked via the generic downstream path;
  options-dependent whether it changes anything. The inline
  `optimizeSPIRV` call site is `#if 0`'d out (doc-only).
- **`simplifyIRForSpirvLegalization` outer/inner loop bounds**
  (`kMaxIterations = 8`, `kMaxFuncIterations = 16`). The
  loops terminate by fixed-point in practice; no documented
  user-source pattern hits the bound.
- **Forward-declared-pointer fixup loop** (Phase D step 12).
  Only fires on recursive struct-pointer graphs.
  `checkForRecursiveTypes` errors out before Phase D for
  user-declared recursive structs.
- **`introduceExplicitGlobalContext`** — gated on
  `EnableExperimentalPasses`.
- **`invertYOfPositionOutput` / `rcpWOfPositionInput`** —
  gated on `VulkanInvertY` / `VulkanUseDxPositionW`.
- **`legalizeMeshOutputTypes` /
  `legalizeDispatchMeshPayloadForGLSL`** — mesh entry points.
- **`collectCooperativeMetadata`** — cooperative matrix /
  vector capability set.
- **`unexportNonEmbeddableIR`** — gated on `EmbedDownstreamIR`.
- **`coverageTracing`-gated passes**
  (`instrumentCoverage`, `finalizeCoverageInstrumentationMetadata`).
- **`autodiff` / `higherOrderFunc` passes**
  (`checkAutodiffPatterns`, `specializeHigherOrderParameters`,
  `finalizeAutoDiffPass`).
- **`dynamicResourceHeap`** — SM 6.6 dynamic resource heap.
- **`insertFragmentShaderInterlock`** — raster-ordered
  resources in a fragment entry point.
- **`removeUnreachableCodeAfterDiscardForOpKill`** — requires
  `discard` and SPIR-V `< 1.6`; Slang defaults to `>= 1.5`
  and gates the pass on `!shouldEmitDiscardAsDemote()`.
- **`replaceLocationIntrinsicsWithRaytracingObject`** — DXR
  entry points.
- **`legalizeUniformBufferLoad`** — anchored to an IR-level
  canonicalization without a documented text-emit marker.
- **Pass-ordering claims** (Phase A passes 1-19, Phase B
  passes 1-65, Phase C passes 1-41, Phase D steps 1-20).
  Pass _existence_ is observable via emit; pass _ordering_
  would require `-dump-ir` cross-pass comparison without
  doc-anchored ordering markers. Covered by
  `pipeline/05-ir-passes`.
- **Cross-target probes.** The bundle is single-target by
  design — SPIR-V observations only. "X fires on SPIR-V but
  Y doesn't" claims are documented in the `## Conditional
  gates` section of the doc but not exercised as multi-target
  CHECK tests in this bundle.
