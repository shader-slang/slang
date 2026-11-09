---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-02T21:23:00Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: 434c98b6b03bec03448f6e6a2edb7332984d1006ddfb79bd35d3933b4f25bc49
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# IR Passes

This document is a categorized inventory of the IR passes that run
between AST → IR lowering ([04-ast-to-ir.md](04-ast-to-ir.md)) and
target emission ([06-emit.md](06-emit.md)). The intended reader is a
developer adding a pass, debugging a miscompile, or trying to choose
where to insert a new transformation.

At `source_commit` the source tree contains 167 `slang-ir-*.cpp`
files, and every one of them appears in a category table below. A
single file often provides several pass entry points, so the tables
name the file's primary transformation and call out additional entry
points only where a reader would otherwise miss them.

## How the passes are ordered

The orchestrator is `linkAndOptimizeIR` in
[slang-emit.cpp](../../../../source/slang/slang-emit.cpp) (defined at
line 1015 and ending at line 2966 at `source_commit`). Its body holds
184 `SLANG_PASS(...)` call sites; that macro, defined at line 1024,
wraps each pass call in `wrapPass` so that IR dumping and profiling
apply uniformly to every pass. The callers are
`CodeGenContext::emitEntryPointsSourceFromIR` (defined at line 2968;
the call is at line 3112) and the three entry points used for
non-textual output: `emitSPIRVForEntryPointsDirectly` (defined at line
3774, call at line 3782), `emitHostVMCode` (defined at line 3819, call
at line 3824), and `emitLLVMForEntryPoints` (defined at line 3862, call
at line 3878). The function:

1. Links the per-translation-unit IR modules together with `linkIR`
   ([slang-ir-link.cpp](../../../../source/slang/slang-ir-link.cpp)),
   producing the `LinkedIR` that all later passes mutate.
2. Runs a sequence of validations, specializations, and legalizations.
3. Inserts target-specific passes when the `TargetRequest` indicates
   a target that needs them (ray-tracing payload legalization, GLSL
   entry-point legalization, Metal legalization, WGSL legalization,
   CUDA immutable-load handling, etc.).
4. Returns the linked and optimized module to its caller, which hands
   it to the chosen emit backend ([06-emit.md](06-emit.md)).

SPIR-V is the exception to point 3: its main legalization is *not*
inside `linkAndOptimizeIR`. `legalizeIRForSPIRV` in
[slang-ir-spirv-legalize.cpp](../../../../source/slang/slang-ir-spirv-legalize.cpp)
(which internally runs `legalizeSPIRV` followed by the
`simplifyIRForSpirvLegalization` fixed point) is invoked from
`emitSPIRVFromIR` in
[slang-emit-spirv.cpp](../../../../source/slang/slang-emit-spirv.cpp)
(line 12495) after `linkAndOptimizeIR` returns. The comment at lines
2693-2695 of `slang-emit.cpp` records that, for SPIR-V, address-space
propagation is done during that legalization rather than inside
`linkAndOptimizeIR`.

That call is a direct one rather than a `SLANG_PASS(...)`, so the
SPIR-V legalization never goes through `wrapPass`, and no
`BEFORE legalizeSPIRV` / `AFTER legalizeSPIRV` header appears in
`-dump-ir` output; the only `dumpIR` calls around it in
[slang-emit-spirv.cpp](../../../../source/slang/slang-emit-spirv.cpp)
are compiled out behind `#if 0`. The same holds for the pre-link
region described below. `-dump-ir` emits before/after headers only for
the `SLANG_PASS` call sites of `linkAndOptimizeIR`, so a plain call
has no headers of its own. The combined result of the pre-link plain
calls does appear, as the `LOWER-TO-IR` snapshot that
`slang-lower-to-ir.cpp` (lines 15846-15855) dumps after them. Plain calls also occur inside
`linkAndOptimizeIR` itself: `validateMutableGlobalVariableTypes`
(line 1571), `lowerDiffTypeInfoInsts` (line 1612), one
`eliminateDeadCode` before the `getStringHash` check (line 1804),
`reportCheckpointIntermediates` (line 1910), and `checkStaticAssert`
(line 2178).

The pipeline is **not** a fixed list — different targets, different
optimization levels, and the presence of differentiation or coverage
instrumentation all change the sequence. Most optional passes are
gated on a `RequiredLoweringPassSet` flag; that set is computed by
`calcRequiredLoweringPassSet` (defined at line 409), called at line 1085
(immediately after linking) and again at line 1666 (after
specialization and the first round of type lowering); its flags
accumulate rather than being reset between the two scans. Compiling for
`CodeGenTarget::HostVM` returns early at lines 1832-1841, after
`performForceInlining`, `cleanUpVoidType`, and a final `simplifyIR`,
skipping every target-dependent pass below that point.

### Phase skeleton of `linkAndOptimizeIR`

The authoritative ordering is the body of the function. The table
below is a navigation aid: one row per contiguous region, with the
line range at `source_commit` and the passes that characterize it.
Passes marked with a target or option in parentheses are gated.

| Lines | Phase | Passes, in order |
| --- | --- | --- |
| 1050-1090 | Link and scan | `linkIR`, `calcRequiredLoweringPassSet`, `validateAndRemoveAssumeAddress` |
| 1101-1110 | Early strip and borrow | `stripDebugInfo` (`-g0`), `lowerGLSLShaderStorageBufferObjectsToStructuredBuffers` (non-Khronos), `translateEntryPointInParamToBorrow`, `replaceGlobalConstants` |
| 1122-1455 | Layout-dependent parameter handling | `bindExistentialSlots`, `instrumentCoverage`, `collectGlobalUniformParameters`, `checkEntryPointDecorations`, `addDenormalModeDecorations`, `collectOptiXEntryPointUniformParams` / `collectEntryPointUniformParams`, `moveEntryPointUniformParamsToGlobalScope`, `removeTorchAndCUDAEntryPoints`, `finalizeCoverageInstrumentationMetadata` |
| 1474-1512 | Pre-specialization lowering | `lowerLValueCast`, `specializeMatrixLayout`, `lowerEnumType`, `simplifyIR`, `validateUniformity` (`-validate-uniformity`), `fuseCallsToSaturatedCooperation` |
| 1522-1612 | Specialization and autodiff finalization | `generateDerivativeWrappers` (CUDA / PyTorch), `checkAutodiffPatterns`, `diagnoseCircularConformances`, `specializeModule`, `specializeHigherOrderParameters`, `finalizeAutoDiffPass` or `stripAutoDiffDecorations`, `lowerMatrixSwizzleStores`, `eliminateDeadCode`, `finalizeSpecialization`, `lowerDiffTypeInfoInsts` (plain call) |
| 1615-1659 | Post-specialization type lowering | `lowerConditionalType`, `lowerReinterpretOptional`, `checkForOptionalNoneUsage`, `lowerOptionalType`, `lowerResultType`, `legalizeNonVectorCompositeSelect` (HLSL), `lowerComInterfaces` / `generateDllImportFuncs` / `generateDllExportFuncs` (C++ source targets) |
| 1666-1710 | Rescan, host codegen, diagnostics | `calcRequiredLoweringPassSet`, the PyTorch / CUDA binding passes, `detectUninitializedResources`, `removeAvailableInDownstreamModuleDecorations`, `checkForRecursiveTypes`, `checkForRecursiveFunctions`, `checkForOutOfBoundAccess`, `checkForMissingReturns`, `checkForInvalidShaderParameterType` |
| 1719-1826 | Existential, union, and tag lowering | `inferAnyValueSizeWhereNecessary`, `unpinWitnessTables`, `lowerSumVectorMatrixInsts`, `lowerTaggedUnionTypes`, `lowerUntaggedUnionTypes`, `lowerReinterpret`, `lowerSequentialIDTagCasts`, `lowerTagInsts`, `lowerTagTypes`, `lowerExistentials`, `removeWeakUseInsts`, `performTypeInlining`, `checkGetStringHashInsts`, `lowerTuples`, `expandAutodiffParameterContexts` (CUDA source / Metal), `generateAnyValueMarshallingFunctions` |
| 1846-1918 | Stage switch and force inlining | `specializeStageSwitch`, `lowerCooperativeVectors`, `performForceInlining`, `simplifyIR` (or SCCP + DCE under minimum optimization), `threadSwitchOnConstantPhi`, `legalizeOptiXReportIntersectionsForCUDA` (CUDA), `validateBarrierFlagsForHLSL` (D3D) |
| 1938-2073 | Type legalization | `lowerAppendConsumeStructuredBuffers`, `lowerCombinedTextureSamplers`, `addUserTypeHintDecorations`, `legalizeEmptyArray`, `legalizeVectorTypes`, `inlineGlobalConstantsForLegalization`, `legalizeRayTracingPayloads`, `lowerBufferElementTypeToStorageType` (`MetalParameterBlock`), `legalizeExistentialTypeLayout`, `legalizeResourceTypes`, `legalizeEmptyTypes` |
| 2079-2199 | Global state and resource-usage specialization | `translateGlobalVaryingVar`, `fixEntryPointCallsites`, `moveGlobalVarInitializationToEntryPoints`, `legalizeResourceGlobalVars`, `lowerUntypedResourceHandleToUInt`, `lowerDynamicResourceHeap`, `specializeResourceUsage`, `specializeFuncsForBufferLoadArgs`, `deferBufferLoad`, `specializeArrayParameters`, `wrapStructuredBuffersOfMatrices` (HLSL) / `wrapCBufferElementsForMetal` |
| 2209-2533 | Buffers, varyings, per-target entry points | `legalizeByteAddressBufferOps`, `lowerCPUResourceTypes`, `validateAtomicOperations`, `synthesizeActiveMask` (CUDA / PTX), `resolveTextureFormat`, `resolveVaryingInputRef`, `legalizeEntryPointsForGLSL` / `legalizeIRForMetal` / `legalizeIRForWGSL` / `legalizeEntryPointVaryingParamsForCPU` / `...ForCUDA`, `legalizeBoolSwitchForTargetsRequiringIntSwitch`, `floatNonUniformResourceIndex` (textual), `legalizeLogicalAndOr`, `legalizeDynamicResourcesForGLSL`, `legalizeImageSubscript`, `legalizeConstantBufferLoadForGLSL`, `legalizeDispatchMeshPayloadForGLSL`, `introduceExplicitGlobalContext`, `transformParamsToConstRef`, `undoParameterCopy`, `convertEntryPointPtrParamsToRawPtrs` (C++) |
| 2542-2760 | Late cleanup and legalization | `stripLegalizationOnlyInstructions`, `removeRawDefaultConstructors`, `validateVectorsAndMatrices`, `eliminateDeadCode`, `processLateRequireCapabilityInsts`, `cleanUpVoidType`, `lowerBindingQueries`, `legalizeMeshOutputTypes`, `lowerBitCast`, `legalizeUniformBufferLoad`, `invertYOfPositionOutput` / `rcpWOfPositionInput`, `lowerBufferElementTypeToStorageType`, `lowerCopyLogical` (GLSL), `legalizeMatrixTypes`, `transformParamsToConstRef`, `legalizeArrayReturnType`, the `specializeAddressSpace*` variants, `lowerImmutableBufferLoadForCUDA`, `performForceInlining`, `performIntrinsicFunctionInlining`, `eliminateMultiLevelBreak`, `simplifyIR`, `legalizeEmptyTypes` |
| 2784-2839 | Out-of-SSA | The liveness markers (`LivenessUtil::addVariableRangeStarts` / `addRangeEnds`), `eliminatePhis`, `applyGLSLLiveness`, `replaceLocationIntrinsicsWithRaytracingObject`, `simplifyNonSSAIR` |
| 2870-2961 | Metal pointer lowering, metadata, final checks | `lowerBufferElementTypeToStorageType` (`MetalPointerLowering`), `performForceInlining`, `eliminatePhis`, `simplifyNonSSAIR`, `applyVariableScopeCorrection`, `collectCooperativeMetadata`, `unexportNonEmbeddableIR`, `collectMetadata`, `checkUnsupportedResourceLocals`, `checkUnsupportedInst` |

`validateIRModuleIfEnabled` is interleaved throughout and is omitted
above. Several passes appear more than once at different points —
`simplifyIR`, `eliminateDeadCode`, `performForceInlining`,
`legalizeEmptyTypes`, `eliminatePhis`,
`lowerBufferElementTypeToStorageType` (three configurations for
Metal), and `specializeFuncsForBufferLoadArgs` (a second invocation at
line 2719, for direct SPIR-V emission on Khronos targets).

One pass in that skeleton has no row in the category tables below
because its source falls outside this page's watched paths:
`checkForOutOfBoundAccess` is defined in
`source/slang/slang-check-out-of-bound-access.cpp`. That path should be
added to this page's `watched_paths` in the manifest so that changes to
it mark this document stale.

For an ordered control-flow-graph view of every pass that runs for a
specific target, with the conditional gates and iteration bounds drawn
explicitly, see the `target-pipelines/` subtree:
[../target-pipelines/index.md](../target-pipelines/index.md) links the
five per-target pages
([spirv](../target-pipelines/spirv.md),
[hlsl](../target-pipelines/hlsl.md),
[metal](../target-pipelines/metal.md),
[wgsl](../target-pipelines/wgsl.md), and
[cuda](../target-pipelines/cuda.md)).

A substantial number of passes also run *before* `linkAndOptimizeIR`,
inside `generateIRForTranslationUnit` on the per-translation-unit IR
module before it is cached on the `Module`. Those are documented in
[04b-pre-link-passes.md](04b-pre-link-passes.md) as an ordered,
target-agnostic pipeline (Phase A AST walk → Phase B mandatory
lowering → Phase C mandatory optimization, including the
`performMandatoryEarlyInlining` fixed-point loop → Phase D
non-essential validation and stripping). The per-target IR module
that carries `IRLayoutDecoration`s is built separately by
`TargetProgram::createIRModuleForLayout` and documented in
[04c-layout-ir.md](04c-layout-ir.md); it is **not** independently run
through the pass sequence, but its symbols and layout decorations are
consumed by `linkIR` at the very start of `linkAndOptimizeIR`
(`getExistingIRModuleForLayout` is added to the linker's input modules).

Individual category tables below still list a pass even when its
*primary* call site is in the pre-link region above (for example,
`constructSSA`, `eliminateDeadCode`, `simplifyCFG`, and
`peepholeOptimize` are all invoked both in
`generateIRForTranslationUnit` and again from `linkAndOptimizeIR`;
`propagateConstExpr`, by contrast, runs only in the pre-link region,
while the post-link cleanup uses the `simplifyIR` fixed point —
SSA / SCCP / SimplifyCFG / DCE — rather than constexpr propagation).
Treat 04b as the authoritative ordering for the pre-link region and
the per-target pages under [../target-pipelines/](../target-pipelines)
as the authoritative ordering for the post-link region.

## Pass categories

Each row below states what the pass itself does, which is not the same
question as what a reader can see it do. A large fraction of these
entries are analyses that build a side structure and leave the IR
untouched — the entry-point reference graph
(`buildEntryPointReferenceGraph`), `ReachabilityContext`, the
dominator tree, the region tree of `generateRegionTreeForFunc`, and
the autodiff loop-analysis and region passes — and several more are
normalizations whose result exists only for a later pass to consume.
Those have **no independent footprint**: nothing in the emitted source
and no header in the `-dump-ir` stream marks that they ran, and their
effect is observable only through the pass that consumes their result.
Where a pass does have a directly observable surface — a diagnostic, a
synthesized name that survives into the emit, a changed declaration —
the row names it.

### Linking and validation

Run early and again after major transformations to catch invariant
violations.

| Pass | File | Purpose |
| --- | --- | --- |
| Link | [slang-ir-link.cpp](../../../../source/slang/slang-ir-link.cpp) | Pulls in IR for `import`ed modules so all symbols resolve |
| Validate | [slang-ir-validate.cpp](../../../../source/slang/slang-ir-validate.cpp) | Structural sanity checks on the IR module (`validateIRModule`): use-def and parent/child list consistency, operand dominance, one terminator per block, block parameters before ordinary instructions. It runs only when IR validation is switched on (`validateIRModuleIfEnabled`), and a failure (`IrValidationFailed`) means an earlier pass produced malformed IR. The file also hosts `validateAndRemoveAssumeAddress`, run right after linking, and `validateVectorsAndMatrices`, run late against the target's vector / matrix limits |
| Check recursion | [slang-ir-check-recursion.cpp](../../../../source/slang/slang-ir-check-recursion.cpp) | Diagnoses unsupported recursion |
| Check unsupported inst | [slang-ir-check-unsupported-inst.cpp](../../../../source/slang/slang-ir-check-unsupported-inst.cpp) | Per-target reject-list of opcodes (`checkUnsupportedInst`); the file also hosts `checkUnsupportedResourceLocals`, which diagnoses resource types that cannot live in mutable local storage for compiled D3D or Metal output and runs regardless of optimization mode |
| Check push-constant blocks | [slang-ir-check-push-constant-blocks.cpp](../../../../source/slang/slang-ir-check-push-constant-blocks.cpp) | `checkForMultiplePushConstantBlocksPerEntryPoint` reports an error for each entry point that statically uses more than one push-constant block. It is not a `linkAndOptimizeIR` pass: SPIR-V calls it from `legalizeIRForSPIRV` and GLSL from the emitter ([slang-emit-glsl.cpp](../../../../source/slang/slang-emit-glsl.cpp)) |
| Check shader parameter type | [slang-ir-check-shader-parameter-type.cpp](../../../../source/slang/slang-ir-check-shader-parameter-type.cpp) | Validates shader-parameter type rules |
| Check optional `none` usage | [slang-ir-check-optional-none-usage.cpp](../../../../source/slang/slang-ir-check-optional-none-usage.cpp) | Diagnoses incorrect uses of `Optional`'s `none` |
| Check generic-with-existential specialize | [slang-ir-check-specialize-generic-with-existential.cpp](../../../../source/slang/slang-ir-check-specialize-generic-with-existential.cpp) | Validity of generic / existential specialization |
| Detect uninitialized resources | [slang-ir-detect-uninitialized-resources.cpp](../../../../source/slang/slang-ir-detect-uninitialized-resources.cpp) | Diagnoses use of uninitialized resource handles |
| Use of uninitialized values | [slang-ir-use-uninitialized-values.cpp](../../../../source/slang/slang-ir-use-uninitialized-values.cpp) | `checkForUsingUninitializedValues`, the module-wide uninitialized-use diagnostic, run before linking. It also provides `checkForUsingUninitializedGeneratedResourceLocal`, which `legalizeResourceGlobalVars` uses to diagnose entry-point reads of a generated resource local before it is assigned |
| Mesh-output reads | [slang-ir-mesh-output-reads.cpp](../../../../source/slang/slang-ir-mesh-output-reads.cpp) | Diagnoses reads from mesh-shader outputs; target-agnostic pre-link validation (`checkForMeshOutputReads`) |

### SSA construction and basic cleanup

| Pass | File | Purpose |
| --- | --- | --- |
| SSA | [slang-ir-ssa.cpp](../../../../source/slang/slang-ir-ssa.cpp) | Promotes alloca-like locals to SSA values |
| SSA simplification | [slang-ir-ssa-simplification.cpp](../../../../source/slang/slang-ir-ssa-simplification.cpp) | The `simplifyIR` fixed-point driver over SSA, SCCP, CFG simplification, peephole, and DCE (plus `simplifyNonSSAIR` for out-of-SSA code) |
| SSA register allocate | [slang-ir-ssa-register-allocate.cpp](../../../../source/slang/slang-ir-ssa-register-allocate.cpp) | Computes register / coalescing groups (`allocateRegistersForFunc`) consumed by phi elimination |
| Eliminate phis | [slang-ir-eliminate-phis.cpp](../../../../source/slang/slang-ir-eliminate-phis.cpp) | Out-of-SSA conversion for backends that need it |
| DCE | [slang-ir-dce.cpp](../../../../source/slang/slang-ir-dce.cpp) | Dead-code elimination |
| SCCP | [slang-ir-sccp.cpp](../../../../source/slang/slang-ir-sccp.cpp) | Sparse conditional constant propagation |
| Peephole | [slang-ir-peephole.cpp](../../../../source/slang/slang-ir-peephole.cpp) | Local pattern simplifications |
| Simplify CFG | [slang-ir-simplify-cfg.cpp](../../../../source/slang/slang-ir-simplify-cfg.cpp) | Empty-block / trivial-branch removal |
| Thread switch on constant phi | [slang-ir-thread-switch-on-constant-phi.cpp](../../../../source/slang/slang-ir-thread-switch-on-constant-phi.cpp) | `threadSwitchOnConstantPhi` jump-threads a `switch` whose selector is a block parameter carrying a distinct compile-time constant along every incoming path, then removes the dead `switch`. It runs once after force inlining at every optimization level |
| Simplify for emit | [slang-ir-simplify-for-emit.cpp](../../../../source/slang/slang-ir-simplify-for-emit.cpp) | Late simplifications targeted at emit |
| Cleanup void | [slang-ir-cleanup-void.cpp](../../../../source/slang/slang-ir-cleanup-void.cpp) | Removes spurious `void` instructions |
| Strip default construct | [slang-ir-strip-default-construct.cpp](../../../../source/slang/slang-ir-strip-default-construct.cpp) | Drops trivial default-construction insts |
| Strip legalization insts | [slang-ir-strip-legalization-insts.cpp](../../../../source/slang/slang-ir-strip-legalization-insts.cpp) | Removes scaffolding from earlier legalization |
| Strip | [slang-ir-strip.cpp](../../../../source/slang/slang-ir-strip.cpp) | Strips front-end-only decorations (optionally name hints / source locations) and, separately, imported witness-table entries |
| Strip debug info | [slang-ir-strip-debug-info.cpp](../../../../source/slang/slang-ir-strip-debug-info.cpp) | Removes debug instructions when not requested |
| Redundancy removal | [slang-ir-redundancy-removal.cpp](../../../../source/slang/slang-ir-redundancy-removal.cpp) | CSE-style cleanup |
| Single return | [slang-ir-single-return.cpp](../../../../source/slang/slang-ir-single-return.cpp) | Funnels every function to a single `return` |
| Eliminate multilevel break | [slang-ir-eliminate-multilevel-break.cpp](../../../../source/slang/slang-ir-eliminate-multilevel-break.cpp) | Lowers labeled / multilevel `break` |
| Missing return | [slang-ir-missing-return.cpp](../../../../source/slang/slang-ir-missing-return.cpp) | Diagnoses paths missing a `return` |
| Init local var | [slang-ir-init-local-var.cpp](../../../../source/slang/slang-ir-init-local-var.cpp) | Inserts default initialization for locals |
| Variable-scope correction | [slang-ir-variable-scope-correction.cpp](../../../../source/slang/slang-ir-variable-scope-correction.cpp) | Fixes scope hoisting |
| Operator shift overflow | [slang-ir-operator-shift-overflow.cpp](../../../../source/slang/slang-ir-operator-shift-overflow.cpp) | Diagnoses bad shift counts |

### Specialization and generics

The specialization machinery is what turns generic Slang IR into the
concrete IR consumed by emit.

| Pass | File | Purpose |
| --- | --- | --- |
| Specialize | [slang-ir-specialize.cpp](../../../../source/slang/slang-ir-specialize.cpp) | Generic-parameter substitution; the main specialization driver (`specializeModule`). It also lowers witness lookups when the caller sets `SpecializationOptions::lowerWitnessLookups`, which `linkAndOptimizeIR` always does |
| Specialize function call | [slang-ir-specialize-function-call.cpp](../../../../source/slang/slang-ir-specialize-function-call.cpp) | Per-call specialization |
| Specialize address space | [slang-ir-specialize-address-space.cpp](../../../../source/slang/slang-ir-specialize-address-space.cpp) | Address-space-parametric specialization |
| Specialize arrays | [slang-ir-specialize-arrays.cpp](../../../../source/slang/slang-ir-specialize-arrays.cpp) | Specializes calls to functions taking `struct` parameters with array fields, so the arrays can be referenced from global shader parameters directly (`specializeArrayParameters`) |
| Specialize buffer-load arg | [slang-ir-specialize-buffer-load-arg.cpp](../../../../source/slang/slang-ir-specialize-buffer-load-arg.cpp) | Buffer-load argument specialization |
| Specialize matrix layout | [slang-ir-specialize-matrix-layout.cpp](../../../../source/slang/slang-ir-specialize-matrix-layout.cpp) | Resolves matrix layout choices |
| Specialize resources | [slang-ir-specialize-resources.cpp](../../../../source/slang/slang-ir-specialize-resources.cpp) | Specializes resource-typed parameters |
| Specialize stage switch | [slang-ir-specialize-stage-switch.cpp](../../../../source/slang/slang-ir-specialize-stage-switch.cpp) | Resolves `[stage]`-conditional code |
| Specialize target switch | [slang-ir-specialize-target-switch.cpp](../../../../source/slang/slang-ir-specialize-target-switch.cpp) | Resolves `[target]`-conditional code |
| Specialize higher-order parameters | [slang-ir-defunctionalization.cpp](../../../../source/slang/slang-ir-defunctionalization.cpp) | `specializeHigherOrderParameters`: rewrites calls that pass a global function to a higher-order function into calls to a specialized variant that references the global directly (the filename is aspirational; no tagged-union conversion happens) |
| Bind existentials | [slang-ir-bind-existentials.cpp](../../../../source/slang/slang-ir-bind-existentials.cpp) | Resolves dynamic-dispatch interface bindings |
| AnyValue inference | [slang-ir-any-value-inference.cpp](../../../../source/slang/slang-ir-any-value-inference.cpp) | Determines `AnyValue` size for existentials (`inferAnyValueSizeWhereNecessary`); the size it settles on is the `N` that shows up in the marshalling function names below. The same file also holds `diagnoseCircularConformances`, which runs before specialization so a conformance cycle is diagnosed rather than reaching a pass that cannot represent it.. An ordinary source-level inheritance cycle is rejected earlier, during semantic checking (E39999), so this is an IR backstop |
| AnyValue marshalling | [slang-ir-any-value-marshalling.cpp](../../../../source/slang/slang-ir-any-value-marshalling.cpp) | Pack / unpack values into `AnyValue`: synthesizes one `packAnyValue<N>` / `unpackAnyValue<N>` pair per (concrete type, AnyValue size) combination, so the size-based name hints can repeat, and replaces every `PackAnyValue` / `UnpackAnyValue` inst with a call to it, so those two names are what a dynamic-dispatch shader shows in the emitted source |
| Lower dynamic-dispatch insts | [slang-ir-lower-dynamic-dispatch-insts.cpp](../../../../source/slang/slang-ir-lower-dynamic-dispatch-insts.cpp) | Lowers the instructions that back dynamic dispatch: `lowerExistentials` (existential extraction, `InterfaceType`, `RTTIHandleType`), `lowerTaggedUnionTypes` / `lowerUntaggedUnionTypes`, and the tag family (`lowerTagTypes`, `lowerTagInsts`, `lowerSequentialIDTagCasts`) |
| Lower expand type | [slang-ir-lower-expand-type.cpp](../../../../source/slang/slang-ir-lower-expand-type.cpp) | Variadic-pack expansion |
| Remove unused generic param | [slang-ir-remove-unused-generic-param.cpp](../../../../source/slang/slang-ir-remove-unused-generic-param.cpp) | Cleans up specialized generics |
| Deduplicate | [slang-ir-deduplicate.cpp](../../../../source/slang/slang-ir-deduplicate.cpp) | Re-deduplicates hoistable insts after edits |
| Deduplicate generic children | [slang-ir-deduplicate-generic-children.cpp](../../../../source/slang/slang-ir-deduplicate-generic-children.cpp) | Deduplicates within a generic's body |
| Typeflow set | [slang-ir-typeflow-set.cpp](../../../../source/slang/slang-ir-typeflow-set.cpp) | Type-flow set construction |
| Typeflow specialize | [slang-ir-typeflow-specialize.cpp](../../../../source/slang/slang-ir-typeflow-specialize.cpp) | `specializeDynamicInsts`: an interprocedural data-flow analysis that rewrites `LookupWitnessMethod`, `ExtractExistentialValue`, `ExtractExistentialType` and `ExtractExistentialWitnessTable` into specialized forms based on the values that can reach each use, run in a loop with `specializeModule` until neither changes anything. It emits no code of its own; its one direct user surface is the dynamic-dispatch-site report, enabled by `reportDynamicDispatchSites`, which names each call site it could not resolve statically |
| Constexpr | [slang-ir-constexpr.cpp](../../../../source/slang/slang-ir-constexpr.cpp) | Compile-time evaluation |

`specializeHigherOrderParameters` fires on any parameter whose lowered
type is an `IRFuncType`, and the surface form that produces one is the
`functype` type constructor:

```slang
func apply(f : functype (int) -> int, x : int) -> int { return f(x); }
func twice(x : int) -> int { return 2 * x; }
func use(x : int) -> int { return apply(twice, x); }
```

The `apply(twice, x)` call becomes a call to a variant of `apply` that
references `twice` directly, so no function-typed value survives to
emit. The core module declares its higher-order builtins the same way:
`saturated_cooperation` in
[hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang) takes two
`functype (A, B) -> C` parameters. `IFunc<...>` does not reach this
pass — it is an `interface` in
[core.meta.slang](../../../../source/slang/core.meta.slang), so a
parameter of that type is an existential and goes through the
specialization path above instead.

### Differentiation (autodiff)

Forward- and reverse-mode automatic differentiation transformations.
The deeper user-level documentation lives in
[../../../design/autodiff.md](../../../design/autodiff.md).

| Pass | File | Purpose |
| --- | --- | --- |
| Autodiff (driver) | [slang-ir-autodiff.cpp](../../../../source/slang/slang-ir-autodiff.cpp) | Top-level differentiation orchestration (`finalizeAutoDiffPass`). It also provides `stripAutoDiffDecorations`, which `linkAndOptimizeIR` runs instead on modules with no autodiff constructs, so that the core-module autodiff builtins lose their keep-alive decorations and can be dead-code-eliminated |
| Forward mode | [slang-ir-autodiff-fwd.cpp](../../../../source/slang/slang-ir-autodiff-fwd.cpp) | Forward-mode derivative emission |
| Reverse mode | [slang-ir-autodiff-rev.cpp](../../../../source/slang/slang-ir-autodiff-rev.cpp) | Reverse-mode derivative emission |
| Transpose | [slang-ir-autodiff-transpose.cpp](../../../../source/slang/slang-ir-autodiff-transpose.cpp) | Reverse-mode transposition |
| Unzip | [slang-ir-autodiff-unzip.cpp](../../../../source/slang/slang-ir-autodiff-unzip.cpp) | Splits primal / differential code paths |
| CFG normalization | [slang-ir-autodiff-cfg-norm.cpp](../../../../source/slang/slang-ir-autodiff-cfg-norm.cpp) | CFG canonicalization for autodiff |
| Loop analysis | [slang-ir-autodiff-loop-analysis.cpp](../../../../source/slang/slang-ir-autodiff-loop-analysis.cpp) | Loop-structure analysis used by autodiff |
| Pairs | [slang-ir-autodiff-pairs.cpp](../../../../source/slang/slang-ir-autodiff-pairs.cpp) | Primal / differential pair construction |
| Primal hoist | [slang-ir-autodiff-primal-hoist.cpp](../../../../source/slang/slang-ir-autodiff-primal-hoist.cpp) | Hoists primal computation for reuse |
| Region | [slang-ir-autodiff-region.cpp](../../../../source/slang/slang-ir-autodiff-region.cpp) | Differentiable-region analysis |
| Check differentiability | [slang-ir-check-differentiability.cpp](../../../../source/slang/slang-ir-check-differentiability.cpp) | Validates that flagged code is actually differentiable |
| Expand autodiff parameter contexts | [slang-ir-expand-autodiff-parameter-contexts.cpp](../../../../source/slang/slang-ir-expand-autodiff-parameter-contexts.cpp) | `expandAutodiffParameterContexts` expands marked captured-parameter contexts in internal derivative signatures and direct calls. It runs after tuple lowering, only for CUDA source and Metal targets, and keeps external and reference ABIs |

Only the endpoints of this family have names a reader can look for.
Forward mode gives the generated function the name hint
`s_fwd_<original>`; reverse mode produces `s_bwdProp_<original>` for
the propagation function and `s_bwdCallableCtx_<original>` for the
intermediate-context `struct` that carries checkpointed primal values
between the two halves. Those three names are what a
`[Differentiable]` function plus a `fwd_diff` / `bwd_diff` call turns
into in the emitted source. Primal hoist decides, per value, whether
it is stored in that context or recomputed, so it is visible as the
field list of `s_bwdCallableCtx_<original>` rather than as anything of
its own; unzip additionally emits `ReportCheckpointStore` markers when
`ReportCheckpointIntermediates` is set, which
`reportCheckpointIntermediates` in
[slang-emit.cpp](../../../../source/slang/slang-emit.cpp) later turns
into a per-function report of the checkpointed intermediates and their
sizes. Transpose, pairs, CFG normalization, loop analysis and region
analysis are stages inside that same transcription and leave no
separately identifiable token.

### Type and value legalization

Adapt high-level Slang types to the simpler shapes downstream targets
expect.

| Pass | File | Purpose |
| --- | --- | --- |
| Legalize types | [slang-ir-legalize-types.cpp](../../../../source/slang/slang-ir-legalize-types.cpp) | Splits aggregate / parametric types |
| Legalize ray-tracing payloads | [slang-ir-ray-tracing-legalize.cpp](../../../../source/slang/slang-ir-ray-tracing-legalize.cpp) | `legalizeRayTracingPayloads` adapts ray / callable payloads and hit attributes at native target interfaces: empty payloads get physical storage (a nonempty dummy struct on D3D, GLSL and SPIR-V, none on CUDA / OptiX); on D3D a nonempty non-struct payload or hit attribute is wrapped in a one-field struct; ray-payload access qualifiers are normalized. It is gated on `LinkingAndOptimizationOptions::shouldLegalizeExistentialAndResourceTypes` (false for C, C++ and CUDA source output) and runs before resource-type legalization and `fixEntryPointCallsites` |
| Legalize resource globals | [slang-ir-legalize-resource-globals.cpp](../../../../source/slang/slang-ir-legalize-resource-globals.cpp) | `legalizeResourceGlobalVars` replaces mutable file- or namespace-scope `static` resource globals, and the mutable uniform-parameter shadows synthesized for `-Gec`, with per-entry-point-invocation function-local storage, adding generated parameters and call arguments to the non-entry-point functions that touch them. It validates every selected global before changing the module and leaves the module unchanged if any check fails. It runs after `moveGlobalVarInitializationToEntryPoints` and before `specializeResourceUsage` |
| Legalize array return type | [slang-ir-legalize-array-return-type.cpp](../../../../source/slang/slang-ir-legalize-array-return-type.cpp) | Returning arrays by reference |
| Legalize binary operator | [slang-ir-legalize-binary-operator.cpp](../../../../source/slang/slang-ir-legalize-binary-operator.cpp) | Per-target binary-op legalization |
| Legalize composite select | [slang-ir-legalize-composite-select.cpp](../../../../source/slang/slang-ir-legalize-composite-select.cpp) | Lowers `select` on composites |
| Legalize empty array | [slang-ir-legalize-empty-array.cpp](../../../../source/slang/slang-ir-legalize-empty-array.cpp) | Avoids zero-length arrays |
| Legalize global values | [slang-ir-legalize-global-values.cpp](../../../../source/slang/slang-ir-legalize-global-values.cpp) | Legalizes module-scope values: `inlineGlobalValuesAndRemoveIfUnused` clones each global inst the target cannot hold at module scope into every function body that uses it and deletes the global if that leaves it unused, so the global declaration disappears from the emit and its value reappears at each use site. `inlineGlobalConstantsForLegalization` does the same for resource-typed global constants, ahead of resource legalization. Each target supplies its own legality predicates by deriving from `GlobalInstInliningContextGeneric`. Plain constant data such as `static const int kTable[4]` is holdable at module scope on HLSL, WGSL and Metal and survives unchanged; only a global whose value the target has no module-scope form for (e.g. resource handles on some targets) moves. |
| Lower CUDA builtin types | [slang-ir-lower-cuda-builtin-types.cpp](../../../../source/slang/slang-ir-lower-cuda-builtin-types.cpp) | Flattens matrix, vector, array and `struct` parameters of `[CudaKernel]` functions into plain storage structs holding a single `data` array, plus generated pack / unpack functions called at the kernel boundary. A `float2x2` parameter becomes a struct name-hinted `_MatrixStorage_float2x2` (`_ColMajor` suffixed under column-major layout) and a `float3` becomes `_VectorStorage_float3_`; those names show in the CUDA and PyTorch-binding output. Driven from `lowerBuiltinTypesForKernelEntryPoints` in [slang-ir-pytorch-cpp-binding.cpp](../../../../source/slang/slang-ir-pytorch-cpp-binding.cpp) for the CUDA source / header and PyTorch C++ binding targets |
| Lower GLSL SSBO types | [slang-ir-lower-glsl-ssbo-types.cpp](../../../../source/slang/slang-ir-lower-glsl-ssbo-types.cpp) | GLSL SSBO ABI |
| Lower dynamic resource heap | [slang-ir-lower-dynamic-resource-heap.cpp](../../../../source/slang/slang-ir-lower-dynamic-resource-heap.cpp) | Bindless resource heap (`lowerDynamicResourceHeap`), plus `lowerUntypedResourceHandleToUInt`, which reduces any descriptor-heap handle that peephole did not collapse to its underlying `uint` index so emit and layout never see an untyped handle |
| Wrap cbuffer element | [slang-ir-wrap-cbuffer-element.cpp](../../../../source/slang/slang-ir-wrap-cbuffer-element.cpp) | Wraps cbuffer scalars |
| Wrap structured buffers | [slang-ir-wrap-structured-buffers.cpp](../../../../source/slang/slang-ir-wrap-structured-buffers.cpp) | Structured-buffer element wrapping |
| Bit-field accessors | [slang-ir-bit-field-accessors.cpp](../../../../source/slang/slang-ir-bit-field-accessors.cpp) | Fills in the bodies of the getter / setter functions carrying `IRBitFieldAccessorDecoration` with shift-and-mask arithmetic over the backing integer, so a bit-field read emits shifts and masks rather than a field access. Runs before linking, from `generateIRForTranslationUnit` |
| Address inst elimination | [slang-ir-addr-inst-elimination.cpp](../../../../source/slang/slang-ir-addr-inst-elimination.cpp) | Rewrites address load / store into value extract / update — `load(elementPtr(arr, 1))` becomes `elementExtract(load(arr), 1)`, `store(fieldAddr(s, k), v)` becomes a store of `updateField(load(s), k, v)` — so that every remaining `load` / `store` addresses a var or param and SSA construction can promote struct and array elements. Its only caller is `prepareFuncForForwardDiff`, so it runs on the autodiff path rather than for every module |
| Extract value from type | [slang-ir-extract-value-from-type.cpp](../../../../source/slang/slang-ir-extract-value-from-type.cpp) | Not a module pass: `extractValueAtOffset` emits the code that reads a value of a given byte size at a byte offset out of a `struct`, array, vector or basic-typed value. Bit-cast lowering is its only user |
| Resolve texture format | [slang-ir-resolve-texture-format.cpp](../../../../source/slang/slang-ir-resolve-texture-format.cpp) | Resolves texture formats |
| Resolve varying input ref | [slang-ir-resolve-varying-input-ref.cpp](../../../../source/slang/slang-ir-resolve-varying-input-ref.cpp) | Replaces each `ResolveVaryingInputRef` inst with the parameter or global param that actually holds the varying input, directly or through a local variable initialized by loading that parameter, preserving element-address projections. A reference with no recognized parameter assignment is left unchanged. Gated on the presence of that opcode, so the pass is skipped entirely for shaders that never take a reference to a stage input |

### Inlining and call-graph

| Pass | File | Purpose |
| --- | --- | --- |
| Inline | [slang-ir-inline.cpp](../../../../source/slang/slang-ir-inline.cpp) | Function inlining |
| Call graph | [slang-ir-call-graph.cpp](../../../../source/slang/slang-ir-call-graph.cpp) | Call-graph analysis |
| Reachability | [slang-ir-reachability.cpp](../../../../source/slang/slang-ir-reachability.cpp) | Reachability analysis |
| Propagate func properties | [slang-ir-propagate-func-properties.cpp](../../../../source/slang/slang-ir-propagate-func-properties.cpp) | Bottom-up function-property inference: adds `ReadNone` / `NoSideEffect` decorations to functions whose bodies turn out to have neither. Nothing is emitted for the decorations themselves; they change what `doesCalleeHaveSideEffect` reports, so their footprint is that an unused call to an inferred-pure function becomes dead code |
| DLL export | [slang-ir-dll-export.cpp](../../../../source/slang/slang-ir-dll-export.cpp) | Generates a marshalled native-call export wrapper and moves the export decorations onto it |
| DLL import | [slang-ir-dll-import.cpp](../../../../source/slang/slang-ir-dll-import.cpp) | Synthesizes runtime library / function-pointer loading plus a marshalled indirect call body |
| Marshal native call | [slang-ir-marshal-native-call.cpp](../../../../source/slang/slang-ir-marshal-native-call.cpp) | Native ABI marshalling. `NativeCallMarshallingContext` is a helper, not a module pass: it maps a Slang type or function type to its native equivalent and builds the conversion code around a call. Its footprint is whatever the three passes that use it — DLL export, DLL import, and COM-method lowering — emit |
| Defer buffer load | [slang-ir-defer-buffer-load.cpp](../../../../source/slang/slang-ir-defer-buffer-load.cpp) | Pushes a whole-element buffer load down its access chain so only the part that is used is read: `FieldExtract(GetElement(StructuredBufferLoad(s, i), j), k)` becomes an address chain (`RWStructuredBufferGetElementPtr` / `ElementAddress` / `FieldAddress`) ending in a `Load` of the leaf. The emit changes in the width of the read, not statement position. It only bothers with element types worth deferring — arrays and composites containing them, or values above a size threshold — and leaves tiny values alone |

### Entry-point and parameter handling

| Pass | File | Purpose |
| --- | --- | --- |
| Entry-point pass framework | [slang-ir-entry-point-pass.cpp](../../../../source/slang/slang-ir-entry-point-pass.cpp) | Common scaffold for per-entry-point passes |
| Entry-point decorations | [slang-ir-entry-point-decorations.cpp](../../../../source/slang/slang-ir-entry-point-decorations.cpp) | Validates entry-point output-topology decorations against the stage and target (`checkEntryPointDecorations`) |
| Entry-point raw-ptr params | [slang-ir-entry-point-raw-ptr-params.cpp](../../../../source/slang/slang-ir-entry-point-raw-ptr-params.cpp) | Pointer-typed entry-point parameters |
| Entry-point uniforms | [slang-ir-entry-point-uniforms.cpp](../../../../source/slang/slang-ir-entry-point-uniforms.cpp) | Collects uniform entry-point parameters into one aggregate (`collectEntryPointUniformParams`) and, for applicable targets, moves it to global scope (`moveEntryPointUniformParamsToGlobalScope`) |
| Optix entry-point uniforms | [slang-ir-optix-entry-point-uniforms.cpp](../../../../source/slang/slang-ir-optix-entry-point-uniforms.cpp) | OptiX-specific entry-point uniform handling |
| Fix entry-point callsite | [slang-ir-fix-entrypoint-callsite.cpp](../../../../source/slang/slang-ir-fix-entrypoint-callsite.cpp) | Cleans up callers of entry-point functions |
| Transform params to constref | [slang-ir-transform-params-to-constref.cpp](../../../../source/slang/slang-ir-transform-params-to-constref.cpp) | `in` parameters → `const&` (`transformParamsToConstRef`), plus `translateEntryPointInParamToBorrow`, which runs on every target immediately after linking |
| Undo param copy | [slang-ir-undo-param-copy.cpp](../../../../source/slang/slang-ir-undo-param-copy.cpp) | Undoes pessimistic param copies |

No pass in this category lowers `[numthreads(...)]`, which is the
entry-point decoration a reader is most likely to go looking for.
AST-to-IR lowering attaches it to the entry-point function as an
`IRNumThreadsDecoration` and it survives untouched to emit, where each
backend writes its own launch-dimension marker from it —
`numthreads(8, 1, 1)` on HLSL, `local_size_x = 8` on GLSL,
`OpExecutionMode %main LocalSize 8 1 1` on SPIR-V, `@workgroup_size(8`
on WGSL. The passes in between only read it (the varying-parameter
legalizer, global-varying translation, and global-uniform collection
all consult it), with one exception: SPIR-V legalization adds a
default `NumThreadsDecoration` of `(1, 1, 1)` to a compute entry point
that has none.

Two stage-specific rewrites of the entry point itself live in
[slang-ir-glsl-legalize.cpp](../../../../source/slang/slang-ir-glsl-legalize.cpp)
rather than in the passes above. For hull shaders,
`invokePatchConstantFuncInHullShader` splices the
`[patchconstantfunc]` into the entry point, because GLSL and SPIR-V
have no separate patch-constant stage: every `return` becomes a branch
to a new tail block that emits a control barrier, compares
`SV_OutputControlPointID` against 0, and calls the patch-constant
function inside the `true` arm. The two patch kinds are supplied from
different places — an `InputPatch` parameter is satisfied by the entry
point's own `InputPatch` parameter, while an `OutputPatch` parameter
is materialized from the control-point output array the entry point
has just written. A patch-constant parameter that is neither and
carries no recognized system-value semantic (`SV_OutputControlPointID`
or `SV_PrimitiveID`) is diagnosed.

For ray-tracing stages, `consolidateRayTracingParameters` counts the
entry point's varying `out` / `inout` parameters. With at most one it
hands each parameter to the ordinary single-parameter path; with two
or more it merges *all* of them into the fields of one anonymous
`struct`, held in a single global variable in the `IncomingRayPayload`
address space and decorated with ray-payload location 0, and rewrites
each parameter use into a field address of that variable. Hit-object
attributes are not folded in and keep their own variable. Afterwards
`assignRayPayloadHitObjectAttributeLocations` numbers every payload
left without an explicit location, using three independent counters —
ray payload, callable payload, and hit-object attribute — each of
which skips locations already claimed by an explicit annotation.

### Layout and binding

| Pass | File | Purpose |
| --- | --- | --- |
| Layout | [slang-ir-layout.cpp](../../../../source/slang/slang-ir-layout.cpp) | Computes IR-level layout |
| Collect global uniforms | [slang-ir-collect-global-uniforms.cpp](../../../../source/slang/slang-ir-collect-global-uniforms.cpp) | Gathers globals into uniform buffers |
| Explicit global context | [slang-ir-explicit-global-context.cpp](../../../../source/slang/slang-ir-explicit-global-context.cpp) | Threads global context through calls |
| Explicit global init | [slang-ir-explicit-global-init.cpp](../../../../source/slang/slang-ir-explicit-global-init.cpp) | `moveGlobalVarInitializationToEntryPoints` moves global initializers that the target or resource-global replacement requires into each defined module-scope entry point and CUDA kernel; selected globals keep their storage but lose their initializer bodies, and `ActualGlobal` variables are excluded. Native host targets keep module-scope initialization except for selected resource globals; HLSL keeps declaration initializers except for arrays, cooperative vectors and selected resource globals; other kernel targets move every non-`ActualGlobal` initializer. For a selected resource global it first proves that evaluating the initializer at each entry point cannot change observable behavior, and diagnoses every failure without changing the module if the proof fails |
| Translate global varying var | [slang-ir-translate-global-varying-var.cpp](../../../../source/slang/slang-ir-translate-global-varying-var.cpp) | Stage-varying globals |
| Late require capability | [slang-ir-late-require-capability.cpp](../../../../source/slang/slang-ir-late-require-capability.cpp) | `processLateRequireCapabilityInsts` processes and eliminates the deferred `LateRequireCapability` instructions, diagnosing missing capabilities as warnings or errors depending on `-restrictive-capability-check` |
| User type hint | [slang-ir-user-type-hint.cpp](../../../../source/slang/slang-ir-user-type-hint.cpp) | `addUserTypeHintDecorations` records each global param's original type name in a `UserTypeNameDecoration` before type lowering can erase it. The SPIR-V backend turns that into an `OpDecorateString ... UserTypeGOOGLE` annotation (requiring `SPV_GOOGLE_user_type`), which is how `-fspv-reflect` reports user-friendly parameter type names |
| Metadata | [slang-ir-metadata.cpp](../../../../source/slang/slang-ir-metadata.cpp) | Fills in the `ArtifactPostEmitMetadata` at the end of the pipeline: `collectMetadata` for binding and exported-function data, `collectCooperativeMetadata` for cooperative-matrix / cooperative-vector types that survive target lowering. None of it reaches the generated source; it is attached to the compiled artifact as `IArtifactPostEmitMetadata`, and the invocable surface is `IComponentType::getTargetMetadata` / `getEntryPointMetadata` ([slang.h](../../../../include/slang.h) lines 5682 and 5693), which hand back an `IMetadata` (line 4813).. The interface is small: `isParameterLocationUsed(category, spaceIndex, registerIndex, outUsed)` and `getDebugBuildIdentifier()`; there is no `slangc` flag that prints it, so a test observes it through the host rather than the emit. |
| String hash | [slang-ir-string-hash.cpp](../../../../source/slang/slang-ir-string-hash.cpp) | Manages the module's global hashed-string-literal pool (`findGlobalHashedStringLiterals` / `addGlobalHashedStringLiterals`) and validates that every `getStringHash` operand is a string literal (`checkGetStringHashInsts`) |

### Loop transformations

| Pass | File | Purpose |
| --- | --- | --- |
| Loop unroll | [slang-ir-loop-unroll.cpp](../../../../source/slang/slang-ir-loop-unroll.cpp) | `[unroll]` enforcement and unrolling |
| Loop inversion | [slang-ir-loop-inversion.cpp](../../../../source/slang/slang-ir-loop-inversion.cpp) | Rewrites a loop whose first block is a small conditional branching to the break block into a guarded do-while, duplicating that condition block. It is off unless `CompilerOptionName::LoopInversion` is set, and its only call site is in the pre-link region rather than in `linkAndOptimizeIR` |
| Fuse satcoop | [slang-ir-fuse-satcoop.cpp](../../../../source/slang/slang-ir-fuse-satcoop.cpp) | Fuses adjacent calls to the core-module `saturated_cooperation` builtin into one, floating the intervening instructions above or below as their dependencies allow, so two wave-match regions in the emit collapse into one |
| Restructure | [slang-ir-restructure.cpp](../../../../source/slang/slang-ir-restructure.cpp) | Builds the `RegionTree` that gives an IR function a statement-shaped reading of its CFG (`generateRegionTreeForFunc`). It layers information over the CFG instead of rewriting it, so it changes nothing in the IR; the source emitters consume the tree when they turn blocks back into `if` / `for` / `switch`. Control flow it cannot structure is diagnosed |
| Restructure scoping | [slang-ir-restructure-scoping.cpp](../../../../source/slang/slang-ir-restructure-scoping.cpp) | `fixValueScoping` repairs the case where a value is defined in one region and used in a region that is not nested inside it, which the target language's scoping rules would reject. It inserts temporaries at a broad enough scope, so the visible effect is an extra local variable in the emitted source |
| Synthesize active mask | [slang-ir-synthesize-active-mask.cpp](../../../../source/slang/slang-ir-synthesize-active-mask.cpp) | Gives every function that uses the wave active mask an explicit mask parameter and rewrites its wave intrinsics to take it; entry points instead compute the mask as the first operation of their body, so the emitted CUDA gains both an extra argument on those helpers and a mask computation at the top of the kernel |
| Uniformity | [slang-ir-uniformity.cpp](../../../../source/slang/slang-ir-uniformity.cpp) | Uniformity (divergence) analysis, run only under `-validate-uniformity`. Its user-visible output is a diagnostic, and `linkAndOptimizeIR` aborts the compile immediately if it reported an error; the only IR it rewrites is to strip the `TreatAsDynamicUniform` markers once the analysis has consumed them. With the flag off the pass does not run at all |

### Target-specific lowering

These passes are gated on the target given at the start of each
Purpose cell. One of them runs on every target in two modes.

| Pass | File | Purpose |
| --- | --- | --- |
| GLSL legalize | [slang-ir-glsl-legalize.cpp](../../../../source/slang/slang-ir-glsl-legalize.cpp) | GLSL / SPIR-V: entry-point legalization (`legalizeEntryPointsForGLSL`) and related fixes; also holds `legalizeBoolSwitchForTargetsRequiringIntSwitch`, which rewrites a `switch` on `bool` into an integer switch for GLSL, SPIR-V, and WGSL |
| GLSL liveness | [slang-ir-glsl-liveness.cpp](../../../../source/slang/slang-ir-glsl-liveness.cpp) | GLSL / Khronos targets, when liveness tracking is on: `applyGLSLLiveness` turns live-range instructions into calls to declared functions that map to SPIR-V opcodes |
| HLSL legalize | [slang-ir-hlsl-legalize.cpp](../../../../source/slang/slang-ir-hlsl-legalize.cpp) | HLSL / D3D: `validateBarrierFlagsForHLSL` checks barrier flags (the ray-payload legalization that used to live here is now `legalizeRayTracingPayloads` above) |
| Metal legalize | [slang-ir-metal-legalize.cpp](../../../../source/slang/slang-ir-metal-legalize.cpp) | Metal: `legalizeIRForMetal` adapts the module to MSL rules, plus `specializeAddressSpaceForMetal` |
| SPIR-V legalize | [slang-ir-spirv-legalize.cpp](../../../../source/slang/slang-ir-spirv-legalize.cpp) | SPIR-V: legalizes the IR for direct SPIR-V emission, called after `linkAndOptimizeIR` returns |
| WGSL legalize | [slang-ir-wgsl-legalize.cpp](../../../../source/slang/slang-ir-wgsl-legalize.cpp) | WGSL: `legalizeIRForWGSL` adapts the module to WGSL rules, plus `specializeAddressSpaceForWGSL` |
| CUDA immutable load | [slang-ir-cuda-immutable-load.cpp](../../../../source/slang/slang-ir-cuda-immutable-load.cpp) | CUDA: `lowerImmutableBufferLoadForCUDA` rewrites immutable buffer loads to use the `__ldg` intrinsic |
| Vulkan invert Y | [slang-ir-vk-invert-y.cpp](../../../../source/slang/slang-ir-vk-invert-y.cpp) | Khronos targets and HLSL, under `VulkanInvertY`: `invertYOfPositionOutput` flips the y of `SV_Position` outputs right before the write |
| Float non-uniform resource index | [slang-ir-float-non-uniform-resource-index.cpp](../../../../source/slang/slang-ir-float-non-uniform-resource-index.cpp) | All targets, in two modes: SPIRV mode (from `slang-ir-spirv-legalize.cpp`) bubbles the `NonUniformResourceIndex` wrapper to the leaf and propagates the `NonUniform` decoration; textual mode (from `slang-emit.cpp`, all non-SPIR-V targets) only repositions the wrapper and emits no decoration |
| Early raytracing intrinsic simplification | [slang-ir-early-raytracing-intrinsic-simplification.cpp](../../../../source/slang/slang-ir-early-raytracing-intrinsic-simplification.cpp) | Direct SPIR-V on Khronos targets: `replaceLocationIntrinsicsWithRaytracingObject` replaces ray-location operands with the global objects carrying matching decorations, diagnosing a missing match. Gated on `RequiredLoweringPassSet::rayTracingLocationOperand`, run after phi elimination |
| PyTorch C++ binding | [slang-ir-pytorch-cpp-binding.cpp](../../../../source/slang/slang-ir-pytorch-cpp-binding.cpp) | Torch: `generatePyTorchCppBinding` generates the host-side binding functions, with helpers for kernel entry points and auto-bind names |
| COM interface | [slang-ir-com-interface.cpp](../../../../source/slang/slang-ir-com-interface.cpp) | CPU / COM: `lowerComInterfaces` turns COM-decorated interface types (and `this` types) into `ComPtr` types |

### Instrumentation

| Pass | File | Purpose |
| --- | --- | --- |
| Coverage instrument | [slang-ir-coverage-instrument.cpp](../../../../source/slang/slang-ir-coverage-instrument.cpp) | Enabled by `-trace-coverage` ([slang-options.cpp](../../../../source/slang/slang-options.cpp) line 640); the `-trace-coverage-*` options below refine an already-enabled mode rather than turning one on, so `-trace-coverage-boolean` alone emits nothing. Synthesizes a `__slang_coverage` buffer (`RWStructuredBuffer<uint64_t>` by default, `uint` when `counterByteWidth` is 4; the name survives verbatim only on SPIR-V, while source-language targets mangle it, e.g. `_slang_coverage_0`) and rewrites marker ops into atomic adds; honors `-trace-coverage-binding` / `-trace-coverage-reserved-space`, and `-trace-coverage-boolean` records execution as a non-atomic store of `1`. For Metal, `linkAndOptimizeIR` caps counting-mode counters at 4 bytes first (MSL has no 64-bit atomic fetch-add), warning if 64 bits was requested; boolean mode is exempt. The linked module must contain coverage marker opcodes (`IncrementCoverageCounter`, `IncrementFunctionCoverageCounter`, `IncrementBranchCoverageCounter`), which set the `coverageTracing` flag in `RequiredLoweringPassSet`, and `CodeGenContext::shouldTraceAnyCoverage()` must be true. Counter width is set by `-trace-coverage-counter-width` (bits, 32 or 64) or `CompilerOptionName::TraceCoverageCounterByteWidth` (bytes, 4 or 8); the default is 8 |
| Finalize coverage metadata | [slang-ir-coverage-instrument.cpp](../../../../source/slang/slang-ir-coverage-instrument.cpp) | `finalizeCoverageInstrumentationMetadata`; runs after global / entry-point uniform packing to fill in CPU/CUDA uniform-marshaling fields determined by the final post-packing layout |
| Insert debug value store | [slang-ir-insert-debug-value-store.cpp](../../../../source/slang/slang-ir-insert-debug-value-store.cpp) | Debug-info preservation across optimization |
| Liveness | [slang-ir-liveness.cpp](../../../../source/slang/slang-ir-liveness.cpp) | Liveness analysis used by debug info |
| Obfuscate loc | [slang-ir-obfuscate-loc.cpp](../../../../source/slang/slang-ir-obfuscate-loc.cpp) | Optional source-loc obfuscation for distributed modules |

### Other passes

A handful of passes do not fit the categories above. They are still
listed for completeness.

| Pass | File | Purpose |
| --- | --- | --- |
| SPIR-V opcode info / snippet | [slang-ir-spirv-snippet.cpp](../../../../source/slang/slang-ir-spirv-snippet.cpp) | SPIR-V code-snippet helpers used by SPIR-V passes |

## Adding a new pass

When adding a pass:

1. Create a `slang-ir-<name>.{h,cpp}` pair under
   [source/slang/](../../../../source/slang). The header declares the
   public entry function (e.g. `void doMyPass(IRModule*, ...)`); the
   `.cpp` contains the implementation.
2. Use the helpers in
   [slang-ir.h](../../../../source/slang/slang-ir.h) and
   [slang-ir-clone.cpp](../../../../source/slang/slang-ir-clone.cpp) /
   [slang-ir-dominators.cpp](../../../../source/slang/slang-ir-dominators.cpp) /
   [slang-ir-call-graph.cpp](../../../../source/slang/slang-ir-call-graph.cpp)
   instead of writing one-off traversals; respect the hoistable / global-
   value rules described in
   [../../../design/ir.md](../../../design/ir.md).
3. Insert a call site at the stage that owns the transformation. For a
   post-link pass owned by this orchestrator, add it to
   `linkAndOptimizeIR`
   ([slang-emit.cpp](../../../../source/slang/slang-emit.cpp)) at the
   appropriate point in the sequence, using the local `SLANG_PASS`
   macro rather than calling the function directly, so the pass
   participates in IR dumping and profiling like its neighbors. A pass
   that must run before linking belongs in
   `generateIRForTranslationUnit` instead (see
   [04b-pre-link-passes.md](04b-pre-link-passes.md)), as a plain call.
   A pass owned by a backend belongs in that backend's legalization
   orchestrator — for example direct SPIR-V passes go into
   `legalizeIRForSPIRV`, which `emitSPIRVFromIR` runs *after*
   `linkAndOptimizeIR` has returned.
4. If the pass is target-specific, gate it on the `TargetRequest`. If
   it is only needed when a particular construct is present, add a
   flag to `RequiredLoweringPassSet` and set it from
   `calcRequiredLoweringPassSet`, so that modules without the
   construct skip the whole-module walk.
5. Add tests under [tests/](../../../../tests) — typically a
   `COMPARE_COMPUTE` or `INTERPRET` test, plus a `DIAGNOSTIC_TEST` if
   the pass emits errors.
6. Run `./extras/formatting.sh` before committing.

## Pass utilities

These files do not implement transformations but are linked into
many passes. They provide IR walking, instruction-info lookup, and
cloning support that the categorized passes above rely on.

| Module | File | Purpose |
| --- | --- | --- |
| Clone | [slang-ir-clone.cpp](../../../../source/slang/slang-ir-clone.cpp) | Generic IR clone helpers (used by inlining, specialization, generics) |
| Dominators | [slang-ir-dominators.cpp](../../../../source/slang/slang-ir-dominators.cpp) | Dominator-tree construction; used by SSA construction, loop and SCCP passes |
| Util | [slang-ir-util.cpp](../../../../source/slang/slang-ir-util.cpp) | Common IR walking / mutation primitives, including the side-effect queries (`doesCalleeHaveSideEffect`, with an optional caller-owned memo that `simplifyIR` shares across its DCE invocations) |
| HLSL util | [slang-ir-util-hlsl.cpp](../../../../source/slang/slang-ir-util-hlsl.cpp) | Predicates over HLSL `Barrier` flag values (`isBarrierFlagGetterOp`, `getBarrierFlagValueInst`, the known memory / semantic flag masks), shared by HLSL barrier validation, l-value-cast lowering, and the HLSL emitter |
| Insts info | [slang-ir-insts-info.cpp](../../../../source/slang/slang-ir-insts-info.cpp) | Opcode tables, name lookup, and per-opcode metadata used by the pretty printer and passes that switch on opcode |
| Insts stable names | [slang-ir-insts-stable-names.cpp](../../../../source/slang/slang-ir-insts-stable-names.cpp) | Maps between opcode enum values and serialization-stable string names |
| Translate | [slang-ir-translate.cpp](../../../../source/slang/slang-ir-translate.cpp) | Shared translation dictionary and `TranslationContext` used by specialization and related analysis |

## What is not in this document

- The exact order in which passes run for any given target, with every
  gate resolved. The phase skeleton above is a coarse map; the
  per-target pages under
  [../target-pipelines/](../target-pipelines) diagram the resolved
  sequences, and `linkAndOptimizeIR` in
  [slang-emit.cpp](../../../../source/slang/slang-emit.cpp) remains the
  authoritative source.
- The IR design itself — see
  [../../../design/ir.md](../../../design/ir.md).
- The opcode catalog — see
  [../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).
- The code-emission stage that consumes the optimized IR — see
  [06-emit.md](06-emit.md).
