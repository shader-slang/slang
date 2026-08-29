# Lower compact CUDA parameter-group vector storage

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM supports selected CUDA-natural constant-buffer and parameter-block
storage containing 32-bit numeric vectors, including compact three-lane fields and physical matrix
arrays with an explicit 12-byte `float3` stride. The existing
`tests/compute/constant-buffer-memory-packing.slang` fixture should pass its unchanged CUDA-natural
result `1` through direct runtime and PTX lanes.

## Progress

- [x] (2026-08-29) Completed Slice 118 as `54515b280` with recursively nested copyable aggregates
  and a 385/385 NVVM unit prefix.
- [x] (2026-08-29) Captured the real-provider stop and optimized final IR for
  `constant-buffer-memory-packing.slang` with `TARGET_WITHOUT_PACKING`.
- [x] (2026-08-29) Added an exact parameter-group storage family and a role-distinct compact
  representation without changing the builder ABI.
- [x] (2026-08-29) Reconstructed canonical vector values after compact whole loads and carried immutable component
  addressing through direct vector fields and compact vector arrays.
- [x] (2026-08-29) Added focused positive and adjacent negative coverage, promoted and validated
  the real fixture, assembled PTX, ran the full 386/386 NVVM prefix, documented, formatted, and
  self-reviewed the slice. Commit follows this final plan update.

## Surprises and Discoveries

- The first diagnostic is `struct field address`, but widening only that resolver would be wrong.
  The rejected top-level field contains a constant buffer whose element type is not in the current
  scalar-or-physical-array parameter-group family.
- Final IR contains two physical matrix structs. Each has one
  `Array<Vec<Float, 3>, 3, stride=12>` field and a total size of 36 bytes. The ordinary
  `NeedsPadding` element has two `float3` fields at offsets 0 and 12 and a total size of 24 bytes.
- LLVM's normal three-lane vector representation is a first-class value with a wider aggregate
  alignment/stride than CUDA's natural `float3` storage. Reusing it for these fields would silently
  read the wrong rows and the wrong second field even if validation and provider calls succeeded.
- The existing builder already composes the required representation: a three-element scalar array
  has the exact compact footprint, aggregate extraction can recover lanes after a whole load, and
  vector construction restores the canonical value representation. No textual LLVM rewrite or ABI
  callback is required.
- The optimized fixture loads whole matrix rows but scalarizes source component access in the
  ordinary two-`float3` constant buffer. Focused source therefore needs both a whole vector helper
  argument and a component read to prove storage/value separation and immutable lane addressing.
- Deriving the provider layout explicitly exposed a useful validation boundary. Comparing only
  CUDA and ordinary LLVM layouts would reject the intended scalar-array representation, while
  trusting the classifier alone would make future field additions unsafe. Walking the exact direct
  field declaration proves both offsets and the final aggregate size.

## Decision Log

- Decision: add a distinct parameter-group-storage lowering role and cache rather than changing the
  canonical vector value representation.
  Rationale: the same Slang `float3` can be an SSA vector or compact CUDA storage. One provider type
  handle cannot satisfy both contracts, and cache order must not make the result accidental.
  Date/author: 2026-08-29, Codex.
- Decision: represent exact compact three-lane 32-bit numeric storage as a provider fixed array of
  three scalar elements, while retaining provider vectors for ordinary value roles and naturally
  compatible two- and four-lane storage.
  Rationale: this spells the measured CUDA footprint using existing generic IR types without
  synthesizing padding or expanding the builder interface.
  Date/author: 2026-08-29, Codex.
- Decision: reconstruct a canonical vector immediately after a whole compact-storage load.
  Rationale: downstream vector arithmetic and helper signatures must receive the established vector
  type, not an array handle that merely has the same lanes.
  Date/author: 2026-08-29, Codex.
- Decision: keep nested parameter groups and nested user structs rejected in this slice.
  Rationale: the fixture proves direct numeric-vector fields and canonical physical matrix wrappers;
  nested block ownership and recursive packing rules remain distinct layout work.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Slice 119 supports CUDA-natural direct constant-buffer and parameter-block storage with direct
selected 32-bit numeric vectors and exact compact three-lane physical matrix arrays. Compact
three-lane storage lowers to `[3 x scalar]` in a dedicated type role and cache; a whole load is
converted immediately to the established SSA vector. Direct component pointers are accepted only
when their resolved producer is an immutable parameter-group field or compact array element.

The layout validator derives the selected provider size, alignment, and direct field offsets and
compares them with CUDA layout before emission. This keeps nested structs, nested parameter groups,
arbitrary strides, mutable compact stores, and unrelated pointer spellings rejected. Existing
generic builder operations were sufficient; builder ABI revision 19 remains current.

`constant-buffer-memory-packing.slang` passes direct runtime and PTX lanes with its unchanged
CUDA-natural result. Its 4,742-byte PTX assembles with CUDA 12.9.86 to a 4,328-byte cubin. The whole
fixture prefix is 8/9 plus two ignored; the only failure is an unrelated WebGPU Dawn bind-group
validation error, and both CUDA lanes pass. Focused fake tests prove the distinct scalar-array and
vector representations, reconstruction, and helper call. Adjacent scalar parameter-group and
unsupported nested-group tests pass. The complete NVVM prefix passes 386/386.

Self-review inventory:

- The parameter-group vector and array classifiers survive. They recognize exact canonical final
  IR types: three-lane 32-bit numeric vectors and a positive fixed array with literal stride 12.
  The real fixture fails at its first field address without this family.
- The parameter-group storage role and cache survive. The same canonical `float3` legitimately has
  distinct storage and SSA representations; the focused test observes both in one module and would
  fail if cache order selected either representation globally.
- The layout helpers survive. They walk the canonical direct-field declaration produced by final
  IR, derive the chosen representation rather than rediscovering source syntax, and compare every
  offset plus aggregate size/alignment with CUDA. They reject a mismatch before provider mutation.
- The immutable pointer branches survive. They revalidate an exact field or sequential-element
  producer and do not walk arbitrary operand graphs. Component access in the promoted fixture and
  focused test fails without them.
- Compact-load reconstruction survives. It converts the provider storage aggregate to the
  canonical vector at the representation boundary; helper parameter type assertions and the real
  matrix-row loads fail without it.
- The test directives survive. Runtime proves numerical offsets, PTX checks the intended backend
  route and global memory operations, and `ptxas` proves emitted-module acceptance.

No new custom semantic equivalence, source-syntax reconstruction, emitter byte-offset patch,
arbitrary explicit-stride fallback, nested-group widening, provider feature, or text rewrite was
introduced.

## Context and Current Pipeline

With CUDA-natural expectations selected, optimized final IR contains this physical storage:

    struct RowMajorStorage {
        float3 data[3]; // explicit stride 12, size 36
    };

    struct ColumnMajorStorage {
        float3 data[3]; // explicit stride 12, size 36
    };

    struct NeedsPaddingStorage {
        float3 data1;   // offset 0
        float3 data2;   // offset 12, total size 24
    };

The synthesized global parameter block stores pointers to those three constant buffers plus the
existing `RWStructuredBuffer<uint>`. Loads of a matrix row currently have semantic type `float3`,
followed by constant lane extraction. Loads from `NeedsPadding` are already scalar because source
component access becomes a field pointer followed by a vector-lane pointer. The output is a long
short-circuit Boolean chain converted to UInt and stored to the raw output view.

## Scope and Non-Goals

In scope are direct parameter-group structs containing selected integer/Float32 scalars and 32-bit
numeric vectors; canonical physical one-array wrappers whose array is already selected or has an
explicit compact three-lane 32-bit vector stride; a distinct provider representation for compact
parameter-group storage; immutable vector field and lane addressing; canonical reconstruction of
whole compact-vector loads; and runtime/PTX promotion of the existing fixture.

Out of scope are nested user structs or nested parameter groups, arrays beyond the measured direct
numeric array family, Boolean/Float16/Float64 storage, HLSL constant-buffer packing synthesis,
mutable compact vector stores, matrices outside canonical physical wrappers, padding fields,
arbitrary explicit strides, source member-name lookup, new builder callbacks, and LLVM text edits.

## Architecture and Invariants

- Parameter-group classification accepts only nonempty direct fields from its exact scalar/vector
  storage family or a canonical physical struct with one accepted numeric array.
- `NVVMTypeUse::ParameterGroupStorage` is distinct from `Value` and ordinary `Storage`; its cache
  cannot be populated by prior SSA vector lowering.
- Only exact three-lane Int/UInt/Float storage uses `[3 x scalar]`. Other accepted vectors retain
  their canonical provider vector representation.
- An explicit compact array is accepted only when its element is an exact three-lane 32-bit numeric
  vector and its literal stride is 12 bytes. Arbitrary stride acceptance is not inferred.
- A whole load from a validated compact immutable pointer is converted from the storage aggregate
  to a canonical vector before it enters the ordinary value map.
- Nested component addressing remains rooted in a resolved immutable parameter-group field or
  physical-array element producer; arbitrary explicit pointers remain rejected.

## Interfaces and Dependencies

No builder ABI or LLVM provider change is planned. The type-family helpers, role-aware type-lowering
cache, direct emitter field/sequential/load paths, fake-provider fixture/test, existing compute
fixture, durable design status, and this plan are the expected committed areas. CUDA 12.9 runtime
and `ptxas` provide semantic and assembly evidence.

## Milestones

1. Define exact direct parameter-group vector structs and explicit compact numeric arrays.
2. Add role-distinct compact storage type lowering and provider representation caching.
3. Admit immutable vector field/lane producers and reconstruct canonical vectors after whole loads.
4. Add focused fake traces and preserve nested-block, unsupported stride/type, and mutable-storage
   rejection boundaries.
5. Promote the real fixture, validate runtime/PTX and `ptxas`, run the complete NVVM prefix, update
   durable status and this plan, self-review, format, and commit.

## Validation and Acceptance

Acceptance requires Release host builds; focused fake evidence for scalar-array storage types,
parameter-group struct fields, immutable field/lane pointers, canonical vector reconstruction, and
zero new provider features; adjacent nested-block rejection before provider mutation; direct
runtime/PTX lanes for the existing fixture; CUDA 12.9 `ptxas -arch=sm_70`; the complete
`slang-unit-test-tool/nvvm` prefix; pinned formatting; and `git diff --check`.

The self-review inventories the new storage classifiers, role/cache, field and sequential producer
branches, load reconstruction, and tests. For each, record the exact final-IR producer, why the
shape is canonical, and which test fails without it. Remove any cache-order dependence, general
explicit-stride acceptance, array-as-vector value leak, nested-block widening, source syntax
reconstruction, or emitter-side byte-offset patch.

## Failure and Recovery

If the generic array representation or canonical reconstruction does not preserve runtime results,
keep exact IR, LLVM assembly, PTX, cubin, and logs under ignored `build/slice119-*`, narrow the slice
to the independently proven direct-vector field subset, and record the next layout boundary. Do not
reinterpret the input as HLSL packing, synthesize offsets in the emitter, patch LLVM text, weaken
the layout classifier, reset unrelated work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, PTX, cubin, and logs under ignored `build/slice119-*`. Distill the final compact
storage contract, validation evidence, and next measured corpus boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly
requested.
