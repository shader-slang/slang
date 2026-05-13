# Prompt: target-pipelines/spirv.md

See [_common.md](_common.md) for the universal rules and the
**Target-pipeline page contract**.

## Target

Produce `docs/llm-generated/target-pipelines/spirv.md` — the ordered,
control-flow-aware reference for the IR pass sequence that runs when
the SPIR-V target is selected via the direct-emit path
(`shouldEmitSPIRVDirectly() == true`).

Audience: a compiler developer who needs to find where in the SPIR-V
codegen pipeline a particular pass runs, what condition selects it,
and which iterative passes loop until fixed point.

## Scope

- **Direct-emit only.** Cover the path triggered by
  `CodeGenTarget::SPIRV` with `shouldEmitSPIRVDirectly() == true`.
  The legacy via-GLSL path
  (`isKhronosTarget && !emitSpirvDirectly`) is a one-paragraph
  callout pointing to the divergence point at
  `legalizeModesOfNonCopyableOpaqueTypedParamsForGLSL` (around line
  2229 of `slang-emit.cpp`); a full via-GLSL CFG belongs to the
  future GLSL target page.
- **Includes binary post-processing.** Show spirv-link and spirv-val
  as `(downstream)` rectangles in Phase D. The in-source
  `optimizeSPIRV` call is currently in `#if 0`; mark its node
  `[disabled]`.
- **Filters out non-SPIR-V branches.** Passes gated on
  `isCPUTarget`, `isMetalTarget`, `isCUDATarget`, `isWGPUTarget`,
  `target == HLSL`, `target == GLSL` (when SPIR-V is on a sibling
  arm), `target == CodeGenTarget::PyTorchCppBinding`, etc. are
  omitted from diagrams and tables. The intro acknowledges the
  filter.

## Phase decomposition

Four phases, each its own diagram + ordered table:

- **Phase A — Link and entry-point prep.** From `linkIR` through
  the entry-point uniforms split (`moveEntryPointUniformParamsToGlobalScope`
  for SPIR-V's `default` arm). Roughly lines 927-1170 of
  [slang-emit.cpp](../../../source/slang/slang-emit.cpp).
- **Phase B — Specialization and type legalization.** From the
  first `simplifyIR` through `specializeArrayParameters` /
  `checkStaticAssert`. Roughly lines 1172-1714.
- **Phase C — SPIR-V legalization, lowering, phi elimination.**
  From `legalizeByteAddressBufferOps` through `simplifyNonSSAIR`
  and `collectMetadata`. Roughly lines 1745-2360.
- **Phase D — IR-to-SPIR-V emit, simplification loop, downstream
  tools.** From `emitSPIRVForEntryPointsDirectly` (line ~3075) into
  `linkAndOptimizeIR`'s caller-side wrapping, then
  `createArtifactFromIR` (line ~2910) which invokes
  `emitSPIRVFromIR` (line ~11104 of
  [slang-emit-spirv.cpp](../../../source/slang/slang-emit-spirv.cpp))
  including `legalizeIRForSPIRV` (line 2935 of
  [slang-ir-spirv-legalize.cpp](../../../source/slang/slang-ir-spirv-legalize.cpp))
  with `simplifyIRForSpirvLegalization` (the iterative loop), the
  forward-declared-pointer fixup loop, and the conditional
  spirv-link / spirv-val downstream chain.

## Diagram conventions (in addition to the contract)

- Use `flowchart TD` for every phase diagram.
- Group long unconditional segments only when the diagram becomes
  illegible; default is one node per `SLANG_PASS`.
- Conditional diamonds must show the gate expression verbatim,
  abbreviated only when the expression is very long (e.g.
  `targetCompilerOptions.shouldEmitSeparateDebugInfo()` is fine;
  `targetProgram->getOptionSet().getBoolOption(CompilerOptionName::PreserveParameters)`
  may be abbreviated to `getBoolOption(PreserveParameters)`).
- Use `subgraph` only when it materially aids reading (e.g. the
  inner per-function loop inside
  `simplifyIRForSpirvLegalization`).

## Conditional gates table

Group rows by gate kind, in this order:

1. `requiredLoweringPassSet.*` flags
   (`debugInfo`, `glslSSBO`, `globalVaryingVar`,
   `resolveVaryingInputRef`, `bindExistential`, `coverageTracing`,
   `enumType`, `derivativePyBindWrapper`, `autodiff`,
   `higherOrderFunc`, `matrixSwizzleStore`, `resultType`,
   `conditionalType`, `optionalType`, `nonVectorCompositeSelect`,
   `missingReturn`, `generics`, `reinterpret`, `specializeStageSwitch`,
   `combinedTextureSamplers`, `dynamicResource`, `dynamicResourceHeap`,
   `existentialTypeLayout`, `byteAddressBuffer`, `bindingQuery`,
   `meshOutput`, `bitcast`).
2. `targetCompilerOptions.*` / option-set toggles
   (`getDebugInfoLevel() == DebugInfoLevel::None`,
   `shouldEmitSeparateDebugInfo`,
   `shouldRunNonEssentialValidation`,
   `getBoolOption(ValidateUniformity)`,
   `getBoolOption(PreserveParameters)`,
   `getBoolOption(EnableExperimentalPasses)`,
   `getBoolOption(VulkanEmitReflection)`,
   `getBoolOption(VulkanInvertY)`,
   `getBoolOption(VulkanUseDxPositionW)`,
   `getBoolOption(EmbedDownstreamIR)`,
   `shouldPerformMinimumOptimizations`).
3. Context predicates
   (`!codeGenContext->isSpecializationDisabled()`,
   `codeGenContext->shouldReportCheckpointIntermediates()`,
   `codeGenContext->shouldTrackLiveness()`,
   `codeGenContext->removeAvailableInDownstreamIR`,
   `codeGenContext->shouldSkipDownstreamLinking()`,
   `shouldRunSPIRVValidation(codeGenContext)`,
   `spirvFiles.getCount() > 1`).
4. Simplification-mode predicates
   (`!fastIRSimplificationOptions.minimalOptimization`,
   `fastIRSimplificationOptions.minimalOptimization`).
5. SPIR-V-specific runtime constants
   (`context->shouldEmitDiscardAsDemote()`,
   `context->isSpirv16OrLater()`).

## Loops in the pipeline

Cover exactly two loops:

- The outer `while (changed && iterationCounter < kMaxIterations)`
  (bound 8) plus inner `while (funcChanged && funcIterationCount <
  kMaxFuncIterations)` (bound 16) in
  `simplifyIRForSpirvLegalization` (line 2709 of
  [slang-ir-spirv-legalize.cpp](../../../source/slang/slang-ir-spirv-legalize.cpp)).
- The `do { ... } while
  (m_forwardDeclaredPointers.getCount() != 0)` loop in
  `emitSPIRVFromIR` (around line 11265 of
  [slang-emit-spirv.cpp](../../../source/slang/slang-emit-spirv.cpp)).

Note explicitly that no other loops execute IR passes for SPIR-V.

## Notable passes

Cover at least:

- `legalizeIRForSPIRV` — its three internal stages
  (`legalizeSPIRV`, `simplifyIRForSpirvLegalization`,
  `removeUnreachableCodeAfterDiscardForOpKill` +
  `eliminateDeadCode` + `buildEntryPointReferenceGraph` +
  `insertFragmentShaderInterlock`).
- `eliminatePhis` — SPIR-V-specific
  `phiEliminationOptions.eliminateCompositeTypedPhiOnly = false`
  and `useRegisterAllocation = true` (line ~2267 of
  `slang-emit.cpp`).
- `specializeFuncsForBufferLoadArgs` — invoked twice: once early
  in Phase B (line ~1701) and again SPIR-V-specifically after
  `lowerBufferElementTypeToStorageType` (line ~2203); rationale
  tied to SPIR-V rule 2.16.1 (`VariablePointer` not declared).
- Address-space propagation — SPIR-V is the only target that
  defers address-space specialization to its legalization pass
  (note the GLSL / Metal / WGSL arms run
  `specializeAddressSpace*` in `linkAndOptimizeIR`; the SPIR-V
  arm intentionally does not).
- `legalizeEntryPointsForGLSL` — runs for SPIR-V too (line
  ~1928); the name reflects history rather than current scope.
- `removeRawDefaultConstructors` — only runs for SPIR-V when
  `shouldEmitSPIRVDirectly()`.
- `transformParamsToConstRef` — invoked for SPIR-V in the
  `case CodeGenTarget::SPIRV:` arm of the
  `moveGlobalVarInitializationToEntryPoints` switch.
- Post-emit chain: `compiler->link` (spirv-link, multi-module
  link, gated on `spirvFiles.getCount() > 1`),
  `compiler->validate` (spirv-val, gated on
  `shouldRunSPIRVValidation`), the in-source `optimizeSPIRV`
  call disabled by `#if 0` (mention but mark `[disabled]`).

## Quality checklist (in addition to the universal one and the contract)

- [ ] Every `SLANG_PASS(...)` reachable for `CodeGenTarget::SPIRV`
      + `shouldEmitSPIRVDirectly() == true` appears in exactly one
      phase table.
- [ ] Each diamond's gate label matches the source verbatim
      (allowing minor abbreviation noted above).
- [ ] Loops show both the bound and the fixed-point condition.
- [ ] No HLSL/GLSL/CUDA/Metal/WGSL/CPU/PyTorch-only passes in the
      diagrams.
- [ ] Phase A starts at `linkIR` and ends before the first
      `simplifyIR` in Phase B.
- [ ] Phase D ends at the artifact handed back from
      `emitSPIRVForEntryPointsDirectly`.
