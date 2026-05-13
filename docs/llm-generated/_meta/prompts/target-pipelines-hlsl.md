# Prompt: target-pipelines/hlsl.md

See [_common.md](_common.md) for the universal rules and the
**Target-pipeline page contract**.

## Target

Produce `docs/llm-generated/target-pipelines/hlsl.md` — the
ordered, control-flow-aware reference for the IR pass sequence that
runs when the HLSL target family is selected.

Audience: a compiler developer who needs to find where in the HLSL
codegen pipeline a particular pass runs, what condition selects it,
and how the emitted HLSL source flows into DXC (DXIL) or fxc
(DXBytecode).

## Scope

- **Targets covered**: `CodeGenTarget::HLSL` plus the downstream
  binary sub-targets `DXIL` (via DXC) and `DXBytecode` (via fxc).
  Inside `linkAndOptimizeIR` only `CodeGenTarget::HLSL` ever appears
  as a switch arm — the downstream targets ride the same IR path
  and diverge only in `createArtifactFromIR` / the downstream
  compiler dispatch.
- **Includes downstream tools.** Show DXC and fxc as `(downstream)`
  rectangles in Phase D. spirv-link and spirv-val do not run.
- **Filters out non-HLSL branches.** Passes gated on
  `isCPUTarget`, `isCUDATarget`, `isMetalTarget`, `isWGPUTarget`,
  `isSPIRV(target)`, `isKhronosTarget`, `target == CodeGenTarget::PyTorchCppBinding`,
  GLSL-only arms, etc. are omitted.

## Phase decomposition

- **Phase A — Link and entry-point prep.** From `linkIR` through
  `moveEntryPointUniformParamsToGlobalScope` (HLSL hits `default`
  arms throughout). Roughly lines 927-1170 of
  [slang-emit.cpp](../../../source/slang/slang-emit.cpp).
- **Phase B — Specialization and type legalization.** From the
  first `simplifyIR` through `specializeArrayParameters`. Roughly
  lines 1172-1714. HLSL-specific gates fire here:
  `legalizeNonVectorCompositeSelect` (line ~1285,
  `case CodeGenTarget::HLSL`), `lowerCooperativeVectors` SKIPPED
  (HLSL is in the `case CodeGenTarget::HLSL` short-circuit at
  line 1455), `lowerCombinedTextureSamplers` runs (line ~1517).
- **Phase C — HLSL legalization, lowering, phi elimination.** From
  `legalizeByteAddressBufferOps` through `simplifyNonSSAIR`. HLSL
  uses `scalarizeVectorLoadStore = true` for byte-address ops.
  Other HLSL-specific arms:
  `legalizeNonStructParameterToStructForHLSL` (line ~1606),
  `wrapStructuredBuffersOfMatrices` (line ~1727),
  `legalizeEmptyRayPayloadsForHLSL` (HLSL arm of the
  `legalizeEmptyRayPayloads*` switch),
  `floatNonUniformResourceIndex` (line ~1980 `!isSPIRV`),
  `legalizeArrayReturnType` (line ~2151 `!isMetalTarget && !isSPIRV`),
  `legalizeUniformBufferLoad` (`isKhronosTarget || target == HLSL`),
  `eliminatePhis` with **default** options (no register
  allocation),
  `applyVariableScopeCorrection` (line ~2324 `target != SPIRV`).
- **Phase D — HLSL emit and downstream tools.** From
  `emitEntryPointsSourceFromIR` (line ~2365) through the
  `HLSLSourceEmitter` (`new HLSLSourceEmitter(desc)` at line 2454),
  `simplifyForEmit` (line 2513), and `sourceEmitter->emitModule`
  (line 2521). Then the downstream compile chain: when the
  requested `CodeGenTarget` is `DXIL`, DXC compiles the emitted
  HLSL; when it is `DXBytecode`, fxc does. Validation is delegated
  to the downstream compiler.

## Diagram conventions (in addition to the contract)

- Use `flowchart TD` per phase.
- Default to one node per `SLANG_PASS`; collapse only contiguous
  unconditional runs that exceed ~8 nodes.
- Mention the historical name `legalizeEntryPointsForGLSL`
  explicitly when it does *not* run for HLSL (it is in the
  `case GLSL / SPIRV / SPIRVAssembly` arm, so HLSL falls into the
  `default` arm and skips it).

## Conditional gates table

Group rows as in the SPIR-V page:

1. `requiredLoweringPassSet.*` flags — list only those that gate a
   pass for HLSL (most do; the GLSL-SSBO flag does *not* fire for
   HLSL because the surrounding `!isKhronosTarget && reqSet.glslSSBO`
   gate is HLSL-reachable, so HLSL does run the
   `lowerGLSLShaderStorageBufferObjectsToStructuredBuffers` pass).
2. `targetCompilerOptions.*` / option-set toggles.
3. Context predicates.
4. Simplification-mode predicates.
5. HLSL-specific runtime predicates if any (e.g.
   `targetProgram->getTargetFloatingPointMode()`,
   `getHlslToVulkanLayoutOptions`).

## Loops in the pipeline

- `linkAndOptimizeIR` itself contains no iterative passes for the
  HLSL path.
- HLSL has no in-source legalization loop equivalent to
  `simplifyIRForSpirvLegalization`. Document this explicitly so
  the reader does not look for one.
- Downstream DXC has its own optimization loops; those are out
  of scope.

## Notable passes

Cover at least:

- `legalizeNonStructParameterToStructForHLSL` — why HLSL wraps
  non-struct entry-point params (DXC requirement).
- `wrapStructuredBuffersOfMatrices` — works around DXC's handling
  of structured buffers whose element type is a matrix.
- `legalizeEmptyRayPayloadsForHLSL` — DXC requirement for
  non-empty payloads.
- `legalizeByteAddressBufferOps` with `scalarizeVectorLoadStore =
  true` — different from SPIR-V which keeps vectors.
- `lowerGLSLShaderStorageBufferObjectsToStructuredBuffers` — only
  fires for non-Khronos targets, so it runs for HLSL but **not**
  for SPIR-V / GLSL.
- `eliminatePhis` with default options — contrast with SPIR-V
  which sets `useRegisterAllocation = true`.
- `applyVariableScopeCorrection` — emits scope-correcting copies
  required by HLSL but not SPIR-V.
- The downstream DXC / fxc chain: DXC is the default for shader
  model 6.0+; fxc is the legacy path for `DXBytecode`. Validation
  is delegated.

## Quality checklist (in addition to the universal one and the contract)

- [ ] Every `SLANG_PASS(...)` reachable for `CodeGenTarget::HLSL`
      appears in exactly one phase table.
- [ ] Phase D covers both DXC (DXIL) and fxc (DXBytecode) arms.
- [ ] No SPIR-V / Metal / WGSL / CUDA / GLSL / CPU passes in the
      diagrams.
- [ ] The loops section explicitly states "no iterative passes in
      `linkAndOptimizeIR` for HLSL".
