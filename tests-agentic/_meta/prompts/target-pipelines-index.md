# Prompt: tests-agentic/target-pipelines/index/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/target-pipelines/index/`,
anchored to
[`docs/llm-generated/target-pipelines/index.md`](../../../docs/llm-generated/target-pipelines/index.md).

Audience: nightly CI. The bundle exercises the **subtree-level
orientation** claims made by `target-pipelines/index.md`. The index
doc is a navigation hub: it names the five peer pages (spirv, hlsl,
metal, wgsl, cuda), the shared four-phase shape, and a small
cross-target comparison table. The per-target details belong to the
five peer bundles (`spirv`, `hlsl`, `metal`, `wgsl`, `cuda`). This
bundle therefore should be **small** and focus on the
**cross-cutting / multi-target dispatcher** claims the index doc
itself makes that no single peer page is responsible for.

The bundle target is **3–8 tests**. If you cannot anchor a claim to
text in the index doc itself, route it to a peer bundle via
`## Out of scope` (e.g. `see target-pipelines/spirv`) and **do not**
duplicate a peer claim here.

## What counts as a cross-cutting index claim

A claim is in scope for this bundle iff the index doc itself
asserts it. The index doc's load-bearing claims are:

1. **Multi-target dispatcher**: a single Slang source compiled to N
   distinct text-emit targets observably routes to N different
   backend emitters. The index page enumerates spirv-asm, hlsl,
   metal, wgsl, and cuda as text-emit targets backed by distinct
   per-target pages.
2. **Shared four-phase shape**: every target passes through the
   same four-phase `linkAndOptimizeIR` shape (link/entry-prep,
   specialize/type-legalize, target-legalize/lowering/phi-elim,
   emit/downstream). Observable by checking that the same source
   reaches successful emission on multiple targets without the
   user touching the pipeline.
3. **Per-target Phase D emitter divergence**: distinct targets
   produce distinct surface syntax for the same Slang construct.
   The cross-target table explicitly maps each target to its own
   emitter (`emitSPIRVForEntryPointsDirectly`, `HLSLSourceEmitter`,
   `MetalSourceEmitter`, `WGSLSourceEmitter`, `CUDASourceEmitter`).
   Observable by FileCheck on the emitted text differing
   per-target.
4. **Phi-elimination gating** (named in the prose under the
   cross-target table): `eliminatePhis` runs with
   **register-allocation enabled** only for SPIR-V
   (`isKhronosTarget && emitSpirvDirectly`) and with **default
   options** for HLSL, Metal, WGSL, CUDA. The user-observable
   cross-cutting consequence is that all five targets still
   produce successful text emit from the same SSA-shaped source.
5. **CodeGenTarget enum spread** (named explicitly in the
   cross-target table): each target page covers several
   `CodeGenTarget` enum values (e.g. HLSL also gates DXIL /
   DXBytecode variants; Metal covers MetalLib / MetalLibAssembly;
   WGSL covers WGSLSPIRV / WGSLSPIRVAssembly; CUDA covers
   CUDASource / CUDAHeader / PTX). Observable cross-cutting
   consequence: text-emit targets (spirv-asm, hlsl, metal, wgsl,
   cuda) all compile without invoking downstream tools, while
   downstream-binary variants need external compilers and are
   out-of-scope for a no-GPU runner.

Pick **3–8** claims from this list. Prefer claims that exercise
the **multi-target dispatcher** angle, since the per-target
details belong to peer bundles.

## What is NOT in scope for this bundle

The index doc explicitly delegates these to peer pages. Route them
to peer bundles in `## Out of scope`; do not write tests here:

- SPIR-V direct-emit specifics, `legalizeIRForSPIRV` arms,
  `simplifyIRForSpirvLegalization` outer/inner loops,
  forward-declared-pointer fixup, spirv-link / spirv-val /
  spirv-opt downstream — see `target-pipelines/spirv`.
- HLSL per-pass arms (`wrapStructuredBuffersOfMatrices`,
  `legalizeNonStructParameterToStructForHLSL`,
  `legalizeNonVectorCompositeSelect`, etc.), DXC / fxc downstream,
  register-class assignments — see `target-pipelines/hlsl`.
- Metal-specific lowering (`legalizeIRForMetal`,
  `specializeAddressSpaceForMetal`), Metal `[[buffer(N)]]`
  attributes, Apple `metal` compiler downstream — see
  `target-pipelines/metal`.
- WGSL-specific lowering (`legalizeIRForWGSL`,
  `specializeAddressSpaceForWGSL`), Tint downstream for
  `WGSLSPIRV` — see `target-pipelines/wgsl`.
- CUDA per-pass arms (`synthesizeActiveMask`,
  `legalizeEntryPointVaryingParamsForCUDA`,
  `lowerImmutableBufferLoadForCUDA`), nvrtc downstream,
  `__ldg` factoring, CPP/CUDA shared `undoParameterCopy` — see
  `target-pipelines/cuda`.
- Unordered topical pass catalog (`#ir-passes` shape) — see
  the `pipeline/05-ir-passes` bundle, not this one.

If you find yourself writing a test whose sole purpose is a
single-target behavior, stop and route it to the peer bundle.

## Required structure

1. `README.md` with the structure named in `_common.md`, including
   a `## Out of scope` section listing the peer bundles for the
   topics not covered here, and a `## Sibling-bundle overlap`
   section recording intentionally-avoided peer claims.
2. **3–8 `.slang` tests**, each anchored to an anchor in the
   index doc (`#target-pipelines`, `#shared-shape`,
   `#cross-target-comparison`, `#filtering-rules`). The `#pages`
   and `#see-also` anchors are pure pointer sections and should
   not be cited.
3. Coverage rules:
   - At least one **multi-target dispatcher** test: one Slang
     source compiles cleanly across multiple text-emit targets
     (use multiple `//TEST:SIMPLE` directives in one file).
   - At least one **emitter divergence** test: the same source
     produces distinctively different emitted surface text on
     two or more targets (per-target `CHECK` patterns).
   - **No more than 2 tests** for any single index anchor; the
     bundle is breadth-over-depth.

4. Naming: `<topic>-<axis>.slang`, e.g.
   `multi-target-dispatcher.slang`,
   `same-source-distinct-emitters.slang`,
   `four-phase-shape-end-to-end.slang`. Avoid name collisions
   with sibling bundles.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/target-pipelines/index.md`

Secondary (allowed citations; use sparingly and only when the
index doc explicitly references them):

- `docs/llm-generated/target-pipelines/spirv.md`
- `docs/llm-generated/target-pipelines/hlsl.md`
- `docs/llm-generated/target-pipelines/metal.md`
- `docs/llm-generated/target-pipelines/wgsl.md`
- `docs/llm-generated/target-pipelines/cuda.md`
- `docs/llm-generated/pipeline/05-ir-passes.md`
- `docs/llm-generated/cross-cutting/targets.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Test directives

Cross-cutting dispatcher is by definition multi-target. Pick
multi-directive tests:

- Multi-target dispatcher → multiple `//TEST:SIMPLE(filecheck=CHECK):-target <T>`
  directives in one file, one per backend, with per-target
  `CHECK` patterns where the emitted text differs.
- Phi-elim / shared shape end-to-end → multiple text-emit
  directives with minimal per-target `CHECK` patterns that confirm
  each backend produced output.
- Single-target observations that nevertheless cite the index
  doc's cross-cutting prose (rare) → a single
  `//TEST:SIMPLE(filecheck=CHECK):-target <T>` is acceptable.

Do **not** use any GPU-only directive. Do **not** use `-dump-ir`
in this bundle — observing IR pass internals belongs to the
`pipeline/05-ir-passes` bundle.

## Sibling-bundle anti-duplication

Before writing each test, confirm the observation is not already
made by the sibling bundles under
`tests-agentic/target-pipelines/`. If a sibling already exercises
the same exact per-target behavior, do **not** duplicate. Either
skip the test or change the angle to be specifically about
**dispatcher behavior across targets**.

Record the duplications you intentionally avoided in
`README.md` under `## Sibling-bundle overlap`.

## Drop policy

If you cannot anchor a candidate test to text in the index doc
after **3 attempts** at re-reading the doc, drop the test and
record the would-be claim in `README.md` under `## Doc gaps
observed`. The bundle's purpose is orientation; failing to find a
testable index-level claim is itself a useful signal about the
doc.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Bundle has 3–8 tests; not fewer, not more.
- [ ] Every test's `doc_ref` anchor exists in `index.md` (not
      `#pages`, not `#see-also`).
- [ ] At least one multi-target dispatcher test exists (multiple
      `//TEST` directives in one file).
- [ ] At least one emitter-divergence test exists (per-target
      `CHECK` patterns that diverge).
- [ ] No test duplicates a sibling-bundle test verbatim.
- [ ] `README.md` has `## Out of scope` listing the peer bundles
      that own each delegated topic.
- [ ] `README.md` has `## Sibling-bundle overlap` listing
      intentionally-avoided peer claims.
- [ ] No `-dump-ir` directive is used. No GPU-only directive.
