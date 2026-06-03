# Prompt: target-pipelines/wgsl.md

See [_common.md](_common.md) for the universal rules and the
**Target-pipeline page contract**.

## Target

Produce `docs/generated/design/target-pipelines/wgsl.md` — the
ordered, control-flow-aware reference for the IR pass sequence that
runs when the WGSL target family is selected.

Audience: a compiler developer who needs to find where in the WGSL
codegen pipeline a particular pass runs, what condition selects it,
and how `legalizeIRForWGSL` and `WGSLSourceEmitter` cooperate.

## Scope

- **Targets covered**: `CodeGenTarget::WGSL`,
  `CodeGenTarget::WGSLSPIRV`, `CodeGenTarget::WGSLSPIRVAssembly`.
  Inside `linkAndOptimizeIR` the three appear together in every
  switch arm (`isWGPUTarget(...)`).
- **Includes downstream tools.** Show the WGSL → SPIR-V translator
  (Tint, the Dawn / Chromium WGSL implementation) as a
  `(downstream)` rectangle in Phase D for the
  `WGSLSPIRV` / `WGSLSPIRVAssembly` arms.
- **Filters out non-WGSL branches.** Passes gated on
  `isCPUTarget`, `isCUDATarget`, `isMetalTarget`,
  `isSPIRV(target)`, `isKhronosTarget`, `target == HLSL`,
  `target == GLSL`, PyTorch, etc. are omitted.

## Phase decomposition

- **Phase A — Link and entry-point prep.** From `linkIR` through
  `moveEntryPointUniformParamsToGlobalScope` (WGSL hits `default`
  arms throughout). Roughly lines 927-1170 of
  [slang-emit.cpp](../../../../source/slang/slang-emit.cpp).
- **Phase B — Specialization and type legalization.** From the
  first `simplifyIR` through `specializeArrayParameters`. Roughly
  lines 1172-1714. WGSL-specific gates: `lowerCooperativeVectors`
  runs (WGSL is in the `default` arm at line ~1465),
  `lowerCombinedTextureSamplers` runs (line ~1517),
  `lowerGLSLShaderStorageBufferObjectsToStructuredBuffers` fires
  (WGSL is non-Khronos), `legalizeArrayReturnType` runs (line
  ~2151 `!isMetalTarget && !isSPIRV`).
- **Phase C — WGSL legalization, lowering, phi elimination.**
  Anchored at `legalizeIRForWGSL` (line ~1970 of `slang-emit.cpp`,
  defined at line 215 of
  [slang-ir-wgsl-legalize.cpp](../../../../source/slang/slang-ir-wgsl-legalize.cpp)).
  Other WGSL-specific passes:
  `legalizeEntryPointVaryingParamsForWGSL` (called from
  `legalizeIRForWGSL`),
  `specializeAddressSpaceForWGSL` (line ~2191),
  `floatNonUniformResourceIndex` runs (line ~1980 / 1983-1984 also
  covers `isWGPUTarget`),
  `legalizeLogicalAndOr` runs (`isD3DTarget || isKhronosTarget ||
  isWGPUTarget || isMetalTarget`),
  `eliminatePhis` with **default** options,
  `applyVariableScopeCorrection` runs (line ~2324 `target !=
  SPIRV`).
- **Phase D — WGSL emit and downstream tools.** From
  `emitEntryPointsSourceFromIR` through the `WGSLSourceEmitter`
  (line ~2469), `simplifyForEmit`, and `sourceEmitter->emitModule`.
  Then the downstream compile chain: for `WGSL` the artifact is
  the source text; for `WGSLSPIRV` / `WGSLSPIRVAssembly` Tint is
  invoked downstream to translate WGSL → SPIR-V.

## Diagram conventions

- Same as the SPIR-V page.
- `legalizeIRForWGSL` is a single node in Phase C; its sub-passes
  are expanded in the "## Notable passes" section.

## Conditional gates table

Group rows as in the SPIR-V page:

1. `requiredLoweringPassSet.*` flags.
2. `targetCompilerOptions.*` / option-set toggles.
3. Context predicates.
4. Simplification-mode predicates.
5. WGSL-specific runtime predicates (e.g. `isWGPUTarget`).

## Loops in the pipeline

- `linkAndOptimizeIR` itself contains no iterative passes for WGSL.
- Document any loops inside `legalizeIRForWGSL` if they exist
  (read the implementation; if absent, say so).

## Notable passes

Cover at least:

- `legalizeIRForWGSL` — the WGSL legalization driver and what it
  contains (varying-param legalization, struct unpacking,
  address-space fix-ups).
- `specializeAddressSpaceForWGSL` — WGSL has distinct storage and
  uniform address spaces that the IR must mark up before emit.
- `legalizeLogicalAndOr` — WGSL is in the Khronos / D3D / WGPU /
  Metal arm because all four targets disallow C-style
  short-circuit eval of logical-and/or on vectors.
- `floatNonUniformResourceIndex` — WGSL is one of the `!isSPIRV`
  arms; explain why WGSL needs a textual non-uniform-resource-index
  representation.
- `eliminatePhis` with default options — contrast with SPIR-V.
- The downstream Tint translator — invoked only for
  `WGSLSPIRV` / `WGSLSPIRVAssembly`. Validation is delegated.

## Quality checklist (in addition to the universal one and the contract)

- [ ] Every `SLANG_PASS(...)` reachable for `CodeGenTarget::WGSL`
      / `WGSLSPIRV` / `WGSLSPIRVAssembly` appears in exactly one
      phase table.
- [ ] Phase D distinguishes the source-only arm (`WGSL`) from the
      Tint-via-SPIR-V arms.
- [ ] No SPIR-V / HLSL / Metal / CUDA / GLSL / CPU passes in the
      diagrams.
- [ ] The loops section is explicit about loop presence/absence.
