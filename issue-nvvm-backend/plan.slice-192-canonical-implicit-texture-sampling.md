# Preserve canonical implicit texture-sample semantics

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The maintainers have
explicitly asked that each direct-NVVM slice commit include its plan, so this plan is a deliberate
exception to the repository's usual working-log policy.

## Purpose and Observable Result

Make the frozen-v1 `tests/cuda/cuda-texture.slang` workload compile and compare correctly through
direct NVVM at O0 and O3. Preserve the CUDA prelude's implicit `Texture.Sample` intent as a
producer-owned semantic tag, derive texture shape and arrayness from the canonical selected
resource type, and emit the matching unified NVVM texture intrinsic through the typed provider.

## Progress

- [x] (2026-09-03) Captured the frozen workload's first unsupported canonical helper and traced it
  to the ordinary texture/sampler producer in `hlsl.meta.slang`.
- [x] (2026-09-03) Tagged the complete ordinary no-offset implicit-sample producer family and
  legalized it without CUDA text.
- [x] (2026-09-03) Added and validated one typed implicit-sample provider operation across supported
  texture shapes.
- [x] (2026-09-03) Promoted the frozen workload, ran all focused and broad regression gates, and
  regenerated both corpus snapshots and representative measurements.
- [x] (2026-09-03) Completed the self-review, durable documentation, and outcome record.
- [x] (2026-09-03) Prepared the complete Slice 192 implementation, plan, evidence, and report for
  its slice commit.

## Surprises and Discoveries

- The ordinary HLSL-style helper takes `(texture, sampler, coordinate)`. CUDA's selected texture
  object owns sampler state, so the typed provider consumes only texture and coordinate.
- LLVM 14 defines implicit unified intrinsics for 1D, 2D, 3D, cube, and their valid array forms,
  parallel to the explicit-level intrinsics already used by the provider.
- Slice 182's semantic decoration currently stores provider value-operation IDs. Texture sampling
  needs an internal semantic ID outside that range because it lowers through the texture interface,
  not the generic value-operation interface.
- The neighboring combined `Sampler2D` producer reaches `helper function parameter: Sampler2D`
  before its operation. It remains out of scope rather than speculatively widening resource types.

## Decision Log

- Decision: extend the producer-owned semantic namespace with `TextureSample`, while preserving the
  existing numeric identity of generic value-operation semantics. Do not encode texture sampling as
  a generic value operation. Date/author: 2026-09-03, Codex.
- Decision: derive shape, arrayness, coordinate lanes, and result type from the selected
  `NVVMReadOnlyTextureType` and helper signature. The CUDA assembly spelling is not an input to
  direct-NVVM classification. Date/author: 2026-09-03, Codex.
- Decision: append a typed `SAMPLE` operation to the provider texture interface and advance the
  forward-only ABI. Existing generic builder operations cannot express an LLVM/NVVM texture
  intrinsic. Date/author: 2026-09-03, Codex.

## Outcomes and Retrospective

The frozen `cuda-texture` workload is correct through native CUDA and direct O0/O3, and its two
direct lanes are permanent. Frozen v1 keeps exactly 452 identities and 427 healthy references and
advances from 420/420/420 to 421/421/421 with one gain and no old-correct loss. Its
generic-asm-texture cluster disappears. Discovery keeps exactly 82 identities and 72 healthy
references at 72/72/72.

The selected prefix passes 438/438 and the permanent category passes 98/98. All five measured
native/direct configurations assemble. Median compilation measured 353.5 ms native and
239.1/239.0 ms direct O0/O3 SM70; PTX measured 8,810, 4,648, and 871 bytes respectively. The
combined sampler probe usefully prevented an unjustified widening: that resource family stops at
its parameter type and remains a future slice.

## Context and Current Pipeline

Consider the frozen workload:

```slang
Texture2D<float> texture;
SamplerState sampler;
outputBuffer[tid] = texture.Sample(sampler, uv);
```

`hlsl.meta.slang` selects a CUDA `__intrinsic_asm` helper whose finalized signature is
`float(Texture2D, SamplerState, float2)`. NVVM-ready legalization currently leaves that helper as
`IRGenericAsm`, so preflight stops with the exact `tex2D<$T0>` spelling. The producer already uses
semantic tags for ordinary operations; legalization turns those tagged terminators into
`IRNVVMIntrinsic` and deliberately discards target text. Direct preflight must classify a tagged
texture helper from its semantic ID and its exact selected types, record the typed requirement,
and emission must lower its texture and coordinate through the existing texture callback.

## Scope and Non-Goals

In scope is the no-offset implicit `Sample` producer for separate texture/sampler calls, Float32
scalar/vector element types already supported by explicit-level sampling, and every shape/array
combination for which LLVM exposes the corresponding unified intrinsic. Out of scope are combined
sampler resource types, offsets, bias, gradients, comparison sampling, Half elements, integer
sampling, malformed helper ABIs, and unrelated remaining frozen failures.

## Architecture and Invariants

- The producer tag is the operation source of truth. Direct NVVM does not parse or retain CUDA text.
- Generic value-operation semantic IDs retain their provider enum values; resource-family IDs live
  in a disjoint internal range beginning at `SLANG_NVVM_VALUE_OPERATION_COUNT`.
- The selected texture type owns shape, arrayness, semantic element type, and coordinate width.
- The separate sampler parameter is checked but intentionally absent from provider
  operands because the CUDA texture object already owns sampler state.
- Preflight records the complete typed texture requirement. Provider discovery and emission consume
  that requirement without fixture or source-name checks.
- Unsupported neighboring texture operations retain deterministic preflight diagnostics.

## Interfaces and Dependencies

Add an internal semantic-ID definition shared by AST-to-IR lowering and direct-NVVM preflight. Add
`SLANG_NVVM_TEXTURE_OP_SAMPLE` to the existing descriptor-based texture provider interface and
advance `SLANG_NVVM_BUILDER_ABI_REVISION` from 34 to 35. Reuse the existing texture callback; no new
callback or interface table is required. LLVM 14's `llvm.nvvm.tex.unified.*.v4f32.f32` intrinsics
are the provider-side external contract.

## Milestones

1. Add the internal texture semantic ID and annotate all ordinary no-offset CUDA Sample producer
   branches.
2. Resolve tagged helpers by selected texture type and exact three-parameter ABI, then emit
   the typed requirement without consulting assembly.
3. Extend the real and fake providers and builder facade with implicit sampling, including shape
   queries and LLVM intrinsic selection.
4. Add provider and compiler unit coverage, promote `cuda-texture.slang`, and retain unsupported
   neighboring diagnostics.
5. Run builds, focused/runtime/broad tests, corpus replays, PTX assembly/measurements, self-review,
   documentation, and commit.

## Validation and Acceptance

Run all builds and tests outside the sandbox with Windows-native tools. At minimum:

- Build the Release provider and compiler/unit-test targets.
- Run provider query/emission unit tests and compiler fake-provider implicit-sample coverage.
- Run native, direct O0, and direct O3 `cuda-texture.slang` comparisons.
- Run the permanent NVVM category and selected direct-NVVM regression prefix.
- Regenerate frozen-v1 and discovery snapshots without changing either exact identity set or
  denominator; require zero old-correct regressions.
- Assemble representative SM70/SM80/SM90 PTX and retain exploratory native/direct metrics where
  the harness permits.

## Failure and Recovery

All changes are forward-only and localized to the semantic producer, direct classifier/emitter, and
typed texture provider. If an advertised LLVM intrinsic fails verification or libNVVM compilation,
retain the producer tag but leave that exact selected shape unsupported and record the provider
diagnostic. Do not fall back to assembly parsing. Generated probes and census mirrors remain under
`build/` and may be regenerated.

## Artifacts and Hand-Off

Commit this completed plan with Slice 192 as explicitly requested. Retain permanent test directives,
exact frozen/discovery artifacts, a five-part report, and durable architecture/capability updates.
Keep transient IR, PTX, logs, and generated corpus mirrors below `build/`.
