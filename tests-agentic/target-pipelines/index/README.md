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
route them there via `## Out of scope` rather than duplicating.

Strategy: one observation per cross-cutting claim, using the
lightest set of `//TEST:SIMPLE(-target ...)` directives that make
the multi-target dispatcher visible. Each test in this bundle runs
multiple `//TEST` directives in a single file with per-target
`CHECK_<TARGET>` prefixes so a single source file actually exercises
the dispatcher.

## Claims enumerated

| Claim ID | Anchor                      | Claim (one line)                                                                                                                                       | Tests                                     |
| -------- | --------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ | ----------------------------------------- |
| C-01     | `#target-pipelines`         | The subtree dispatches one Slang source through `linkAndOptimizeIR` to each of five text-emit backends (spirv, hlsl, metal, wgsl, cuda).               | [`multi-target-dispatcher.slang`](multi-target-dispatcher.slang)           |
| C-02     | `#cross-target-comparison`  | The cross-target table assigns a distinct Phase D emitter per target; the same source produces distinctively different surface text on each.           | [`same-source-distinct-emitters.slang`](same-source-distinct-emitters.slang)     |
| C-03     | `#shared-shape`             | The shared four-phase shape (link → specialize → target-legalize → emit) runs end-to-end on every text-emit target without user pipeline intervention. | [`four-phase-shape-end-to-end.slang`](four-phase-shape-end-to-end.slang)       |
| C-04     | `#cross-target-comparison`  | `eliminatePhis` is gated per-target (reg-alloc only for SPIR-V; default options for HLSL, Metal, WGSL, CUDA); SSA-merging code still emits on each.    | [`phi-elim-cross-target-success.slang`](phi-elim-cross-target-success.slang)     |
| C-05     | `#cross-target-comparison`  | The text-emit `CodeGenTarget` rows reach Phase D as source / asm without invoking the downstream binary tools (DXC, fxc, Apple `metal`, Tint, nvrtc).  | [`text-emit-no-downstream-tools.slang`](text-emit-no-downstream-tools.slang)     |

## Tests in this bundle

| File                                    | Intent     | Doc anchor                  |
| --------------------------------------- | ---------- | --------------------------- |
| [`multi-target-dispatcher.slang`](multi-target-dispatcher.slang)         | functional | `#target-pipelines`         |
| [`same-source-distinct-emitters.slang`](same-source-distinct-emitters.slang)   | functional | `#cross-target-comparison`  |
| [`four-phase-shape-end-to-end.slang`](four-phase-shape-end-to-end.slang)     | functional | `#shared-shape`             |
| [`phi-elim-cross-target-success.slang`](phi-elim-cross-target-success.slang)   | functional | `#cross-target-comparison`  |
| [`text-emit-no-downstream-tools.slang`](text-emit-no-downstream-tools.slang)   | functional | `#cross-target-comparison`  |

## Out of scope

The index doc explicitly delegates these to peer pages; tests for
them belong to peer bundles, not here:

- SPIR-V direct-emit specifics, `legalizeIRForSPIRV`,
  `simplifyIRForSpirvLegalization` outer/inner loops, the
  forward-declared-pointer fixup, the spirv-link / spirv-val /
  spirv-opt downstream chain -- see
  `tests-agentic/target-pipelines/spirv/`.
- HLSL per-pass arms (`wrapStructuredBuffersOfMatrices`,
  `legalizeNonStructParameterToStructForHLSL`,
  `legalizeNonVectorCompositeSelect`,
  `legalizeEmptyRayPayloadsForHLSL`,
  `legalizeUniformBufferLoad`, `applyVariableScopeCorrection`),
  DXC and fxc downstream, per-register class assignments -- see
  `tests-agentic/target-pipelines/hlsl/`.
- Metal-specific lowering (`legalizeIRForMetal`,
  `specializeAddressSpaceForMetal`), Metal `[[buffer(N)]]`
  positional binding, Apple `metal` compiler downstream -- see
  `tests-agentic/target-pipelines/metal/`.
- WGSL-specific lowering (`legalizeIRForWGSL`,
  `specializeAddressSpaceForWGSL`), Tint downstream for
  `WGSLSPIRV*` variants -- see
  `tests-agentic/target-pipelines/wgsl/`.
- CUDA per-pass arms (`synthesizeActiveMask`,
  `legalizeEntryPointVaryingParamsForCUDA`,
  `lowerImmutableBufferLoadForCUDA`), nvrtc downstream, `__ldg`
  factoring, the CPU/Metal/CUDA shared `undoParameterCopy` /
  `transformParamsToConstRef` arms, adjacent
  PyTorch / OptiX / host-CPP targets -- see
  `tests-agentic/target-pipelines/cuda/`.
- Unordered topical catalog of every IR pass -- see the
  `pipeline/05-ir-passes` bundle.
- Per-target options, capability sets, target predicates
  (`isSPIRV`, `isMetalTarget`, `isWGPUTarget`, `isCUDATarget`,
  `isD3DTarget`, `isKhronosTarget`) -- see the
  `cross-cutting/targets` bundle.

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

## Out of scope (no-GPU runner)

(none) -- all tests use only `//TEST:SIMPLE(filecheck=...)` text-
emit directives. No directive in this bundle requires a GPU and
no downstream binary tool (DXC, fxc, Apple `metal`, Tint, nvrtc)
is invoked.

## Doc gaps observed

- The index doc's `#shared-shape` section enumerates the four
  phases (A link/entry-prep, B specialize/type-legalize, C
  target-legalize/lowering/phi-elim, D emit) but does not state
  any **user-observable** property that distinguishes the phases
  from each other at the source level. A reader cannot, from the
  index alone, write a test that pins "this observation comes
  from Phase B specifically". The four-phase shape test in this
  bundle therefore observes end-to-end success only; per-phase
  observations are routed to the peer bundles. The doc could
  add a sentence per phase naming one user-visible artefact of
  that phase's work.
- The cross-target comparison table's "Loops" column tells the
  reader which targets re-run inner legalization loops, but the
  index doc does not describe a **user-observable** consequence
  of looping vs not looping (a source that requires looped
  legalization is target-specific by definition and so belongs
  to the SPIR-V bundle). A future revision could add a sentence
  about what happens to a SPIR-V-only construct compiled to a
  non-looping target -- the answer would scope a future
  cross-cutting test.
- The `#pages` and `#see-also` sections are pure pointer tables;
  they are explicitly excluded from citation per the per-section
  prompt. No gap, just noted for future readers.
