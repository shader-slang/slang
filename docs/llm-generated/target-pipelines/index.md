---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-13T16:30:00+00:00
source_commit: 07b911645ab895c59decdcc25c6c56ee245833af
watched_paths_digest: 7532e7bf149144a93210a030f872f283de9ddb9dfafdf5293db3ee653732d2ad
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Target Pipelines

This page is a navigation hub for the per-target pipeline pages
in `target-pipelines/`. Each peer page documents one target's
ordered IR-pass and downstream-tool sequence as a four-phase
control-flow-graph view of the shared orchestrator
`linkAndOptimizeIR` (line ~892 of
[../../../source/slang/slang-emit.cpp](../../../source/slang/slang-emit.cpp)).
For an unordered, topical catalog of every IR pass — grouped by
category rather than by execution order — see
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).

## Pages

- [spirv.md](spirv.md) — SPIR-V via the direct-emit path
  (`emitSPIRVForEntryPointsDirectly`), plus the spirv-link /
  spirv-val / spirv-opt downstream chain.
- [hlsl.md](hlsl.md) — HLSL source with DXC (DXIL) and fxc
  (DXBytecode) downstream.
- [metal.md](metal.md) — Metal source with the Apple `metal`
  compiler downstream for `MetalLib` / `MetalLibAssembly`.
- [wgsl.md](wgsl.md) — WGSL source with Tint (WGSL → SPIR-V)
  downstream for `WGSLSPIRV` / `WGSLSPIRVAssembly`.
- [cuda.md](cuda.md) — CUDA C++ source / header with nvrtc (PTX)
  downstream, plus an Adjacent targets section that briefly
  cross-links PyTorch / OptiX / host-CPP paths.

## Shared shape

All five pages obey the **Target-pipeline page contract** in
[../_meta/prompts/_common.md](../_meta/prompts/_common.md) and
decompose their target's invocation of `linkAndOptimizeIR` into
four phases:

- **Phase A — Link and entry-point prep.** From `linkIR` through
  the per-target entry-point-uniform handling
  (`collectEntryPointUniformParams` /
  `moveEntryPointUniformParamsToGlobalScope` /
  `collectOptiXEntryPointUniformParams`) up to the first
  `simplifyIR`. Roughly lines 927-1170.
- **Phase B — Specialization and type legalization.** From the
  first `simplifyIR` through `specializeArrayParameters` /
  `checkStaticAssert`. Roughly lines 1172-1714. The big
  cross-target divergences (existential and resource-type
  legalization, cooperative-vector lowering, target-specific
  wrappers) live here.
- **Phase C — Target legalization, lowering, phi elimination.**
  From `legalizeByteAddressBufferOps` through `simplifyNonSSAIR`,
  `applyVariableScopeCorrection`, and `collectMetadata`. Roughly
  lines 1745-2360. The target-specific legalization driver
  (`legalizeIRForSPIRV` / `legalizeIRForMetal` /
  `legalizeIRForWGSL`) lives here, where one exists.
- **Phase D — Emit and downstream tools.** From
  `emitEntryPointsSourceFromIR` (line ~2365) through the
  per-target `SourceEmitter` and `createArtifactFromIR` (line
  ~2910), then into the downstream compiler chain
  (spirv-link / DXC / Apple `metal` / Tint / nvrtc).

Reading any single per-target page yields the **filtered** view
of `linkAndOptimizeIR` — passes that fire only for sibling
targets are omitted from the diagrams and tables. The shared
orchestrator runs unconditionally for every target; what differs
is which switch arm each target lands in.

## Cross-target comparison

| Target | CodeGenTarget enum values | Phase C entry | Phase D emitter | Downstream tools | Loops |
| --- | --- | --- | --- | --- | --- |
| SPIR-V | `SPIRV`, `SPIRVAssembly` | `legalizeIRForSPIRV` ([slang-ir-spirv-legalize.cpp](../../../source/slang/slang-ir-spirv-legalize.cpp)) | `emitSPIRVForEntryPointsDirectly` ([slang-emit-spirv.cpp](../../../source/slang/slang-emit-spirv.cpp)) | spirv-link, spirv-val, spirv-opt | **Yes** — `simplifyIRForSpirvLegalization` (outer 8 × inner 16); forward-declared-pointer fixup loop in `emitSPIRVFromIR`. |
| HLSL | `HLSL` (plus downstream `DXIL`, `DXBytecode`, and their `*Assembly` variants) | (no single driver — per-pass HLSL arms: `wrapStructuredBuffersOfMatrices`, `legalizeNonStructParameterToStructForHLSL`, `legalizeNonVectorCompositeSelect`, `legalizeEmptyRayPayloadsForHLSL`, `legalizeUniformBufferLoad`, `applyVariableScopeCorrection`) | `HLSLSourceEmitter` ([slang-emit-hlsl.cpp](../../../source/slang/slang-emit-hlsl.cpp)) | DXC (for `DXIL*`), fxc (for `DXBytecode*`) | **No** loops in `linkAndOptimizeIR`. |
| Metal | `Metal`, `MetalLib`, `MetalLibAssembly` | `legalizeIRForMetal` ([slang-ir-metal-legalize.cpp](../../../source/slang/slang-ir-metal-legalize.cpp)) | `MetalSourceEmitter` ([slang-emit-metal.cpp](../../../source/slang/slang-emit-metal.cpp)) | Apple `metal` compiler (for `MetalLib*`) | **No** loops in `linkAndOptimizeIR`; `legalizeIRForMetal` is single-pass. |
| WGSL | `WGSL`, `WGSLSPIRV`, `WGSLSPIRVAssembly` | `legalizeIRForWGSL` ([slang-ir-wgsl-legalize.cpp](../../../source/slang/slang-ir-wgsl-legalize.cpp)) | `WGSLSourceEmitter` ([slang-emit-wgsl.cpp](../../../source/slang/slang-emit-wgsl.cpp)) | Tint (for `WGSLSPIRV*`) | **No** loops in `linkAndOptimizeIR`; `legalizeIRForWGSL` is single-pass. |
| CUDA | `CUDASource`, `CUDAHeader`, `PTX` | (no single driver — per-pass CUDA arms: `synthesizeActiveMask`, `legalizeEntryPointVaryingParamsForCUDA`, `lowerImmutableBufferLoadForCUDA`, plus shared `undoParameterCopy` / `transformParamsToConstRef` with CPU and Metal) | `CUDASourceEmitter` ([slang-emit-cuda.cpp](../../../source/slang/slang-emit-cuda.cpp), inheriting from `CPPSourceEmitter`) | nvrtc / runtime CUDA compiler (for `PTX`) | **No** loops in `linkAndOptimizeIR`. |

Several conditional gates apply across multiple targets — most
notably `eliminatePhis` runs with **register-allocation enabled**
only for SPIR-V (when `isKhronosTarget && emitSpirvDirectly`), and
with **default options** for HLSL, Metal, WGSL, and CUDA. SPIR-V
is also the only target that defers address-space propagation
into its legalizer; Metal and WGSL run
`specializeAddressSpaceForMetal` / `specializeAddressSpaceForWGSL`
inside `linkAndOptimizeIR`.

## Filtering rules

Each per-target page filters out switch arms gated on a sibling
target (`isSPIRV`, `isMetalTarget`, `isWGPUTarget`, `isCUDATarget`,
`isD3DTarget`, `isKhronosTarget`, `target == HLSL`,
`target == GLSL`, `target == CodeGenTarget::PyTorchCppBinding`,
the CPU / Host / LLVM variants, etc.). A glance at one page does
**not** show the global ordering of `linkAndOptimizeIR`; it shows
only the passes reachable for that target. Where two targets
share an arm (for example, CUDA, Metal, and CPU all hit the
`undoParameterCopy` arm at line ~2041), each page that lists the
pass also documents the shared arm in its prose.

For a single, unfiltered view of every pass — independent of
target — read
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) and
the source of `linkAndOptimizeIR` directly. For the unordered
topical catalog see
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md);
for the cross-cutting per-target option / capability discussion
see [../cross-cutting/targets.md](../cross-cutting/targets.md).

## See also

- [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) —
  AST → IR lowering.
- [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) —
  unordered topical catalog of IR passes.
- [../pipeline/06-emit.md](../pipeline/06-emit.md) — backend
  emit overview.
- [../cross-cutting/targets.md](../cross-cutting/targets.md) —
  per-target options, capability sets, and target predicates.
- [../ir-reference/index.md](../ir-reference/index.md) —
  per-opcode catalog.
