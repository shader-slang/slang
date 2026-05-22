---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:58:33Z
source_commit: 30ae111120515b7406aa6f427a4eaaa28a0903d8
watched_paths_digest: f48c602efa824ba1c41f8f80a982d156007c51018b730902f0853d28ce4f3da0
source_doc: docs/llm-generated/target-pipelines/index.md
source_doc_digest: 9ce9a410a0bad8427f3bd6458dd60bf8d59797430a9668ce295ca0b62a01feea
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for target-pipelines/index

## Intent
Tests verify the **cross-cutting / multi-target dispatcher claims**
made by
[`docs/llm-generated/target-pipelines/index.md`](../../../docs/llm-generated/target-pipelines/index.md):
the subtree's role as the per-target view of the shared
`linkAndOptimizeIR` orchestrator, the shared four-phase shape that
every target page obeys, the cross-target comparison table
(distinct Phase D emitters per target, per-target phi-elim gating,
the spread of `CodeGenTarget` enum values across source-text vs
downstream-binary outputs), and the no-downstream-tool boundary
that separates the five text-emit targets from their downstream
counterparts.

The bundle is intentionally small (5 tests). The index doc is a
navigation hub; the per-target legalization details belong to the
five peer bundles under `tests-agentic/target-pipelines/`, and we
route them there via `## Untested claims` rather than duplicating.

Strategy: one observation per cross-cutting claim, using the
lightest set of `//TEST:SIMPLE(-target ...)` directives that make
the multi-target dispatcher visible. Each test in this bundle runs
multiple `//TEST` directives in a single file with per-target
`CHECK_<TARGET>` prefixes so a single source file actually exercises
the dispatcher.


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| Phi-elimination is gated per-target (register-allocation only for SPIR-V; default options for HLSL, Metal, WGSL, CUDA); a source with merging control flow still emits successfully on every backend. | functional | [#cross-target-comparison](../../../docs/llm-generated/target-pipelines/index.md#cross-target-comparison) | [`phi-elim-cross-target-success.slang`](phi-elim-cross-target-success.slang) |
| The cross-target table maps each target to a distinct Phase D emitter; the same Slang source therefore produces distinctively different surface syntax on each backend. | functional | [#cross-target-comparison](../../../docs/llm-generated/target-pipelines/index.md#cross-target-comparison) | [`same-source-distinct-emitters.slang`](same-source-distinct-emitters.slang) |
| The text-emit CodeGenTarget rows in the cross-target table (hlsl, metal, wgsl, cuda source, spirv-asm) reach Phase D source without invoking downstream tools. | functional | [#cross-target-comparison](../../../docs/llm-generated/target-pipelines/index.md#cross-target-comparison) | [`text-emit-no-downstream-tools.slang`](text-emit-no-downstream-tools.slang) |
| The shared four-phase shape (link / specialize / target-legalize / emit) of linkAndOptimizeIR runs end-to-end for every text-emit target named by the index doc. | functional | [#shared-shape](../../../docs/llm-generated/target-pipelines/index.md#shared-shape) | [`four-phase-shape-end-to-end.slang`](four-phase-shape-end-to-end.slang) |
| A single Slang source dispatches through the multi-target orchestrator to each of the five text-emit backends named by the index doc (spirv, hlsl, metal, wgsl, cuda). | functional | [#target-pipelines](../../../docs/llm-generated/target-pipelines/index.md#target-pipelines) | [`multi-target-dispatcher.slang`](multi-target-dispatcher.slang) |


## Untested claims
| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| Unordered topical catalog of every IR pass — see the `pipeline/05-ir-passes` bundle. | out-of-bundle | (unspecified) | The index doc routes per-pass enumeration to the pipeline bundle. |
| CUDA per-pass arms (`synthesizeActiveMask`, `legalizeEntryPointVaryingParamsForCUDA`, `lowerImmutableBufferLoadForCUDA`), nvrtc downstream, `__ldg` factoring, the CPU/Metal/CUDA shared `undoParameterCopy` / `transformParamsToConstRef` arms, adjacent PyTorch / OptiX / host-CPP targets. | out-of-bundle | [#synthesizeactivemask](../../../docs/llm-generated/target-pipelines/index.md#synthesizeactivemask) | The index doc routes these to `tests-agentic/target-pipelines/cuda/`. |
| HLSL per-pass arms (`wrapStructuredBuffersOfMatrices`, `legalizeNonStructParameterToStructForHLSL`, `legalizeNonVectorCompositeSelect`, `legalizeEmptyRayPayloadsForHLSL`, `legalizeUniformBufferLoad`, `applyVariableScopeCorrection`), DXC and fxc downstream, per-register class assignments. | out-of-bundle | [#wrapstructuredbuffersofmatrices](../../../docs/llm-generated/target-pipelines/index.md#wrapstructuredbuffersofmatrices) | The index doc routes these to `tests-agentic/target-pipelines/hlsl/`. |
| Metal-specific lowering (`legalizeIRForMetal`, `specializeAddressSpaceForMetal`), Metal `[[buffer(N)]]` positional binding, Apple `metal` compiler downstream. | out-of-bundle | [#legalizeirformetal](../../../docs/llm-generated/target-pipelines/index.md#legalizeirformetal) | The index doc routes these to `tests-agentic/target-pipelines/metal/`. |
| SPIR-V direct-emit specifics, `legalizeIRForSPIRV`, `simplifyIRForSpirvLegalization` outer/inner loops, the forward-declared-pointer fixup, the spirv-link / spirv-val / spirv-opt downstream chain. | out-of-bundle | [#legalizeirforspirv](../../../docs/llm-generated/target-pipelines/index.md#legalizeirforspirv) | The index doc routes these to `tests-agentic/target-pipelines/spirv/`. |
| WGSL-specific lowering (`legalizeIRForWGSL`, `specializeAddressSpaceForWGSL`), Tint downstream for `WGSLSPIRV*` variants. | out-of-bundle | [#legalizeirforwgsl](../../../docs/llm-generated/target-pipelines/index.md#legalizeirforwgsl) | The index doc routes these to `tests-agentic/target-pipelines/wgsl/`. |
| Per-target options, capability sets, target predicates (`isSPIRV`, `isMetalTarget`, `isWGPUTarget`, `isCUDATarget`, `isD3DTarget`, `isKhronosTarget`). | out-of-bundle | [#isspirv](../../../docs/llm-generated/target-pipelines/index.md#isspirv) | The index doc routes these to the `cross-cutting/targets` bundle. |


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#shared-shape](../../../docs/llm-generated/target-pipelines/index.md#shared-shape) | undocumented-behavior | The index doc's `#shared-shape` section enumerates the four phases (A link/entry-prep, B specialize/type-legalize, C target-legalize/lowering/phi-elim, D emit) but does not state any **user-observable** property that distinguishes the phases from each other at the source level. A reader cannot, from the index alone, write a test that pins "this observation comes from Phase B specifically". The four-phase shape test in this bundle therefore observes end-to-end success only; per-phase observations are routed to the peer bundles. The doc could add a sentence per phase naming one user-visible artefact of that phase's work. |  |
| [#loops](../../../docs/llm-generated/target-pipelines/index.md#loops) | undocumented-behavior | The cross-target comparison table's "Loops" column tells the reader which targets re-run inner legalization loops, but the index doc does not describe a **user-observable** consequence of looping vs not looping (a source that requires looped legalization is target-specific by definition and so belongs to the SPIR-V bundle). A future revision could add a sentence about what happens to a SPIR-V-only construct compiled to a non-looping target -- the answer would scope a future cross-cutting test. |  |
| [#pages](../../../docs/llm-generated/target-pipelines/index.md#pages) | undocumented-behavior | The `#pages` and `#see-also` sections are pure pointer tables; they are explicitly excluded from citation per the per-section prompt. No gap, just noted for future readers. |  |


## Sibling-bundle overlap
The following peer-bundle behaviors are intentionally not
re-tested here to avoid duplication:

- `entry-point-glcompute.slang` (spirv bundle) covers the
  `OpEntryPoint GLCompute` per-target shape in isolation. This
  bundle's `multi-target-dispatcher.slang` cites the same marker
  but in the cross-cutting role: it is one of five per-target
  signatures that prove the dispatcher reached each backend.
- `numthreads-attribute-survives.slang` (hlsl bundle) covers
  HLSL `[numthreads(...)]` survival on its own. The
  multi-target dispatcher test uses the same marker but cites
  the index doc's enumeration of the five targets, not the
  HLSL-specific emit claim.
- `eliminate-phis-default-options.slang` (hlsl bundle) and
  `eliminate-phis-register-allocation.slang` (spirv bundle) cover
  the per-target phi-elim option choice in isolation. This
  bundle's `phi-elim-cross-target-success.slang` cites the index
  doc's claim that the user-observable cross-cutting consequence
  is uniform: every target produces successful text emission
  regardless of phi-elim option choice.
- Per-target binding / register / `[[buffer(N)]]` / `@binding`
  spellings each live in the respective peer bundle. This
  bundle's `same-source-distinct-emitters.slang` combines them
  in a single file to assert the cross-target divergence claim
  from the comparison table, not the individual spellings.
