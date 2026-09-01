# Generalize resource-bearing aggregate storage layout

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM has one principled representation for canonical aggregate storage
that mixes selected resource values with ordinary fields or finite arrays. Field-address and
launch-storage layout checks consume the same physical representation rather than rejecting a
resource solely because its pointer retains `ScalarLayout` metadata.

The bounded cross-corpus probes are frozen `hlsl/cbuffer-float3-offsets-unaligned` and discovery
`bugs/type-legalize-bug-1` plus `compute/buffer-type-splitting`. Promote only exact O0/O3 native
differential successes. The existing aligned constant-buffer runtime mismatch is measured as an
adjacent layout signal but is not patched speculatively.

## Progress

- [x] (2026-09-01) Completed and committed Slice 169 as `c6161c764`; frozen v1 advances to
  407/407/407 over 427 and discovery remains 68/68/68 over 72.
- [x] (2026-09-01) Re-ranked the separate Pareto tables. Resource-bearing aggregate layout is the
  strongest shared in-MVP representation boundary: one frozen and two discovery workloads reach
  field-pointer or storage-layout stops.
- [x] (2026-09-01) Preserved final IR and physical launch layouts for the three probes. Identified the exact
  aggregate/type producers, field offsets, storage sizes, and first downstream consumers.
- [x] (2026-09-01) Implemented one reusable retained-layout proof using the existing provider
  operations prove it. Carry each workload to correctness or record its next independent blocker.
- [x] (2026-09-01) Regenerated both exact corpora and bounded measurements; documented, validated, self-reviewed, and
  commit Slice 170.

## Surprises and Discoveries

- Frozen `cbuffer-float3-offsets-unaligned` first stops at a generic pointer to
  `RWStructuredBuffer<float>` whose pointer metadata is `ScalarLayout`. Its sibling aligned
  workload already compiles but produces a runtime mismatch, so field-pointer admission alone is
  not evidence that the complete constant-buffer launch layout is correct.
- Discovery `type-legalize-bug-1` reaches the same pointer spelling for
  `RWStructuredBuffer<int>` inside a much larger parameter-block/dynamic-dispatch graph.
- Discovery `buffer-type-splitting` stops earlier because `S[2]`, where each `S` owns two raw
  byte-address views, fails the conventional-global aggregate storage compatibility proof.
- The retained layout for that field is complete even though context-free CUDA queries are not:
  `S[2]` is 64 bytes at alignment eight and stride 32; `S` is 32 bytes; its two selected raw views
  occupy offsets zero and 16. Matching those field layouts by key unlocks correct O0/O3 execution.
- Requiring every collected-global sibling to be supported inside producer recognition hid the
  next real blockers. Separating recognition from validation advances the frozen probe to its
  cbuffer pointer and `type-legalize-bug-1` to exact `ParameterBlock<B>` field representation.

## Decision Log

- Decision: investigate field-pointer and aggregate-storage compatibility together, but require
  each accepted layout to agree with the native CUDA launch ABI before sharing a representation.
  Rationale: the shapes share a physical resource aggregate boundary, while the aligned runtime
  mismatch demonstrates that opcode admission without layout proof would be unsafe.
  Date/author: 2026-09-01, Codex.
- Decision: keep provider ABI revision 32 unless a concrete canonical address or aggregate
  operation cannot be expressed through existing typed structs, arrays, GEPs, loads, and stores.
  Rationale: current first failures occur in compiler-side representation classification.
  Date/author: 2026-09-01, Codex.
- Decision: use retained target-layout metadata only at the collected conventional-global boundary
  and keep the strict context-free proof elsewhere. Match struct layout entries by semantic key and
  require exact recursive offsets, strides, sizes, and alignments.
  Rationale: this is the producer-owned ABI source of truth for opaque resource fields and does not
  invent padding or relax layouts lacking canonical metadata.
  Date/author: 2026-09-01, Codex.
- Decision: recognize a canonical collected global independently from whether every field is
  supported, then validate each field explicitly.
  Rationale: sibling support is a consumer capability, not part of producer identity; separating
  them yields deterministic exact-type blockers without allowing unsupported storage.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Slice 170 unlocks discovery `buffer-type-splitting` in both modes and promotes two permanent lanes.
Frozen v1 remains exactly 452/427 and 407/407/407 with no changed row. Discovery remains exactly
82/72 and advances from 68/68/68 to 69/69/69 with exactly one gain and no loss. The two adjacent
probes retain deterministic, more precise parameter-group blockers rather than receiving
speculative support.

The selected prefix passes 433/433 and the permanent `nvvm` category passes 62/62. The promoted
gate produces accepted native NVRTC and direct PTX/cubins; direct O3 assembles for SM70, SM80, and
SM90. The provider ABI remains revision 32. Self-review removed both the provisional homogeneous-
resource offset guess and an unproven context-free alignment relaxation.

## Context and Current Pipeline

Conventional CUDA entry lowering synthesizes global parameter storage, then direct NVVM lowers
selected resource leaves to opaque handles or raw `{data, count}` values. Aggregate compatibility
currently requires the source storage layout to match the provider representation recursively.
`IRFieldAddress` validation separately proves the base pointer, field identity, result pointer,
address space, access, and layout metadata before emitting a generic struct-field pointer.

## Scope and Non-Goals

In scope are the exact three bounded aggregate producers, selected resource leaves already owned by
the backend, fixed arrays, explicit field offsets/layout metadata, conventional global storage,
field addressing, existing generic provider aggregate/pointer operations, both fixed corpora, and
bounded measurements.

Out of scope are arbitrary resource pointers, source/fixture-name checks, repacking without a launch
ABI proof, parameter-block fallback layouts, dynamic-dispatch redesign, syntax reconstruction,
serialized IR patches, aligned constant-buffer mismatch fixes without a shared root cause, new
provider callbacks without a concrete operation gap, and external workloads.

## Architecture and Invariants

- One physical aggregate representation must determine type lowering, size/alignment validation,
  field offsets, storage creation, field-address result types, and loads.
- Resource leaves retain their selected opaque/raw provider representation; an address of a
  resource field is valid only when the containing storage actually owns that representation.
- `ScalarLayout` metadata is evidence to audit, not a reason to erase layout distinctions.
- Native CUDA launch argument layout is the reference contract. Direct NVVM may not reinterpret a
  machine-local packed host block as a differently sized LLVM aggregate.
- Every widening names its canonical producer and is proven by exact positive and adjacent negative
  tests. Unsupported shapes stop deterministically before provider mutation.

## Interfaces and Dependencies

Aggregate/storage classifiers live in `source/slang/slang-emit-nvvm-type-lowering.cpp` and
`source/slang/slang-emit-nvvm.cpp`. Entry storage, field-address validation, type lowering, and
emission use the existing revision-32 generic provider operations. Real repository fixtures own
differential launch/runtime proof; focused fake-provider coverage is retained only for a new
classifier or negative invariant not already observed there.

## Milestones

1. Dump final linked IR and native/direct layout evidence for the three probes in O0 and O3.
2. Trace every failing aggregate and pointer to its canonical producer and exact physical ABI.
3. Implement the smallest shared classifier/representation and retain strict adjacent negatives.
4. Promote exact successes; run builds, focused tests, selected prefix, permanent category, both
   corpora, and SM70/SM80/SM90 measurements.
5. Update the design, ledger, five-part report, and this plan; format, audit, stage exactly the slice
   files excluding `external/slang-binaries/`, and commit.

## Validation and Acceptance

All builds/tests run outside the sandbox with Windows-native tools and the isolated Release
provider. Acceptance requires exact corpus identities 452/427 and 82/72; O0/O3 differential
results; zero old-correct regression; retained diagnostic ownership for unsuccessful probes;
selected-prefix and permanent-category success; PTX assembly where a gate is promoted; formatting;
artifact integrity; and an exact staged-file audit.

## Failure and Recovery

If native layout evidence disagrees with the existing provider aggregate representation, retain the
deterministic preflight stop and document the need for a distinct physical representation. If a
probe advances to dynamic dispatch, parameter-block ABI, or another independent shape, do not widen
this slice. Never erase layout metadata, patch field offsets downstream, or admit an arbitrary
resource pointer merely to pass one fixture.

## Artifacts and Hand-Off

Keep dumps and logs under ignored `build/nvvm-census` paths. Retain the completed plan only with a
committed result under the user's workflow exception. Distill durable representation rules into
`docs/design/nvvm-backend.md`, exact status into the capability ledger and separate corpus
artifacts, and every producer/input-shape decision into the Slice 170 five-part report.
