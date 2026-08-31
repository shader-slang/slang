# Materialize canonical selected-resource default values

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM materializes the exact resource placeholder values produced while
lowering `none` for selected optional payloads. A default `StructuredBuffer<T>` becomes the
existing raw-buffer provider struct containing a null global data pointer and zero element count.
A default selected `DescriptorHandle<Texture2D<...>>` or `DescriptorHandle<SamplerState>` becomes
the zero value of its existing 64-bit underlying resource-handle representation.

The bounded targets are the two healthy frozen-v1 workloads currently sharing the exact
`defaultConstruct` first blocker:
`language-feature/dynamic-dispatch/optional-single-concrete-layout.slang` and
`language-feature/optional-descriptor-handle.slang`. Both must become differentially correct at
direct O0 and O3 before promotion.

## Progress

- [x] (2026-08-31) Re-ranked remaining healthy failures after Slice 158 and selected the two-row
  frozen default-resource cluster rather than following recency alone.
- [x] (2026-08-31) Captured final linked IR and traced the two exact optional-none producers to
  their canonical resource-leaf `defaultConstruct` instructions.
- [x] (2026-08-31) Implemented one exact selected-resource default descriptor and compiler-owned emission recipe
  through existing provider operations, without changing ABI revision 30.
- [x] (2026-08-31) Built, ran focused real-provider checks, validated both targets at O0/O3, and promoted only
  correct stable lanes.
- [x] (2026-08-31) Ran the selected prefix, complete frozen-v1 and discovery corpora, representative
  measurements, formatting/integrity checks, and self-review.
- [x] (2026-08-31) Completed the report, design/ledger updates, plan, and Slice 159 commit.

## Surprises and Discoveries

- `OptionalTypeLoweringContext::processMakeOptionalNone` deliberately synthesizes a payload
  placeholder before constructing `{payload, false}`. For `Optional<DomeLight>`, recursive
  `IRBuilder::emitDefaultConstruct` leaves exact defaults for the two opaque descriptor-handle
  fields and constructs `DomeLight` from them.
- The single-concrete existential optional follows a different upstream path.
  `TypeFlowSpecializationContext::specializeMakeOptionalNone` constructs the tagged-union `none`
  value with a default untagged payload; after later lowering, the remaining exact leaf is
  `defaultConstruct<StructuredBuffer<int>>`.
- Slice 154 already proved that each admitted `DescriptorHandle<T>` has exactly `T`'s provider
  representation. The two descriptor defaults therefore need no wrapper construction or new
  provider type.
- `_lowerRawBufferType` already defines a raw structured-buffer value as
  `{global element pointer, i64 count}`. Existing generic integer constants, pointer types,
  integer-to-pointer bit casts, and aggregate construction can express its null/empty default.
- Complete-prefix validation exposed an unrelated test-only regression introduced by Slice 158:
  the stateful aggregate fixture's optimizer-tolerant `emitStoreCallCount >= 4` assertion had been
  accidentally tightened to `== 5`. The current valid lowering emits four stores. Restoring the
  pre-Slice-158 contract returns the prefix from 426/427 to 427/427 without changing production
  behavior.

## Decision Log

- Decision: accept only `IRDefaultConstruct` with zero operands and an exact selected raw
  structured-buffer type or selected descriptor handle whose underlying resource is a read-only
  texture or sampler.
  Rationale: those are the complete canonical shapes proven by the two motivating workloads.
  Byte-address buffers, writable surfaces, bare texture/sampler defaults, arbitrary resource
  structs, and unrelated defaultable values have no evidence in this slice.
  Date/author: 2026-08-31, Codex.
- Decision: describe default construction in the compiler and compose existing generic provider
  operations; retain ABI revision 30.
  Rationale: the physical resource representations are already selected and the provider can
  express typed zero constants, the null global pointer bit pattern, and aggregate construction.
  Date/author: 2026-08-31, Codex.
- Decision: improve rejection to include the exact unsupported default result type.
  Rationale: `defaultConstruct` alone merges unrelated canonical producers and prevents precise
  Pareto ownership. A typed diagnostic preserves deterministic preflight without weakening it.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Both exact frozen targets are now correct at direct O0 and O3 and have permanent lanes. Frozen v1
remains exactly 452/427 and improves from 384/388/384 to 386/390/386 healthy O0/O3/both-mode
correctness, with zero old-correct loss. Across all rows, direct O0 is 399 correct, 40 preflight,
eight runtime mismatch, and five provider; direct O3 is 404 correct, 40 preflight, and eight
runtime mismatch. Native CUDA remains 449 correct/three infrastructure.

Discovery remains exactly 82/72 and 60/60/60, with zero old-correct loss and no newly unlocked
row. Each direct mode remains 60 correct, 12 preflight, two provider, seven infrastructure, and
one runtime mismatch. The selected prefix passes 427/427 after restoring the unrelated fixture's
original minimum-store contract.

All sixteen representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90. The
new structured-buffer optional gate measures 251.4 ms/590-byte PTX at direct O3 SM70 versus
361.0 ms/8570 bytes through NVRTC O3; the descriptor optional gate measures 253.1 ms/645 bytes
versus 365.2 ms/8584 bytes. Direct O0 measures 247.9 ms/6968 bytes and 248.0 ms/2198 bytes,
respectively. These remain exploratory measurements.

The repository formatter was attempted but this machine does not provide gersemi, clang-format,
prettier, or shfmt. Manual review, `git diff --check`, JSON parsing, and exact TSV row-count checks
all pass.

The result supports the intended design: both producers share resource-placeholder semantics but
retain their established distinct physical representations. Existing generic provider operations
were sufficient, so provider ABI revision 30 did not change. No adjacent default family was
needed or admitted.

## Context and Current Pipeline

For `optional-descriptor-handle.slang`, source `return none` first becomes `IRMakeOptionalNone`.
`OptionalTypeLoweringContext::processMakeOptionalNone` creates one default payload with
`IRBuilder::emitDefaultConstruct`. That builder recursively constructs the `DomeLight` struct but
retains `IRDefaultConstruct` leaves for `DescriptorHandle<Texture2D<float4>>` and
`DescriptorHandle<SamplerState>`. Slice 154's type lowering maps each handle to the exact 64-bit
texture or sampler representation.

For `optional-single-concrete-layout.slang`, type-flow specialization owns the existential shape.
`TypeFlowSpecializationContext::specializeMakeOptionalNone` calls `emitDefaultConstruct` for the
untagged union payload. Final linked IR contains one exact
`defaultConstruct<StructuredBuffer<int>>`, then constructs `FooImpl` and the UInt-tagged tuple.
The buffer's canonical provider value is the raw view produced by `_lowerRawBufferType`: a global
element pointer followed by an i64 count.

`_validateNVVMFunction` currently has no `kIROp_DefaultConstruct` case, so it reports only the
opcode before provider mutation. Once admitted, the ordinary function emitter likewise has no
case. The new resolver must be shared by both boundaries, validate zero operands and the exact
type family, and leave every adjacent default type rejected with a typed diagnostic.

## Scope and Non-Goals

In scope are selected raw structured-buffer defaults, selected texture/sampler descriptor-handle
defaults, exact typed preflight diagnostics, existing-operation emission, focused fake-provider
coverage where practical, real O0/O3 comparison, stable promotion, complete cross-corpus metrics,
and representative PTX measurements.

Out of scope are byte-address default values, bare sampled texture or sampler defaults, writable
surface defaults, arbitrary recursive resource structs, default construction for scalar/copyable
values already lowered elsewhere, generic undefined values, pointer-null source literals,
parameter-block defaults, new provider callbacks, ABI revision, source reconstruction,
fixture-name checks, compatibility fallbacks, and corpus-v2 activation.

## Architecture and Invariants

- One exact resolver owns both preflight and emission classification for selected resource
  `IRDefaultConstruct`; rejected adjacent types receive their exact result type in the diagnostic.
- Raw structured-buffer default construction uses the same physical type contract as
  `_lowerRawBufferType`: field zero is a null pointer in LLVM global address space and field one is
  an i64 zero count.
- A selected texture/sampler descriptor handle defaults to the zero value of its underlying exact
  64-bit resource representation, preserving Slice 154's alias invariant.
- Every provider type and value is created through existing typed operations. No emitted-text
  manipulation or new callback is permitted.
- Frozen corpus v1 remains exactly 452/427 and discovery remains exactly 82/72, with separate
  metrics and zero old-correct regression required.

## Interfaces and Dependencies

Production work is limited to `source/slang/slang-emit-nvvm.cpp` unless exact type-lowering reuse
requires a narrowly scoped compiler-internal helper. Focused fake-provider coverage may touch
`tools/slang-unit-test/unit-test-nvvm-support.h` and
`tools/slang-unit-test/unit-test-nvvm-emitter.cpp`. The two motivating shaders may gain direct O0/O3
lanes only after successful differential execution. Provider ABI revision 30 remains unchanged.

## Milestones

1. Add a shared descriptor for exact zero-operand default construction of a raw structured buffer
   or selected texture/sampler descriptor handle. Reject adjacent types with an exact typed
   diagnostic.
2. Emit an opaque descriptor default as a typed i64 zero. Emit a raw structured-buffer default by
   creating its exact global element-pointer type, bit-casting an i64 zero to that pointer, pairing
   it with an i64 zero count, and constructing the established raw-view type.
3. Build and run focused coverage, then run both frozen targets through native CUDA and direct
   NVVM O0/O3. Promote only stable correct rows and record any exact cascade.
4. Run promoted file directives, the 427-test selected prefix, complete frozen-v1 and discovery
   corpora, and generate separate TSV/JSON/Pareto artifacts.
5. Refresh the representative measurement manifest if a newly unlocked workload adds a useful
   semantic combination; run SM70/80/90 assembly, update documentation, complete the
   input-shape/unprincipled-change audit, and commit Slice 159.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools as required by
`AGENTS.md`. Acceptance requires:

- exact fake or real-provider evidence for both the null raw-view struct and zero descriptor
  handle representations;
- both motivating workloads compare correctly with native CUDA at direct O0 and O3, or any later
  independent blocker is recorded without speculative widening;
- the selected NVVM prefix passes with zero old-correct regression;
- frozen v1 remains exactly 452/427 and discovery remains exactly 82/72, with separate
  O0/O3/both, classifications, Pareto, and newly-unlocked reporting;
- representative direct-O3 PTX assembles for SM70, SM80, and SM90 where practical;
- provider ABI revision remains 30; and
- formatting is attempted, `git diff --check` and JSON/TSV integrity pass, and
  `external/slang-binaries/` remains unstaged.

## Failure and Recovery

If integer-to-global-pointer bit casting fails LLVM verification or provider validation, do not
patch emitted text or invent an integer raw-buffer ABI. Record the provider evidence and revisit
whether a concrete generic null-pointer callback is the one operation the current interface
cannot express. If either workload reaches an independent operation, preserve the narrow valid
default representation and record the new exact producer/type/diagnostic without expanding this
slice. Generated `build/` artifacts are reproducible.

## Artifacts and Hand-Off

Commit this completed plan with the implementation because the user explicitly requires it for
this experiment. Also retain the focused coverage, promoted lanes, Slice 159 frozen/discovery
tables and Pareto JSON, any refreshed measurement manifest, five-part report, and durable
design/ledger updates. Raw IR, logs, PTX, and cubins remain generated under `build/`.
