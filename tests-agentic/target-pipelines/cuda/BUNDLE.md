---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:30:46+00:00
source_commit: 330c9a8d807b9f9352e4754f466d1244ae681cff
watched_paths_digest: 9929a933b7e23daa2d56e2d0c08666ee4781a53381b498c723250e39826823db
source_doc: docs/llm-generated/target-pipelines/cuda.md
source_doc_digest: f3067b9f2ce32537355b651dfeef65895e40ee6f911d553c5cf02a6415cdd130
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for target-pipelines/cuda

## Intent

Tests verify the CUDA target pipeline described in
[`docs/llm-generated/target-pipelines/cuda.md`](../../../docs/llm-generated/target-pipelines/cuda.md):
the ordered IR-pass sequence run by `linkAndOptimizeIR` and
`emitEntryPointsSourceFromIR` when `CodeGenTarget::CUDASource`
is requested, and the CUDA C++ text the Phase D emit produces.
The bundle exercises Phase A / B / C / D observable claims:
the `#include "slang-cuda-prelude.h"` prelude marker, the
`extern "C" __global__ void` kernel wrapping, the
`extern "C" __constant__ GlobalParams_0` resource collection
(`moveEntryPointUniformParamsToGlobalScope` is skipped), the
`legalizeEntryPointVaryingParamsForCUDA` rewrite of
`SV_DispatchThreadID` into `blockIdx * blockDim + threadIdx`,
`lowerImmutableBufferLoadForCUDA`'s `__ldg(...)` injection
(scalar and per-field struct variants), the
`atomicAdd` lowering of `InterlockedAdd`, `lowerEnumType`'s
collapse of enums to integer literals,
`transformParamsToConstRef` and `undoParameterCopy`'s
pointer-form lowering, the default
`legalizeByteAddressBufferOps` keeping `Load<T>(...)` as a
templated call, `lowerAppendConsumeStructuredBuffers` for
CUDA (target != HLSL), `legalizeArrayReturnType` producing a
`FixedArray<T, N> *` out-parameter, `eliminatePhis` default
options yielding named locals, `inlineGlobalConstantsForLegalization`
inlining static constants at use sites, the
`lowerBuiltinTypesForKernelEntryPoints` replacement of
`Texture2D` with `CUtexObject`, `groupshared` becoming
`__device__ __shared__` with `__syncthreads()` for barriers,
`SourceWriter` `#line` directives, the non-D3D / non-Khronos
contrast (no `register(uN)` annotations), and the
`__device__` decoration for non-entry-point functions.
The bundle stops at CUDA C++ text — nvrtc / PTX downstream and
OptiX-specific arms are out of scope on the no-GPU runner.

Coverage strategy: one test per concrete claim in the doc's
Phase A/B/C/D tables that can be observed in
`slangc -target cuda` text. Default directive is
`//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute`.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                              | Claim (one line)                                                                                                                                              | Tests                                                                                       |
| -------- | --------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| C-01     | [#phase-d-cuda-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools)              | CUDA emit opens with `#include "slang-cuda-prelude.h"` so the emitted text compiles under nvrtc.                                                              | `cuda-prelude-include.slang`                                                                |
| C-02     | [#phase-d-cuda-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools)              | A compute entry point is emitted as `extern "C" __global__ void <name>()`.                                                                                    | `extern-c-global-kernel.slang`                                                              |
| C-03     | [#phase-a-link-and-entry-point-prep](../../../docs/llm-generated/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep)                        | Global uniforms are gathered into a `GlobalParams_0` struct bound to `__constant__` memory because `moveEntryPointUniformParamsToGlobalScope` is skipped.     | `global-params-constant-memory.slang`, `no-uniform-params-to-global-scope.slang`            |
| C-04     | [#legalizeentrypointvaryingparamsforcuda](../../../docs/llm-generated/target-pipelines/cuda.md#legalizeentrypointvaryingparamsforcuda)              | `legalizeEntryPointVaryingParamsForCUDA` rewrites `SV_DispatchThreadID` into `blockIdx * blockDim + threadIdx`.                                               | `dispatch-thread-id-lowered.slang`                                                          |
| C-05     | [#lowerimmutablebufferloadforcuda](../../../docs/llm-generated/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda)                            | `lowerImmutableBufferLoadForCUDA` turns a StructuredBuffer load into a `__ldg(...)` intrinsic call.                                                           | `ldg-on-immutable-buffer.slang`                                                             |
| C-06     | [#lowerimmutablebufferloadforcuda](../../../docs/llm-generated/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda)                            | For struct-typed StructuredBuffer elements the pass generates per-field `__ldg(&ptr->field)` reads through a `slang_ldg` helper.                              | `ldg-on-immutable-struct.slang`                                                             |
| C-07     | [#phase-c-cuda-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination) | An atomic operation passes `validateAtomicOperations` and emits as the CUDA `atomicAdd` intrinsic on CUDA.                                                    | `atomic-lowered-to-atomicadd.slang`                                                         |
| C-08     | [#phase-a-link-and-entry-point-prep](../../../docs/llm-generated/target-pipelines/cuda.md#phase-a-link-and-entry-point-prep)                        | `lowerEnumType` collapses an enum to its underlying integer literal; the enumerator name does not appear.                                                     | `enum-lowered-to-integer.slang`                                                             |
| C-09     | [#undoparametercopy-and-transformparamstoconstref](../../../docs/llm-generated/target-pipelines/cuda.md#undoparametercopy-and-transformparamstoconstref) | `transformParamsToConstRef` rewrites struct value parameters as pointer-form for pass-by-reference on CUDA.                                                   | `transform-params-to-constref.slang`                                                        |
| C-10     | [#undoparametercopy-and-transformparamstoconstref](../../../docs/llm-generated/target-pipelines/cuda.md#undoparametercopy-and-transformparamstoconstref) | `undoParameterCopy` rewrites explicit `inout` copy-in copy-out wrappers as pass-by-pointer on CUDA emit.                                                      | `inout-via-pointer.slang`                                                                   |
| C-11     | [#phase-c-cuda-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination) | CUDA uses the default `legalizeByteAddressBufferOps` options so `ByteAddressBuffer.Load<T>(...)` survives as a templated method call.                         | `byte-address-buffer-load-template.slang`                                                   |
| C-12     | [#phase-b-specialization-and-type-legalization](../../../docs/llm-generated/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization)  | `lowerAppendConsumeStructuredBuffers` runs for CUDA (target != HLSL): Append turns into an `AppendStructuredBufferxN` wrapper using `atomicAdd` on a counter. | `lower-append-structured-buffer.slang`                                                      |
| C-13     | [#phase-c-cuda-legalization-lowering-phi-elimination](../../../docs/llm-generated/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination) | `legalizeArrayReturnType` rewrites `T[N] foo()` into a `void` function with a `FixedArray<T, N> *` out parameter on CUDA.                                     | `legalize-array-return-type.slang`                                                          |
| C-14     | [#eliminatephis-with-default-options](../../../docs/llm-generated/target-pipelines/cuda.md#eliminatephis-with-default-options)                       | CUDA uses `eliminatePhis` default options so an if/else-merged value becomes a named function-local temporary assigned in each branch.                        | `eliminate-phis-default-options.slang`                                                      |
| C-15     | [#inlineglobalconstantsforlegalization-for-cuda](../../../docs/llm-generated/target-pipelines/cuda.md#inlineglobalconstantsforlegalization-for-cuda) | CUDA always runs `inlineGlobalConstantsForLegalization`; a `static const` module-scope value is inlined at the use site rather than emitted as a `__device__` global. | `static-global-init-moved.slang`                                                            |
| C-16     | [#phase-b-specialization-and-type-legalization](../../../docs/llm-generated/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization)  | `lowerBuiltinTypesForKernelEntryPoints` replaces `Texture2D` with the CUDA primitive `CUtexObject`.                                                           | `lower-combined-texture-sampler.slang`                                                      |
| C-17     | [#phase-d-cuda-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools)              | A `groupshared` array becomes `__device__ __shared__` and `GroupMemoryBarrierWithGroupSync` lowers to `__syncthreads()` on CUDA.                              | `groupshared-to-device-shared.slang`                                                        |
| C-18     | [#phase-d-cuda-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools)              | The shared `SourceWriter` emits `#line N "<file>"` directives in the CUDA C++ text so nvrtc can map errors back to Slang source.                              | `line-directive-survives.slang`                                                             |
| C-19     | [#cuda-specific-runtime-predicates](../../../docs/llm-generated/target-pipelines/cuda.md#cuda-specific-runtime-predicates)                          | CUDA is non-D3D / non-Khronos so HLSL `register(uN)`/`register(tN)` annotations do not appear; resources are reached through `globalParams_0`.               | `no-register-binding-on-cuda.slang`                                                         |
| C-20     | [#phase-d-cuda-emit-and-downstream-tools](../../../docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools)              | Non-entry-point Slang functions are emitted with `__device__` decoration so nvrtc compiles them as device code.                                               | `device-function-decoration.slang`                                                          |

## Tests in this bundle

| File                                            | Intent     | Doc anchor                                                  |
| ----------------------------------------------- | ---------- | ----------------------------------------------------------- |
| `cuda-prelude-include.slang`                    | functional | `#phase-d-cuda-emit-and-downstream-tools`                   |
| `extern-c-global-kernel.slang`                  | functional | `#phase-d-cuda-emit-and-downstream-tools`                   |
| `global-params-constant-memory.slang`           | functional | `#phase-a-link-and-entry-point-prep`                        |
| `no-uniform-params-to-global-scope.slang`       | functional | `#phase-a-link-and-entry-point-prep`                        |
| `dispatch-thread-id-lowered.slang`              | functional | `#legalizeentrypointvaryingparamsforcuda`                   |
| `ldg-on-immutable-buffer.slang`                 | functional | `#lowerimmutablebufferloadforcuda`                          |
| `ldg-on-immutable-struct.slang`                 | functional | `#lowerimmutablebufferloadforcuda`                          |
| `atomic-lowered-to-atomicadd.slang`             | functional | `#phase-c-cuda-legalization-lowering-phi-elimination`       |
| `enum-lowered-to-integer.slang`                 | functional | `#phase-a-link-and-entry-point-prep`                        |
| `transform-params-to-constref.slang`            | functional | `#undoparametercopy-and-transformparamstoconstref`          |
| `inout-via-pointer.slang`                       | functional | `#undoparametercopy-and-transformparamstoconstref`          |
| `byte-address-buffer-load-template.slang`       | functional | `#phase-c-cuda-legalization-lowering-phi-elimination`       |
| `lower-append-structured-buffer.slang`          | functional | `#phase-b-specialization-and-type-legalization`             |
| `legalize-array-return-type.slang`              | functional | `#phase-c-cuda-legalization-lowering-phi-elimination`       |
| `eliminate-phis-default-options.slang`          | functional | `#eliminatephis-with-default-options`                       |
| `static-global-init-moved.slang`                | functional | `#inlineglobalconstantsforlegalization-for-cuda`            |
| `lower-combined-texture-sampler.slang`          | functional | `#phase-b-specialization-and-type-legalization`             |
| `groupshared-to-device-shared.slang`            | functional | `#phase-d-cuda-emit-and-downstream-tools`                   |
| `line-directive-survives.slang`                 | functional | `#phase-d-cuda-emit-and-downstream-tools`                   |
| `no-register-binding-on-cuda.slang`             | functional | `#cuda-specific-runtime-predicates`                         |
| `device-function-decoration.slang`              | functional | `#phase-d-cuda-emit-and-downstream-tools`                   |

## Doc gaps observed

- The doc's Phase D table mentions `CUDASourceEmitter` but does not
  name `#include "slang-cuda-prelude.h"` or
  `extern "C" __global__ void` as the canonical CUDA emit-prelude
  markers, or `extern "C" __constant__ GlobalParams_0` as the
  canonical resource-collection marker. A one-line statement of
  these emit-prelude facts would let tests anchor them precisely;
  this bundle anchors them to the general
  `#phase-d-cuda-emit-and-downstream-tools` section.
- The doc states `legalizeEntryPointVaryingParamsForCUDA`
  restructures kernel-entry-point parameter shapes but does not
  give the exact text-emit lowering of `SV_DispatchThreadID`
  (`blockIdx * blockDim + threadIdx`). Documenting the lowering
  expression would let a test pin the expression instead of
  inferring it from the source.
- The doc's `## lowerImmutableBufferLoadForCUDA` says the pass
  translates loads to `__ldg(...)` but does not state that
  struct-typed loads go through a generated `slang_ldg` helper
  that issues per-field `__ldg(&ptr->field)` reads. Documenting
  the per-field expansion would let the test pin its shape.
- The doc lists `applyVariableScopeCorrection` as running for
  CUDA but does not name an observable CUDA-emit pattern. No
  test.
- The doc's Phase B table lists `lowerBuiltinTypesForKernelEntryPoints`
  as stripping shader types from kernel signatures and replacing
  them with CUDA primitives, but does not enumerate the
  Slang -> CUDA-primitive substitutions (e.g.
  `Texture2D -> CUtexObject`, `SamplerState` unchanged). Listing
  the mapping would let a test pin each substitution.
- The doc's `## synthesizeActiveMask` describes converting
  IR-level active-mask references into a synthesized mask
  parameter but does not give a Slang-language surface that
  forces the pass to fire on a compute-stage entry point that
  the no-GPU bundle can compile. The pass exists but is not
  exercisable without a warp-sync intrinsic; no test in this
  bundle.
- The doc's `## collectOptiXEntryPointUniformParams` is gated on
  OptiX entry-point shapes (ray-tracing pipeline) which the
  no-GPU compute bundle does not exercise; no test.
- The doc states "CUDA has no iterative passes in
  `linkAndOptimizeIR`" but the consequence is not observable
  through `slangc -target cuda` text. No test.
- The doc lists `addDenormalModeDecorations` as always-on in
  Phase A but does not name an observable CUDA-emit marker for
  denormal mode. No test.
- The doc's Phase C table lists `processLateRequireCapabilityInsts`
  and `cleanUpVoidType` but does not name observable CUDA-emit
  markers for them. No test.
- The doc's `## Phase D` table lists `simplifyForEmit` but the
  effect is observable only as an absence of redundant variables
  in the emitted text — there is no doc-anchored positive
  marker. No test.

## Out of scope (no-GPU runner)

- **nvrtc / PTX downstream invocation**
  (`#downstream-nvrtc`). Requires the nvrtc shared library
  on the runner; `-target cuda` stops at CUDA C++ text.
- **`collectOptiXEntryPointUniformParams`**
  (`#collectoptixentrypointuniformparams`). Requires OptiX
  ray-tracing entry points (`raygeneration` / `closesthit` /
  `anyhit` / `miss`). The no-GPU compute runner does not
  exercise the ray-tracing pipeline shape.
- **`synthesizeActiveMask`**
  (`#synthesizeactivemask`). Requires a warp-sync intrinsic
  (`WaveActiveMin` / `__shfl_sync`) surface that triggers the
  active-mask synthesis; the doc anchors the pass to PTX
  subgroup intrinsics that are not on the compute-test
  surface.
- **`lowerCooperativeVectors`** is gated on the
  `optix_coopvec` capability not being present
  (`#phase-b-specialization-and-type-legalization`). Requires
  a cooperative-vector test surface that the no-GPU bundle
  does not exercise.
- **`collectCooperativeMetadata`**
  (`#phase-c-cuda-legalization-lowering-phi-elimination`).
  Requires the cooperative-matrix / cooperative-vector
  capability set.
- **`coverageTracing`-gated passes**
  (`instrumentCoverage`,
  `finalizeCoverageInstrumentationMetadata`). Coverage
  instrumentation is a debugging flag, not user-observable
  through text emit.
- **`autodiff` / `higherOrderFunc` / `derivativePyBindWrapper`
  passes**
  (`checkAutodiffPatterns`,
  `specializeHigherOrderParameters`,
  `generateDerivativeWrappers`,
  `finalizeAutoDiffPass`, etc.). Covered by other bundles;
  the doc anchors them to Phase B but the emit-stage
  observable is a downstream language feature.
- **`PyTorchCppBinding`** adjacent target arm
  (`#adjacent-targets`). Out of scope: shares some Phase B
  passes but emits via `TorchCppSourceEmitter`, not
  `CUDASourceEmitter`.
- **`dynamicResourceHeap`**, **`meshOutput`**,
  **`bindingQuery`** Phase C gates. Each requires a feature
  surface that is not on the compute-test envelope.
- **Pass-ordering claims** (Phase A passes 1-18, Phase B
  passes 1-62, Phase C passes 1-32). The doc enumerates the
  ordered list; pass _existence_ is observable through emit
  side effects, but pass _ordering_ would require
  `-dump-ir` cross-pass comparison without doc-anchored
  ordering markers. Covered by `pipeline/05-ir-passes`.
- **`-target ptx` / `-target cuda-header`** validation.
  Both are sibling `CodeGenTarget` values that share the
  same IR pipeline; the doc differentiates only in the
  downstream-compile dispatch.
- **`validateAndRemoveAssumeAddress` with `validate=false`**.
  The doc states CUDA passes `validate=false` (line 960) but
  the consequence (no validation diagnostic on a malformed
  address-of) requires a deliberately-malformed input that
  is out of scope here.
