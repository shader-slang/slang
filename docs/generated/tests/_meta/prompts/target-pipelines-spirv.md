# Prompt: docs/generated/tests/target-pipelines/spirv/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `docs/generated/tests/target-pipelines/spirv/`,
anchored to
[`docs/generated/design/target-pipelines/spirv.md`](../../../docs/generated/design/target-pipelines/spirv.md).

Audience: nightly CI. The bundle exercises the **SPIR-V direct-emit
target pipeline**: the ordered IR-pass + emit sequence executed when
`CodeGenTarget::SPIRV` (or `CodeGenTarget::SPIRVAssembly`) is the
target and `shouldEmitSPIRVDirectly() == true`. The doc enumerates
four phases:

- **Phase A** — link and entry-point prep (`linkIR`,
  `translateGlobalVaryingVar`, `lowerEnumType`, etc.).
- **Phase B** — specialization and high-level type legalization
  (specialization, Optional/Result/Conditional/tagged-union/tuple
  lowering, `lowerAppendConsumeStructuredBuffers`,
  `legalizeMatrixTypes`, `specializeFuncsForBufferLoadArgs` 1st
  invocation, `deferBufferLoad`, `specializeArrayParameters`).
- **Phase C** — SPIR-V legalization, lowering, phi elimination
  (`legalizeByteAddressBufferOps` with SPIR-V options,
  `legalizeEntryPointsForGLSL`, `legalizeLogicalAndOr`,
  `legalizeImageSubscript`, `legalizeConstantBufferLoadForGLSL`,
  `moveGlobalVarInitializationToEntryPoints`,
  `transformParamsToConstRef`, `removeRawDefaultConstructors`,
  `legalizeUniformBufferLoad`, `lowerBufferElementTypeToStorageType`,
  `specializeFuncsForBufferLoadArgs` 2nd invocation,
  `performIntrinsicFunctionInlining`, `eliminateMultiLevelBreak`,
  `legalizeEmptyTypes`, `eliminatePhis` with SPIR-V options).
- **Phase D** — `legalizeIRForSPIRV` + the iterative
  `simplifyIRForSpirvLegalization` loop +
  `removeUnreachableCodeAfterDiscardForOpKill` +
  `insertFragmentShaderInterlock` + SPIR-V word emission +
  forward-pointer fixup + spirv-link / spirv-val / spirv-opt
  downstream chain.

The defining characteristic of this bundle is that almost every
claim is observable in **emitted SPIR-V assembly text**
(`-target spirv-asm`). spirv-val / spirv-opt / spirv-link are
out-of-band tools that don't change the assembly text we check
against (unless `SLANG_RUN_SPIRV_VALIDATION` is set to refuse
invalid output — separate concern).

The bundle is therefore **single-target by design**: the default
test directive is

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute
```

Cross-target probes are not part of this bundle. If a claim names a
sibling target that is filtered out (e.g. "GLSL runs
`specializeAddressSpace` but SPIR-V does not"), it lives in the
**Out of scope** or **Doc gaps** section, not in a multi-target
test file.

## The translation rule: claims to observations

`target-pipelines/spirv.md` describes:

- The ordered `SLANG_PASS` sequence for SPIR-V direct-emit in
  `linkAndOptimizeIR` (Phase A/B/C tables).
- The post-`linkAndOptimizeIR` SPIR-V backend in `emitSPIRVFromIR`,
  the iterative `simplifyIRForSpirvLegalization` loop, the
  forward-declared-pointer fixup loop, and the downstream
  spirv-link / spirv-val / spirv-opt chain (Phase D).
- SPIR-V-only configuration tweaks: `eliminatePhis` with
  `useRegisterAllocation = true`, the second SPIR-V-only
  `specializeFuncsForBufferLoadArgs` invocation, the deferred
  address-space propagation, the `removeRawDefaultConstructors`
  pass gated on `shouldEmitSPIRVDirectly()`,
  `replaceLocationIntrinsicsWithRaytracingObject` gated on
  `isKhronosTarget && emitSpirvDirectly`,
  `performIntrinsicFunctionInlining` gated on `emitSpirvDirectly`.
- Always-on emit shapes: the `; SPIR-V` text prelude with
  version / generator / bound; `OpCapability Shader`;
  `OpExtension "SPV_KHR_storage_buffer_storage_class"`;
  `OpMemoryModel Logical GLSL450`;
  `OpEntryPoint GLCompute %main "main" ...`;
  `OpExecutionMode %main LocalSize X Y Z`;
  `OpSource Slang 1`;
  the `BuiltIn GlobalInvocationId` decoration on
  `gl_GlobalInvocationID`;
  the `Block` / `Offset 0` / `Binding N` / `DescriptorSet 0`
  decorations on `StructuredBuffer` / `RWStructuredBuffer`;
  the `NonWritable` decoration on `StructuredBuffer` /
  `ByteAddressBuffer`; the `Workgroup` storage class for
  `groupshared`; the `TypeImage` / `TypeSampledImage` split
  for `Sampler2D`; the `Uniform` storage class for `cbuffer` /
  `ConstantBuffer<T>` with a `*_std140` block layout struct;
  the `OpAtomicIAdd` lowering of `InterlockedAdd`;
  the `OpLogicalAnd` element-wise lowering of a vector `&&`.
- Downstream: spirv-link / spirv-val / spirv-opt are invoked
  outside the assembly text we check.

### Observable claims (write tests for these)

- **`; SPIR-V` header prelude.** Every SPIR-V emit begins with the
  `; SPIR-V` comment, a version line (`; Version: 1.X`), and the
  generator + bound metadata. (Anchor:
  `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`.)
- **`OpCapability Shader` is the first declaration.** (Anchor:
  same.)
- **`OpMemoryModel Logical GLSL450` follows the capabilities.**
  SPIR-V direct-emit uses the `Logical` addressing model with the
  `GLSL450` memory model. (Anchor: same.)
- **`SPV_KHR_storage_buffer_storage_class` is the extension used
  for `StructuredBuffer` storage.** (Anchor: same.)
- **`OpEntryPoint GLCompute %<func> "main" ...`** for a compute
  entry point. The execution model is `GLCompute`; the second
  operand is the user's entry-point name (which is
  `entry-point-name`, not the function symbol name). (Anchor:
  same.)
- **`OpExecutionMode %<func> LocalSize X Y Z`.** A
  `[numthreads(8,4,2)]` lowers to `LocalSize 8 4 2`. (Anchor:
  same.)
- **`OpSource Slang 1`.** The source language is `Slang`, version
  `1`. (Anchor: same.)
- **`gl_GlobalInvocationID` is decorated `BuiltIn
  GlobalInvocationId`.** This is `legalizeEntryPointsForGLSL`
  feeding the SPIR-V emit. (Anchor:
  `#legalizeentrypointsforglsl-despite-the-name`.)
- **`StructuredBuffer` and `RWStructuredBuffer` get the `Block`
  + `Offset 0` decorations** and the `StorageBuffer` storage
  class on the pointer type. (Anchor:
  `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`.)
- **`Binding` and `DescriptorSet` decorations.** Resources are
  decorated with their assigned binding and descriptor set; the
  defaults are `Binding 0` / `DescriptorSet 0`. (Anchor: same.)
- **Read-only buffers carry `NonWritable`.** A `StructuredBuffer`
  or `ByteAddressBuffer` gets the `NonWritable` decoration in
  SPIR-V emit. (Anchor: same.)
- **`groupshared` lowers to the `Workgroup` storage class.**
  (Anchor: same.)
- **`cbuffer` / `ConstantBuffer<T>` lower to a `Uniform`-storage
  block with a `*_std140` layout struct.** (Anchor:
  `#legalizeconstantbufferloadforglsl` via
  `#phase-c-spir-v-legalization-lowering-phi-elimination`.)
- **`Sampler2D` produces a `TypeSampledImage`** (combined
  texture+sampler — SPIR-V supports the combined type natively,
  so `lowerCombinedTextureSamplers` is **skipped** for SPIR-V,
  unlike HLSL). (Anchor:
  `#phase-c-spir-v-legalization-lowering-phi-elimination`.)
- **`RWTexture2D` lowers to `OpTypeImage` with
  `Image` operand `2`** (`Sampled=2` = "image used for read/write
  access"). Writes are `OpImageWrite`; reads through subscript
  are `OpImageRead`. This exercises `legalizeImageSubscript`.
  (Anchor: `#phase-c-spir-v-legalization-lowering-phi-elimination`.)
- **`InterlockedAdd` lowers to `OpAtomicIAdd`.** Atomic
  operations survive Phase C and reach SPIR-V emit. (Anchor:
  `#phase-c-spir-v-legalization-lowering-phi-elimination` —
  `validateAtomicOperations` is called inside `legalizeIRForSPIRV`,
  per the doc.)
- **Vector `&&` lowers to `OpLogicalAnd`.** SPIR-V is in the
  `isKhronosTarget` arm of `legalizeLogicalAndOr`. (Anchor:
  `#phase-c-spir-v-legalization-lowering-phi-elimination`.)
- **`ByteAddressBuffer.Load<T>(offset)` is translated to a
  structured-buffer-style load.** SPIR-V uses
  `translateToStructuredBufferOps=true` in
  `legalizeByteAddressBufferOps`. The emitted SPIR-V contains
  an `OpAccessChain` through the underlying runtime array, not
  a `Load<T>` template call (templates don't exist in SPIR-V).
  (Anchor: `#phase-c-spir-v-legalization-lowering-phi-elimination`.)
- **A compute entry point with a renamed function name keeps
  the function symbol but the SPIR-V `OpEntryPoint`'s string
  operand is the user-visible entry-point name.** For
  `-entry myKernel`, the emit shows
  `OpEntryPoint GLCompute %myKernel "main"` — Slang names the
  SPIR-V function with the symbol `myKernel` but the SPIR-V
  entry-point string is the canonical `"main"` slot the
  driver looks up. (Anchor:
  `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`.)
- **A bitcast (e.g. `uint`→`int`) becomes `OpBitcast`.** The
  cast is preserved through Phase B/C/D legalization. (Anchor:
  `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`.)
- **`SV_DispatchThreadID` and `SV_GroupIndex` survive as
  `gl_GlobalInvocationID` / `gl_LocalInvocationIndex`
  `Input`-storage variables.** (Anchor:
  `#legalizeentrypointsforglsl-despite-the-name`.)
- **`OpAccessChain` is the lowering for buffer-element indexing.**
  An `outBuf[tid.x] = ...` becomes
  `OpAccessChain %_ptr_StorageBuffer_<T> %outBuf %int_0 %<idx>`,
  reflecting the wrapped runtime array. (Anchor:
  `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`.)
- **`OpDecorate %_runtimearr_<T> ArrayStride <N>`** appears for
  the runtime array inside the `StructuredBuffer` block.
  (Anchor: same.)
- **`OpFunctionEnd` and `OpReturn` close the entry-point
  function**, and `OpFunction %void None %<typefunc>` opens
  it. (Anchor: same.)
- **`eliminatePhis` register-allocation mode produces named
  locals.** A phi-shaped `if/else` value lowers to an
  `OpVariable ... Function` plus `OpStore` /`OpLoad` pairs
  in SPIR-V (rather than the `OpPhi` instruction that the
  default mode would leave). (Anchor:
  `#eliminatephis-with-spir-v-specific-options`.)
- **`moveGlobalVarInitializationToEntryPoints` runs for SPIR-V**:
  a module-scope `static int g = 0;` becomes a function-scope
  init in the entry point. (Anchor:
  `#phase-c-spir-v-legalization-lowering-phi-elimination`.)
- **`performForceInlining` + `performIntrinsicFunctionInlining`**
  collapse short functions: a tiny helper has no surviving
  `OpFunction` in the emitted SPIR-V. (Anchor:
  `#phase-c-spir-v-legalization-lowering-phi-elimination`.)
- **`for` loops survive as SPIR-V structured control flow**
  (`OpLoopMerge` / `OpBranchConditional`). (Anchor:
  `#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools`.)
- **Debug info is opt-in.** Without `-g`, no `DebugLine`
  extension-instruction calls appear. With `-g`, `DebugLine`
  calls appear (NonSemantic.Shader.DebugInfo.100 extension).
  (Anchor: `#option-set-toggles` —
  `targetCompilerOptions.getDebugInfoLevel()`.)

### Not testable through `slangc -target spirv-asm` (record under
`## Untested claims`)

- **spirv-link**: requires more than one input SPIR-V module
  (an `IREmbeddedDownstreamIR` of `CodeGenTarget::SPIRV`).
  Without an embedded module the downstream `compiler->link`
  branch does not fire.
- **spirv-val**: enabled only when `SLANG_RUN_SPIRV_VALIDATION`
  is non-empty or `-validate-spirv` is passed; the bundle
  doesn't change the assembly text and is not a test target.
- **spirv-opt**: invoked unconditionally but its effect on
  the emitted assembly is not deterministic enough to FileCheck.
- **`simplifyIRForSpirvLegalization` outer/inner loop bounds**
  (`kMaxIterations=8`, `kMaxFuncIterations=16`). These are
  upper bounds; no user-source pattern is documented that
  reliably hits the bound. The loop terminates by fixed-point
  in practice.
- **Forward-declared-pointer fixup loop.** The loop only fires
  on recursive struct-pointer graphs. Slang does not let user
  code declare a recursive struct (`checkForRecursiveTypes`
  errors out), so this loop is unreachable from user code.
- **`introduceExplicitGlobalContext`** (gated on
  `EnableExperimentalPasses`).
- **`invertYOfPositionOutput` / `rcpWOfPositionInput`** (gated
  on `VulkanInvertY` / `VulkanUseDxPositionW`).
- **`legalizeMeshOutputTypes`** (mesh entry points).
- **`legalizeDispatchMeshPayloadForGLSL`** (mesh-shader
  payload).
- **`collectCooperativeMetadata`** (cooperative matrix /
  vector capability set).
- **`unexportNonEmbeddableIR`** (gated on `EmbedDownstreamIR`).
- **`coverageTracing`-gated passes**
  (`instrumentCoverage`, `finalizeCoverageInstrumentationMetadata`).
- **`autodiff` / `higherOrderFunc` passes**.
- **`dynamicResourceHeap`** (SM 6.6 dynamic resource heap is
  not the SPIR-V default).
- **`insertFragmentShaderInterlock`** (requires raster-ordered
  resources in a fragment entry point).
- **`removeUnreachableCodeAfterDiscardForOpKill`** (requires
  `discard` and SPIR-V `<= 1.5` — Slang's default is `>= 1.5`
  and the pass is gated on `!shouldEmitDiscardAsDemote()`).
- **`replaceLocationIntrinsicsWithRaytracingObject`** (DXR
  entry points).
- **Pass-ordering claims**. The doc enumerates the ordered
  Phase A/B/C/D pass list; pass _existence_ is observable
  via emit side-effects, but pass _ordering_ would require
  `-dump-ir` cross-pass comparison without doc-anchored
  ordering markers. Out of scope.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. 20 to 35 `.slang` test files. Bundle is SPIR-V-target-only,
   so every file carries the single `//TEST:SIMPLE` directive
   targeting `spirv-asm`.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/target-pipelines/spirv.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/generated/design/pipeline/05-ir-passes.md`
- `docs/generated/design/pipeline/06-emit.md`
- `docs/generated/design/target-pipelines/index.md`
- `docs/generated/design/cross-cutting/targets.md`

If you would cite anything else, stop and instead record a
doc-gap finding in `README.md`.

## Source files you may consult for _verification only_

Use these to confirm a specific SPIR-V emit token. Do **not**
mine them for claims that the doc does not state.

- `source/slang/slang-emit.cpp`
- `source/slang/slang-emit-spirv.cpp`
- `source/slang/slang-ir-spirv-legalize.cpp`
- `source/slang/slang-ir-glsl-legalize.cpp`
- `source/slang/slang-ir-legalize-binary-operator.cpp`
- `source/slang/slang-ir-byte-address-legalize.cpp`
- `source/slang/slang-ir-eliminate-phis.cpp`
- `source/slang/slang-ir-explicit-global-init.cpp`

## Test directives

Default for this bundle:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute
```

This bundle does not use cross-target probes. SPIR-V is the only
target whose behaviors are observed in this file's tests.

## Lessons captured for the SPIR-V target pipeline

These are in addition to `_common.md` and `pipeline-06-emit.md`.

- **SPIR-V assembly is line-oriented and stable across runs.**
  IDs (`%N`) are not stable across compiler versions, but token
  spellings (`OpEntryPoint`, `OpCapability`, `OpExecutionMode`,
  builtin names like `GlobalInvocationId`, storage class
  spellings like `StorageBuffer` / `Workgroup` / `Uniform`) are.
  Use `{{%[0-9]+}}` or `{{%[a-zA-Z_0-9]+}}` for IDs, but match
  decoration tokens and storage classes literally.
- **The OpEntryPoint string is "main", not the function name.**
  `OpEntryPoint GLCompute %userFunc "main"` is the pattern;
  the third operand `"main"` is the canonical entry-point slot
  the driver looks up. Slang's `-entry myKernel` puts
  `%myKernel` on the function and `"main"` on the entry-point
  string — verify with the emit before pinning.
- **`OpCapability Shader` and `OpExtension
  "SPV_KHR_storage_buffer_storage_class"` are present on
  every storage-buffer-using compile.** Use them as canonical
  prelude markers.
- **`OpExecutionMode %<entry> LocalSize X Y Z`** is the
  numthreads lowering. The %entry is the function symbol, not
  the string.
- **`StorageBuffer` storage class** is what `RWStructuredBuffer`
  / `StructuredBuffer` get. `Uniform` is for `cbuffer` /
  `ConstantBuffer<T>`. `Workgroup` is for `groupshared`.
  `Input` is for entry-point varying inputs
  (`gl_GlobalInvocationID` etc.). `UniformConstant` is for
  textures / samplers / `Sampler2D`. `Function` is for
  function-scope locals.
- **`gl_GlobalInvocationID` / `gl_LocalInvocationIndex`** are
  the SPIR-V names for `SV_DispatchThreadID` /
  `SV_GroupIndex` after `legalizeEntryPointsForGLSL`. Match
  them literally.
- **The standard `OpSource Slang 1` line** is always emitted.
  Use as a sanity marker.
- **`OpAtomicIAdd` (not `InterlockedAdd`)** for `uint` atomic
  add; lowering happens inside `legalizeIRForSPIRV`.
- **`OpLogicalAnd` on `%v3bool` operands** is the element-wise
  vector `&&` lowering. SPIR-V accepts this directly; the
  `legalizeLogicalAndOr` rewrite to element-wise selects is
  the IR-level shape but the emitted SPIR-V can collapse back
  to a single `OpLogicalAnd` when the operand is already a
  vector-bool.
- **`OpAccessChain %_ptr_StorageBuffer_<elem> %buf %int_0 %idx`**
  is the buffer-element indexing pattern. The leading `%int_0`
  selects the runtime-array member of the wrapping struct; the
  second index selects the element.
- **`_runtimearr_<T>`** is the SPIR-V name for the runtime
  array type inside the `StructuredBuffer` wrapper struct.
- **Decorations live in a contiguous block** after `; Annotations`.
  Use `// CHECK-DAG:` if order between decorations is
  irrelevant to the claim.
- **DCE strips locally-unused code before SPIR-V emit.** Always
  write the value you want to observe into an
  `RWStructuredBuffer` parameter — pure-internal computations
  are removed before the CHECK can find them.
- **The compiler inlines short helpers.** A tiny `int sum(S
  s) { ... }` may not appear as its own `OpFunction` in the
  emitted SPIR-V — `performForceInlining` and
  `performIntrinsicFunctionInlining` (the SPIR-V-only second
  inlining pass) fold it into `main`. To observe a separate
  function, mark it `[noinline]` or make the body large
  enough that the inliner declines.
- **`ConstantBuffer<T>` and `cbuffer { ... }` both emit a
  `*_std140`-suffixed wrapper struct** with `Block`
  decoration and `Uniform` storage class. The std140 layout
  comes from `legalizeConstantBufferLoadForGLSL`.
- **An entry-point function named anything (`main`,
  `myKernel`, ...) shows up as `%name = OpFunction` and the
  SPIR-V entry-point string remains `"main"`.** Slang does
  not rewrite the symbol but does normalize the
  driver-visible entry-point name slot.
- **Slang emits `OpSource Slang 1` even with `-g`.** Debug
  info uses `NonSemantic.Shader.DebugInfo.100` `DebugLine`
  instructions (`OpExtInst ... DebugLine ...`) in addition.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `target-pipelines/spirv.md` (or one of the listed
      secondary docs).
- [ ] Each `.slang` file declares
      `//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute`
      as its single directive.
- [ ] CHECK patterns avoid raw `[[...]]` and use FileCheck
      wildcards for SPIR-V `%N` IDs.
- [ ] No test depends on a GPU; compute-only entry points;
      no DXR / mesh / raster pipeline stages.
- [ ] `## Doc gaps observed` records claims that lack a
      checkable marker in the doc, or behaviors observed in
      slangc emit that the doc does not currently describe.
- [ ] `## Untested claims` enumerates the
      spirv-link / spirv-val / spirv-opt / DXR / mesh /
      capability-gated items.
