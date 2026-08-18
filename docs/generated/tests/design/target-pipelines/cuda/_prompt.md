# Prompt: docs/generated/tests/design/target-pipelines/cuda/

See [`_common.md`](../../../_meta/prompts/_common.md) for universal rules and
[`_claims.md`](../../../_meta/prompts/_claims.md) for the claim methodology.
Those rules apply to this bundle and override nothing here unless explicitly
noted.

## Target

Produce the test bundle at `docs/generated/tests/design/target-pipelines/cuda/`,
anchored to
[`docs/generated/design/target-pipelines/cuda.md`](../../../../design/target-pipelines/cuda.md).

Audience: nightly CI. The bundle exercises the **CUDA target family** —
`CUDASource` (`-target cuda`), `CUDAHeader` (`-target cuh`) and `PTX`
(`-target ptx`) — as an ordered IR-pass plus emit sequence run by
`linkAndOptimizeIR` and `emitEntryPointsSourceFromIR`.

The defining characteristic of this bundle is that nearly every claim is
observable in **emitted CUDA C++ text**. `nvrtc` is downstream and is not
invoked by `-target cuda`, so the bundle stops at the text artifact. The
default directive is therefore:

```
//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute
```

Use `-target cuh` only for claims the doc states as a `CUDAHeader`-vs-
`CUDASource` divergence, and `-target ptx` only where the doc names PTX as the
artifact endpoint.

## The translation rule: claims to observations

`target-pipelines/cuda.md` is organized as four phases plus a set of notable
passes, and the bundle mirrors that structure. The anchors below are the ones
`doc_ref` must resolve into.

**Phase A — `#phase-a-link-and-entry-point-prep`.** The entry-point prep arms
CUDA takes that other shader targets do not:

- `collectOptiXEntryPointUniformParams` runs in place of
  `collectEntryPointUniformParams` (anchor
  `#collectoptixentrypointuniformparams`).
- `moveEntryPointUniformParamsToGlobalScope` is **skipped**, so uniforms land in
  a `__constant__ GlobalParams` block rather than as loose globals — that block
  is the observable.
- `lowerEnumType` collapses an `enum` to its underlying integer, so the emitted
  text carries the integer, not the enumerator name.

**Phase B — `#phase-b-specialization-and-type-legalization`.** CUDA's largest
divergence from the shader targets:

- The CUDA-only `lowerBuiltinTypesForKernelEntryPoints` / `removeTorchKernels` /
  `handleAutoBindNames` cluster (anchor
  `#lowerbuiltintypesforkernelentrypoints-removetorchkernels-handleautobindnames`).
- The `lowerCooperativeVectors` gate, which differs between `CUDASource` and
  `CUDAHeader` — the pair of targets is the test.
- `lowerCombinedTextureSamplers` is **skipped**: CUDA is not a CPU-like
  artifact, so a Slang `Sampler2D` does not split into a texture/sampler pair.
- `performTypeInlining` / `checkGetStringHashInsts` run for the same reason.
- `checkForOutOfBoundAccess` and `checkStaticAssert` (the latter after
  specialization) are diagnostic-observable.
- `shouldLegalizeExistentialAndResourceTypes = false` — an interface-typed value
  is not legalized away on CUDA (anchor
  `#effect-of-shouldlegalizeexistentialandresourcetypes--false`).

**Phase C — `#phase-c-cuda-legalization-lowering-phi-elimination`.** The three
CUDA-specific passes plus the arms CUDA shares with CPU and Metal:

- `synthesizeActiveMask` (anchor `#synthesizeactivemask`).
- `legalizeEntryPointVaryingParamsForCUDA` — `SV_DispatchThreadID` and friends
  become the CUDA thread/block index expressions (anchor
  `#legalizeentrypointvaryingparamsforcuda`).
- `lowerImmutableBufferLoadForCUDA` (anchor
  `#lowerimmutablebufferloadforcuda`).
- `undoParameterCopy` and `transformParamsToConstRef` (anchor
  `#undoparametercopy-and-transformparamstoconstref`).
- `legalizeArrayReturnType`, default-option `legalizeByteAddressBufferOps`,
  `floatNonUniformResourceIndex`, and default-option `eliminatePhis` (anchor
  `#eliminatephis-with-default-options`).
- `inlineGlobalConstantsForLegalization` for CUDA (anchor
  `#inlineglobalconstantsforlegalization-for-cuda`).

**Phase D — `#phase-d-cuda-emit-and-downstream-tools`.** The
`CUDASourceEmitter` text contract: the prelude, the `__global__` /
`extern "C"` kernel signature, the `__constant__ GlobalParams` block, thread
indexing, and the fact that `-target cuda` stops at CUDA C++ text.

**Adjacent targets — `#adjacent-targets`.** Claims of the form "CUDA does X
where CPU/Metal do Y". Verify by compiling the same source to the sibling target
with a second directive and a distinct CHECK prefix.

**Conditional gates.** `#required-lowering-pass-set-flags`,
`#option-set-toggles-and-emit-side-options`,
`#context-predicates-and-capability-gates`, and
`#cuda-specific-runtime-predicates` — a gate is testable when the doc names a
source shape that trips it and an emit difference that follows.

### Not testable through `slangc -target cuda` (record under `## Untested claims`)

- **`nvrtc` invocation and PTX/cubin bytecode** beyond what `-target ptx`
  produces locally. The doc anchors this at `#downstream-nvrtc`; a real CUDA
  toolchain is not present on the no-GPU runner.
- **Pass _ordering_ within a phase.** Pass existence is observable through its
  effect on emitted text; ordering is an IR-level claim owned by
  `pipeline/05-ir-passes`.
- **The autodiff gate** (`#the-autodiff-gate`) where it requires a differentiable
  entry point whose emitted difference is not a stable CUDA token.
- **`#loops-in-the-pipeline`.** "The pipeline iterates until fixpoint" has no
  single emitted token; record it as a non-emit-observable claim.
- **OptiX-only shapes** that need the OptiX headers or an `optix_*` capability
  the runner cannot satisfy.

Everything recorded as untested must carry a reason in the README table; use
`implementation-detail` for claims whose only consequence is inside the
compiler, and `toolchain-absent` for anything needing `nvrtc`.

## Required structure

1. `README.md` with the structure named in `_common.md`.
2. 60 to 95 `.slang` files (manifest `size_cap_files` is 100). One
   `//TEST:SIMPLE` directive per file is the norm; add a sibling-target
   directive only for an `#adjacent-targets` claim.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/generated/design/target-pipelines/cuda.md`

Secondary (allowed citations; only where the primary doc hands off):

- `docs/generated/design/pipeline/04-ast-to-ir.md`
- `docs/generated/design/pipeline/05-ir-passes.md`
- `docs/generated/design/pipeline/06-emit.md`
- `docs/generated/design/ir-reference/index.md`
- `docs/generated/design/cross-cutting/targets.md`

If you would cite anything else, stop and record a doc-gap finding in
`README.md` instead.

## Source files you may consult for _verification only_

Use these to confirm a specific emitted token. Do **not** mine them for claims
the doc does not state.

- `source/slang/slang-emit.cpp`
- `source/slang/slang-emit-cuda.cpp`
- `source/slang/slang-emit-cpp.cpp`
- `source/slang/slang-emit-c-like.cpp`
- `source/slang/slang-ir-cuda-immutable-load.cpp`

## Test directives

Default:

```
//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute
```

Variants this bundle legitimately needs:

| Need                                        | Directive                                                           |
| ------------------------------------------- | ------------------------------------------------------------------- |
| `CUDAHeader`-vs-`CUDASource` divergence     | `-target cuh` alongside `-target cuda`                              |
| PTX as the artifact endpoint                | `-target ptx`                                                       |
| Ray-tracing entry-point shapes              | `-stage raygeneration` / `closesthit` / `anyhit` / `intersection`   |
| Cooperative-vector gate                     | `-capability optix_coopvec`                                         |
| SM-gated behavior                           | `-capability cuda_sm_7_0`                                           |
| A value-level result rather than a spelling | `COMPARE_COMPUTE:-cpu -output-using-type` with `TEST_INPUT` buffers |

## Lessons captured for the CUDA target pipeline

- **Uniforms live in `__constant__ GlobalParams`.** Because
  `moveEntryPointUniformParamsToGlobalScope` is skipped, a uniform parameter is
  a field of that block — not a loose global. Match the block, then the field.
- **Entry points emit as `extern "C" __global__ void`.** The kernel signature is
  the canonical Phase D marker.
- **`SV_DispatchThreadID` becomes a CUDA index expression**, not a preserved
  semantic name; `legalizeEntryPointVaryingParamsForCUDA` is observed through
  that rewrite.
- **Enum lowering eats the enumerator name** — match the integer.
- **DCE strips locally-unused code.** Write computed values to a buffer or
  return them, or the pattern will not appear in the emit.
- **CUDA is not CPU.** Claims that hold for `-target cpp` (combined-sampler
  splitting in particular) must be checked on CUDA rather than assumed from the
  CPU pipeline page.
- **`cuh` and `cuda` share most of the pipeline.** Only assert a divergence the
  doc actually names; otherwise the two targets emit the same text and the test
  proves nothing.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every `doc_ref` resolves to an anchor in `target-pipelines/cuda.md` (or a
      listed secondary doc), and every `doc_section_digest` is current.
- [ ] The default directive is `-target cuda`; sibling-target directives appear
      only for `#adjacent-targets` claims and carry a distinct CHECK prefix.
- [ ] CHECK patterns use wildcards for mangled/generated identifiers.
- [ ] No test requires `nvrtc`, an NVIDIA driver, or a GPU.
- [ ] `## Untested claims` enumerates the nvrtc/PTX-bytecode/OptiX/pass-ordering
      items with a reason each.
- [ ] `## Doc gaps observed` records claims with no checkable marker in the emit.
