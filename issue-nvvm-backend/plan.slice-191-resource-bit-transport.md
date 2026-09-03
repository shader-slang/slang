# Generalize canonical CUDA resource bit transport

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The maintainers have
explicitly asked that each direct-NVVM slice commit include its plan, so this plan is a deliberate
exception to the repository's usual working-log policy.

## Purpose and Observable Result

Replace the emitter's descriptor-handle-only `uint4` special case with one planned representation
for the canonical bit transport produced by AnyValue and resource reinterpret lowering. The
representation must cover both selected 64-bit CUDA resource handles transported as `uint2` and
selected 16-byte raw-buffer views transported as `uint4`, whether the semantic side is a resource
or `DescriptorHandle<Resource>`. The motivating `anyvalue-layout` workload must compare correctly
through native CUDA and direct NVVM at O0 and O3. The larger
`reinterpret-structured-buffer` workload should advance deterministically to its next independent
unsupported operation if its resource transport becomes valid.

## Progress

- [x] (2026-09-03) Traced the native CUDA and direct final-IR producers in both motivating tests.
- [x] (2026-09-03) Confirmed the exact physical contracts: selected texture/sampler handles are
  64-bit values and selected raw buffers are `{global element*, uint64 count}` values.
- [x] (2026-09-03) Implemented one preflight-owned resource-bit-transport plan that validation and
  emission consume without reclassification.
- [x] (2026-09-03) Promoted the stable `anyvalue-layout` workload and proved the next blocker in
  `reinterpret-structured-buffer` without absorbing unrelated operations into this slice.
- [x] (2026-09-03) Built, validated all regression/coverage gates, regenerated both corpus snapshots, measured,
  self-review, and document the outcome.

## Surprises and Discoveries

- Native CUDA lowers `DescriptorHandle<Texture2D<float4>>` to one `CUtexObject` and transports it
  through AnyValue as `uint2`. It lowers `DescriptorHandle<StructuredBuffer<float>>` to the same
  16-byte structured-buffer view as the resource and transports it as `uint4`.
- `reinterpret<RWStructuredBuffer<half2>>(*inputBuffer)` is not a descriptor conversion. Its
  producer packs the source raw resource into AnyValue and unpacks the bytes directly as another
  raw resource, exposing the same `resource <-> uint4` contract.
- The old Slice 172 implementation classifies the raw-buffer descriptor relation independently in
  initial preflight, SSA validation, and emission. That predates the immutable emission-plan
  architecture established by the cleanup slices.
- The provider's generic construction `emitBitCast` is intentionally restricted to pointer-bit
  transport. The existing typed `BIT_REINTERPRET` operation already maps to LLVM `bitcast`; its
  semantic family needed to accept distinct scalar/vector shapes with equal total bit width.

## Decision Log

- Decision: classify by selected semantic resource kind and its target-owned physical CUDA
  representation, not by fixture, generic argument spelling, or arbitrary equal-sized types.
  Date/author: 2026-09-03, Codex.
- Decision: represent the decision in `NVVMEmissionPlan`; validation and emission must look it up
  by source instruction and must not repeat resource classification. Date/author: 2026-09-03,
  Codex.
- Decision: preserve the existing raw-buffer pointer/count recipe and use the builder's generic
  bitcast for 64-bit opaque resource values. No provider callback or ABI revision is needed.
  Date/author: 2026-09-03, Codex.
- Decision: do not implement the half-vector atomic reduction exposed after raw resource
  reinterpret succeeds. That is an independent canonical operation and belongs to a later atomic
  slice. Date/author: 2026-09-03, Codex.

## Outcomes and Retrospective

`anyvalue-layout` is now correct through direct O0 and O3 and has two permanent comparison lanes.
`reinterpret-structured-buffer` clears both raw resource bitcasts and now stops at the exact
`__slang_atomic_reduce_add` Half2 GenericAsm in both modes. Frozen v1 advances from 419/419/419 to
420/420/420 over 427, with no old-correct regression. Discovery remains 72/72/72 over 72. The
selected prefix passes 437/437 and the expanded permanent category passes 96/96.

The provider ABI remains revision 34. All five native/direct SM70/SM80/SM90 measurement outputs
assemble. Direct O3 produces a compact 1,120-byte PTX result; direct O0's 46,395-byte PTX exposes a
useful future optimization/benchmarking concern but does not affect correctness.

## Context and Current Pipeline

Consider these existing source operations:

```slang
uint2 textureBits = reinterpret<uint2>(foo.texHandle);
RWStructuredBuffer<half2> pairs = reinterpret<RWStructuredBuffer<half2>>(*inputBuffer);
```

AnyValue lowering produces a one-operand `bitCast` between the resource-bearing semantic type and
an unsigned 32-bit word vector. CUDA layout owns the byte representation: a selected texture or
sampler is one 64-bit handle, while a raw buffer is a two-field pointer/count aggregate. Direct
NVVM type lowering already implements both physical representations. Preflight currently accepts
only `DescriptorHandle<RawBuffer> <-> uint4`, and the emitter re-resolves that case at every phase.

## Scope and Non-Goals

In scope are exact one-operand bitcasts between an unsigned `uint2`/`uint4` payload and a selected
resource value whose established physical representation has that exact size. The resource side
may be the canonical resource or its supported descriptor handle. Out of scope are arbitrary
numeric aggregate bitcasts, unsupported resource families, byte swapping, malformed AnyValue
layouts, texture operations, and atomic reductions.

## Architecture and Invariants

- Only canonical selected resources admitted by NVVM type lowering participate.
- The payload shape is derived from the physical resource kind: two unsigned i32 lanes for an
  opaque 64-bit handle and four for a raw pointer/count view.
- Raw-buffer reconstruction retains the exact canonical element type, storage use, access, and
  address-space contract already selected by type lowering.
- Preflight owns the complete decision and provider-operation closure. SSA validation and emission
  consume an immutable plan entry keyed by the original bitcast instruction.
- Unsupported type relations retain the existing deterministic `bitCast type` diagnostic.

## Interfaces and Dependencies

Add a compiler-owned planned resource-bitcast record and index. Reuse generic builder bitcast,
aggregate/vector construction and extraction, and the existing integer recipe operations. Keep
provider ABI revision 34 unchanged.

## Milestones

1. Capture the exact native CUDA representation and direct first blocker for both motivating
   workloads.
2. Move the raw-buffer recipe into the emission plan and generalize its semantic resource side.
3. Add exact selected 64-bit resource-handle transport through the same planned family.
4. Promote newly-correct runtime comparisons and verify the partially unlocked workload's next
   canonical failure.
5. Regenerate frozen-v1/discovery results, retain measurements, update durable documentation, and
   commit the complete slice.

All five milestones are complete.

## Validation and Acceptance

Run all builds and tests outside the sandbox with Windows-native tools. At minimum:

- Build the Release provider and compiler/test targets.
- Run focused native/direct O0/direct O3 coverage for `anyvalue-layout`.
- Run `reinterpret-structured-buffer` to prove resource transport is no longer its first blocker.
- Run the selected direct-NVVM regression prefix and permanent NVVM category.
- Regenerate frozen-v1 and discovery snapshots without changing either active denominator.
- Require zero old-correct regressions and compare the exact failure Pareto before/after.
- Assemble representative SM70/SM80/SM90 PTX and record exploratory native/direct metrics where
  the harness permits.

## Failure and Recovery

Generated probes and corpus mirrors remain below `build/` and may be regenerated. If a resource's
physical representation is not already an invariant of type lowering, leave that shape
unsupported rather than deriving a size from source syntax. If the provider cannot express a
proven representation through existing generic operations, stop and document the missing
operation before revising the ABI.

## Artifacts and Hand-Off

Commit this completed plan with Slice 191 as explicitly requested. Retain permanent test
directives only for stable semantic combinations, exact census snapshots, a five-part report, and
durable architecture/capability-ledger updates. Keep transient CUDA, IR, PTX, and logs below
`build/`.
