# Prompt: target-pipelines/metal.md

See [_common.md](_common.md) for the universal rules and the
**Target-pipeline page contract**.

## Target

Produce `docs/generated/design/target-pipelines/metal.md` — the
ordered, control-flow-aware reference for the IR pass sequence that
runs when the Metal target family is selected.

Audience: a compiler developer who needs to find where in the Metal
codegen pipeline a particular pass runs, what condition selects it,
and how `legalizeIRForMetal` and `MetalSourceEmitter` cooperate.

## Scope

- **Targets covered**: `CodeGenTarget::Metal`,
  `CodeGenTarget::MetalLib`, `CodeGenTarget::MetalLibAssembly`.
  Inside `linkAndOptimizeIR` the three are grouped together in
  every switch arm; the difference is the downstream tool that
  consumes the emitted Metal text.
- **Includes downstream tools.** Show the Metal compiler (the
  Apple `metal` tool) as a `(downstream)` rectangle in Phase D for
  the `MetalLib` / `MetalLibAssembly` arms.
- **Filters out non-Metal branches.** Passes gated on
  `isCPUTarget`, `isCUDATarget`, `isWGPUTarget`,
  `isSPIRV(target)`, `isKhronosTarget`, `target == HLSL`,
  `target == GLSL`, PyTorch, etc. are omitted.

## Phase decomposition

- **Phase A — Link and entry-point prep.** From `linkIR` through
  `moveEntryPointUniformParamsToGlobalScope` (Metal hits `default`
  arms). Roughly lines 927-1170 of
  [slang-emit.cpp](../../../../source/slang/slang-emit.cpp).
- **Phase B — Specialization and type legalization.** From the
  first `simplifyIR` through `specializeArrayParameters`. Roughly
  lines 1172-1714. Metal-specific gates: `lowerCooperativeVectors`
  runs (Metal is in the `default` arm at line ~1465),
  `lowerCombinedTextureSamplers` runs (line ~1517),
  `legalizeEmptyTypes` fires for Metal (line ~1553, only
  `isMetalTarget` arm), `wrapCBufferElementsForMetal` runs (line
  ~1735), `legalizeNonStructParameterToStructForHLSL` does NOT run
  (HLSL-only), `lowerGLSLShaderStorageBufferObjectsToStructuredBuffers`
  fires (Metal is non-Khronos).
- **Phase C — Metal legalization, lowering, phi elimination.**
  Anchored at `legalizeIRForMetal` (line ~1942 of
  `slang-emit.cpp`, defined at line 250 of
  [slang-ir-metal-legalize.cpp](../../../../source/slang/slang-ir-metal-legalize.cpp)).
  Other Metal-specific passes:
  `legalizeEntryPointVaryingParamsForMetal` (called from inside
  `legalizeIRForMetal`),
  `specializeAddressSpaceForMetal` (line ~2187),
  `undoParameterCopy` (line ~2048 CPU/CUDA/Metal arm),
  `floatNonUniformResourceIndex` runs (line ~1980 — Metal is in
  the `!isSPIRV` arm), `legalizeArrayReturnType` SKIPPED (line
  ~2150 — Metal is excluded), `legalizeUniformBufferLoad`
  SKIPPED (Khronos / HLSL arm),
  `eliminatePhis` with **default** options,
  `applyVariableScopeCorrection` (line ~2324 `target != SPIRV`).
- **Phase D — Metal emit and downstream tools.** From
  `emitEntryPointsSourceFromIR` through the `MetalSourceEmitter`
  (line ~2464), `simplifyForEmit`, and `sourceEmitter->emitModule`.
  Then the downstream compile chain: for `Metal` the artifact is
  the source text; for `MetalLib` / `MetalLibAssembly` Apple's
  `metal` compiler is invoked downstream.

## Diagram conventions

- Same as the SPIR-V page; per-phase `flowchart TD` with a node
  per `SLANG_PASS`.
- `legalizeIRForMetal` is treated as a single node in Phase C with
  a "## Notable passes" callout exploring its internals
  (varying-param legalization, struct legalization, etc.).

## Conditional gates table

Group rows as in the SPIR-V page:

1. `requiredLoweringPassSet.*` flags — only those that gate a
   Metal pass.
2. `targetCompilerOptions.*` / option-set toggles.
3. Context predicates.
4. Simplification-mode predicates.
5. Metal-specific runtime predicates if any.

## Loops in the pipeline

- `linkAndOptimizeIR` itself contains no iterative passes for
  Metal.
- Document any loops inside `legalizeIRForMetal` if they exist
  (read the implementation to confirm; it appears to be a
  single-pass driver).
- If no loops exist, state so explicitly.

## Notable passes

Cover at least:

- `legalizeIRForMetal` — the Metal legalization driver, what it
  contains (varying-param legalization, struct legalization,
  fix-ups for `[[buffer(N)]]` / `[[texture(N)]]` attributes).
- `specializeAddressSpaceForMetal` — Metal-specific address-space
  propagation; contrast with SPIR-V which defers this to
  `legalizeIRForSPIRV`.
- `wrapCBufferElementsForMetal` — works around Metal's restrictions
  on constant-buffer element types.
- `undoParameterCopy` — undoes parameter copies for CPU-like
  targets; Metal is in the CPU/CUDA/Metal arm because Apple's
  `metal` compiler benefits from the same parameter shape.
- `legalizeEmptyTypes` — Metal-only branch (the SPIR-V path has a
  separate `legalizeEmptyTypes` invocation later).
- `eliminatePhis` with default options — contrast with SPIR-V.
- The downstream Apple `metal` compiler — invoked only for
  `MetalLib` / `MetalLibAssembly`.

## Quality checklist (in addition to the universal one and the contract)

- [ ] Every `SLANG_PASS(...)` reachable for `CodeGenTarget::Metal`
      / `MetalLib` / `MetalLibAssembly` appears in exactly one
      phase table.
- [ ] Phase D explicitly distinguishes the source-only arm
      (`Metal`) from the binary arms (`MetalLib*`).
- [ ] No SPIR-V / HLSL / WGSL / CUDA / GLSL / CPU passes in the
      diagrams.
- [ ] The loops section is explicit about loop presence/absence.
