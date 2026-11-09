---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-03T00:00:00Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: 7003d9ffd97f502ef36deaf8cec832320bcf008e9a79f32f8c5f2570288a7ffc
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Decorations

This page is the per-opcode reference for the `Decoration` family —
the largest single family in
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua),
spanning lines 1763-2738 and holding 201 concrete opcodes at
`source_commit`. Decorations attach metadata to other IR
instructions: names, layout binding, control-flow hints,
target-specific intrinsic spellings, capability requirements,
inlining preferences, autodiff markers, and so on. The intended
reader is a compiler engineer reading IR around a function, type, or
variable and trying to identify what each decoration says about it.

## Source

The opcodes live under the top-level `Decoration` entry of
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) at
line 1763. Per-opcode info (mnemonics, fixed operand counts, op
flags) is registered in the generated `kIROps` table in
[slang-ir-insts-info.cpp](../../../../source/slang/slang-ir-insts-info.cpp).
C++ wrappers are declared in
[slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h). Of the
201 wrappers, 64 are hand-written there and marked `‡` in the
`C++ wrapper` column; the generator
([slang-ir.h.lua](../../../../source/slang/slang-ir.h.lua)) skips any
struct already defined and emits only the remaining 137 (see
[../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)).
Infrastructure (op flags, `IRBuilder` helpers such as
`addSimpleDecoration<T>` and the per-decoration `add*Decoration`
emitters) is split between
[slang-ir.h](../../../../source/slang/slang-ir.h) for the op flags,
[slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) for
`addSimpleDecoration<T>` and most inline `add*Decoration` helpers, and
[slang-ir.cpp](../../../../source/slang/slang-ir.cpp) for the
out-of-line helper definitions such as `addLayoutDecoration`.

Most decorations originate from AST-side modifiers and attributes.
In
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
the main producers are `addVarDecorations` (line 3160) for
parameters, fields, and global variables; `addLinkageDecoration`
(line 1431) for the import / export / public / extern family;
`addTargetIntrinsicDecorations` (line 13451),
`addSpecializedForTargetDecorations` (line 13432), and
`addTargetRequirementDecorations` (line 13531) for the
target-specific group; `lowerFuncDeclInContext` (line 13862) for
function- and entry-point-level attributes; and
`lowerFrontEndEntryPointToIR` (line 15280) for decorations that are
meaningful only on an entry point. The primal/diff transcription
markers, varying-parameter legalization markers, and SPIR-V backend
hints are introduced by the IR passes themselves, but some autodiff
markers do come from lowering: `visitTreatAsDifferentiableExpr` (line
5341) emits `TreatCallAsDifferentiableDecoration` and
`DifferentiableCallDecoration`, and the `[PreferCheckpoint]` /
`[PreferRecompute]` attributes lower at line 14684.

## Family hierarchy

```mermaid
flowchart TD
  IRInst --> Decoration
  Decoration --> TargetSpecificDecoration
  Decoration --> GeometryInputPrimitiveTypeDecoration
  Decoration --> LinkageDecoration
  Decoration --> DerivativeGroupDecoration
  Decoration --> MeshOutputDecoration
  Decoration --> StageAccessDecoration
  Decoration --> AutodiffInstDecoration
  Decoration --> CheckpointHintDecoration
  Decoration --> leaves["all other leaf opcodes"]
  TargetSpecificDecoration --> TargetSpecificDefinitionDecoration
  TargetSpecificDecoration --> requirePreludeNode[requirePrelude]
  TargetSpecificDefinitionDecoration --> targetNode[target]
  TargetSpecificDefinitionDecoration --> targetIntrinsicNode[targetIntrinsic]
  LinkageDecoration --> importNode[import]
  LinkageDecoration --> exportNode[export]
  DerivativeGroupDecoration --> DerivativeGroupQuad
  DerivativeGroupDecoration --> DerivativeGroupLinear
  AutodiffInstDecoration --> primalInst[primalInstDecoration]
  AutodiffInstDecoration --> diffInst[diffInstDecoration]
  AutodiffInstDecoration --> mixedDiffInst[mixedDiffInstDecoration]
  AutodiffInstDecoration --> RecomputeBlockDecoration
```

The diagram shows only the abstract groups nested in the Lua
`Decoration` entry; the `### ` headings under `## Opcodes` are
semantic buckets of this page, not Lua nodes.

## Opcodes

### Naming and provenance

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `nameHint` | `IRNameHintDecoration`‡ | `nameOperand: IRStringLit` | | Decl / parameter name, via `addNameHintDecoration` in `slang-lower-to-ir.cpp` | Carries a human-readable name across IR passes; backends use it for variable / function naming. |
| `highLevelDecl` | `IRHighLevelDeclDecoration`‡ | `declOperand: IRPtrLit` | | `slang-lower-to-ir.cpp` lowering (records source `Decl*`) | Records a pointer to the originating AST `Decl` (debug / diagnostic aid). |
| `BuiltinDecoration` | `IRBuiltinDecoration` | — | | `[builtin]` attribute (`BuiltinAttribute`) on an `interface` declaration, via `visitInterfaceDecl` | Marks a core-module interface as a compiler builtin; see the callout. |
| `KnownBuiltinDecoration` | `IRKnownBuiltinDecoration`‡ | `nameOperand: IRIntLit` | | `[KnownBuiltin(name)]` attribute (`KnownBuiltinAttribute`) | Names a builtin by enum tag so later passes can find it. |
| `UserTypeName` | `IRUserTypeNameDecoration`‡ | `userTypeName: IRStringLit` | | (synthesized by the user-type-hint pass in `slang-ir-user-type-hint.cpp`) | Records the original user type name for a shader parameter. |
| `COMInterface` | `IRComInterfaceDecoration` | — | | `[COM(guid)]` attribute (`ComInterfaceAttribute`) | Marks an interface as a COM interface declaration. |
| `COMWitnessDecoration` | `IRCOMWitnessDecoration` | `witnessTable` | | (synthesized by the COM-method lowering pass in `slang-ir-lower-com-methods.cpp`) | Marks a class type as a COM interface implementation. |
| `UserExtern` | `IRUserExternDecoration` | — | | `extern` modifier on an imported decl, via `addLinkageDecoration` | Marks an inst as coming from user-side `extern`. |
| `transitory` | `IRTransitoryDecoration` | — | | no producer at HEAD (only a temporary lookup key built in `slang-ir.cpp`) | Marks an inst as transitory; should never survive into the output. |

### Layout and binding

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `layout` | `IRLayoutDecoration`‡ | (1 unnamed) | | Layout-pass output | Attaches a `Layout` opcode (see [metadata.md](metadata.md)) to a parameter or type. |
| `AlignedAddressDecoration` | `IRAlignedAddressDecoration` | `alignment` | | SPIR-V legalization pass (`slang-ir-spirv-legalize.cpp`) | Marks an address inst as aligned to a specific byte boundary. |
| `SizeAndAlignment` | `IRSizeAndAlignmentDecoration`‡ | `layoutNameOperand, sizeOperand: IRIntLit, alignmentOperand: IRIntLit` | | `getSizeAndAlignment` (`slang-ir-layout.cpp`) | Records size/alignment of a type under a named layout. |
| `Offset` | `IROffsetDecoration`‡ | `layoutNameOperand, offsetOperand: IRIntLit` | | layout computation in `slang-ir-layout.cpp` | Records the offset of a struct field under a named layout. |
| `packoffset` | `IRPackOffsetDecoration`‡ | `registerOffset: IRIntLit, componentOffset: IRIntLit` | | `packoffset(...)` HLSL semantic (`HLSLPackOffsetSemantic`) | HLSL packoffset binding. |
| `glslLocation` | `IRGLSLLocationDecoration` | `location: IRIntLit` | | `[vk::location(N)]`, declared as `vk_location` in `core.meta.slang` (`GLSLLocationAttribute`) | GLSL / Vulkan location binding. |
| `glslOffset` | `IRGLSLOffsetDecoration` | `offset: IRIntLit` | | `GLSLOffsetLayoutAttribute` (GLSL `layout(offset=...)`) | GLSL / Vulkan offset binding. |
| `vkStructOffset` | `IRVkStructOffsetDecoration` | `offset: IRIntLit` | | `[vk_offset(index)]` attribute (`VkStructOffsetAttribute`) | Vulkan struct-member offset. |
| `HasExplicitHLSLBinding` | `IRHasExplicitHLSLBindingDecoration` | — | | An `HLSLLayoutSemantic` (e.g. `register(...)`) on a global parameter | Marks a parameter as having an explicit HLSL register binding. |
| `synthesizedParameterGroup` | `IRSynthesizedParameterGroupDecoration`‡ | — | | (synthesized when entry-point or global uniforms are collected into a group) | Marks a parameter-group element struct as compiler-generated rather than source-authored. |
| `BinaryInterfaceType` | `IRBinaryInterfaceTypeDecoration` | — | | global-uniform collection and entry-point-uniform passes | Marks a type as being used as a binary-interface type so `legalizeEmptyType` does not eliminate it. |
| `PhysicalType` | `IRPhysicalTypeDecoration` | (1 unnamed) | | SPIR-V legalization pass | Marks the physical lowered type of a logical value. |
| `output` | `IRGlobalOutputDecoration` | — | | `out` modifier on global parameter | Marks a global parameter as an output. |
| `input` | `IRGlobalInputDecoration` | — | | `in` modifier on global parameter | Marks a global parameter as an input. |
| `glslOuterArray` | `IRGLSLOuterArrayDecoration`‡ | `outerArrayNameOperand: IRStringLit` | | GLSL legalization | Records the outer-array variable name for GLSL emission. |

### Loop and branch hints

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `branch` | `IRBranchDecoration` | — | | `[branch]` attribute | Hints the backend to emit a branching select. |
| `flatten` | `IRFlattenDecoration` | — | | `[flatten]` attribute | Hints the backend to flatten a conditional. |
| `loopControl` | `IRLoopControlDecoration`‡ | `modeOperand: IRConstant` | | `[unroll]` / `[loop]` attributes | Records loop-control mode (unroll, loop, ...). |
| `loopMaxIters` | `IRLoopMaxItersDecoration`‡ | (1 unnamed) | | `[MaxIters(count)]` attribute (`MaxItersAttribute`) | Records the maximum-iteration bound for a loop. |
| `loopExitPrimalValue` | `IRLoopExitPrimalValueDecoration`‡ | `targetInst, loopExitValInst` | | no producer at HEAD (only the `IRBuilder` helper) | Records the primal value of an exit-condition for reverse-mode use. |
| `ForceUnroll` | `IRForceUnrollDecoration` | — (builder appends `count: IRIntLit`) | | `[ForceUnroll(count = 0)]` attribute (`ForceUnrollAttribute`), declared `__attributeTarget(LoopStmt)` and so valid only on a loop statement | Forces loop unrolling; the operand is the requested count, `0` when omitted. |
| `loopCounterDecoration` | `IRLoopCounterDecoration` | — | | `ensurePrimalAvailability` (autodiff primal hoisting) | Marks an instruction as a loop counter. |
| `loopCounterUpdateDecoration` | `IRLoopCounterUpdateDecoration` | — | | no producer at HEAD (only the `IRBuilder` helper) | Marks the per-iteration update of a loop counter. |

### Target-specific definition and intrinsics

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `target` | `IRTargetDecoration` | (1 unnamed) | | `__specialized_for_target(...)` modifier (`SpecializedForTargetModifier`) | Marks a function as the implementation for one specific target. |
| `targetIntrinsic` | `IRTargetIntrinsicDecoration`‡ | `target, definitionOperand: IRStringLit, predicate?: IRStringLit, typeScrutinee?` | | `__target_intrinsic(...)` modifier (`TargetIntrinsicModifier`) | Carries the target-specific spelling of an [intrinsic](../glossary.md). |
| `requirePrelude` | `IRRequirePreludeDecoration`‡ | (2 unnamed) | | `[RequirePrelude(target, prelude)]` attribute | Records a prelude snippet that the backend must include when the decorated function is reachable. |
| `intrinsicOp` | `IRIntrinsicOpDecoration`‡ | `intrinsicOpOperand: IRIntLit` | | `__intrinsic_op(...)` modifier | Identifies the built-in IR opcode that implements an intrinsic. |
| `spirvOpDecoration` | `IRSPIRVOpDecoration` | (1 unnamed) | | `[vk_spirv_instruction(op, set)]` attribute (`SPIRVInstructionOpAttribute`) | Records the SPIR-V opcode for a function. |

### Capability and availability

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `requireCapabilityAtom` | `IRRequireCapabilityAtomDecoration`‡ | `capabilityAtomOperand: IRConstant` | | Layout-IR lowering (`TargetProgram::createIRModuleForLayout`), from the entry point's *inferred* capability set — not from a `[require(...)]` attribute directly | Requires one capability atom (SPIR-V version and Metal-library atoms only); see the callout. |
| `requireSPIRVVersion` | `IRRequireSPIRVVersionDecoration`‡ | `SPIRVVersionOperand: IRConstant` | | `__spirv_version` modifier (`RequiredSPIRVVersionModifier`) | Records a minimum SPIR-V version. |
| `requireGLSLVersion` | `IRRequireGLSLVersionDecoration`‡ | `languageVersionOperand: IRConstant` | | `__glsl_version` modifier (`RequiredGLSLVersionModifier`) | Records a minimum GLSL version. |
| `requireGLSLExtension` | `IRRequireGLSLExtensionDecoration`‡ | `extensionNameOperand: IRStringLit` | | `__glsl_extension` modifier (`RequiredGLSLExtensionModifier`) | Records a required GLSL extension. |
| `requireWGSLExtension` | `IRRequireWGSLExtensionDecoration`‡ | `extensionNameOperand: IRStringLit` | | `__wgsl_extension` modifier (`RequiredWGSLExtensionModifier`) | Records a required WGSL extension. |
| `requireCUDASMVersion` | `IRRequireCUDASMVersionDecoration`‡ | `CUDASMVersionOperand: IRConstant` | | `__cuda_sm_version` modifier (`RequiredCUDASMVersionModifier`) | Records a minimum CUDA SM version. |
| `shader64BitIndexing` | `IRShader64BitIndexingDecoration` | — | | `[Shader64BitIndexing]` attribute, lifted onto the entry point from its inferred capability set | Requests the SPIR-V `Shader64BitIndexingEXT` execution mode (plus its capability and extension) for an entry point. |
| `availableInDownstreamIR` | `IRAvailableInDownstreamIRDecoration`‡ | (1 unnamed) | | `Module::precompileForTarget` | Marks an inst as available through the downstream-IR import. |
| `RequireSPIRVDescriptorIndexingExtensionDecoration` | `IRRequireSPIRVDescriptorIndexingExtensionDecoration` | — | | SPIR-V legalization pass | Marks a function as requiring SPIR-V descriptor indexing. |
| `requiresNVAPI` | `IRRequiresNVAPIDecoration` | — | | NVAPI core-module markers | Requires NVAPI prelude when targeting D3D. |
| `nvapiMagic` | `IRNVAPIMagicDecoration`‡ | `nameOperand: IRStringLit` | | `NVAPIMagicModifier` on a core-module decl | Marks an inst as part of the NVAPI magic naming. |
| `nvapiSlot` | `IRNVAPISlotDecoration`‡ | `registerNameOperand: IRStringLit, spaceNameOperand: IRStringLit` | | NVAPI core-module markers | Records the NVAPI register/space binding. |

### Interpolation and shader IO

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `interpolationMode` | `IRInterpolationModeDecoration`‡ | `modeOperand: IRConstant` (an `IRInterpolationMode` value) | | `linear` / `noperspective` / etc. modifiers, via `addVarDecorations` | Records the interpolation mode of a varying parameter as an integer; see the callout. |
| `TargetSystemValue` | `IRTargetSystemValueDecoration`‡ | `semanticOperand: IRStringLit, index: IRIntLit` | | (synthesized by varying-parameter and Metal legalization) | Records the target-specific system-value binding. |
| `semantic` | `IRSemanticDecoration`‡ | `semanticNameOperand: IRStringLit, semanticIndexOperand: IRIntLit` | | `HLSLSimpleSemantic` and `HLSLLayoutSemantic` AST nodes | Records the HLSL semantic on a parameter or field. |
| `raypayload` | `IRRayPayloadDecoration` | — | | `[raypayload]` attribute (`RayPayloadAttribute`) | Marks a type as usable as a ray payload. |
| `vulkanRayPayload` | `IRVulkanRayPayloadDecoration` | — (builder appends `location: IRIntLit`) | | `[__vulkanRayPayload(location)]` attribute | Marks a variable as a Vulkan ray payload (outgoing). |
| `vulkanRayPayloadIn` | `IRVulkanRayPayloadInDecoration` | — (builder appends `location: IRIntLit`) | | `VulkanRayPayloadInAttribute` (GLSL `rayPayloadInEXT`) | Marks a variable as a Vulkan ray payload (incoming). |
| `vulkanHitAttributes` | `IRVulkanHitAttributesDecoration` | — | | `[__vulkanHitAttributes]` attribute | Marks a variable as Vulkan hit attributes. |
| `vulkanHitObjectAttributes` | `IRVulkanHitObjectAttributesDecoration` | — (builder appends `location: IRIntLit`) | | `[__vulkanHitObjectAttributes(location)]` attribute | Marks a variable as Vulkan hit-object attributes. |
| `vulkanCallablePayload` | `IRVulkanCallablePayloadDecoration` | — (builder appends `location: IRIntLit`) | | `[__vulkanCallablePayload(location)]` attribute | Marks a variable as a Vulkan callable payload (outgoing). |
| `vulkanCallablePayloadIn` | `IRVulkanCallablePayloadInDecoration` | — (builder appends `location: IRIntLit`) | | `VulkanCallablePayloadInAttribute` (GLSL `callableDataInEXT`) | Marks a variable as a Vulkan callable payload (incoming). |
| `earlyDepthStencil` | `IREarlyDepthStencilDecoration` | — | | `[earlydepthstencil]` attribute | Requests early-depth-stencil test for a pixel shader. |
| `postDepthCoverage` | `IRPostDepthCoverageDecoration` | — | | `[postdepthcoverage]` attribute (`PostDepthCoverageAttribute`, `core.meta.slang`), lowered in `lowerFuncDeclInContext` with the attribute's source location kept | Requests that the input coverage mask reflect post-depth-test coverage; targets that cannot honour it get a diagnostic from `slang-ir-entry-point-decorations.cpp`. |
| `glslFragDepthGreater` | `IRGLSLFragDepthGreaterDecoration` | — | | (synthesized by GLSL legalization from `SV_DepthGreaterEqual`) | Marks a fragment entry point whose `gl_FragDepth` only ever increases the fixed-function depth. |
| `glslFragDepthLess` | `IRGLSLFragDepthLessDecoration` | — | | (synthesized by GLSL legalization from `SV_DepthLessEqual`) | Marks a fragment entry point whose `gl_FragDepth` only ever decreases the fixed-function depth. |
| `precise` | `IRPreciseDecoration` | — | | `precise` modifier (`PreciseModifier`) | Requests bit-precise math. |
| `format` | `IRFormatDecoration`‡ | `formatOperand: IRConstant` | | `[format("rgba32f")]` / `[vk_image_format("rgba32f")]` attribute (`FormatAttribute`); the argument is a format-name *string*, not a bare identifier | Records the UAV image format as an integer `ImageFormat` value, never the string. |
| `perprimitive` | `IRGLSLPrimitivesRateDecoration` | — | | (synthesized by GLSL legalization in `slang-ir-glsl-legalize.cpp`) | GLSL `per_primitiveEXT` rate qualifier. |
| `glslPatch` | `IRGLSLPatchDecoration` | — | | (synthesized by GLSL legalization, `slang-ir-glsl-legalize.cpp` line 307, on each non-system-value leaf of a legalized patch varying) | Marks a global as part of a tessellation patch-constant interface (hull outputs, domain inputs); SPIR-V emits `Patch`, GLSL emits the `patch` qualifier. |

### Mesh shader, geometry shader, and per-vertex

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `pointPrimitiveType` | `IRPointInputPrimitiveTypeDecoration` | — | | `[shader("geometry")]` point variant | Marks a geometry input as `point`. |
| `linePrimitiveType` | `IRLineInputPrimitiveTypeDecoration` | — | | Geometry shader `line` variant | Marks a geometry input as `line`. |
| `trianglePrimitiveType` | `IRTriangleInputPrimitiveTypeDecoration` | — | | Geometry shader `triangle` variant | Marks a geometry input as `triangle`. |
| `lineAdjPrimitiveType` | `IRLineAdjInputPrimitiveTypeDecoration` | — | | Geometry shader `lineadj` | Marks a geometry input as `lineadj`. |
| `triangleAdjPrimitiveType` | `IRTriangleAdjInputPrimitiveTypeDecoration` | — | | Geometry shader `triangleadj` | Marks a geometry input as `triangleadj`. |
| `streamOutputTypeDecoration` | `IRStreamOutputTypeDecoration`‡ | `streamType: IRHLSLStreamOutputType` | | (synthesized by GLSL legalization in `slang-ir-glsl-legalize.cpp`, line 4182, from a geometry entry point's stream-output parameter) | Records the stream-output type on the entry-point function. |
| `vertices` | `IRVerticesDecoration` | (1 unnamed) | | Mesh-shader vertex output | Marks a parameter as the mesh-shader vertex output. |
| `indices` | `IRIndicesDecoration` | (1 unnamed) | | Mesh-shader index output | Marks a parameter as the mesh-shader index output. |
| `primitives` | `IRPrimitivesDecoration` | (1 unnamed) | | Mesh-shader primitive output | Marks a parameter as the mesh-shader primitive output. |
| `HLSLMeshPayloadDecoration` | `IRHLSLMeshPayloadDecoration` | — | | `payload` modifier (`HLSLPayloadModifier`) | Marks a parameter as the HLSL mesh-payload. |
| `PositionOutput` | `IRGLPositionOutputDecoration` | — | | (synthesized by GLSL legalization from `SV_Position`) | Marks a varying as the `gl_Position` output. |
| `PositionInput` | `IRGLPositionInputDecoration` | — | | (synthesized by GLSL legalization from `SV_Position`) | Marks a varying as the `gl_Position` input. |
| `PerVertex` | `IRPerVertexDecoration` | — | | (synthesized by GLSL legalization in `slang-ir-glsl-legalize.cpp`) | Marks a fragment-shader input array as per-vertex. |
| `stageReadAccess` | `IRStageReadAccessDecoration` | — | | `RayPayloadReadSemantic` via `lowerRayPayloadAccessModifier` in `lowerMemberVarDecl`; defaults from `addDefaultPayloadAccessQualifiersToField` | Records the read-access stage of a resource. |
| `stageWriteAccess` | `IRStageWriteAccessDecoration` | — | | `RayPayloadWriteSemantic` via `lowerRayPayloadAccessModifier` in `lowerMemberVarDecl`; defaults from `addDefaultPayloadAccessQualifiersToField` | Records the write-access stage of a resource. |

### Entry-point and stage

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `entryPoint` | `IREntryPointDecoration`‡ | `profileInst: IRIntLit, name: IRStringLit, moduleName?: IRStringLit` | | `[shader(...)]` attribute | Marks a function as an entry point with a given profile and name. |
| `entryPointParam` | `IREntryPointParamDecoration`‡ | `entryPoint: IRFunc` | | entry-point-uniform collection pass and `declareSimpleVar` (type legalization) | Marks a global parameter that was moved from an entry-point parameter. |
| `patchConstantFunc` | `IRPatchConstantFuncDecoration`‡ | `func: IRInst` | | `[patchconstantfunc(...)]` attribute | Records the hull-shader patch-constant function. |
| `maxTessFactor` | `IRMaxTessFactorDecoration` | `maxTessFactor: IRFloatLit` | | `[maxtessfactor(...)]` attribute | Records the maximum tessellation factor. |
| `outputControlPoints` | `IROutputControlPointsDecoration` | `controlPointCount: IRIntLit` | | `[outputcontrolpoints(...)]` attribute | Records the hull-shader output control-point count. |
| `outputTopology` | `IROutputTopologyDecoration`‡ | `topology: IRStringLit, topologyTypeOperand: IRIntLit` | | `[outputtopology(...)]` attribute | Records the hull-shader output topology. |
| `partitioning` | `IRPartitioningDecoration` | `partitioning: IRStringLit` | | `[partitioning(...)]` attribute | Records the tessellation partitioning mode. |
| `domain` | `IRDomainDecoration` | `domain: IRStringLit` | | `[domain(...)]` attribute | Records the tessellation domain. |
| `maxVertexCount` | `IRMaxVertexCountDecoration` | `count: IRIntLit` | | `[maxvertexcount(...)]` attribute | Records the geometry-shader vertex-count limit. |
| `instance` | `IRInstanceDecoration` | `count: IRIntLit` | | `[instance(...)]` attribute | Records the geometry-shader instance count. |
| `numThreads` | `IRNumThreadsDecoration`‡ | (3 unnamed) | | `[numthreads(x,y,z)]` attribute | Records the compute-shader workgroup size. |
| `fpDenormalPreserve` | `IRFpDenormalPreserveDecoration` | `width: IRIntLit` | | (synthesized in `slang-emit.cpp` from the denormal-mode compiler options) | Requests denormal-preserve behavior at a given precision. |
| `fpDenormalFlushToZero` | `IRFpDenormalFlushToZeroDecoration` | `width: IRIntLit` | | (synthesized in `slang-emit.cpp` from the denormal-mode compiler options) | Requests denormal-flush-to-zero. |
| `waveSize` | `IRWaveSizeDecoration` | `numLanes: IRIntLit` | | `[WaveSize(numLanes)]` attribute | Requests a specific wave size. |
| `DerivativeGroupQuad` | `IRDerivativeGroupQuadDecoration` | — | | `[DerivativeGroupQuad]` attribute | Quad-form derivative grouping. |
| `DerivativeGroupLinear` | `IRDerivativeGroupLinearDecoration` | — | | `[DerivativeGroupLinear]` attribute | Linear-form derivative grouping. |
| `MaximallyReconverges` | `IRMaximallyReconvergesDecoration` | — | | `[MaximallyReconverges]` attribute | Requests maximal-reconvergence execution. |
| `QuadDerivatives` | `IRQuadDerivativesDecoration` | — | | `[QuadDerivatives]` attribute | Requests quad-derivative execution. |
| `RequireFullQuads` | `IRRequireFullQuadsDecoration` | — | | `[RequireFullQuads]` attribute | Requires full quads. |

### Work-graph nodes

The work-graph attributes are declared as `attribute_syntax` in
[workgraph.slang](../../../../source/standard-modules/experimental/workgraph.slang)
rather than in `core.meta.slang`; see
[../ast-reference/modifiers.md](../ast-reference/modifiers.md) for the
AST classes. The function-level ones are lowered by
`lowerFuncDeclInContext` (line 14545 onward) and the parameter-level
ones by `addVarDecorations` (line 3304 onward) in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp).

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `nodeLaunch` | `IRNodeLaunchDecoration` | `mode: IRStringLit` | | `[NodeLaunch("broadcasting" \| "thread" \| "coalescing")]` (`NodeLaunchAttribute`) | Records the node launch mode as a string so HLSL emit can re-spell it as a named constant. |
| `nodeMaxDispatchGrid` | `IRNodeMaxDispatchGridDecoration` | `x: IRIntLit, y: IRIntLit, z: IRIntLit` | | `[NodeMaxDispatchGrid(x,y,z)]` (`NodeMaxDispatchGridAttribute`) | Upper bound on a broadcasting node's dispatch grid. |
| `nodeDispatchGrid` | `IRNodeDispatchGridDecoration` | `x: IRIntLit, y: IRIntLit, z: IRIntLit` | | `[NodeDispatchGrid(x,y,z)]` (`NodeDispatchGridAttribute`) | Fixed dispatch-grid size for a broadcasting node. |
| `maxRecords` | `IRMaxRecordsDecoration` | `count: IRIntLit` | | `[MaxRecords(count)]` (`MaxRecordsAttribute`), on a node function or on an input / output record parameter | Maximum record count for the node or for that parameter. |
| `nodeID` | `IRNodeIDDecoration` | `name: IRStringLit, arrayIndex: IRIntLit` | | `[NodeID(name, arrayIndex = 0)]` (`NodeIDAttribute`), on a node function or an output parameter | Overrides the node identifier (name and array index). |
| `nodeIsProgramEntry` | `IRNodeIsProgramEntryDecoration` | — | | `[NodeIsProgramEntry]` (`NodeIsProgramEntryAttribute`) | Marks a node shader as a program entry point in the work graph. |
| `nodeArraySize` | `IRNodeArraySizeDecoration` | `count: IRIntLit` | | `[NodeArraySize(count)]` (`NodeArraySizeAttribute`) on an output-node array parameter | Size of an output node array. |
| `allowSparseNodes` | `IRAllowSparseNodesDecoration` | — | | `[AllowSparseNodes]` (`AllowSparseNodesAttribute`) on an output-node array parameter | Permits unpopulated entries in an output node array. |
| `workGraphRecordType` | `IRWorkGraphRecordTypeDecoration` | — | | — (no producer at `source_commit`) | Reserved opcode identity; see the callout below. |
| `workGraphRecordElementType` | `IRWorkGraphRecordElementTypeDecoration` | `elementType: IRType` | | — (no producer at `source_commit`) | Reserved opcode identity; see the callout below. |

### Linkage and lifetime

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `import` | `IRImportDecoration` | (1 unnamed) | | `isImportedDecl` classification in `addLinkageDecoration` | Marks an inst as imported under a mangled name. |
| `export` | `IRExportDecoration` | (1 unnamed) | | non-imported branch of `addLinkageDecoration` | Marks an inst as exported under a mangled name. |
| `public` | `IRPublicDecoration` | — | | `public` modifier (`PublicModifier`) | Public visibility. |
| `hlslExport` | `IRHLSLExportDecoration` | — | | `export` modifier (`HLSLExportModifier`) | HLSL export. |
| `downstreamModuleExport` | `IRDownstreamModuleExportDecoration` | — | | `Module::precompileForTarget` | Marks an inst as exported through the downstream module bridge. |
| `downstreamModuleImport` | `IRDownstreamModuleImportDecoration` | — | | no producer at HEAD (only matched in consumers) | Marks an inst as imported through the downstream module bridge. |
| `externCpp` | `IRExternCppDecoration`‡ | `nameOperand: IRStringLit` | | `__extern_cpp` modifier (`ExternCppModifier`) | Emits the function without C++ mangling. |
| `externC` | `IRExternCDecoration` | — | | (synthesized by the PyTorch/CUDA binding pass in `slang-ir-pytorch-cpp-binding.cpp`) | Wraps a generated wrapper function in `extern "C"`. |
| `dllImport` | `IRDllImportDecoration`‡ | `libraryNameOperand: IRStringLit, functionNameOperand: IRStringLit` | | `[DllImport(modulePath)]` attribute | Generates dynamic-library load logic. |
| `dllExport` | `IRDllExportDecoration`‡ | `functionNameOperand: IRStringLit` | | `[DllExport]` attribute | Generates DLL-export wrapper. |
| `cudaDeviceExport` | `IRCudaDeviceExportDecoration` | (1 unnamed) | | `[CudaDeviceExport]` attribute | Exports a function as a CUDA `__device__` function. |
| `CudaKernel` | `IRCudaKernelDecoration` | — | | `[CudaKernel]` attribute | Marks a function as a CUDA kernel. |
| `CudaHost` | `IRCudaHostDecoration` | — | | `[CudaHost]` attribute | Marks a function as a CUDA host helper. |
| `TorchEntryPoint` | `IRTorchEntryPointDecoration`‡ | `functionNameOperand: IRStringLit` | | `[TorchEntryPoint]` attribute | Marks a Torch / Slang interop entry point. |
| `AutoPyBindCUDA` | `IRAutoPyBindCudaDecoration`‡ | `functionNameOperand: IRStringLit, fwdDiffFunc?, bwdDiffFunc?` | | `[AutoPyBindCUDA]` attribute | Generates Python bindings for a CUDA function. |
| `CudaKernelFwdDiffRef` | `IRCudaKernelForwardDerivativeDecoration` | `forwardDerivativeFunc?` | | `generateDerivativeWrappers` (PyTorch binding pass) | Records the forward-mode derivative of a CUDA kernel. |
| `CudaKernelBwdDiffRef` | `IRCudaKernelBackwardDerivativeDecoration` | `backwardDerivativeFunc?` | | `generateDerivativeWrappers` (PyTorch binding pass) | Records the reverse-mode derivative of a CUDA kernel. |
| `PyBindExportFuncInfo` | `IRAutoPyBindExportInfoDecoration` | — | | `[AutoPyBindCUDA]` lowering | Reflection info for Python binding generation. |
| `PyExportDecoration` | `IRPyExportDecoration`‡ | `exportNameOperand: IRStringLit` | | `[PyExport(...)]` attribute | Marks a function as exported to Python. |
| `dependsOn` | `IRDependsOnDecoration` | (1 unnamed) | | (synthesized by GLSL legalization when a function must keep a global parameter alive) | Adds an extra dependency edge to the parent inst. |
| `keepAlive` | `IRKeepAliveDecoration` | — | | (synthesized: `addLinkageDecoration`, witness-table lowering, and IR passes) | Prevents DCE from eliminating the inst. |
| `TargetBuiltinVar` | `IRTargetBuiltinVarDecoration`‡ | `builtinVarOperand: IRIntLit` | | GLSL legalization (`legalizeTargetBuiltinVar`, `createVarLayoutForLegalizedGlobalParam`) | Marks a global variable as a target builtin variable. |

### Inlining and optimization

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `unsafeForceInlineEarly` | `IRUnsafeForceInlineEarlyDecoration` | — | | `[__unsafeForceInlineEarly]` attribute | Inlines calls immediately after codegen. |
| `ForceInline` | `IRForceInlineDecoration` | — | | `[ForceInline]` attribute | Inlines calls during normal IR passes. |
| `AllowPreTranslationInlining` | `IRAllowPreTranslationInliningDecoration` | — | | no producer at HEAD (only the `IRBuilder` helper) | Permits inlining after translation passes. |
| `noInline` | `IRNoInlineDecoration` | — | | `[noinline]` attribute | Suppresses inlining. |
| `noRefInline` | `IRNoRefInlineDecoration` | — | | `[NoRefInline]` attribute (`NoRefInlineAttribute`) in `lowerFuncDeclInContext` | Suppresses inlining of reference-type calls. |
| `alwaysFold` | `IRAlwaysFoldIntoUseSiteDecoration` | — | | `[__AlwaysFoldIntoUseSiteAttribute]` attribute | Always fold call result into its use site. |
| `noSideEffect` | `IRNoSideEffectDecoration` | — | | `[__NoSideEffect]` attribute | Marks a callee as side-effect free. |
| `ignoreSideEffectsDecoration` | `IRIgnoreSideEffectsDecoration` | — | | autodiff passes (`maybeTranslateLegacyBackwardDerivative`, `splitApplyAndPropFuncs`) | DCE may treat the call as side-effect free. |
| `NonDynamicUniformReturnDecoration` | `IRNonDynamicUniformReturnDecoration` | — | | `NonDynamicUniformAttribute` in `lowerFuncDeclInContext` | Marks a function whose return value is never dynamically uniform. |
| `optimizableTypeDecoration` | `IROptimizableTypeDecoration` | — | | tuple-type lowering and autodiff unzip (`splitApplyAndPropFuncs`) | Marks a type as eligible for field trimming. |
| `readNone` | `IRReadNoneDecoration` | — | | `[__readNone]` attribute | Marks a function as pure (no reads, no writes). |
| `DisableCopyEliminationDecoration` | `IRDisableCopyEliminationDecoration` | — | | SPIR-V legalization pass | Prevents copy-elimination on the inst. |
| `nonCopyable` | `IRNonCopyableTypeDecoration` | — | | `[__NonCopyableType]` attribute | Marks a type as non-copyable; SSA skips it. |
| `DynamicUniform` | `IRDynamicUniformDecoration` | — | | `DynamicUniformModifier` in `addVarDecorations` and `visitAggTypeDecl` | Marks a value as dynamically uniform. |
| `bindExistentialSlots` | `IRBindExistentialSlotsDecoration` | — | | `lowerProgramEntryPointToIR` | Records existential-binding slot info. |
| `DefaultValue` | `IRDefaultValueDecoration` | (1 unnamed) | | Default-value modifier | Records the default value of a parameter or member. |
| `InParamProxyVar` | `IRInParamProxyVarDecoration` | (1 unnamed) | | `in`-parameter proxy local in `lowerFuncDeclInContext` | Marks a local var as the legacy-mutated form of an `in` parameter. |
| `TempCallArgImmutableVar` | `IRTempCallArgImmutableVarDecoration` | — | | params-to-constref pass (`slang-ir-transform-params-to-constref.cpp`) | Marks a temporary used to materialize an immutable call argument. |
| `TempCallArgVar` | `IRTempCallArgVarDecoration` | — | | varying-parameter legalization (`slang-ir-legalize-varying-params.cpp`) | Marks a temporary used to materialize a mutable call argument. |
| `GlobalVariableShadowingGlobalParameterDecoration` | `IRGlobalVariableShadowingGlobalParameterDecoration` | (2 unnamed) | | global-varying-var translation (`slang-ir-translate-global-varying-var.cpp`) | Marks a global var that shadows a global parameter. |

### Specialization, conformance, and existentials

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `SpecializeDecoration` | `IRSpecializeDecoration` | — | | `SpecializeAttribute` in `visitInterfaceDecl` | Hints the specialization pass to specialize the inst. |
| `SpecializationDepthDecoration` | `IRSpecializationDepthDecoration` | `specializationDepthOperand: IRIntLit` | | generic specialization pass (`addSpecializationDepthDecoration`) | Records how deeply specialized the inst is. |
| `SequentialIDDecoration` | `IRSequentialIDDecoration`‡ | `sequentialIdOperand: IRIntLit` | | `generateIRForSpecializedComponentType` (conformance-ID override) and `lowerUntaggedUnionTypes` | Stable integer ID used by `GetSequentialID`. |
| `DynamicDispatchWitnessDecoration` | `IRDynamicDispatchWitnessDecoration` | — | | `generateIRForSpecializedComponentType` | Marks a witness table as participating in dynamic dispatch. |
| `StaticRequirementDecoration` | `IRStaticRequirementDecoration` | — | | `HLSLStaticModifier` on a requirement in `visitInterfaceDecl` | Marks an interface requirement as static. |
| `BuiltinRequirementDecoration` | `IRBuiltinRequirementDecoration`‡ | `kindOperand: IRIntLit` | | `slang-lower-to-ir.cpp` (requirement-key lowering of a `BuiltinRequirementModifier`) | Marks an interface requirement key with the `BuiltinRequirementKind` role (e.g. `IDifferentiable.Differential`) of the built-in requirement it represents, so consumers identify it by role rather than by position in the requirement list. |
| `DispatchFuncDecoration` | `IRDispatchFuncDecoration` | `func` | | no producer at HEAD (only the `IRBuilder` helper) | Records the dispatch function for an interface call. |
| `TypeConstraintDecoration` | `IRTypeConstraintDecoration` | `constraintType` | | `GenericTypeConstraintDecl` lowering | Records the interface constraint of a generic parameter. |
| `ResultWitness` | `IRResultWitnessDecoration`‡ | `witness` | | generic specialization pass (`slang-ir-specialize.cpp`) | Records the original interface witness when a function used to return an existential. |
| `RTTI_typeSize` | `IRRTTITypeSizeDecoration`‡ | `typeSizeOperand: IRIntLit` | | no producer at HEAD (only the `IRBuilder` helper) | Records the size used by an RTTI object. |
| `AnyValueSize` | `IRAnyValueSizeDecoration`‡ | `sizeOperand: IRIntLit` | | `AnyValueSizeAttribute` in `visitInterfaceDecl`; `inferAnyValueSizeWhereNecessary` | Records the `AnyValueType` blob size on a type. |
| `SpecializationConstantDecoration` | `IRSpecializationConstantDecoration` | (1 unnamed) | | `[SpecializationConstant]` / `[vk_specialization_constant]` attribute | Marks a global as a specialization constant. |

### Differentiation markers

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `AutoDiffOriginalValueDecoration` | `IRAutoDiffOriginalValueDecoration`‡ | `originalValue` | | `maybeAddTypeAnnotationsForHigherOrderDiff` (forward-mode autodiff) | Records the original (pre-transcribe) value. |
| `AutoDiffBuiltinDecoration` | `IRAutoDiffBuiltinDecoration` | — | | `[AutoDiffBuiltin]` attribute (`AutoDiffBuiltinAttribute`) in `visitAggTypeDecl` and `visitInheritanceDecl` | Marks a type as an autodiff built-in. |
| `BackwardDerivativePrimalContextDecoration` | `IRBackwardDerivativePrimalContextDecoration`‡ | `backwardDerivativePrimalContextVar` | | no producer at HEAD (only the `IRBuilder` helper) | Records the primal-context variable of a reverse-mode function. |
| `BackwardDerivativePrimalReturnDecoration` | `IRBackwardDerivativePrimalReturnDecoration`‡ | `backwardDerivativePrimalReturnValue` | | autodiff unzip pass (`slang-ir-autodiff-unzip.cpp`) | Records the primal-return value of a reverse-mode function. |
| `PrimalContextDecoration` | `IRPrimalContextDecoration` | — | | no producer at HEAD (only read in `slang-ir-autodiff-unzip.cpp`) | Marks a parameter as the autodiff primal context. |
| `ParamsContextDecoration` | `IRParamsContextDecoration` | `value` | | `ensurePrimalAvailability` (autodiff primal hoisting) | Records the parameters context for autodiff. |
| `AutodiffParameterContextTypeDecoration` | `IRAutodiffParameterContextTypeDecoration` | — | | `maybeTranslateLegacyToNewBackwardDerivative` and `splitApplyAndPropFuncs` (autodiff), copied by tuple lowering | Marks a context type whose fields are captured primal parameters. |
| `primalInstDecoration` | `IRPrimalInstDecoration` | — | | `markInstAsPrimal`, called by the forward-mode, unzip, and primal-hoist passes | Marks an inst as computing a primal value. |
| `diffInstDecoration` | `IRDifferentialInstDecoration`‡ | `primalType: IRType, primalInst?, witness?` | | `markInstAsDifferential`, called by the forward-mode, unzip, and transpose passes | Marks an inst as computing a differential value, with link back to the primal inst. |
| `mixedDiffInstDecoration` | `IRMixedDifferentialInstDecoration`‡ | `pairType: IRType` | | `markInstAsMixedDifferential`, called by the forward-mode and reverse-mode passes | Marks an inst as computing both primal and differential. |
| `RecomputeBlockDecoration` | `IRRecomputeBlockDecoration` | — | | `createPrimalRecomputeBlocks` (autodiff primal hoisting) | Marks a block as a recomputation block. |
| `primalValueKey` | `IRPrimalValueStructKeyDecoration`‡ | `firstKey: IRStructKey, secondKey: IRStructKey` | | autodiff unzip pass (`slang-ir-autodiff-unzip.cpp`) | Records the keys used to store a primal value in the intermediate-context struct. |
| `primalElementType` | `IRPrimalElementTypeDecoration`‡ | `primalElementType` | | forward-mode `updateElement` transcription (`slang-ir-autodiff-fwd.cpp`) | Records the primal element type of a forward-diffed `updateElement`. |
| `IntermediateContextFieldDifferentialTypeDecoration` | `IRIntermediateContextFieldDifferentialTypeDecoration`‡ | `differentialWitness` | | no producer at HEAD (only the `IRBuilder` helper) | Records the differential type of an intermediate-context field. |
| `ReturnValueContextFieldDecoration` | `IRReturnValueContextFieldDecoration` | — | | no producer at HEAD (only the `IRBuilder` helper) | Marks an intermediate-context field as the return value. |
| `derivativeMemberDecoration` | `IRDerivativeMemberDecoration`‡ | `derivativeMemberStructKey` | | `[DerivativeMember(memberName)]` attribute, plus autodiff synthesis | Cross-references a differential member of a type. |
| `treatCallAsDifferentiableDecoration` | `IRTreatCallAsDifferentiableDecoration` | — | | `TreatAsDifferentiableExpr` (`NoDiff` flavor) in `visitTreatAsDifferentiableExpr` | Forces a call to be treated as differentiable. |
| `differentiableCallDecoration` | `IRDifferentiableCallDecoration` | — | | `TreatAsDifferentiableExpr` (`Differentiable` flavor) in `visitTreatAsDifferentiableExpr` | Marks a call as an explicitly differentiable invocation. |
| `PreferCheckpointDecoration` | `IRPreferCheckpointDecoration` | — | | `[PreferCheckpoint]` attribute | Hints that the result should be checkpointed for reverse mode. |
| `PreferRecomputeDecoration` | `IRPreferRecomputeDecoration` | `sideEffectBehavior: IRIntLit` | | `[PreferRecompute(behavior)]` attribute | Hints that the result should be recomputed for reverse mode. |
| `DifferentiableTypeDictionaryDecoration` | `IRDifferentiableTypeDictionaryDecoration` | (children) | P | no producer at HEAD (only the `IRBuilder` helper) | Parent of the per-type differentiable-type dictionary entries. |

### SPIR-V backend hints

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `spvBufferBlock` | `IRSPIRVBufferBlockDecoration` | — | | (synthesized for SPIR-V) | Requests SPIR-V `BufferBlock` decoration on a struct. |
| `spvBlock` | `IRSPIRVBlockDecoration` | — | | (synthesized for SPIR-V) | Requests SPIR-V `Block` decoration on a struct. |
| `NonUniformResource` | `IRSPIRVNonUniformResourceDecoration`‡ | `SPIRVNonUniformResourceOperand: IRConstant` | | `NonUniformResourceIndex` lowering | Marks a SPIR-V inst as `NonUniform`. |
| `MemoryQualifierSetDecoration` | `IRMemoryQualifierSetDecoration`‡ | (1 unnamed) | | Memory-qualifier modifiers | Records memory-qualifier flag bits. |

### Debug and reflection

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `DebugLocation` | `IRDebugLocationDecoration`‡ | `source, line, col, scope?` | | (synthesized by debug-info pass) | Attaches a debug source location to an inst. |
| `DebugFunction` | `IRDebugFuncDecoration`‡ | `debugFunc` | | (synthesized by debug-info pass) | Links a function to its `DebugFunction` metadata opcode. |
| `CounterBuffer` | `IRCounterBufferDecoration`‡ | `counterBuffer` | | (synthesized when append/consume structured buffers are lowered in `slang-ir-lower-append-consume-structured-buffer.cpp`) | Records the associated UAV counter buffer. |

### Other

| Opcode | C++ wrapper | Operands | Flags | AST origin | Summary |
| --- | --- | --- | --- | --- | --- |
| `BitFieldAccessorDecoration` | `IRBitFieldAccessorDecoration`‡ | (3 unnamed) | | `BitFieldModifier` on the field owning an `AccessorDecl` (`int x : 3` syntax) | Records bitfield accessor info (backing key, width, offset). |
| `constructor` | `IRConstructorDecoration`‡ | (1 unnamed: an `IRBoolLit`, read by `getSynthesizedStatus()`) | | `__init` declaration | Marks a function as a constructor; `true` means the compiler-synthesized default rather than a user-written `__init`. |
| `method` | `IRMethodDecoration` | — | | Member-function declaration | Marks a function as a method. |
| `FloatingPointModeOverride` | `IRFloatingPointModeOverrideDecoration`‡ | (1 unnamed) | | (synthesized: forward-mode autodiff forces `FloatingPointMode::Fast` on the generated derivative function) | Overrides the floating-point mode for one function. |
| `experimentalModule` | `IRExperimentalModuleDecoration` | — | | `[ExperimentalModule]` attribute, emitted on the module inst at `slang-lower-to-ir.cpp` line 15485 | Marks a module as experimental; kept as derived metadata because import validation uses the checked AST attribute while the serialized-module path reads this marker to emit E00104. |
| `fileOrNamespaceScopeMutableVar` | `IRFileOrNamespaceScopeMutableVarDecoration` | — | | file- or namespace-scope `static` variable, or a mutable uniform-parameter shadow (`slang-lower-to-ir.cpp` lines 11619 and 11690) | Records membership in that combined category for linked-type storage validation; not set on static data members or locals. |
| `uninitializedGlobalCheckFoundPossibleWrite` | `IRUninitializedGlobalCheckFoundPossibleWriteDecoration` | — | | (synthesized by the uninitialized-global check, `slang-ir-use-uninitialized-values.cpp` line 1674) | Records that the pre-link check found a possible write to the global, so a later analysis does not treat its reads as uninitialized after linking removes the writer. |
| `DisallowSpecializationWithExistentialsDecoration` | `IRDisallowSpecializationWithExistentialsDecoration` | — | | `addDecorationsForGenericsSpecializedWithExistentialsRec` | Prevents specialization with existential arguments. |

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
implemented by a target-specific spelling". The first two operands
are a capability-set operand identifying the target(s) it applies to,
and an `IRStringLit` operand carrying the target-language source
code or instruction name. A predicate-bearing target intrinsic adds
two more: an `IRStringLit` predicate and a type scrutinee, so
`IRBuilder::addTargetIntrinsicDecoration`
([slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) lines
4938-4981) builds either the two- or the four-operand form. The same
function inst can carry
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
name, and an optional module name. The tag is a `Profile::RawVal`,
not a bare stage code: `IREntryPointDecoration::getProfile`
([slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) line
382) rebuilds a `Profile` from it, and `Profile`
([slang-profile.h](../../../../source/slang/slang-profile.h) lines
68-116) packs the `ProfileVersion` in the high 16 bits and the
`Stage` in the low 16. An entry point compiled with `-stage vertex`
and no explicit profile version therefore prints as `1`, the raw
`Stage` value. The `Stage` values are the `SlangStage` enumerators in
[slang.h](../../../../include/slang.h) (lines 907-923), bound to
`Stage` by `PROFILE_STAGE` entries
([slang-profile.h](../../../../source/slang/slang-profile.h) lines
50-58):

| Tag | Stage | Tag | Stage |
| --- | --- | --- | --- |
| 1 | vertex | 9 | anyhit |
| 2 | hull | 10 | closesthit |
| 3 | domain | 11 | miss |
| 4 | geometry | 12 | callable |
| 5 | fragment (pixel) | 13 | mesh |
| 6 | compute | 14 | amplification |
| 7 | raygeneration | 15 | dispatch |
| 8 | intersection | 16 | node |

A profile that names a version (for example a `-profile` value rather
than `-stage` alone) sets the high 16 bits, so the printed integer is
then `(version << 16) | stage` and a table lookup needs the low 16
bits only. The link-time pass walks every `entryPoint`
decoration to select the functions exposed in the final binary.

### `nodeLaunch` and the work-graph node decorations

`nodeLaunch` stores the launch mode as an `IRStringLit` (`"broadcasting"`,
`"thread"`, `"coalescing"`) rather than as an integer tag, so the
launch mode reaches the backends as a named constant rather than as a
positional value; see
[../pipeline/06-emit.md](../pipeline/06-emit.md) for how targets
consume it. The rest of the set follows the same
split as the surface syntax — `nodeMaxDispatchGrid` / `nodeDispatchGrid`
carry three `IRIntLit` extents, `nodeID` carries a name plus an array
index, and `nodeArraySize` / `allowSparseNodes` land on output-node-array
*parameters* rather than on the node function (`maxRecords` and `nodeID`
can appear in either position). The record *types* themselves are not
decorations: they are
the `WorkGraphRecordTypeBase` opcodes documented in
[types.md](types.md).

### `workGraphRecordType` / `workGraphRecordElementType`

These two opcodes are declared but have no producer or consumer at
`source_commit`; the Lua comment states that the names are kept
"reserved for stable IR serialization" because newer work-graph records
use the dedicated `Type.WorkGraphRecordTypeBase` opcodes instead. Both
have entries in
[slang-ir-insts-stable-names.lua](../../../../source/slang/slang-ir-insts-stable-names.lua),
which is what makes the reservation meaningful — a stable name is
assigned once and never reused, so retaining the entries keeps those
identities from being handed to a future, unrelated opcode.

### `shader64BitIndexing`

`shader64BitIndexing` is nullary and is attached only to an entry-point
function, by `lowerFrontEndEntryPointToIR`
([slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
line 15280, via `lowerEntryPointShader64BitIndexingRequirement`
at line 15260, which `TargetProgram::createIRModuleForLayout` also
calls at line 16565). It is not lowered from the `[Shader64BitIndexing]`
attribute directly. Instead lowering reads the entry point's
`inferredCapabilityRequirements` and looks for the
`spvShader64BitIndexingEXT` atom in *any* alternative of the capability
set, which covers three spellings uniformly: the attribute written on
the entry point, the attribute written on a callee that the entry point
reaches through its call graph, and an explicit
`[require(spvShader64BitIndexingEXT)]`. The requirement is lifted to the
entry point because the matching SPIR-V `Shader64BitIndexingEXT`
execution mode is entry-point scoped and would be invalid on a callee.
For what the backend then emits from this decoration, see
[../pipeline/06-emit.md](../pipeline/06-emit.md).

### `requireCapabilityAtom`

No AST modifier lowers directly to `requireCapabilityAtom`. A
`[require(...)]` attribute on a user function does not put the
decoration on that function; it feeds the entry point's
`inferredCapabilityRequirements`. The decoration is added only while
`TargetProgram::createIRModuleForLayout` builds the layout IR module
([slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
lines 16563-16577): for each entry point it walks every atom of every
alternative in that set and calls `addRequireCapabilityAtomDecoration`
for those in the SPIR-V version range (`_spirv_1_0` through the latest
SPIR-V atom) or the Metal library range (`metallib_2_3` through the
latest Metal atom). Every other atom, including a user's
`[require(...)]` of any non-version capability, is not recorded.

Consider this entry point, compiled for SPIR-V:

```slang
[require(spirv_1_5)]
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() {}
```

The `[require(spirv_1_5)]` requirement lands in the entry point's
inferred capability set, so the layout IR module's `computeMain` (the
imported function inst, not the function in the user's lowered module)
carries one `requireCapabilityAtom` per SPIR-V version atom in that
set. A dump of the user's own `LOWER-TO-IR` module shows no such
decoration on `computeMain`.

### `BuiltinDecoration`

`BuiltinDecoration` is attached by `visitInterfaceDecl`
([slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
lines 12423-12425) and only to an `interface` carrying `[builtin]`;
the core and HLSL modules declare it on interfaces such as
`__BuiltinType` and `IBufferDataLayout`. It is therefore absent from
structs, functions, and user declarations, but present on every such
core-module interface that a program links. Declaring a
`RWStructuredBuffer<float>` is enough to pull in `IBufferDataLayout`
([hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang) line 24),
so a search of a dump for `BuiltinDecoration` matches those library
interfaces; searching for a user symbol first narrows the dump to the
code of interest.

### `synthesizedParameterGroup`

This nullary decoration marks a parameter-group element struct that the
compiler built itself — for example the struct produced when
entry-point `uniform` parameters are collected
(`slang-ir-entry-point-uniforms.cpp`, line 550) or when global uniforms
are wrapped (`slang-ir-collect-global-uniforms.cpp`, line 157). Diagnostics that only
make sense for a source-authored group, such as the warning that a
special type leaks out of a parameter group, are suppressed when it is
present. Type legalization propagates it when it rebuilds the struct, so
the marker survives the legalization rewrite.

Both producers are reachable from a few lines of ordinary code: an
entry point declared
`void computeMain(uniform float scale, uint3 tid : SV_DispatchThreadID)`
and compiled with `-target hlsl -entry computeMain` carries the marker
on its synthesized `EntryPointParams` struct, and moving `scale` to
file scope as a global `uniform float scale` moves it to
`GlobalParams` — but only because that global is *ordinary* data,
since global collection is skipped when the global scope holds nothing
but resources
([slang-ir-collect-global-uniforms.cpp](../../../../source/slang/slang-ir-collect-global-uniforms.cpp)
lines 120-122). Both passes run during target lowering rather than AST
lowering, so neither struct is in the `LOWER-TO-IR` snapshot of a
`-dump-ir` trace; the decoration first appears under
`AFTER collectEntryPointUniformParams` or
`AFTER collectGlobalUniformParameters`.

### `interpolationMode`

The `modeOperand` is a plain integer `IRInterpolationMode` value, not
a bit mask: `Linear` 0, `NoPerspective` 1, `NoInterpolation` 2,
`Centroid` 3, `Sample` 4, `PerVertex` 5
([slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) lines
181-190). `addVarDecorations`
([slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
lines 3165-3187) maps one source modifier to one value —
`linear`, `noperspective`, `nointerpolation`, `centroid`, `sample`,
and Slang's own `pervertex` respectively — so a dump that shows
`[interpolationMode(2 : Int)]` was written `nointerpolation`.

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

### `Differentiable` / `BackwardDifferentiable`

Neither name is a decoration opcode in the current `Decoration` family,
so no row exists for them. Header checking turns `[Differentiable]`
into an interface-conformance requirement (`IForwardDifferentiable` /
`IBackwardDifferentiable`) carried by witness-table entries, not into a
decoration; see
[../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) (section
"Differentiability arrives as a conformance") and
[differentiation.md](differentiation.md). The decorations listed under
"Differentiation markers" are the IR-side bookkeeping used by the autodiff
passes, not the user-level intent markers.

## See also

- [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)
  — schema, op flags, hoistable / parent conventions, and the
  "add an opcode" workflow that applies to decorations too.
- [types.md](types.md) — the `WorkGraphRecordTypeBase` record
  types that the work-graph node decorations annotate.
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
