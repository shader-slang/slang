# Add sampled texture level operations

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs every enabled lane of
`tests/compute/half-texture-simple.slang`. One typed texture-operation descriptor covers scalar
Float32 `SampleLevel` for 1D, 2D, 3D, and cube textures and the 1D, 2D, and cube array variants.
The provider emits LLVM's unified texture-level intrinsics, the compiler carries the CUDA texture
and sampler placeholder ABI through ordinary typed values, and the fixture's seven samples return
the established value seven through the direct CUDA runtime path.

## Progress

- [x] (2026-08-29) Reproduced the Slice 101 boundary and captured the finalized seven helper
  signatures, GenericAsm strings, conventional-global layout, and call-site shapes.
- [x] (2026-08-29) Verified LLVM 7 and LLVM 14 expose matching unified scalar-result texture-level
  intrinsic families and captured reference CUDA 12.9 PTX for all seven shapes.
- [x] (2026-08-29) Added one descriptor-driven texture-operation interface and proved the provider's seven
  intrinsic mappings through LLVM verification, libNVVM, and `ptxas`.
- [x] (2026-08-29) Generalized selected texture/sampler handle lowering, recognized exact SampleLevel helpers, and
  carried typed texture requirements from preflight into emission.
- [x] (2026-08-29) Added the exact Float32 square-root dependency required by cube-coordinate normalization.
- [x] (2026-08-29) Added focused API/compiler coverage and direct runtime/PTX lanes to the existing fixture and two adjacent scalar SampleLevel fixtures.
- [x] (2026-08-29) Formatted, built, ran focused/full/changed-shader validation, completed self-review, updated durable docs, and prepared the completed slice commit. Both Release builds pass; exact changed shader lanes pass 7/7; the complete NVVM prefix passes 374/374.

## Surprises and Discoveries

- The finalized helper always has four parameters—texture, sampler, coordinate, and level—but all
  seven CUDA GenericAsm bodies omit `$1`. CUDA texture objects own sampling state; `SamplerState`
  is an unused pointer-sized ABI placeholder retained only to simplify cross-target lowering.
- LLVM's unified texture intrinsics return four scalar lanes even when Slang's semantic result is a
  scalar. This slice can extract lane zero without inventing a scalar-only intrinsic or exposing
  LLVM aggregate representation through the ABI.
- Array intrinsics take the integer layer before floating coordinates, while Slang's canonical
  coordinate packs the layer in its final floating lane. The provider must perform the exact
  float-to-signed-i32 conversion and operand reordering selected by the GenericAsm text.
- The fixture normalizes cube coordinates through `sqrt`. Once texture handles are admitted, exact
  Float32 `$P_sqrt($0)` is the next retained helper dependency and belongs in this demonstrable
  slice rather than becoming an artificial stop after the resource work.
- Array texture shapes retain the array flag in `IRTextureTypeBase::getShape()`. Classification
  must use the canonical base shape plus `isArray()`; switching on the combined shape rejects all
  three valid array variants.
- LLVM 14 gives `llvm.sqrt.f32` the same six optimization-only attributes that the legacy writer
  recognizes for selected intrinsics. The writer previously rewrote that attribute row but had no
  semantic declaration count for sqrt, so its fail-closed accounting returned
  `SLANG_E_NOT_AVAILABLE`. Exact sqrt declaration validation and shared unique-set accounting make
  the LLVM 7 translation explicit.

## Decision Log

- Decision: add a separate descriptor-driven texture-operation interface rather than callbacks per
  texture shape or overload the surface descriptor.
  Rationale: sampled textures and surfaces have different coordinate, boundary, storage, and
  result contracts. One texture descriptor plus one query/emit pair scales across operation kind,
  shape, arrayness, and semantic result type without making irrelevant surface fields meaningful.
  Date/author: 2026-08-29, Codex.
- Decision: make the texture operation consume texture, semantic coordinate, and level, while the
  helper's sampler value remains an ordinary unused parameter.
  Rationale: the CUDA prelude and generated CUDA source establish that the texture object contains
  sampling state and every GenericAsm body omits the sampler. Transporting the placeholder into
  provider semantics would falsely imply that LLVM's unified intrinsic consumes it.
  Date/author: 2026-08-29, Codex.
- Decision: support the complete seven-shape scalar SampleLevel family in one slice.
  Rationale: they differ only by descriptor data and intrinsic selection. Splitting each shape
  would repeat the same compiler/provider work and return to the overly small slices already
  rejected for this prototype.
  Date/author: 2026-08-29, Codex.
- Decision: add exact Float32 square root through the existing value-operation catalog.
  Rationale: cube-coordinate normalization is real fixture code. Sqrt is an ordinary typed value
  operation and does not justify a resource-specific workaround or libdevice dependency.
  Date/author: 2026-08-29, Codex.
- Decision: classify every retained intrinsic whose LLVM-14-only attribute set is translated.
  Rationale: suffix-based rewriting deliberately fails closed when semantic and rewritten counts
  differ. Adding exact sqrt declaration validation preserves that invariant instead of weakening
  the compatibility writer or special-casing a count mismatch.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

The finalized helper family is:

    Float sample1D(Texture1D<Float>, SamplerState, Float coordinate, Float level)
    Float sample2D(Texture2D<Float>, SamplerState, Float2 coordinate, Float level)
    Float sample3D(Texture3D<Float>, SamplerState, Float3 coordinate, Float level)
    Float sampleCube(TextureCube<Float>, SamplerState, Float3 coordinate, Float level)
    Float sample1DArray(Texture1DArray<Float>, SamplerState, Float2 coordinateLayer, Float level)
    Float sample2DArray(Texture2DArray<Float>, SamplerState, Float3 coordinateLayer, Float level)
    Float sampleCubeArray(TextureCubeArray<Float>, SamplerState, Float4 coordinateLayer, Float level)

Each entry-point call loads its texture and sampler from the collected conventional global. The
seven exact GenericAsm strings are `tex1DLod`, `tex2DLod`, `tex3DLod`, `texCubemapLod` and their
three `LayeredLod` variants. All resource types are read-only, non-multisampled, non-shadow,
non-combined Float textures. The texture type completely carries shape and arrayness; unlike
formatted surfaces, no source field decoration is needed to select physical storage because the
runtime texture object owns the bound format and conversion behavior.

LLVM 7.0.1 and LLVM 14.0.6 both expose `llvm.nvvm.tex.unified.*.level.v4f32.f32` IDs for all seven
variants. Non-array inputs are handle, floating coordinates, and level. Array inputs insert a
signed-i32 layer immediately after the handle. The intrinsic returns an LLVM aggregate of four
Float32 lanes.

## Scope and Non-Goals

In scope:

- read-only, non-MS, non-shadow, non-combined scalar Float32 1D/2D/3D/cube textures;
- non-array and the valid 1D/2D/cube array forms;
- exact ordinary `SamplerState` placeholder values;
- explicit-level floating-coordinate sampling, exact GenericAsm matching, typed capability
  preflight, unified intrinsic emission, lane-zero extraction, runtime/PTX fixture coverage;
- exact scalar Float32 square root needed by the fixture.

Out of scope:

- Float2/Float4, integer, Half, comparison, depth, multisample, combined, buffer, or feedback
  texture results;
- implicit-derivative Sample, SampleBias, SampleGrad, SampleCmp, Load/fetch, gather, offsets,
  status results, or sparse textures;
- `SamplerComparisonState`, bindless/descriptor-heap resources, arbitrary resource casts, or
  combined texture-sampler construction;
- backward compatibility.

## Architecture and Invariants

- The texture descriptor owns operation kind, shape, arrayness, and complete semantic result type.
  It does not reuse surface boundary/storage fields.
- Exactly one query/emit callback pair covers the family. Compiler capability queries finish before
  module creation.
- Selected texture and ordinary sampler values lower to opaque i64 handles in helper, value, load,
  and conventional-global storage roles. Their semantic types remain distinct in Slang IR.
- A SampleLevel helper must match one exact GenericAsm string and complete four-parameter
  signature. Shape, arrayness, coordinate width, result, and level must agree.
- Emission looks up a preflighted helper requirement. It does not parse GenericAsm or reconstruct
  texture semantics a second time.
- The sampler parameter remains in the typed function/call ABI but is intentionally absent from
  the texture-operation operands because CUDA's texture object owns sampling state.
- The provider validates every operand and selects an intrinsic before LLVM mutation. It converts
  only the final array-coordinate lane to signed i32, reorders it before floating coordinates,
  calls the exact unified intrinsic, and extracts lane zero.

## Interfaces and Dependencies

Revise the forward-only builder ABI to revision 14 with texture operation/shape enums, descriptor,
required interface, facade methods, fake provider, C compile probe, and LLVM provider. Extend NVVM
type lowering with selected sampled-texture and executable ordinary-sampler classifiers. Extend
compiler requirements and GenericAsm resolution/emission. Add one sqrt catalog row and provider
implementation. Update `half-texture-simple.slang`, focused negative/API tests, and
`docs/design/nvvm-backend.md`.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. CMake builds, tests, and native formatting run outside the sandbox per
repository instructions.

## Milestones

1. Add ABI revision 14 and provider/fake/facade texture descriptor support; verify each intrinsic
   mapping in isolated provider tests.
2. Admit exact sampled texture and sampler value roles, resolve seven helper variants, persist
   requirements, and emit typed operations.
3. Add Float32 sqrt, enable the existing fixture's direct lanes, validate runtime/PTX/ptxas, audit
   input shapes, update durable docs and this plan, then commit.

## Validation and Acceptance

Acceptance requires API negotiation and invalid-descriptor tests; focused compiler helper-shape
tests; the complete `slang-unit-test-tool/nvvm` prefix; every enabled lane of
`half-texture-simple.slang`; optimized direct PTX with all seven `tex.level` rows; CUDA 12.9
`ptxas` assembly; pinned clang-format 17; and `git diff --check`.

Record exact counts, runtime output, PTX/cubin sizes, the seven emitted instructions, and the next
measured fixture boundary as work completes.

## Self-Review and Input-Shape Audit

Inventory the new texture/sampler classifiers, exact helper resolver, persisted requirement lookup,
provider descriptor validator/intrinsic selector, layer conversion, and sqrt row. Confirm each
shape is produced by the finalized CUDA prelude and conventional-global collector, no format is
guessed, the unused sampler matches the established CUDA ABI, preflight remains the sole semantic
resolver, and no arbitrary GenericAsm text crosses the ABI. For every new helper or branch, name
the test that fails without it and reject any fallback that masks malformed resource IR.

## Failure and Recovery

If LLVM verification, libNVVM, runtime comparison, or `ptxas` rejects a unified intrinsic mapping,
preserve the exact IR/PTX/diagnostic and stop the loop. Do not transport GenericAsm, substitute a
different texture shape, or silently drop array layers. Generated dumps, PTX, and cubins stay under
ignored `build/`. Never reset unrelated work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Record descriptors, intrinsic signatures, texture/sampler ABI evidence, PTX/runtime results, test
counts, self-review, and the next exact fixture stop. Distill durable sampled-texture architecture
and coverage into `docs/design/nvvm-backend.md`.

## Outcomes and Retrospective

Builder ABI revision 14 now exposes one required texture-operation interface. The compiler admits
only scalar Float read-only sampled textures and ordinary samplers in the exact storage, value,
helper, load, and call roles required by finalized IR. It matches all seven complete SampleLevel
helpers, persists their descriptors through preflight, and omits the sampler only at the typed
provider-operation boundary established by CUDA's texture-object ABI.

The provider maps the seven shapes to unified level intrinsics, converts and reorders array layers,
and extracts scalar lane zero. Scalar Float sqrt uses the value catalog and `llvm.sqrt.f32`; the
LLVM 7 compatibility writer now validates and translates its optimization attribute set. The
neighboring `Texture2D<float2>` test remains closed before provider discovery.

`half-texture-simple.slang`, `texture-simple.slang`, and `texture-simpler.slang` now have direct
runtime and PTX lanes. The first two prove all seven PTX operations while the last provides a
minimal 2D case. `half-texture-simple.slang` returns Float 7 in all four output lanes. Its direct
PTX is 2,093 bytes, contains the ordered 1D/2D/3D/cube/a1D/a2D/aCube `tex.level` rows, and CUDA
12.9.86 `ptxas -arch=sm_70` emits a 6,632-byte cubin.

The new helpers and special cases survive the input-shape audit: sampled type classification reads
only canonical texture operands; exact GenericAsm resolution checks the whole finalized helper;
the requirement is the sole semantic source used during emission; provider selection validates
the descriptor and all operands before mutation; and the layer conversion is owned by the known
Slang-coordinate to LLVM-intrinsic ABI boundary. Removing any one re-exposes either the positive
fixture boundary, invalid array order, unsupported vector negative, or LLVM 7 attribute failure.
No syntax or resource shape is reconstructed from a downstream semantic value.

Final build and full-suite counts are recorded in the last progress item after validation.
