---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:13:48Z
source_commit: ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
watched_paths_digest: 4efe93afbd22f4572d6d334ca82947cebf8058c7572291261103fd18aa04f6bd
source_doc: docs/llm-generated/ir-reference/decorations.md
source_doc_digest: fa437f161a85c114f3d1cf9292f389f617cff6186ed550e01d5b609a06a35fd8
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/decorations

## Intent

Tests verify the per-opcode catalog of the IR `Decoration` family
described in
[`docs/llm-generated/ir-reference/decorations.md`](../../../docs/llm-generated/ir-reference/decorations.md):
that each documented decoration opcode, in a representative sample
across the catalog's categories, appears in `-dump-ir` output with
the schema-described operand shape when its natural AST-side
surface is present.

The bundle does not enumerate all ~180 documented decoration
opcodes (per the prompt). It samples 2-4 decorations per
observable category: Naming and provenance, Layout and binding,
Loop and branch hints, Target-specific definition and intrinsics,
Interpolation and shader IO, Entry-point and stage, Linkage and
lifetime, Inlining and optimization, and Other. Synthesized
decorations (autodiff markers, specialization/dispatch decorations,
debug markers introduced by IR passes) are out-of-scope for this
representative bundle — they are not anchored at a stable
AST-side surface.

The primary observation mechanism is `-target spirv-asm -dump-ir
-o /dev/null -entry main -stage compute` followed by a FileCheck
against the LOWER-TO-IR section. Anchors are decoration spellings
(`[nameHint("...")]`, `[entryPoint(...)]`, `[loopControl(N : Int)]`,
`[targetIntrinsic(...)]`) and, where ordering matters, the
immediately-following user-named anchor (`func %name`, `let %name`,
`struct %name`).

The angle distinguishing this bundle from
`cross-cutting/ir-instructions` (one-shot per IR family) and
`ir-reference/structure` (which observes a couple of decorations
attached to structural parents — `nameHint` on a `func`, `export`
on a struct-field key) is: **the decoration family in its own
right**, sampling its breadth across categories.

## Claims enumerated

| Claim ID | Anchor | Claim (one line) | Tests |
| --- | --- | --- | --- |
| C-01 | [#namehint-namehintdecoration](../../../docs/llm-generated/ir-reference/decorations.md#namehint-namehintdecoration) | A user-named function carries a `nameHint` decoration whose string operand is the source identifier. | `name-hint-on-function.slang` |
| C-02 | [#naming-and-provenance](../../../docs/llm-generated/ir-reference/decorations.md#naming-and-provenance) | `nameHint` attaches to function parameters as well as functions. | `name-hint-on-param.slang` |
| C-03 | [#naming-and-provenance](../../../docs/llm-generated/ir-reference/decorations.md#naming-and-provenance) | `nameHint` attaches to top-level struct declarations carrying the source name. | `name-hint-on-struct.slang` |
| C-04 | [#namehint-namehintdecoration](../../../docs/llm-generated/ir-reference/decorations.md#namehint-namehintdecoration) | Multiple co-existing user-named functions each surface their own `nameHint`. | `name-hint-survives-on-call-result.slang` |
| C-05 | [#naming-and-provenance](../../../docs/llm-generated/ir-reference/decorations.md#naming-and-provenance) | Core-module declarations (`IBufferDataLayout`) carry a `BuiltinDecoration` line. | `builtin-decoration.slang` |
| C-06 | [#layout-layoutdecoration](../../../docs/llm-generated/ir-reference/decorations.md#layout-layoutdecoration) | A global parameter carries a `layout(%N)` decoration linking it to its computed `Layout` opcode. | `layout-decoration.slang` |
| C-07 | [#layout-and-binding](../../../docs/llm-generated/ir-reference/decorations.md#layout-and-binding) | A global with an explicit `register(uN)` carries a bare `HasExplicitHLSLBinding` decoration. | `hlsl-binding-decoration.slang` |
| C-08 | [#layout-and-binding](../../../docs/llm-generated/ir-reference/decorations.md#layout-and-binding) | A field decorated with `[vk::location(N)]` lowers to a `glslLocation(N : Int)` decoration. | `glsl-location-decoration.slang` |
| C-09 | [#loop-and-branch-hints](../../../docs/llm-generated/ir-reference/decorations.md#loop-and-branch-hints) | An `[unroll]` attribute on a for-loop produces a `loopControl(N : Int)` decoration recording the mode. | `loop-control-unroll.slang` |
| C-10 | [#loop-and-branch-hints](../../../docs/llm-generated/ir-reference/decorations.md#loop-and-branch-hints) | A `[flatten]` attribute on a conditional produces a bare `flatten` decoration. | `flatten-decoration.slang` |
| C-11 | [#loop-and-branch-hints](../../../docs/llm-generated/ir-reference/decorations.md#loop-and-branch-hints) | A `[branch]` attribute on a conditional produces a bare `branch` decoration. | `branch-decoration.slang` |
| C-12 | [#loop-and-branch-hints](../../../docs/llm-generated/ir-reference/decorations.md#loop-and-branch-hints) | Two coexisting loop-control hints (`[unroll]` mode 0 and `[loop]` mode 1) each surface independently. | `two-loop-controls-coexist.slang` |
| C-13 | [#targetintrinsic-targetintrinsicdecoration](../../../docs/llm-generated/ir-reference/decorations.md#targetintrinsic-targetintrinsicdecoration) | Calling a core-module built-in pulls in its `[targetIntrinsic(capSet, "spelling")]` decoration. | `target-intrinsic-decoration.slang` |
| C-14 | [#interpolation-and-shader-io](../../../docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io) | A parameter declared with `SV_DispatchThreadID` lowers to a `semantic("SV_DispatchThreadID", -1 : Int)` decoration. | `semantic-decoration.slang` |
| C-15 | [#interpolation-and-shader-io](../../../docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io) | A struct field declared with a `TEXCOORD0` semantic carries a `semantic("TEXCOORD0", -1 : Int)` decoration on its key. | `semantic-on-struct-field.slang` |
| C-16 | [#interpolation-and-shader-io](../../../docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io) | A vertex parameter with `: POSITION` lowers to a `semantic("POSITION", -1 : Int)` decoration. | `system-value-semantic-on-param.slang` |
| C-17 | [#interpolation-and-shader-io](../../../docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io) | A field declared with `nointerpolation` produces an `interpolationMode(N : Int)` decoration. | `interpolation-mode-decoration.slang` |
| C-18 | [#interpolation-and-shader-io](../../../docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io) | A `precise`-modified parameter produces a bare `precise` decoration. | `precise-decoration.slang` |
| C-19 | [#interpolation-and-shader-io](../../../docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io) | An `[earlydepthstencil]` attribute produces a bare `earlyDepthStencil` decoration. | `early-depth-stencil-decoration.slang` |
| C-20 | [#entrypoint-entrypointdecoration](../../../docs/llm-generated/ir-reference/decorations.md#entrypoint-entrypointdecoration) | A compute function with `[numthreads(...)]` lowers to a `func` with `entryPoint(profile : Int, "name", "module")`. | `entry-point-decoration.slang` |
| C-21 | [#entrypoint-entrypointdecoration](../../../docs/llm-generated/ir-reference/decorations.md#entrypoint-entrypointdecoration) | A vertex entry point uses a different profile-tag integer than a compute entry point in its `entryPoint` decoration. | `entry-point-profile-and-module.slang` |
| C-22 | [#entry-point-and-stage](../../../docs/llm-generated/ir-reference/decorations.md#entry-point-and-stage) | `[numthreads(x,y,z)]` lowers to `numThreads(x : Int, y : Int, z : Int)` with the three operands in source order. | `num-threads-decoration.slang` |
| C-23 | [#entry-point-and-stage](../../../docs/llm-generated/ir-reference/decorations.md#entry-point-and-stage) | `[WaveSize(N)]` lowers to `waveSize(N : Int)`. | `wave-size-decoration.slang` |
| C-24 | [#entrypoint-entrypointdecoration](../../../docs/llm-generated/ir-reference/decorations.md#entrypoint-entrypointdecoration) | A user helper called from `main` keeps a `nameHint` but does not receive an `entryPoint` decoration; only the entry-point function does. | `two-entry-points-coexist.slang` |
| C-25 | [#linkage-and-lifetime](../../../docs/llm-generated/ir-reference/decorations.md#linkage-and-lifetime) | Every user-declared top-level function carries an `export` decoration with the function-linkage mangled name. | `export-linkage-decoration.slang` |
| C-26 | [#linkage-and-lifetime](../../../docs/llm-generated/ir-reference/decorations.md#linkage-and-lifetime) | A top-level struct declaration carries an `export` decoration with the type-linkage mangled name (prefix `_ST`). | `export-on-struct.slang` |
| C-27 | [#linkage-and-lifetime](../../../docs/llm-generated/ir-reference/decorations.md#linkage-and-lifetime) | A global parameter carries an `export` decoration with the variable-linkage mangled name (prefix `_SV`). | `export-on-global-param.slang` |
| C-28 | [#keepalivedecoration](../../../docs/llm-generated/ir-reference/decorations.md#keepalivedecoration) | The front-end attaches a `keepAlive` decoration to global parameters so they survive DCE. | `keep-alive-on-global-param.slang` |
| C-29 | [#keepalivedecoration](../../../docs/llm-generated/ir-reference/decorations.md#keepalivedecoration) | The compute entry-point function is also kept alive by a `keepAlive` decoration. | `keep-alive-on-entry-point.slang` |
| C-30 | [#inlining-and-optimization](../../../docs/llm-generated/ir-reference/decorations.md#inlining-and-optimization) | `[ForceInline]` on a function lowers to a bare `ForceInline` decoration. | `force-inline-decoration.slang` |
| C-31 | [#inlining-and-optimization](../../../docs/llm-generated/ir-reference/decorations.md#inlining-and-optimization) | `[noinline]` on a function lowers to a bare `noInline` decoration. | `no-inline-decoration.slang` |
| C-32 | [#inlining-and-optimization](../../../docs/llm-generated/ir-reference/decorations.md#inlining-and-optimization) | `[__NoSideEffect]` on a function lowers to a bare `noSideEffect` decoration. | `no-side-effect-decoration.slang` |
| C-33 | [#inlining-and-optimization](../../../docs/llm-generated/ir-reference/decorations.md#inlining-and-optimization) | `[__unsafe_force_inline_early]` on a function lowers to a bare `unsafeForceInlineEarly` decoration. | `unsafe-force-inline-early-decoration.slang` |
| C-34 | [#inlining-and-optimization](../../../docs/llm-generated/ir-reference/decorations.md#inlining-and-optimization) | Two functions with opposing inlining hints (`[ForceInline]` and `[noinline]`) coexist in the same module without interference. | `two-inlining-hints-coexist.slang` |
| C-35 | [#other](../../../docs/llm-generated/ir-reference/decorations.md#other) | A struct member function carries a bare `method` decoration. | `method-decoration.slang` |
| C-36 | [#other](../../../docs/llm-generated/ir-reference/decorations.md#other) | The synthesized struct `__init` carries a `constructor(true)` decoration with a Bool literal operand. | `constructor-decoration.slang` |

## Tests in this bundle

| File | Intent | Doc anchor |
| --- | --- | --- |
| `name-hint-on-function.slang` | functional | `#namehint-namehintdecoration` |
| `name-hint-on-param.slang` | functional | `#naming-and-provenance` |
| `name-hint-on-struct.slang` | functional | `#naming-and-provenance` |
| `name-hint-survives-on-call-result.slang` | functional | `#namehint-namehintdecoration` |
| `builtin-decoration.slang` | functional | `#naming-and-provenance` |
| `layout-decoration.slang` | functional | `#layout-layoutdecoration` |
| `hlsl-binding-decoration.slang` | functional | `#layout-and-binding` |
| `glsl-location-decoration.slang` | functional | `#layout-and-binding` |
| `loop-control-unroll.slang` | functional | `#loop-and-branch-hints` |
| `flatten-decoration.slang` | functional | `#loop-and-branch-hints` |
| `branch-decoration.slang` | functional | `#loop-and-branch-hints` |
| `two-loop-controls-coexist.slang` | functional | `#loop-and-branch-hints` |
| `target-intrinsic-decoration.slang` | functional | `#targetintrinsic-targetintrinsicdecoration` |
| `semantic-decoration.slang` | functional | `#interpolation-and-shader-io` |
| `semantic-on-struct-field.slang` | functional | `#interpolation-and-shader-io` |
| `system-value-semantic-on-param.slang` | functional | `#interpolation-and-shader-io` |
| `interpolation-mode-decoration.slang` | functional | `#interpolation-and-shader-io` |
| `precise-decoration.slang` | functional | `#interpolation-and-shader-io` |
| `early-depth-stencil-decoration.slang` | functional | `#interpolation-and-shader-io` |
| `entry-point-decoration.slang` | functional | `#entrypoint-entrypointdecoration` |
| `entry-point-profile-and-module.slang` | functional | `#entrypoint-entrypointdecoration` |
| `num-threads-decoration.slang` | functional | `#entry-point-and-stage` |
| `wave-size-decoration.slang` | functional | `#entry-point-and-stage` |
| `two-entry-points-coexist.slang` | functional | `#entrypoint-entrypointdecoration` |
| `export-linkage-decoration.slang` | functional | `#linkage-and-lifetime` |
| `export-on-struct.slang` | functional | `#linkage-and-lifetime` |
| `export-on-global-param.slang` | functional | `#linkage-and-lifetime` |
| `keep-alive-on-global-param.slang` | functional | `#keepalivedecoration` |
| `keep-alive-on-entry-point.slang` | functional | `#keepalivedecoration` |
| `force-inline-decoration.slang` | functional | `#inlining-and-optimization` |
| `no-inline-decoration.slang` | functional | `#inlining-and-optimization` |
| `no-side-effect-decoration.slang` | functional | `#inlining-and-optimization` |
| `unsafe-force-inline-early-decoration.slang` | functional | `#inlining-and-optimization` |
| `two-inlining-hints-coexist.slang` | functional | `#inlining-and-optimization` |
| `method-decoration.slang` | functional | `#other` |
| `constructor-decoration.slang` | functional | `#other` |

## Out of scope (no-GPU runner)

- **All `(synthesized)` decorations** — every row whose `AST origin`
  column reads `(synthesized)` lacks a stable AST-side anchor and is
  out of scope for this representative bundle. This includes
  `BinaryInterfaceType`, `PhysicalType`, `AlignedAddressDecoration`,
  `SizeAndAlignment`, `Offset`, `optimizableTypeDecoration`,
  `NonDynamicUniformReturnDecoration`, `bindExistentialSlots`,
  `DispatchFuncDecoration`, `SpecializationDepthDecoration`,
  `SequentialIDDecoration`, `RTTI_typeSize`, `AnyValueSize`,
  `transitory`, `InParamProxyVar`, `TempCallArgImmutableVar`,
  `TempCallArgVar`,
  `GlobalVariableShadowingGlobalParameterDecoration`,
  `DisableCopyEliminationDecoration`, `DynamicUniform`,
  `availableInDownstreamIR`, `downstreamModuleExport`,
  `downstreamModuleImport`, `PyBindExportFuncInfo`,
  `ignoreSideEffectsDecoration`, `loopCounterDecoration`,
  `loopCounterUpdateDecoration`.

- **All Differentiation marker decorations** —
  `AutoDiffOriginalValueDecoration`, `AutoDiffBuiltinDecoration`,
  `BackwardDerivativePrimalContextDecoration`,
  `BackwardDerivativePrimalReturnDecoration`,
  `PrimalContextDecoration`, `ParamsContextDecoration`,
  `primalInstDecoration`, `diffInstDecoration`,
  `mixedDiffInstDecoration`, `RecomputeBlockDecoration`,
  `primalValueKey`, `primalElementType`,
  `IntermediateContextFieldDifferentialTypeDecoration`,
  `ReturnValueContextFieldDecoration`,
  `derivativeMemberDecoration`,
  `treatCallAsDifferentiableDecoration`,
  `differentiableCallDecoration`, `PreferCheckpointDecoration`,
  `PreferRecomputeDecoration`,
  `DifferentiableTypeDictionaryDecoration`,
  `CudaKernelFwdDiffRef`, `CudaKernelBwdDiffRef`. These are
  introduced by the autodiff pass on transcribed instructions and
  do not have a stable per-decoration AST-side anchor at
  LOWER-TO-IR.

- **SPIR-V backend hint decorations** —
  `spvBufferBlock`, `spvBlock`, `NonUniformResource`,
  `MemoryQualifierSetDecoration`. These are introduced by the
  SPIR-V backend after LOWER-TO-IR; observing them requires a
  later dump stage that is out of scope for the per-opcode
  reference angle of this bundle.

- **Mesh / geometry / hull / domain shader decorations** —
  `pointPrimitiveType`, `linePrimitiveType`,
  `trianglePrimitiveType`, `lineAdjPrimitiveType`,
  `triangleAdjPrimitiveType`, `streamOutputTypeDecoration`,
  `vertices`, `indices`, `primitives`,
  `HLSLMeshPayloadDecoration`, `PositionOutput`, `PositionInput`,
  `PerVertex`, `stageReadAccess`, `stageWriteAccess`,
  `patchConstantFunc`, `maxTessFactor`, `outputControlPoints`,
  `outputTopology`, `partitioning`, `domain`, `maxVertexCount`,
  `instance`, `GeometryInputPrimitiveTypeDecoration`,
  `MeshOutputDecoration`, `StageAccessDecoration`. These require
  pipeline-specific entry-point scaffolding distinct from the
  per-decoration reference angle.

- **NVAPI / CUDA / DLL / Python interop decorations** —
  `requiresNVAPI`, `nvapiMagic`, `nvapiSlot`,
  `RequireSPIRVDescriptorIndexingExtensionDecoration`,
  `dllImport`, `dllExport`, `cudaDeviceExport`, `CudaKernel`,
  `CudaHost`, `TorchEntryPoint`, `AutoPyBindCUDA`,
  `PyExportDecoration`, `externCpp`, `externC`,
  `TargetBuiltinVar`. These require non-default `-target` settings
  or specialized markers.

- **Capability / availability decorations except spot samples** —
  `requireCapabilityAtom`, `requireSPIRVVersion`,
  `requireGLSLVersion`, `requireGLSLExtension`,
  `requireWGSLExtension`, `requireCUDASMVersion`. These attach to
  core-module functions during lowering but do not have stable
  user-side anchors without target-specific surface.

- **Specialization / conformance / existentials decorations** —
  `SpecializeDecoration`, `DynamicDispatchWitnessDecoration`,
  `StaticRequirementDecoration`, `TypeConstraintDecoration`,
  `ResultWitness`, `SpecializationConstantDecoration`. These are
  largely synthesized by the specialization pass.

- **Debug / reflection decorations** —
  `DebugLocation`, `DebugFunction`, `CounterBuffer`. Require
  `-g` or `[counter]` plumbing distinct from a per-opcode
  reference angle.

- **`UserTypeName`, `COMInterface`, `COMWitnessDecoration`,
  `UserExtern`, `highLevelDecl`** — observable but require
  COM / reflection / extern scaffolding that distracts from the
  representative sample goal.

- **`format`, `output`, `input`, `glslOuterArray`,
  `vkStructOffset`, `packoffset`, `glslOffset`** — observable
  in principle but require specialized binding/format surface
  out of scope for the representative sample.

- **`raypayload`, `vulkanRayPayload`, `vulkanRayPayloadIn`,
  `vulkanHitAttributes`, `vulkanHitObjectAttributes`,
  `vulkanCallablePayload`, `vulkanCallablePayloadIn`** —
  raytracing-pipeline-specific markers requiring raytracing
  entry-point scaffolding.

- **`fpDenormalPreserve`, `fpDenormalFlushToZero`,
  `DerivativeGroupQuad`, `DerivativeGroupLinear`,
  `MaximallyReconverges`, `QuadDerivatives`, `RequireFullQuads`,
  `FloatingPointModeOverride`, `experimentalModule`,
  `DisallowSpecializationWithExistentialsDecoration`,
  `BitFieldAccessorDecoration`** — niche surfaces out of scope
  for the representative bundle. Each is single-decoration
  observable in principle; future bundle expansion can add one
  test per such decoration if its AST surface stabilises.

- **`public`, `hlslExport`, `dependsOn`, `noRefInline`,
  `nonCopyable`, `alwaysFold`, `readNone`,
  `AllowPreTranslationInlining`, `DefaultValue`,
  `perprimitive`, `TargetSystemValue`, `intrinsicOp`,
  `requirePrelude`, `spirvOpDecoration`** — observable on
  natural surface but the bundle samples a representative
  subset of the inlining/optimization and target-intrinsic
  categories rather than every per-opcode row.

## Doc gaps observed

- The doc's `### entryPoint / EntryPointDecoration` notable-opcodes
  section states "Its three operands are the profile (an IRIntLit
  tag), the user-visible name, and an optional module name", but
  does not specify the source of the module-name operand. In the
  observed dump it is the source-file stem (e.g. `"two-entry-
  points-coexist"` for `two-entry-points-coexist.slang`), which
  means a test that pins the literal module-name string is
  filename-fragile. A clarifying note that the module-name operand
  is derived from the file name (or compilation-unit name) would
  help test authors avoid this pitfall.

- The `Loop and branch hints` table row for `loopControl` lists
  the operand as `modeOperand: IRConstant` but does not document
  the integer encoding — observed values are `0` for `[unroll]`
  and `1` for `[loop]`. Documenting the enum would make the
  test claim concrete instead of wildcarded.

- The `Interpolation and shader IO` row for `interpolationMode`
  lists `modeOperand: IRConstant` but does not document the
  encoding. Observed values include `0` (for `linear`) and
  `2` (for `nointerpolation`); pinning these would let tests
  assert the exact mode rather than wildcarding.

- The `Inlining and optimization` row for `unsafeForceInlineEarly`
  says "Inlines calls immediately after codegen" but does not
  warn that the decoration's lifetime may be shorter than the
  LOWER-TO-IR dump can capture in adversarial pass-ordering
  scenarios. The current dump still includes it, but a note that
  the decoration may be consumed earlier than other inline hints
  would prevent surprise if pass ordering changes.

- The `Linkage and lifetime` row for `export` lists the operand
  as `(variadic, min=1)` but does not show the mangled-name
  encoding (function `_SR...`, type `_ST...`, variable `_SV...`,
  witness `_SW...`). A worked example showing the prefix-per-
  symbol-kind would help authors of cross-cutting tooling that
  consumes the export linkage.

- The `Naming and provenance` row for `BuiltinDecoration` lists
  no operands and notes "Marks an inst as a compiler-builtin",
  but does not specify which built-in core-module declarations
  carry it. Observed in the dump: `IBufferDataLayout` and other
  `RWStructuredBuffer`-related core types. Documenting the
  invariant that "every core-module-declared interface and
  type-layout carries `BuiltinDecoration`" would make this
  observable test stable.

- The `Loop and branch hints` row for `ForceUnroll` lists AST
  origin as `[ForceUnroll]` attribute, but the actual checker
  rejects `[ForceUnroll]` at function scope: `attribute
  'ForceUnroll' is not valid here`. The attribute appears to be
  valid only on loop statements. A clarifying note that
  `ForceUnroll` is a loop-statement-only attribute (and a list
  of where it's valid) would prevent misleading test attempts.
  This bundle does not include a `ForceUnroll` test because of
  this confusion.

- The doc lists `entryPoint`'s profile operand as `profileInst:
  IRIntLit`, but the integer-to-stage mapping (compute=6,
  vertex=1, ...) is not enumerated in the doc. A note pointing at
  the `Stage` enum that produces these tags would let tests
  pin the exact integer instead of wildcarding.
