# Preserve CUDA DescriptorHandle resource representations

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM represents a selected `DescriptorHandle<T>` exactly as CUDA represents
the underlying supported resource `T`. Handles can live in conventional parameter storage,
structured-buffer elements, arrays, local/helper aggregates, and helper results. Exact
`CastDescriptorHandleToResource` and `CastResourceToDescriptorHandle` operations preserve the same
provider value rather than introducing a second physical representation.

The bounded target is the six healthy cross-corpus workloads whose current first failure or runtime
abort is descriptor-handle representation: frozen `reinterpret-structured-buffer`,
`optional-descriptor-handle`, and the three `layout-descriptor-handle-*` workloads, plus discovery
`gh-6657-bindless-uniform`. Later resource-operation or harness blockers will be recorded rather
than widened speculatively.

## Progress

- [x] (2026-08-31) Reconciled Slice 153 Pareto rows and identified six descriptor-handle workloads
  across the frozen and discovery corpora.
- [x] (2026-08-31) Dumped final IR for a frozen raw-buffer handle and discovery texture handle and
  traced the first rejection to the missing canonical leaf representation.
- [x] (2026-08-31) Verified in `slang-ir-layout.cpp` and CUDA layout rules that
  `DescriptorHandle<T>` has `T`'s size, alignment, and target representation on CUDA.
- [x] (2026-08-31) Added one exact selected-handle classifier and reused it across storage, helper,
  layout, and role-sensitive type lowering.
- [x] (2026-08-31) Admitted exact handle/resource identity conversions through existing generic
  values, without a provider callback or bit-level text rewrite.
- [x] (2026-08-31) Built and ran all six motivating workloads at O0/O3, promoted the one stable
  correct discovery representative, and classified every cascade.
- [x] (2026-08-31) Ran the 427/427 selected unit prefix, both complete corpora, twelve-gate
  SM70/80/90 measurements, integrity checks, documentation, and self-review; Slice 154 is ready to
  commit.

## Surprises and Discoveries

- Discovery `gh-6657-bindless-uniform` first fails while selecting the conventional-global
  `StructuredBuffer<Data>` field, not while extracting `Data.followingData`. Its element struct
  contains a `DescriptorHandle<Texture2D>`, so the unsupported leaf makes the entire typed raw view
  and collected global block unavailable.
- Frozen `reinterpret-structured-buffer` preserves an exact
  `DescriptorHandle<RWStructuredBuffer<half>>`, loads it from the collected global block, then emits
  `CastDescriptorHandleToResource`. CUDA represents both sides as the same raw-view aggregate.
- The three dynamic-dispatch layout tests currently reach an internal `Unsupported value size`
  abort rather than a deterministic preflight diagnostic. They exercise arrays and aggregate
  transport of structured-buffer handles and are useful acceptance gates for recursive coverage.
- The unaligned-half constant-buffer workload shares the outer field-address diagnostic but has no
  descriptor handle. It remains outside this slice.
- `gh-6657-bindless-uniform` becomes correct at O0 and O3. Its `StructuredBuffer<Data>` proves the
  selected handle leaf inside external aggregate storage without ever dereferencing the unused
  texture handle.
- `reinterpret-structured-buffer` advances through collected-global field addressing, loading, the
  raw-buffer handle helper ABI, and exact handle-to-resource conversion. Its next instruction is
  canonical `bitCast(RWStructuredBuffer<half>, vector<uint,4>)`, owned by raw-view bit transport.
- `optional-descriptor-handle` advances from an unsupported helper result to `defaultConstruct` of
  the optional aggregate. Generic zero construction is a separate value-production invariant.
- The three dynamic-dispatch rows still abort in `slang-ir-extract-value-from-type.cpp` while
  `AnyValue` lowering treats a 16-byte structured-buffer descriptor as one unsupported leaf. That
  failure precedes NVVM preflight and needs a producer-side marshalling slice, not an emitter guard.
- Read-write surface handles were removed during self-review. No motivating workload proves their
  conversion-to-operation provenance or storage-format contract, so accepting them here would be
  speculative despite ordinary surface values already having a provider representation.
- `extras/formatting.sh --check-only --modified` cannot run in the available Windows bash because
  `gersemi`, `clang-format`, `prettier`, and `shfmt` are absent. The focused C++ diff was manually
  style-reviewed and `git diff --check` passes.

## Decision Log

- Decision: make supported descriptor handles aliases of their underlying resource provider type.
  Rationale: this is the existing CUDA semantic source of truth in layout, and it makes the exact
  handle/resource casts representation-preserving identities.
  Date/author: 2026-08-31, Codex.
- Decision: select only handles whose underlying resource already has a tested direct-NVVM value
  representation in the motivating set: raw buffers, read-only textures, and samplers.
  Rationale: a handle must not make an otherwise unsupported opaque resource executable.
  Date/author: 2026-08-31, Codex.
- Decision: extend existing recursive aggregate classifiers through the selected handle leaf.
  Rationale: arrays and structs are canonical finite containers; the handle's underlying resource
  determines alignment and provider type, while existing active sets retain cycle safety.
  Date/author: 2026-08-31, Codex.
- Decision: keep provider ABI revision 30.
  Rationale: type construction, aggregate transport, loads/stores, and value aliasing already exist.
  No new LLVM operation is required.
  Date/author: 2026-08-31, Codex.
- Decision: retain the three newly exposed frozen failures rather than widening this slice.
  Rationale: raw-view bit transport, default construction, and `AnyValue` byte packing have
  different canonical producers and reusable invariants. None is part of descriptor type aliasing.
  Date/author: 2026-08-31, Codex.
- Decision: exclude read-write surface handles from the selected classifier.
  Rationale: surface operations require field-owned format provenance, and none of this slice's
  tests proves that a descriptor conversion preserves that canonical producer. A later surface
  workload must establish the complete invariant.
  Date/author: 2026-08-31, Codex.
- Decision: promote only `gh-6657-bindless-uniform`.
  Rationale: it is deterministic, differentially correct in both direct modes, and proves a new
  resource-in-aggregate storage combination. The other five rows are not yet correct.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Frozen corpus v1 remains exactly 452 workloads and 427 healthy MVP references. O0/O3/both
correctness remains 380/384/380 with zero old-correct regression. The frozen handle rows do not
become correct, but their first failures are now separated into exact raw-view bit transport,
default construction, and `AnyValue` marshalling clusters.

Discovery remains exactly 82 workloads and 72 healthy native references. O0/O3/both correctness
reaches 55/55/55, up one in every numerator with zero old-correct regression.
`bugs/gh-6657-bindless-uniform.slang#discovery-1` is newly correct and has permanent O0/O3 direct
lanes. Each complete direct mode contains 55 correct, 18 preflight, one provider, seven
infrastructure, and one runtime-mismatch result.

The selected unit prefix passes 427/427 and the promoted source passes 6/6 across its reflection,
CPU, and direct lanes. All twelve measurement gates assemble through CUDA 12.9 at direct O3 for
SM70, SM80, and SM90. Provider ABI revision 30 is unchanged.

## Context and Current Pipeline

Consider this source shape:

```slang
struct Data
{
    float4 prefix;
    Texture2D<float4>.Handle texture;
    uint2 result;
}
StructuredBuffer<Data> inputData;
```

CUDA buffer-element lowering preserves the handle as canonical `IRDescriptorHandleType(Texture2D)`.
Final linking retains `StructuredBuffer<Data>` in the synthesized conventional global block and
loads a `Data` value from that view. `slang-ir-layout.cpp` asks the underlying texture for the
handle's CUDA layout, which is one 64-bit texture handle.

Direct NVVM already lowers `Texture2D` to i64 and a raw structured buffer to
`{ global element pointer, i64 count }`, but its type classifiers do not recognize
`IRDescriptorHandleType`. The missing leaf causes resource-struct, aggregate-storage,
structured-buffer-storage, conventional-global, helper-value, and alignment proofs to fail at
their owning boundaries. Adding isolated exceptions at each failed fixture would duplicate the
same target representation.

The shared type-lowering boundary should instead select an exact descriptor handle once, validate
that its underlying resource is already supported, and reuse the underlying resource's ordinary
value representation. An exact handle-to-resource or resource-to-handle cast then changes only the
semantic type, not the provider bits, and emission can reuse the operand handle.

## Scope and Non-Goals

In scope are selected raw-buffer, read-only-texture, and sampler descriptor handles;
recursive aggregate/helper/storage classification; CUDA layout compatibility; exact bidirectional
handle/resource casts; focused O0/O3 validation; permanent promotion; separate corpus artifacts;
and durable documentation.

Out of scope are descriptor-heap indexing, untyped handles, unsupported resource kinds, handle
integer casts unless a motivating workload reaches them, resource creation, fixture-name checks,
syntax reconstruction, arbitrary opaque values, unrelated half/float3 layout, provider callbacks,
ABI revision, frozen-corpus identity changes, and corpus v2.

## Architecture and Invariants

- A selected `DescriptorHandle<T>` is legal only when `T` has an established direct-NVVM resource
  value representation.
- On CUDA, the handle and `T` share one provider type, size, alignment, and value bits.
- Aggregate classification recurses through the semantic handle leaf and remains finite/cycle-safe.
- Structured-buffer and parameter-group layout checks compare the canonical CUDA layout with the
  provider type derived from `T`; they do not guess a universal i64 handle.
- Exact handle/resource casts require semantic type equality between the handle's resource operand
  and the opposite value type.
- Unsupported handle kinds fail deterministically before provider mutation.
- Frozen corpus v1 and discovery retain separate fixed identities and denominators.

## Interfaces and Dependencies

`source/slang/slang-emit-nvvm-type-lowering.h/.cpp` will own the selected descriptor-handle
classifier and provider-type alias. `source/slang/slang-emit-nvvm.cpp` will reuse it for exact
conversion validation/emission and layout proofs. Existing revision-30 generic builder operations
remain sufficient; no provider or external dependency change is planned.

## Milestones

1. Add a selected descriptor-handle classifier that returns the exact underlying supported resource
   type. Reuse it in resource alignment, aggregate/structured storage, helper value/alignment,
   parameter-group representation, conventional-global fields, and type-use legality.
2. Lower every selected handle role to the underlying resource's existing value representation,
   caching the alias by canonical handle type.
3. Resolve exact handle-to-resource and resource-to-handle casts, validate their operand
   availability, and map the result to the same provider value.
4. Build and run all six target workloads against NVRTC at direct O0/O3. Promote stable semantic
   representatives and capture each later first blocker with producer and diagnostic.
5. Run the 427-test selected prefix, frozen v1, discovery, and SM70/80/90 measurement gates. Update
   separate TSV/JSON evidence, report, design, ledger, and this plan; self-review and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools as required by
`AGENTS.md`. Acceptance requires:

- final-IR probes retain canonical `IRDescriptorHandleType` rather than reconstructing syntax or
  rewriting it upstream;
- the selected handle and underlying resource lower to one provider type in each tested resource
  family;
- exact casts reuse provider values and adjacent mismatched/unsupported handles remain rejected;
- every promoted workload is differentially correct at direct O0 and O3;
- frozen v1 remains exactly 452 workloads/427 healthy references and discovery exactly 82/72, with
  separate O0/O3/both, classifications, Pareto, and zero old-correct regression;
- the selected NVVM prefix passes and representative O3 PTX assembles for SM70/80/90;
- provider ABI revision remains 30; and
- Python/artifact integrity and `git diff --check` pass without staging
  `external/slang-binaries/`.

## Failure and Recovery

If a workload reaches a new operation, record it and keep the handle representation if its focused
type/layout tests pass. If CUDA and provider layouts differ for a resource family, remove that family
from the selected classifier rather than pad or reinterpret downstream. Generated dumps and census
outputs under `build/` are reproducible and remain untracked.

## Artifacts and Hand-Off

Commit this completed plan with the implementation because the user explicitly requires them
together. Retain final-IR probes and measurement outputs under `build/`; commit stable direct lanes,
Slice 154 corpus snapshots, the five-part report, and durable design/ledger updates. The report must
trace the semantic handle type to the CUDA layout rule, underlying provider type, and exact consumer.
