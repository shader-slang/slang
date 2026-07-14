---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-29T15:31:21Z
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
watched_paths_digest: 1f43055876c44da0b02f3e913b22e16142720f3f452022bd3fdfcf1687346392
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Decorations

This page is the per-opcode reference for the `Decoration` family —
the largest single family in
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua),
spanning lines ~1623-2484. Decorations attach metadata to other IR
instructions: names, layout binding, control-flow hints,
target-specific intrinsic spellings, capability requirements,
inlining preferences, autodiff markers, and so on.

The intended reader is a compiler engineer reading IR around a
function, type, or variable and trying to identify what each
decoration says about it.

## Source

The opcodes live under the top-level `Decoration` entry of
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) at
line ~1623. Per-opcode info (names, operand counts) is registered
in
[slang-ir-insts-info.cpp](../../../../source/slang/slang-ir-insts-info.cpp).
C++ wrappers are declared in
[slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h).
Infrastructure (op flags, `IRBuilder` helpers) is in
[slang-ir.h](../../../../source/slang/slang-ir.h) and
[slang-ir.cpp](../../../../source/slang/slang-ir.cpp).

Most decorations originate from AST-side modifiers and attributes:
the helpers in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
(`applyModifiers*` family, `addNameHintDecoration`, ...) attach
them to the IR instruction produced by the corresponding
declaration. The autodiff markers, primal/diff annotations, and
SPIR-V backend hints are introduced by the IR passes
themselves.

## Family hierarchy

```mermaid
flowchart TD
  IRInst --> Decoration
  Decoration --> Naming["Naming and provenance"]
  Decoration --> LayoutBinding["Layout and binding"]
  Decoration --> LoopBranchHints["Loop and branch hints"]
  Decoration --> TargetSpecificDecoration
  Decoration --> CapAvail["Capability and availability"]
  Decoration --> IO["Interpolation and IO"]
  Decoration --> Stage["Entry-point / stage"]
  Decoration --> Linkage["Linkage and lifetime"]
  Decoration --> Inlining["Inlining and optimization"]
  Decoration --> AutoDiff["Differentiation markers"]
  Decoration --> SpecConform["Specialization and conformance"]
  Decoration --> SpvHints["SPIR-V backend hints"]
  Decoration --> Mesh["Mesh-shader and per-vertex"]
  Decoration --> Debug["Debug and reflection"]
  Decoration --> Misc["Other"]
  TargetSpecificDecoration --> TargetSpecificDefinitionDecoration
  TargetSpecificDecoration --> requirePreludeNode[requirePrelude]
  TargetSpecificDefinitionDecoration --> targetNode[target]
  TargetSpecificDefinitionDecoration --> targetIntrinsicNode[targetIntrinsic]
  AutoDiff --> AutodiffInstDecoration
  AutoDiff --> CheckpointHintDecoration
  AutodiffInstDecoration --> primalInst[primalInstDecoration]
  AutodiffInstDecoration --> diffInst[diffInstDecoration]
  AutodiffInstDecoration --> mixedDiffInst[mixedDiffInstDecoration]
  AutodiffInstDecoration --> RecomputeBlockDecoration
  Stage --> GeometryInputPrimitiveTypeDecoration
  IO --> MeshOutputDecoration
  IO --> StageAccessDecoration
```

## Opcodes

### Naming and provenance

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `nameHint` | `IRNameHintDecoration` | `nameOperand: IRStringLit` | | `NameModifier` and synthesized name preservation in `slang-lower-to-ir.cpp` | Carries a human-readable name across IR passes; backends use it for variable / function naming. |
| `highLevelDecl` | `IRHighLevelDeclDecoration` | `declOperand: IRPtrLit` | | `slang-lower-to-ir.cpp` lowering (records source `Decl*`) | Records a pointer to the originating AST `Decl` (debug / diagnostic aid). |
| `BuiltinDecoration` | `IRBuiltinDecoration` | — | | Core-module lowering | Marks an inst as a compiler-builtin. |
| `KnownBuiltinDecoration` | `IRKnownBuiltinDecoration` | `nameOperand: IRIntLit` | | Core-module lowering | Names a builtin by enum tag so later passes can find it. |
| `UserTypeName` | `IRUserTypeNameDecoration` | `userTypeName: IRStringLit` | | `slang-lower-to-ir.cpp` (reflection metadata) | Records the original user type name for a shader parameter. |
| `COMInterface` | `IRComInterfaceDecoration` | — | | `COMInterfaceAttribute` | Marks an interface as a COM interface declaration. |
| `COMWitnessDecoration` | `IRCOMWitnessDecoration` | `witnessTable` | | `slang-check-conformance.cpp` (COM conformance) | Marks a class type as a COM interface implementation. |
| `UserExtern` | `IRUserExternDecoration` | — | | `extern` declarations | Marks an inst as coming from user-side `extern`. |
| `transitory` | `IRTransitoryDecoration` | — | | (synthesized) | Marks an inst as transitory; should never survive into the output. |

### Layout and binding

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `layout` | `IRLayoutDecoration` | (variadic, `min=1`) | | Layout-pass output | Attaches a `Layout` opcode (see [metadata.md](metadata.md)) to a parameter or type. |
| `AlignedAddressDecoration` | `IRAlignedAddressDecoration` | `alignment` | | (synthesized) | Marks an address inst as aligned to a specific byte boundary. |
| `SizeAndAlignment` | `IRSizeAndAlignmentDecoration` | `layoutNameOperand, sizeOperand: IRIntLit, alignmentOperand: IRIntLit` | | (synthesized) | Records size/alignment of a type under a named layout. |
| `Offset` | `IROffsetDecoration` | `layoutNameOperand, offsetOperand: IRIntLit` | | (synthesized) | Records the offset of a struct field under a named layout. |
| `packoffset` | `IRPackOffsetDecoration` | `registerOffset: IRIntLit, componentOffset: IRIntLit` | | `[packoffset(...)]` modifier | HLSL packoffset binding. |
| `glslLocation` | `IRGLSLLocationDecoration` | `location: IRIntLit` | | `[vk::location(...)]` modifier | GLSL / Vulkan location binding. |
| `glslOffset` | `IRGLSLOffsetDecoration` | `offset: IRIntLit` | | `[vk::offset(...)]` modifier | GLSL / Vulkan offset binding. |
| `vkStructOffset` | `IRVkStructOffsetDecoration` | `offset: IRIntLit` | | Vulkan struct-offset modifier | Vulkan struct-member offset. |
| `HasExplicitHLSLBinding` | `IRHasExplicitHLSLBindingDecoration` | — | | `register(...)` modifier | Marks a parameter as having an explicit HLSL register binding. |
| `BinaryInterfaceType` | `IRBinaryInterfaceTypeDecoration` | — | | (synthesized) | Marks a type as being used as a binary-interface type so `legalizeEmptyType` does not eliminate it. |
| `PhysicalType` | `IRPhysicalTypeDecoration` | (variadic, `min=1`) | | (synthesized) | Marks the physical lowered type of a logical value. |
| `output` | `IRGlobalOutputDecoration` | — | | `out` modifier on global parameter | Marks a global parameter as an output. |
| `input` | `IRGlobalInputDecoration` | — | | `in` modifier on global parameter | Marks a global parameter as an input. |
| `glslOuterArray` | `IRGLSLOuterArrayDecoration` | `outerArrayNameOperand: IRStringLit` | | GLSL legalization | Records the outer-array variable name for GLSL emission. |

### Loop and branch hints

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `branch` | `IRBranchDecoration` | — | | `[branch]` attribute | Hints the backend to emit a branching select. |
| `flatten` | `IRFlattenDecoration` | — | | `[flatten]` attribute | Hints the backend to flatten a conditional. |
| `loopControl` | `IRLoopControlDecoration` | `modeOperand: IRConstant` | | `[unroll]` / `[loop]` attributes | Records loop-control mode (unroll, loop, ...). |
| `loopMaxIters` | `IRLoopMaxItersDecoration` | (variadic, `min=1`) | | `[loop_count(...)]` attribute | Records the maximum-iteration bound for a loop. |
| `loopExitPrimalValue` | `IRLoopExitPrimalValueDecoration` | `targetInst, loopExitValInst` | | (synthesized by autodiff) | Records the primal value of an exit-condition for reverse-mode use. |
| `ForceUnroll` | `IRForceUnrollDecoration` | — | | `[ForceUnroll]` attribute | Forces loop unrolling. |
| `loopCounterDecoration` | `IRLoopCounterDecoration` | — | | (synthesized by autodiff) | Marks an instruction as a loop counter. |
| `loopCounterUpdateDecoration` | `IRLoopCounterUpdateDecoration` | — | | (synthesized by autodiff) | Marks the per-iteration update of a loop counter. |

### Target-specific definition and intrinsics

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `target` | `IRTargetDecoration` | (variadic, `min=1`) | | `__target(...)` core-module markers | Marks a function as the implementation for one specific target. |
| `targetIntrinsic` | `IRTargetIntrinsicDecoration` | `target, definitionOperand: IRStringLit` | | `__target_intrinsic(...)` core-module markers | Carries the target-specific spelling of an intrinsic; cite the glossary for `target intrinsic`. |
| `requirePrelude` | `IRRequirePreludeDecoration` | (variadic, `min=2`) | | `__require_prelude(...)` core-module markers | Records a prelude snippet that the backend must include when the decorated function is reachable. |
| `intrinsicOp` | `IRIntrinsicOpDecoration` | `intrinsicOpOperand: IRIntLit` | | `__intrinsic_op(...)` core-module markers | Identifies the built-in IR opcode that implements an intrinsic. |
| `spirvOpDecoration` | `IRSPIRVOpDecoration` | (variadic, `min=1`) | | `__spirv_op(...)` core-module markers | Records the SPIR-V opcode for a function. |

### Capability and availability

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `requireCapabilityAtom` | `IRRequireCapabilityAtomDecoration` | `capabilityAtomOperand: IRConstant` | | `require(...)` / capability declarations | Requires one capability atom to be available. |
| `requireSPIRVVersion` | `IRRequireSPIRVVersionDecoration` | `SPIRVVersionOperand: IRConstant` | | SPIR-V version requirement | Records a minimum SPIR-V version. |
| `requireGLSLVersion` | `IRRequireGLSLVersionDecoration` | `languageVersionOperand: IRConstant` | | GLSL version requirement | Records a minimum GLSL version. |
| `requireGLSLExtension` | `IRRequireGLSLExtensionDecoration` | `extensionNameOperand: IRStringLit` | | GLSL extension requirement | Records a required GLSL extension. |
| `requireWGSLExtension` | `IRRequireWGSLExtensionDecoration` | `extensionNameOperand: IRStringLit` | | WGSL extension requirement | Records a required WGSL extension. |
| `requireCUDASMVersion` | `IRRequireCUDASMVersionDecoration` | `CUDASMVersionOperand: IRConstant` | | CUDA SM version requirement | Records a minimum CUDA SM version. |
| `availableInDownstreamIR` | `IRAvailableInDownstreamIRDecoration` | (variadic, `min=1`) | | (synthesized) | Marks an inst as available through the downstream-IR import. |
| `RequireSPIRVDescriptorIndexingExtensionDecoration` | `IRRequireSPIRVDescriptorIndexingExtensionDecoration` | — | | (synthesized) | Marks a function as requiring SPIR-V descriptor indexing. |
| `requiresNVAPI` | `IRRequiresNVAPIDecoration` | — | | NVAPI core-module markers | Requires NVAPI prelude when targeting D3D. |
| `nvapiMagic` | `IRNVAPIMagicDecoration` | `nameOperand: IRStringLit` | | NVAPI core-module markers | Marks an inst as part of the NVAPI magic naming. |
| `nvapiSlot` | `IRNVAPISlotDecoration` | `registerNameOperand: IRStringLit, spaceNameOperand: IRStringLit` | | NVAPI core-module markers | Records the NVAPI register/space binding. |

### Interpolation and shader IO

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `interpolationMode` | `IRInterpolationModeDecoration` | `modeOperand: IRConstant` | | `linear` / `noperspective` / etc. modifiers | Records the interpolation mode of a varying parameter. |
| `TargetSystemValue` | `IRTargetSystemValueDecoration` | `semanticOperand: IRStringLit, index: IRIntLit` | | `[vk::system_value(...)]` etc. | Records the target-specific system-value binding. |
| `semantic` | `IRSemanticDecoration` | `semanticNameOperand: IRStringLit, semanticIndexOperand: IRIntLit` | | `HLSLSimpleSemantic` and `HLSLLayoutSemantic` AST nodes | Records the HLSL semantic on a parameter or field. |
| `raypayload` | `IRRayPayloadDecoration` | — | | `[shader("anyhit"/"closesthit")]` ray-payload markers | Marks a parameter as a ray payload. |
| `vulkanRayPayload` | `IRVulkanRayPayloadDecoration` | — | | Vulkan raytracing payload | Marks a variable as a Vulkan ray payload (outgoing). |
| `vulkanRayPayloadIn` | `IRVulkanRayPayloadInDecoration` | — | | Vulkan raytracing payload | Marks a variable as a Vulkan ray payload (incoming). |
| `vulkanHitAttributes` | `IRVulkanHitAttributesDecoration` | — | | Vulkan raytracing hit attributes | Marks a variable as Vulkan hit attributes. |
| `vulkanHitObjectAttributes` | `IRVulkanHitObjectAttributesDecoration` | — | | Vulkan raytracing hit-object attributes | Marks a variable as Vulkan hit-object attributes. |
| `vulkanCallablePayload` | `IRVulkanCallablePayloadDecoration` | — | | Vulkan callable-shader payload | Marks a variable as a Vulkan callable payload (outgoing). |
| `vulkanCallablePayloadIn` | `IRVulkanCallablePayloadInDecoration` | — | | Vulkan callable-shader payload | Marks a variable as a Vulkan callable payload (incoming). |
| `earlyDepthStencil` | `IREarlyDepthStencilDecoration` | — | | `[earlydepthstencil]` attribute | Requests early-depth-stencil test for a pixel shader. |
| `glslFragDepthGreater` | `IRGLSLFragDepthGreaterDecoration` | — | | (synthesized by GLSL legalization from `SV_DepthGreaterEqual`) | Marks a fragment entry point whose `gl_FragDepth` only ever increases the fixed-function depth. |
| `glslFragDepthLess` | `IRGLSLFragDepthLessDecoration` | — | | (synthesized by GLSL legalization from `SV_DepthLessEqual`) | Marks a fragment entry point whose `gl_FragDepth` only ever decreases the fixed-function depth. |
| `precise` | `IRPreciseDecoration` | — | | `precise` modifier | Requests bit-precise math. |
| `format` | `IRFormatDecoration` | `formatOperand: IRConstant` | | `[format(...)]` attribute | Records the image format for a UAV. |
| `perprimitive` | `IRGLSLPrimitivesRateDecoration` | — | | `[perprimitive]` modifier | GLSL `per_primitiveEXT` rate qualifier. |

### Mesh shader, geometry shader, and per-vertex

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `pointPrimitiveType` | `IRPointInputPrimitiveTypeDecoration` | — | | `[shader("geometry")]` point variant | Marks a geometry input as `point`. |
| `linePrimitiveType` | `IRLineInputPrimitiveTypeDecoration` | — | | Geometry shader `line` variant | Marks a geometry input as `line`. |
| `trianglePrimitiveType` | `IRTriangleInputPrimitiveTypeDecoration` | — | | Geometry shader `triangle` variant | Marks a geometry input as `triangle`. |
| `lineAdjPrimitiveType` | `IRLineAdjInputPrimitiveTypeDecoration` | — | | Geometry shader `lineadj` | Marks a geometry input as `lineadj`. |
| `triangleAdjPrimitiveType` | `IRTriangleAdjInputPrimitiveTypeDecoration` | — | | Geometry shader `triangleadj` | Marks a geometry input as `triangleadj`. |
| `streamOutputTypeDecoration` | `IRStreamOutputTypeDecoration` | `streamType: IRHLSLStreamOutputType` | | Geometry shader output declaration | Records the stream-output type. |
| `vertices` | `IRVerticesDecoration` | (variadic, `min=1`) | | Mesh-shader vertex output | Marks a parameter as the mesh-shader vertex output. |
| `indices` | `IRIndicesDecoration` | (variadic, `min=1`) | | Mesh-shader index output | Marks a parameter as the mesh-shader index output. |
| `primitives` | `IRPrimitivesDecoration` | (variadic, `min=1`) | | Mesh-shader primitive output | Marks a parameter as the mesh-shader primitive output. |
| `HLSLMeshPayloadDecoration` | `IRHLSLMeshPayloadDecoration` | — | | HLSL mesh-shader payload | Marks a parameter as the HLSL mesh-payload. |
| `PositionOutput` | `IRGLPositionOutputDecoration` | — | | `SV_Position` system value (output) | Marks a varying as the `gl_Position` output. |
| `PositionInput` | `IRGLPositionInputDecoration` | — | | `SV_Position` system value (input) | Marks a varying as the `gl_Position` input. |
| `PerVertex` | `IRPerVertexDecoration` | — | | `[pervertex]` (HLSL pixel-shader) | Marks a fragment-shader input as per-vertex. |
| `stageReadAccess` | `IRStageReadAccessDecoration` | — | | (synthesized) | Records the read-access stage of a resource. |
| `stageWriteAccess` | `IRStageWriteAccessDecoration` | — | | (synthesized) | Records the write-access stage of a resource. |

### Entry-point and stage

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `entryPoint` | `IREntryPointDecoration` | `profileInst: IRIntLit, name: IRStringLit, moduleName?: IRStringLit` | | `[shader(...)]` attribute | Marks a function as an entry point with a given profile and name. |
| `entryPointParam` | `IREntryPointParamDecoration` | `entryPoint: IRFunc` | | (synthesized) | Marks a global parameter that was moved from an entry-point parameter. |
| `patchConstantFunc` | `IRPatchConstantFuncDecoration` | `func: IRInst` | | `[patchconstantfunc(...)]` attribute | Records the hull-shader patch-constant function. |
| `maxTessFactor` | `IRMaxTessFactorDecoration` | `maxTessFactor: IRFloatLit` | | `[maxtessfactor(...)]` attribute | Records the maximum tessellation factor. |
| `outputControlPoints` | `IROutputControlPointsDecoration` | `controlPointCount: IRIntLit` | | `[outputcontrolpoints(...)]` attribute | Records the hull-shader output control-point count. |
| `outputTopology` | `IROutputTopologyDecoration` | `topology: IRStringLit, topologyTypeOperand: IRIntLit` | | `[outputtopology(...)]` attribute | Records the hull-shader output topology. |
| `partitioning` | `IRPartitioningDecoration` | `partitioning: IRStringLit` | | `[partitioning(...)]` attribute | Records the tessellation partitioning mode. |
| `domain` | `IRDomainDecoration` | `domain: IRStringLit` | | `[domain(...)]` attribute | Records the tessellation domain. |
| `maxVertexCount` | `IRMaxVertexCountDecoration` | `count: IRIntLit` | | `[maxvertexcount(...)]` attribute | Records the geometry-shader vertex-count limit. |
| `instance` | `IRInstanceDecoration` | `count: IRIntLit` | | `[instance(...)]` attribute | Records the geometry-shader instance count. |
| `numThreads` | `IRNumThreadsDecoration` | (variadic, `min=3`) | | `[numthreads(x,y,z)]` attribute | Records the compute-shader workgroup size. |
| `fpDenormalPreserve` | `IRFpDenormalPreserveDecoration` | `width: IRIntLit` | | FP denormal control attribute | Requests denormal-preserve behavior at a given precision. |
| `fpDenormalFlushToZero` | `IRFpDenormalFlushToZeroDecoration` | `width: IRIntLit` | | FP denormal control attribute | Requests denormal-flush-to-zero. |
| `waveSize` | `IRWaveSizeDecoration` | `numLanes: IRIntLit` | | `[WaveSize(...)]` attribute | Requests a specific wave size. |
| `DerivativeGroupQuad` | `IRDerivativeGroupQuadDecoration` | — | | Compute-shader derivative-group attribute | Quad-form derivative grouping. |
| `DerivativeGroupLinear` | `IRDerivativeGroupLinearDecoration` | — | | Compute-shader derivative-group attribute | Linear-form derivative grouping. |
| `MaximallyReconverges` | `IRMaximallyReconvergesDecoration` | — | | Quad-control execution mode | Requests maximal-reconvergence execution. |
| `QuadDerivatives` | `IRQuadDerivativesDecoration` | — | | Quad-control execution mode | Requests quad-derivative execution. |
| `RequireFullQuads` | `IRRequireFullQuadsDecoration` | — | | Quad-control execution mode | Requires full quads. |

### Linkage and lifetime

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `import` | `IRImportDecoration` | (variadic, `min=1`) | | `import` declaration | Marks an inst as imported under a mangled name. |
| `export` | `IRExportDecoration` | (variadic, `min=1`) | | `export` declaration | Marks an inst as exported under a mangled name. |
| `public` | `IRPublicDecoration` | — | | `public` modifier | Public visibility. |
| `hlslExport` | `IRHLSLExportDecoration` | — | | `[export]` attribute (HLSL libraries) | HLSL export. |
| `downstreamModuleExport` | `IRDownstreamModuleExportDecoration` | — | | (synthesized) | Marks an inst as exported through the downstream module bridge. |
| `downstreamModuleImport` | `IRDownstreamModuleImportDecoration` | — | | (synthesized) | Marks an inst as imported through the downstream module bridge. |
| `externCpp` | `IRExternCppDecoration` | `nameOperand: IRStringLit` | | `__extern_cpp` modifier | Emits the function without C++ mangling. |
| `externC` | `IRExternCDecoration` | — | | `extern "C"` modifier | Wraps a function in `extern "C"`. |
| `dllImport` | `IRDllImportDecoration` | `libraryNameOperand: IRStringLit, functionNameOperand: IRStringLit` | | `__dllimport` modifier | Generates dynamic-library load logic. |
| `dllExport` | `IRDllExportDecoration` | `functionNameOperand: IRStringLit` | | `__dllexport` modifier | Generates DLL-export wrapper. |
| `cudaDeviceExport` | `IRCudaDeviceExportDecoration` | (variadic, `min=1`) | | `__cuda_device_export` modifier | Exports a function as a CUDA `__device__` function. |
| `CudaKernel` | `IRCudaKernelDecoration` | — | | `[CudaKernel]` attribute | Marks a function as a CUDA kernel. |
| `CudaHost` | `IRCudaHostDecoration` | — | | `[CudaHost]` attribute | Marks a function as a CUDA host helper. |
| `TorchEntryPoint` | `IRTorchEntryPointDecoration` | `functionNameOperand: IRStringLit` | | `[TorchEntryPoint]` attribute | Marks a Torch / Slang interop entry point. |
| `AutoPyBindCUDA` | `IRAutoPyBindCudaDecoration` | `functionNameOperand: IRStringLit` | | `[AutoPyBindCUDA]` attribute | Generates Python bindings for a CUDA function. |
| `CudaKernelFwdDiffRef` | `IRCudaKernelForwardDerivativeDecoration` | `forwardDerivativeFunc?` | | (synthesized by autodiff) | Records the forward-mode derivative of a CUDA kernel. |
| `CudaKernelBwdDiffRef` | `IRCudaKernelBackwardDerivativeDecoration` | `backwardDerivativeFunc?` | | (synthesized by autodiff) | Records the reverse-mode derivative of a CUDA kernel. |
| `PyBindExportFuncInfo` | `IRAutoPyBindExportInfoDecoration` | — | | `[AutoPyBindCUDA]` lowering | Reflection info for Python binding generation. |
| `PyExportDecoration` | `IRPyExportDecoration` | `exportNameOperand: IRStringLit` | | `[PyExport(...)]` attribute | Marks a function as exported to Python. |
| `dependsOn` | `IRDependsOnDecoration` | (variadic, `min=1`) | | `[dependsOn(...)]` attribute | Adds an extra dependency edge to the parent inst. |
| `keepAlive` | `IRKeepAliveDecoration` | — | | `[keepAlive]` attribute or synthesized | Prevents DCE from eliminating the inst. |
| `TargetBuiltinVar` | `IRTargetBuiltinVarDecoration` | `builtinVarOperand: IRIntLit` | | (synthesized) | Marks a global variable as a target builtin variable. |

### Inlining and optimization

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `unsafeForceInlineEarly` | `IRUnsafeForceInlineEarlyDecoration` | — | | `[__unsafe_force_inline_early]` attribute | Inlines calls immediately after codegen. |
| `ForceInline` | `IRForceInlineDecoration` | — | | `[ForceInline]` attribute | Inlines calls during normal IR passes. |
| `AllowPreTranslationInlining` | `IRAllowPreTranslationInliningDecoration` | — | | (synthesized) | Permits inlining after translation passes. |
| `noInline` | `IRNoInlineDecoration` | — | | `[noinline]` attribute | Suppresses inlining. |
| `noRefInline` | `IRNoRefInlineDecoration` | — | | (synthesized) | Suppresses inlining of reference-type calls. |
| `alwaysFold` | `IRAlwaysFoldIntoUseSiteDecoration` | — | | `[__alwaysFold]` attribute | Always fold call result into its use site. |
| `noSideEffect` | `IRNoSideEffectDecoration` | — | | `[__NoSideEffect]` attribute | Marks a callee as side-effect free. |
| `ignoreSideEffectsDecoration` | `IRIgnoreSideEffectsDecoration` | — | | (synthesized) | DCE may treat the call as side-effect free. |
| `NonDynamicUniformReturnDecoration` | `IRNonDynamicUniformReturnDecoration` | — | | (synthesized) | Marks a function whose return value is never dynamically uniform. |
| `optimizableTypeDecoration` | `IROptimizableTypeDecoration` | — | | (synthesized) | Marks a type as eligible for field trimming. |
| `readNone` | `IRReadNoneDecoration` | — | | `[readnone]` attribute | Marks a function as pure (no reads, no writes). |
| `DisableCopyEliminationDecoration` | `IRDisableCopyEliminationDecoration` | — | | (synthesized) | Prevents copy-elimination on the inst. |
| `nonCopyable` | `IRNonCopyableTypeDecoration` | — | | `[NonCopyable]` modifier | Marks a type as non-copyable; SSA skips it. |
| `DynamicUniform` | `IRDynamicUniformDecoration` | — | | (synthesized) | Marks a value as dynamically uniform. |
| `bindExistentialSlots` | `IRBindExistentialSlotsDecoration` | — | | (synthesized) | Records existential-binding slot info. |
| `DefaultValue` | `IRDefaultValueDecoration` | (variadic, `min=1`) | | Default-value modifier | Records the default value of a parameter or member. |
| `InParamProxyVar` | `IRInParamProxyVarDecoration` | (variadic, `min=1`) | | (synthesized) | Marks a local var as the legacy-mutated form of an `in` parameter. |
| `TempCallArgImmutableVar` | `IRTempCallArgImmutableVarDecoration` | — | | (synthesized) | Marks a temporary used to materialize an immutable call argument. |
| `TempCallArgVar` | `IRTempCallArgVarDecoration` | — | | (synthesized) | Marks a temporary used to materialize a mutable call argument. |
| `GlobalVariableShadowingGlobalParameterDecoration` | `IRGlobalVariableShadowingGlobalParameterDecoration` | (variadic, `min=2`) | | (synthesized) | Marks a global var that shadows a global parameter. |

### Specialization, conformance, and existentials

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `SpecializeDecoration` | `IRSpecializeDecoration` | — | | (synthesized) | Hints the specialization pass to specialize the inst. |
| `SpecializationDepthDecoration` | `IRSpecializationDepthDecoration` | `specializationDepthOperand: IRIntLit` | | (synthesized) | Records how deeply specialized the inst is. |
| `SequentialIDDecoration` | `IRSequentialIDDecoration` | `sequentialIdOperand: IRIntLit` | | (synthesized) | Stable integer ID used by `GetSequentialID`. |
| `DynamicDispatchWitnessDecoration` | `IRDynamicDispatchWitnessDecoration` | — | | (synthesized) | Marks a witness table as participating in dynamic dispatch. |
| `StaticRequirementDecoration` | `IRStaticRequirementDecoration` | — | | (synthesized) | Marks an interface requirement as static. |
| `BuiltinRequirementDecoration` | `IRBuiltinRequirementDecoration` | `kindOperand: IRIntLit` | | `slang-lower-to-ir.cpp` (requirement-key lowering of a `BuiltinRequirementModifier`) | Marks an interface requirement key with the `BuiltinRequirementKind` role (e.g. `IDifferentiable.Differential`) of the built-in requirement it represents, so consumers identify it by role rather than by position in the requirement list. |
| `DispatchFuncDecoration` | `IRDispatchFuncDecoration` | `func` | | (synthesized) | Records the dispatch function for an interface call. |
| `TypeConstraintDecoration` | `IRTypeConstraintDecoration` | `constraintType` | | `GenericTypeConstraintDecl` lowering | Records the interface constraint of a generic parameter. |
| `ResultWitness` | `IRResultWitnessDecoration` | `witness` | | (synthesized) | Records the original interface witness when a function used to return an existential. |
| `RTTI_typeSize` | `IRRTTITypeSizeDecoration` | `typeSizeOperand: IRIntLit` | | (synthesized) | Records the size used by an RTTI object. |
| `AnyValueSize` | `IRAnyValueSizeDecoration` | `sizeOperand: IRIntLit` | | (synthesized) | Records the `AnyValueType` blob size on a type. |
| `SpecializationConstantDecoration` | `IRSpecializationConstantDecoration` | (variadic, `min=1`) | | `[SpecializationConstant]` attribute | Marks a global as a specialization constant. |

### Differentiation markers

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `AutoDiffOriginalValueDecoration` | `IRAutoDiffOriginalValueDecoration` | `originalValue` | | (synthesized by autodiff) | Records the original (pre-transcribe) value. |
| `AutoDiffBuiltinDecoration` | `IRAutoDiffBuiltinDecoration` | — | | (synthesized by autodiff) | Marks a type as an autodiff built-in. |
| `BackwardDerivativePrimalContextDecoration` | `IRBackwardDerivativePrimalContextDecoration` | `backwardDerivativePrimalContextVar` | | (synthesized by autodiff) | Records the primal-context variable of a reverse-mode function. |
| `BackwardDerivativePrimalReturnDecoration` | `IRBackwardDerivativePrimalReturnDecoration` | `backwardDerivativePrimalReturnValue` | | (synthesized by autodiff) | Records the primal-return value of a reverse-mode function. |
| `PrimalContextDecoration` | `IRPrimalContextDecoration` | — | | (synthesized by autodiff) | Marks a parameter as the autodiff primal context. |
| `ParamsContextDecoration` | `IRParamsContextDecoration` | `value` | | (synthesized by autodiff) | Records the parameters context for autodiff. |
| `primalInstDecoration` | `IRPrimalInstDecoration` | — | | (synthesized by autodiff) | Marks an inst as computing a primal value. |
| `diffInstDecoration` | `IRDifferentialInstDecoration` | `primalType: IRType, primalInst?, witness?` | | (synthesized by autodiff) | Marks an inst as computing a differential value, with link back to the primal inst. |
| `mixedDiffInstDecoration` | `IRMixedDifferentialInstDecoration` | `pairType: IRType` | | (synthesized by autodiff) | Marks an inst as computing both primal and differential. |
| `RecomputeBlockDecoration` | `IRRecomputeBlockDecoration` | — | | (synthesized by autodiff) | Marks a block as a recomputation block. |
| `primalValueKey` | `IRPrimalValueStructKeyDecoration` | `firstKey: IRStructKey, secondKey: IRStructKey` | | (synthesized by autodiff) | Records the keys used to store a primal value in the intermediate-context struct. |
| `primalElementType` | `IRPrimalElementTypeDecoration` | `primalElementType` | | (synthesized by autodiff) | Records the primal element type of a forward-diffed `updateElement`. |
| `IntermediateContextFieldDifferentialTypeDecoration` | `IRIntermediateContextFieldDifferentialTypeDecoration` | `differentialWitness` | | (synthesized by autodiff) | Records the differential type of an intermediate-context field. |
| `ReturnValueContextFieldDecoration` | `IRReturnValueContextFieldDecoration` | — | | (synthesized by autodiff) | Marks an intermediate-context field as the return value. |
| `derivativeMemberDecoration` | `IRDerivativeMemberDecoration` | `derivativeMemberStructKey` | | (synthesized by autodiff) | Cross-references a differential member of a type. |
| `treatCallAsDifferentiableDecoration` | `IRTreatCallAsDifferentiableDecoration` | — | | (synthesized by autodiff) | Forces a call to be treated as differentiable. |
| `differentiableCallDecoration` | `IRDifferentiableCallDecoration` | — | | (synthesized by autodiff) | Marks a call as an explicitly differentiable invocation. |
| `PreferCheckpointDecoration` | `IRPreferCheckpointDecoration` | — | | `[PreferCheckpoint]` attribute | Hints that the result should be checkpointed for reverse mode. |
| `PreferRecomputeDecoration` | `IRPreferRecomputeDecoration` | — | | `[PreferRecompute]` attribute | Hints that the result should be recomputed for reverse mode. |
| `DifferentiableTypeDictionaryDecoration` | `IRDifferentiableTypeDictionaryDecoration` | (children) | P | (synthesized by autodiff) | Parent of the per-type differentiable-type dictionary entries. |

### SPIR-V backend hints

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `spvBufferBlock` | `IRSPIRVBufferBlockDecoration` | — | | (synthesized for SPIR-V) | Requests SPIR-V `BufferBlock` decoration on a struct. |
| `spvBlock` | `IRSPIRVBlockDecoration` | — | | (synthesized for SPIR-V) | Requests SPIR-V `Block` decoration on a struct. |
| `NonUniformResource` | `IRSPIRVNonUniformResourceDecoration` | `SPIRVNonUniformResourceOperand: IRConstant` | | `NonUniformResourceIndex` lowering | Marks a SPIR-V inst as `NonUniform`. |
| `MemoryQualifierSetDecoration` | `IRMemoryQualifierSetDecoration` | (variadic, `min=1`) | | Memory-qualifier modifiers | Records memory-qualifier flag bits. |

### Debug and reflection

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `DebugLocation` | `IRDebugLocationDecoration` | `source, line, col` | | (synthesized by debug-info pass) | Attaches a debug source location to an inst. |
| `DebugFunction` | `IRDebugFuncDecoration` | `debugFunc` | | (synthesized by debug-info pass) | Links a function to its `DebugFunction` metadata opcode. |
| `CounterBuffer` | `IRCounterBufferDecoration` | `counterBuffer` | | `[counter]` attribute | Records the associated UAV counter buffer. |

### Other

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `BitFieldAccessorDecoration` | `IRBitFieldAccessorDecoration` | (variadic, `min=3`) | | `[bitfield]` attribute | Records bitfield accessor info (backing key, width, offset). |
| `constructor` | `IRConstructorDecoration` | (variadic, `min=1`) | | `__init` declaration | Marks a function as a constructor. |
| `method` | `IRMethodDecoration` | — | | Member-function declaration | Marks a function as a method. |
| `FloatingPointModeOverride` | `IRFloatingPointModeOverrideDecoration` | (variadic, `min=1`) | | FP-mode override attribute | Overrides the floating-point mode for one function. |
| `experimentalModule` | `IRExperimentalModuleDecoration` | — | | `__experimental` modifier | Marks a module as experimental. |
| `DisallowSpecializationWithExistentialsDecoration` | `IRDisallowSpecializationWithExistentialsDecoration` | — | | (synthesized) | Prevents specialization with existential arguments. |

## Notable opcodes

### `nameHint` / `NameHintDecoration`

`nameHint` carries a human-readable string through every IR pass.
Most generated debug output (IR dumps, error messages) and many
backends use `nameHint` to choose output variable / function
names. Pass authors should preserve `nameHint` on results when
rewriting an inst — losing it costs debuggability for no IR
correctness benefit.

### `layout` / `LayoutDecoration`

`layout` attaches a `Layout` opcode (documented in
[metadata.md](metadata.md)) to a parameter, type, or variable.
The `Layout` itself carries the offset / size / register binding
information; `layout` is the link that connects a parameter or
type back to its computed layout.

### `targetIntrinsic` / `TargetIntrinsicDecoration`

`targetIntrinsic` is the IR's encoding of "this function is
implemented by a target-specific spelling". The two operands are
a capability-set operand identifying the target(s) it applies to,
and an `IRStringLit` operand carrying the target-language source
code or instruction name. The same function inst can carry
several `targetIntrinsic` decorations — one per target. Backends
walk these decorations to pick the right spelling. See the
glossary entry for `target intrinsic`.

### `intrinsicOp` / `IntrinsicOpDecoration`

`intrinsicOp` is the IR's link from a built-in function declared
in the core module to the actual IR opcode that implements it.
Its single integer operand is the `kIROp_*` tag of the
implementing opcode; the inliner uses it to replace calls with
direct opcode emission when the target supports it.

### `branch` / `flatten` / `loopControl`

These are the control-flow hint decorations. `branch` and
`flatten` attach to a conditional (from the `[branch]` /
`[flatten]` attributes) and select the divergent vs. predicated
emission strategy; `loopControl` records the unroll / loop mode
(from `[unroll]` / `[loop]`). They carry no IR semantics of their
own — they flow through unchanged and are consumed by the backend
emit step to choose the corresponding target control-flow
construct.

### `KeepAliveDecoration`

`KeepAliveDecoration` is the DCE-suppression decoration. Insts
that carry it survive dead-code elimination even when no in-IR
use chain reaches them. The decoration is added both by the
front-end (for entry points, exports, ...) and by IR passes
that need to preserve insts across rewrites.

### `entryPoint` / `EntryPointDecoration`

`entryPoint` marks a function as a shader entry point. Its three
operands are the profile (an `IRIntLit` tag), the user-visible
name, and an optional module name. The link-time pass walks every
`entryPoint` decoration to select the functions exposed in the
final binary.

### `glslFragDepthGreater` / `glslFragDepthLess`

These two nullary decorations record a *constrained* `gl_FragDepth`
output. They are not produced from a modifier directly; instead the
GLSL legalization pass in
[slang-ir-glsl-legalize.cpp](../../../../source/slang/slang-ir-glsl-legalize.cpp)
recognizes the HLSL `SV_DepthGreaterEqual` / `SV_DepthLessEqual`
system-value semantics on a fragment output and attaches the
matching decoration to the *entry-point function* (not the
parameter). How each backend emits the resulting constrained depth
output is covered in
[../pipeline/06-emit.md](../pipeline/06-emit.md).

### `BackwardDerivativePrimalContextDecoration`

The reverse-mode autodiff machinery threads the recorded primal
state through a per-function "primal context" variable.
`BackwardDerivativePrimalContextDecoration` records that variable
on the function inst so that the unzip / propagate passes can find
it without re-deriving the data flow. The companion
`PrimalContextDecoration` marks the *parameter* in the propagate
function that receives the context value.

### `diffInstDecoration` / `mixedDiffInstDecoration`

These two autodiff decorations mark each transcribed
instruction with its role in the dual computation:
`diffInstDecoration` for pure differential insts (with a
`primalType` operand recording the type of the primal it pairs
with), `mixedDiffInstDecoration` for insts that compute both
primal and differential outputs (the `pairType` operand is the
`DifferentialPairType`). The unzip pass uses these markers to
split a mixed function into its primal-side and propagate-side
copies.

### `BuiltinRequirementDecoration`

`BuiltinRequirementDecoration` tags an interface *requirement key*
with the `BuiltinRequirementKind` role (an `IRIntLit` operand) of
the built-in requirement it stands for — for example the
`Differential` type or differential-witness requirements of
`IDifferentiable`. It is attached during requirement-key lowering
in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
to the `IRBuiltinRequirementKey` produced for a requirement decl
that carries an AST `BuiltinRequirementModifier`. Unlike an
ordinary requirement, a recognized built-in requirement is keyed
by a hoistable `IRBuiltinRequirementKey` (deduplicated by
construction from its `kind` operand) rather than a per-decl
`StructKey`. The decoration lets later passes — notably autodiff
— locate a requirement entry by its role instead of by its
position in the interface's (semantically unordered) requirement
list. See [../glossary.md](../glossary.md) for `witness` and
requirement-key terminology.

## See also

- [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)
  — schema, op flags, hoistable / parent conventions, and the
  "add an opcode" workflow that applies to decorations too.
- [metadata.md](metadata.md) — the `Layout` opcodes that
  `layout` decorations attach, and the `Attr` opcodes that
  attach to types via `AttributedType`.
- [differentiation.md](differentiation.md) — the autodiff
  *opcodes* that complement the autodiff *decorations*
  documented here.
- [resources-and-atomics.md](resources-and-atomics.md) — the
  resource and shader-IO opcodes that the binding / interpolation
  / mesh-shader decorations apply to.
- [../ast-reference/modifiers.md](../ast-reference/modifiers.md)
  — the AST-side modifiers that produce most of the decorations
  here.
- [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) —
  how AST modifiers are lowered into IR decorations.
- [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) —
  the passes that introduce the synthesized decorations
  (autodiff, linker, specialization, ...).
- [../pipeline/06-emit.md](../pipeline/06-emit.md) — how each
  backend consumes the decorations during emission.
- [../glossary.md](../glossary.md) — definitions of `decoration`,
  `target intrinsic`, `entry point`, `differential pair`.
