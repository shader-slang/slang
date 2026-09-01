# Prompt: target-pipelines/cuda.md

See [_common.md](_common.md) for the universal rules and the
**Target-pipeline page contract**.

## Target

Produce `docs/generated/design/target-pipelines/cuda.md` — the
ordered, control-flow-aware reference for the IR pass sequence that
runs when the CUDA target family is selected.

Audience: a compiler developer who needs to find where in the CUDA
codegen pipeline a particular pass runs, what condition selects it,
and how OptiX entry-point handling and the `synthesizeActiveMask`
pass interact with the rest of the pipeline.

## Scope

- **Targets covered**: `CodeGenTarget::CUDASource`,
  `CodeGenTarget::CUDAHeader`, `CodeGenTarget::PTX`. Inside
  `linkAndOptimizeIR` these three share switch arms with
  `isCUDATarget(targetRequest)`.
- **Includes downstream tools.** Show nvrtc (or the runtime CUDA
  compiler) as a `(downstream)` rectangle in Phase D for the
  `PTX` arm.
- **Filters out non-CUDA branches.** Passes gated on
  `isCPUTarget` (except where CPU and CUDA share an arm, e.g.
  `undoParameterCopy`), `isMetalTarget`, `isWGPUTarget`,
  `isSPIRV(target)`, `isKhronosTarget`, `target == HLSL`,
  `target == GLSL`, etc. are omitted.
- **Adjacent targets.** Add a short `## Adjacent targets` section
  noting that `CodeGenTarget::PyTorchCppBinding` shares the
  CUDA-adjacent passes
  (`generateHostFunctionsForAutoBindCuda`, `generatePyTorchCppBinding`,
  `handleAutoBindNames`, `lowerBuiltinTypesForKernelEntryPoints`,
  `removeTorchKernels`) but has its own emit arm
  (`TorchCppSourceEmitter`); OptiX (entry-point handling) and the
  host CPP / CPU / HostVM paths are also briefly cross-linked.
  These adjacent paths are **not** drawn in the CUDA diagrams.

## Phase decomposition

- **Phase A — Link and entry-point prep.** Same as SPIR-V up
  through the entry-point uniforms switch (line 1086-1115), where
  CUDA hits `collectOptiXEntryPointUniformParams` (line 1099 —
  `case CUDASource: case CUDAHeader:`). The torch / entry-point-
  removal switch (line 1117-1136) — CUDA hits the
  `default` arm and runs `removeTorchAndCUDAEntryPoints` does NOT
  apply; check the implementation. The CUDA arm of the
  `CUDASource / CUDAHeader / PyTorchCppBinding` switch fires.
  Roughly lines 927-1170 of
  [slang-emit.cpp](../../../../source/slang/slang-emit.cpp).
- **Phase B — Specialization and type legalization.** From the
  first `simplifyIR` through `specializeArrayParameters`. CUDA
  diverges from SPIR-V here:
  - `generateDerivativeWrappers` fires (line ~1198 CUDA/
    CUDAHeader/PyTorch arm).
  - `lowerCooperativeVectors` runs in the conditional CUDA arm
    (line ~1461).
  - `lowerCombinedTextureSamplers` does **not** run for CUDA. CUDA
    reaches the `default:` arm of that `switch`, and the arm falls
    through to the HLSL/Metal/WGSL cases only when
    `ArtifactDescUtil::isCpuLikeTarget(artifactDesc)` holds — which it
    does not for CUDA, because a CUDA artifact is `Kind::Source` with
    payload `CUDA` while `isCpuLikeTarget` accepts only payload `C` or
    `Cpp`, by exact equality rather than `isDerivedFrom`
    ([slang-artifact-desc-util.cpp](../../../../source/compiler-core/slang-artifact-desc-util.cpp)
    line 612). So CUDA simply breaks. An earlier revision of this prompt
    claimed the opposite and produced a critical finding on the page;
    more generally, **do not treat CUDA as CPU-like** — for the same
    reason, `performTypeInlining` and `checkGetStringHashInsts` both do
    run for CUDA.
  - `shouldLegalizeExistentialAndResourceTypes = false` is set
    for CUDA at line 2504 (in `emitEntryPointsSourceFromIR`),
    causing several Phase-B passes to skip:
    `inlineGlobalConstantsForLegalization`,
    `legalizeExistentialTypeLayout`, `legalizeResourceTypes`.
    Document each skipped pass with an annotation in the diagram.
- **Phase C — CUDA legalization, lowering, phi elimination.**
  CUDA-specific arms:
  - `lowerComInterfaces`, `generateDllImportFuncs`,
    `generateDllExportFuncs` SKIPPED (CPP-only, line ~1299-1301).
  - `lowerBuiltinTypesForKernelEntryPoints` for CUDA (line ~1320
    `case CUDASource:`).
  - `synthesizeActiveMask` (line ~1890).
  - `legalizeEntryPointVaryingParamsForCUDA` (line ~1962).
  - `floatNonUniformResourceIndex` SKIPPED (line ~1983-1984; CUDA
    is not D3D / Khronos / WGPU / Metal).
  - `legalizeImageSubscript` SKIPPED (CUDA arm absent).
  - `legalizeArrayReturnType` runs (line ~2151 `!isMetalTarget && !isSPIRV`).
  - `undoParameterCopy` (line ~2048 CPU/CUDA/Metal arm).
  - `eliminatePhis` with **default** options.
  - `applyVariableScopeCorrection` runs (line ~2324 `target !=
SPIRV`).
- **Phase D — CUDA emit and downstream tools.** From
  `emitEntryPointsSourceFromIR` through the `CUDASourceEmitter`
  (line ~2459), `simplifyForEmit`, and `sourceEmitter->emitModule`.
  For `PTX` nvrtc (or the runtime CUDA compiler) compiles the
  emitted source downstream.

## Diagram conventions

- Same as the SPIR-V page.
- Annotate skipped passes (e.g. due to
  `shouldLegalizeExistentialAndResourceTypes = false`) with
  `[skipped: <reason>]` rectangles, since the reader is likely
  hunting for "why didn't pass X run?".

## Conditional gates table

Group rows as in the SPIR-V page:

1. `requiredLoweringPassSet.*` flags.
2. `targetCompilerOptions.*` / option-set toggles.
3. Context predicates.
4. Simplification-mode predicates (note that
   `shouldLegalizeExistentialAndResourceTypes = false` is set by
   `emitEntryPointsSourceFromIR`, not by the option set, but
   belongs in this table for completeness).
5. CUDA-specific runtime predicates (`isCUDATarget`).

## Loops in the pipeline

- `linkAndOptimizeIR` itself contains no iterative passes for
  CUDA.
- Document the lack of a CUDA-specific legalization driver.

## Notable passes

Cover at least:

- `collectOptiXEntryPointUniformParams` — OptiX SBT-aware uniform
  handling; cite
  [slang-ir-optix-entry-point-uniforms.cpp](../../../../source/slang/slang-ir-optix-entry-point-uniforms.cpp).
- `synthesizeActiveMask` — CUDA wavefront active-mask synthesis
  for sub-group operations.
- `legalizeEntryPointVaryingParamsForCUDA` — varying-param
  legalization for CUDA kernel parameters.
- `lowerBuiltinTypesForKernelEntryPoints` — strips Slang-only
  shader types from kernel signatures.
- The effect of `shouldLegalizeExistentialAndResourceTypes =
false` — explain which Phase-B passes skip and why (CUDA's C++
  type system handles existentials and resources directly).
- The CUDA-immutable-load pass
  ([slang-ir-cuda-immutable-load.cpp](../../../../source/slang/slang-ir-cuda-immutable-load.cpp))
  if it appears in the pipeline; verify whether it is invoked
  from `linkAndOptimizeIR`.
- `eliminatePhis` with default options. Do **not** write that this
  contrasts with SPIR-V: the defaults in
  [slang-ir-eliminate-phis.h](../../../../source/slang/slang-ir-eliminate-phis.h)
  (lines 13-14) are `eliminateCompositeTypedPhiOnly = false` and
  `useRegisterAllocation = true`, and the direct-SPIR-V branch assigns
  those same two values, so there is no difference to contrast.

## Quality checklist (in addition to the universal one and the contract)

- [ ] Every `SLANG_PASS(...)` reachable for
      `CodeGenTarget::CUDASource` / `CUDAHeader` / `PTX` appears
      in exactly one phase table.
- [ ] `shouldLegalizeExistentialAndResourceTypes = false` impact
      is annotated in the diagrams.
- [ ] No SPIR-V / HLSL / Metal / WGSL / GLSL / pure-CPP passes in
      the diagrams (passes shared with CPU/Metal/CUDA must be
      included).
- [ ] PyTorch / OptiX / host paths covered only in the
      `## Adjacent targets` callout, not in the phase tables.
- [ ] The loops section is explicit about loop presence/absence.
