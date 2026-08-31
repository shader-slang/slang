# Preserve structured-buffer storage pointers through subobject addressing

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM accepts the canonical pointer chain used for structured-buffer
elements whose storage is a vector, fixed array, or physical matrix wrapper. A physical storage
element may retain Slang `AddressSpace::StorageBuffer`, while a typed vector lane selected from an
ordinary resource element remains a generic pointer. Frozen `bugs/gh-5776.slang` and discovery
`bugs/parameter-block-load.slang` are the bounded end-to-end targets and must execute correctly at
direct O0 and O3 before promotion.

## Progress

- [x] (2026-08-31) Completed and committed Slice 161 as `7f4701577`; ranked remaining cross-corpus
  failures without treating the heterogeneous helper-ABI bucket as one root cause.
- [x] (2026-08-31) Dumped final IR for both targets and identified the exact resource-element and
  sequential-lane producers.
- [x] (2026-08-31) Defined producer-owned resource element pointer and subobject contracts without admitting
  arbitrary address-space spellings.
- [x] (2026-08-31) Implemented with existing generic pointer operations and no provider ABI revision.
- [x] (2026-08-31) Built, probed both targets at O0/O3, recorded cascades, and promoted only stable correct rows.
- [x] (2026-08-31) Ran the selected prefix, exact frozen/discovery corpora, measurements, formatting, integrity
  checks, and self-review.
- [x] (2026-08-31) Completed durable documentation and the five-part report; commit Slice 162 after the final staged audit.

## Surprises and Discoveries

- The nominal frozen `helper-abi-type-contract` cluster contains unrelated substandard-float,
  advanced-wave-context, and append/consume-buffer shapes. It is not one reusable invariant.
- The `aggregate-struct-field-pointer` diagnostic is downstream of all-or-nothing conventional
  global-block recognition. In the frozen example, a half-bearing constant-buffer sibling makes
  the block unsupported; widening field addressing would only mask the real storage-layout gap.
- Final `gh-5776` IR lowers ordinary array and vector structured-buffer loads directly. Its first
  rejected pointer is the physical matrix storage wrapper produced by
  `IRRWStructuredBufferGetElementPtr`; address-space specialization deliberately preserves
  `AddressSpace::StorageBuffer` on that pointer.
- Final `parameter-block-load` IR produces a generic pointer to `uint3` through
  `IRRWStructuredBufferGetElementPtr`, followed by `IRGetElementPtr` to the selected `uint` lane.
  The existing sequential resolver supports local/parameter-group arrays and vectors but does not
  recognize a resource-element pointer as a canonical vector root.
- The first `gh-5776` probe failed before the producer resolver because the physical matrix result
  retained `AddressSpace::StorageBuffer`. Once that exact spelling was admitted, the existing raw
  pointer relation still required read/write access even though the producer is also canonical for
  read-only physical structured-buffer storage.
- The first `parameter-block-load` probe then reached the resource pointer consumer check. Its
  exact child `IRGetElementPtr` is the canonical vector-lane address producer, not an arbitrary
  escape, so the shared sequential resolver must participate in the consumer proof.
- After consolidating these checks in one exact structured-buffer element resolver, both workloads
  compiled and executed correctly at O0 and O3 without a provider change.
- The repository formatting driver ran but could not format because gersemi, clang-format,
  prettier, and shfmt are not installed on this machine. Manual diff review and
  `git diff --check` remain clean.

## Decision Log

- Decision: make Slice 162 a structured-buffer subobject slice rather than a general aggregate
  field-address widening.
  Rationale: the two selected workloads share an exact resource producer and pointer composition
  invariant. The field-address rows are symptoms of distinct unsupported sibling layouts.
  Date/author: 2026-08-31, Codex.
- Decision: treat `AddressSpace::StorageBuffer` as a valid Slang spelling only for an exact
  `IRRWStructuredBufferGetElementPtr` result whose resource and pointee relation is proved.
  Rationale: final address-space specialization owns that spelling for physical storage elements;
  direct NVVM maps the resource data pointer to LLVM global memory. Arbitrary storage-buffer
  pointers remain outside the contract.
  Date/author: 2026-08-31, Codex.
- Decision: compose sequential vector/array addressing only from the admitted resource-element
  producer.
  Rationale: `IRGetElementPtr` is canonical for mutable `buffer[i].lane`; the existing generic
  sequential pointer operation already expresses the typed GEP after the root is proved.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Direct NVVM now resolves `IRRWStructuredBufferGetElementPtr` once, proving the selected structured
buffer kind, access, i32 index, result pointer contract, and exact resource element type. Only that
producer may use the final-IR generic or storage-buffer pointer spellings. Exact array/vector child
GEPs reuse the established sequential pointer operation and retain the resource root's immutable
property.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references and improves to 391 O0, 395
O3, and 391 both-mode correct. Discovery remains exactly 82/72 and improves to 64/64/64. The only
newly correct rows are `bugs/gh-5776.slang#cuda-1` and
`bugs/parameter-block-load.slang#discovery-1`; both corpora have zero old-correct loss. The selected
NVVM prefix passes 428/428.

All 21 representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90. The new
physical matrix gate measures 245.7 ms and 1004-byte PTX at direct O3 SM70 versus 356.9 ms and
9438 bytes through NVRTC O3; direct O0 measures 240.7 ms and emits 8638-byte PTX. The vector
subobject gate measures 231.3 ms and 851-byte PTX versus 342.4 ms and 8858 bytes; direct O0 measures
228.5 ms and emits 2207-byte PTX. These timings remain exploratory. Provider ABI revision 30 and
both corpus identities are unchanged.

## Context and Current Pipeline

`lowerBufferElementType` and address-space specialization preserve physical structured-buffer
storage. In `gh-5776`, a `float2x2` becomes a physical struct containing a fixed array of two
`float2` rows. The canonical element instruction is:

    Ptr<_MatrixStorage_float2x2_ColMajornatural,
        addressSpace=StorageBuffer,
        layout=ScalarLayout> = rwstructuredBufferGetElementPtr(...)

`asNVVMSupportedRWStructuredBufferElementPointerType` already proves the recursively selected
element type but currently requires `AddressSpace::Generic`. Type lowering subsequently maps every
accepted resource element pointer to an LLVM global pointer with the structured-buffer storage
role.

In `parameter-block-load`, final IR contains:

    Ptr<uint3, addressSpace=Generic, layout=ScalarLayout> =
        rwstructuredBufferGetElementPtr(buffer, 0);
    Ptr<uint, addressSpace=Generic, layout=ScalarLayout> = getElementPtr(element, 0);

`_getNVVMSequentialElementPointer` recognizes vector roots from local numeric pointers, field
addresses, and earlier sequential elements, but not the exact resource-element producer. Emission
already uses `emitSequentialElementPointer` once this resolver accepts the shape.

## Scope and Non-Goals

In scope are exact RW structured-buffer element producers; generic and storage-buffer Slang
address-space spellings selected by final IR; recursively supported structured-buffer storage
elements; vector/fixed-array subobject pointer composition when the exact element type and access
match; existing LLVM-global resource lowering; both target workloads; permanent lanes after
differential correctness; both corpus snapshots; representative measurements; and durable
documentation.

Out of scope are arbitrary `AddressSpace::StorageBuffer` parameters or values, address-space casts,
raw buffer reinterpretation, half constant-buffer layout, unsupported conventional-global
siblings, resource arrays, append/consume buffers, immutable structured-buffer stores, pointer to
pointer ABI, new provider callbacks, provider ABI changes, fixture-name checks, source
reconstruction, and corpus-v2 activation.

## Architecture and Invariants

- Resource provenance comes from the exact `IRRWStructuredBufferGetElementPtr` producer and its
  selected raw-buffer operand; the result type alone is not sufficient.
- Final Slang generic and storage-buffer address spaces both map to the established LLVM global
  resource pointer, but no other producer acquires this equivalence.
- A sequential subobject pointer preserves the resource root's mutability and lowered address
  space, requires an i32 index, and names exactly one array/vector element type.
- Structured-buffer storage/value conversion remains owned by the existing load path; this slice
  does not reinterpret physical storage as an ordinary value by pointer type alone.
- Provider ABI revision 30 and both corpus identities remain unchanged.

## Interfaces and Dependencies

Production changes are expected in `source/slang/slang-emit-nvvm-type-lowering.cpp` and
`source/slang/slang-emit-nvvm.cpp`. Focused fake tests should exercise both address-space spelling
and resource-rooted sequential GEP using the existing typed provider calls. The two real workloads
gain direct O0/O3 lanes only after runtime comparison succeeds. No external interface changes are
planned.

## Milestones

1. Replace type-only resource-element admission at validation sites with an exact producer
   resolver, or otherwise prove the producer before permitting storage-buffer address space.
2. Extend sequential aggregate addressing from that producer for exact vector/fixed-array
   subobjects, preserving access and expected pointee identity.
3. Build and probe both targets at O0/O3. Record the next first unsupported shape if either target
   exposes an independent cascade.
4. Add focused positive/negative coverage, promote correct targets, run the prefix and both exact
   corpora, refresh representative measurements, and finish the input-shape audit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools. Acceptance requires
focused fake coverage; O0/O3 differential correctness for every promoted target; zero old-correct
regression; the selected NVVM prefix; frozen identity 452/427; discovery identity 82/72; direct-O3
PTX assembly for the 19 established and any new representative gates at SM70, SM80, and SM90;
provider ABI revision 30; formatting attempt; `git diff --check`; JSON/TSV integrity; and an exact
staged-file audit excluding `external/slang-binaries/`.

## Failure and Recovery

If accepting the exact resource root reaches a storage/value conversion, helper ABI, or unrelated
operation, retain only independently proved producer support and do not count that workload as
unlocked. If the provider cannot express the typed GEP with its existing global pointer, stop at
that concrete operation rather than adding an address-space fallback. Generated IR/PTX and probe
logs remain under ignored `build/nvvm-census` paths.

## Artifacts and Hand-Off

Retain the completed plan with the implementation under the user's established experimental
workflow exception. Keep Slice 162 frozen/discovery TSV and Pareto JSON, any refreshed measurement
manifest, the five-part report, promoted lanes, and design/ledger updates. Raw dumps and logs stay
under `build/`.
