# Generalize raw-buffer views and data-pointer extraction

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM supports the canonical data-pointer path shared by selected
`StructuredBuffer<T>`, `RWStructuredBuffer<T>`, `ByteAddressBuffer`, and `RWByteAddressBuffer`
views. `__getStructuredBufferPtr` and `__getByteAddressBufferPtr` extract the view's existing data
pointer, constant or dynamic indexing composes through the ordinary pointer builder operations,
and scalar loads/stores retain the source access contract.

The existing `tests/cuda/get-buffer-ptr.slang` must pass its CUDA/NVRTC lane and a new direct
libNVVM GPU-comparison lane with the eight expected values `11, 21, 31, 41, 102, 202, 302, 402`.
Its direct PTX must pass CUDA 12.9 `ptxas -arch=sm_70`. No buffer-specific callback is added to the
LLVM builder ABI.

## Progress

- [x] (2026-08-29) Completed and committed Slice 81 as `d57f4bc34`.
- [x] (2026-08-29) Reproduced the existing shader's pre-provider E52017 boundary at
  `conventional global parameter field address`.
- [x] (2026-08-29) Captured final linked IR: three 16-byte buffer views feed canonical
  `getStructuredBufferPtr`/`getUntypedBufferPtr`, unsized-array `getElementPtr`, scalar loads, and
  the established output stores.
- [x] (2026-08-29) Consolidated structured and byte-address resources under one exact raw-buffer
  view contract without changing builder ABI revision 7.
- [x] (2026-08-29) Added exact raw-buffer data-pointer type, producer/consumer shape, SSA,
  immutable-root, and generic field-extraction/pointer-offset emission support.
- [x] (2026-08-29) Registered four focused fake/negative tests and the existing file's direct GPU,
  PTX, and CUDA 12.9 `ptxas` evidence.
- [x] (2026-08-29) Formatted with pinned clang-format 17; built the Release host and isolated
  provider; passed focused coverage 7/7 and the complete NVVM prefix 351/351; updated durable
  documents; and completed the pre-commit self-review.

## Surprises and Discoveries

- The existing shader does not lower `__getByteAddressBufferPtr` to a byte-offset intrinsic. The
  final linked IR preserves `getUntypedBufferPtr(RWByteAddressBuffer)` with result
  `Ptr<UnsizedArray<uint>, ReadWrite, Device, DefaultLayout>`, followed by ordinary
  `getElementPtr` and `load`.
- `__getStructuredBufferPtr` has the same graph with the resource's selected element type. Both
  operations expose field zero of the already-established `{ T addrspace(1)*, i64 }` view. The
  provider therefore has every necessary structural primitive today.
- The first failure is the byte-address field in the conventional parameter struct, but admitting
  that field alone would immediately expose the two data-pointer instructions and unsized-array
  addressing. These are one representation family and should be implemented together.
- The shared `isPointerToImmutableLocation` helper already forwards through
  `getStructuredBufferPtr`, but not `getUntypedBufferPtr`. A read-only byte-address data pointer
  should preserve the same canonical read-only-buffer classification.
- The real provider rejected the first element-address attempt with E52018
  `raw buffer scalar element pointer`. `emitArrayElementPointer` requires a physical pointer to a
  fixed LLVM array and intentionally emits a leading-zero array GEP. Resource field zero is already
  a physical scalar pointer, so the existing generic pointer-offset operation is the exact match.
- Both read-only and read-write core intrinsic overloads return an ordinary read-write-qualified
  `Ptr<UnsizedArray<T>>`. The resource producer, not the pointer type, is the semantic source of
  read-only access. The shared immutable-root classifier preserves that fact for loads; direct
  preflight must separately reject stores rooted in an immutable resource.

## Decision Log

- Decision: replace the structured-buffer-only resource classifier with one raw-buffer descriptor
  carrying the canonical type, selected physical element type, and read-only/read-write access.
  Rationale: structured and byte-address buffers have the same CUDA ABI and differ only in whether
  the element comes from the generic argument or is fixed to `uint`. Keeping parallel lowering
  paths would duplicate the physical representation and access policy.
  Date/author: 2026-08-29, Codex.
- Decision: represent a canonical pointer to an unsized buffer array as the resource's existing
  global element pointer, not as an LLVM pointer to an unsized aggregate.
  Rationale: final IR uses the array pointer only as the base of element addressing. Field zero is
  already exactly `T addrspace(1)*`, and generic pointer offset/GEP preserves the intended
  element-index semantics without inventing a runtime array object.
  Date/author: 2026-08-29, Codex.
- Decision: lower both `getStructuredBufferPtr` and `getUntypedBufferPtr` by generic struct field
  extraction, then lower their `getElementPtr` consumers through the existing generic pointer-offset
  operation.
  Rationale: the canonical instructions are the semantic source of truth, while the builder owns
  only LLVM aggregate and pointer construction. A resource-specific callback would expose Slang
  types without adding an LLVM capability.
  Date/author: 2026-08-29, Codex.
- Decision: keep direct `ByteAddressBuffer.Load/Store`, rasterizer-ordered buffers, atomics, size
  queries, and arbitrary typed reinterpretation outside this slice.
  Rationale: the motivating graph explicitly requests the pointer escape API. Direct buffer
  operations have different byte-offset/alignment semantics and deserve their own measured
  family rather than silently riding on element indexing.
  Date/author: 2026-08-29, Codex.
- Decision: require the core intrinsic's exact canonical read-write-qualified pointer spelling but
  keep access policy solely on the raw-buffer descriptor.
  Rationale: copying the producer's access into a second pointer-access field rejected valid
  read-only core IR and invented an invariant that the producer does not establish. Following the
  producer lets immutable loads reuse the shared classifier and lets preflight reject immutable
  stores without changing the core IR or provider ABI.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The slice admits the complete existing buffer-pointer shader without a provider ABI change.
Structured and byte-address views now share one type-lowering path; both pointer-producing
intrinsics use generic field extraction; and direct unsized-array indexing maps to the already
generic scalar pointer-offset operation. The fake trace observes three field-zero extractions,
three pointer offsets, two loads, one store, and no fixed-array addressing.

The access audit corrected the plan's initial assumption. Read-only source resources do not produce
read-qualified pointers in canonical IR. Access remains a property of the resource root: a
read-only byte-address load is invariant, while a store through the escaped pointer is rejected
before builder discovery. Direct byte-address load keeps its separate E52017 boundary.

The existing fixture passes CUDA/NVRTC, direct GPU comparison, and direct PTX 3/3 with all eight
expected values. CUDA 12.9 `ptxas` accepts that PTX and a separate read-only module. The formatted
focused unit/file set passes 7/7, the full NVVM prefix passes 351/351, and both Release build trees
are clean.

## Context and Current Pipeline

Consider the existing code:

```slang
let sptr = __getStructuredBufferPtr(inputBuffer);
outputBuffer[tid.x] = (*sptr)[tid.x] + 1;

let bptr = __getByteAddressBufferPtr(inputBytes);
outputBuffer[tid.x + 4] = int((*bptr)[tid.x]) + 2;
```

CUDA varying legalization replaces `tid` with the established `blockIdx * blockDim + threadIdx`
vector. Conventional-global collection produces a synthesized 48-byte struct with fields
`RWStructuredBuffer<int>`, `RWByteAddressBuffer`, and `RWStructuredBuffer<int>`, each occupying the
established 16-byte pointer/count ABI.

The final body loads each resource view from its keyed field. `getStructuredBufferPtr` returns a
device pointer to `UnsizedArray<int>`; `getUntypedBufferPtr` returns the analogous `uint` pointer.
Each is consumed by `getElementPtr(index)` and a scalar load. Output addressing already uses the
established read-write structured-buffer path. The direct validator rejects the byte-address field
before builder discovery because `_getNVVMStructFieldAddress` and the type-lowering role classifier
currently know only structured-buffer views.

## Scope and Non-Goals

In scope are exact default-layout `StructuredBuffer<T>` and `RWStructuredBuffer<T>` for selected
integer/float32 scalar `T`; exact `ByteAddressBuffer` and `RWByteAddressBuffer` with physical
`uint`; their conventional-global and raw-entry value/storage roles; canonical
`IRGetStructuredBufferPtr` and `IRGetUntypedBufferPtr`; and canonical `IRGetElementPtr` from the
resulting unsized-array pointer to an exact scalar element pointer. The resource producer remains
the source of access semantics; loads rooted in read-only resources are invariant, and stores
rooted in those resources remain rejected.

Out of scope are rasterizer-ordered byte-address buffers; direct `IRByteAddressBufferLoad` and
`IRByteAddressBufferStore`; byte offsets and explicit alignment; atomics; dimensions; bounds;
descriptor casts; structured/byte buffer reinterpretation; non-default structured-buffer layouts;
non-selected element types; pointer escape into helper ABI, phis, aggregates, or memory; and general
unsized-array values or pointers not produced by the two canonical buffer-data operations.

## Architecture and Invariants

One raw-buffer descriptor recognizes only exact admitted canonical types. It reports the physical
scalar element and access policy. Structured buffers take the element from their canonical generic
argument and require `DefaultLayout`; byte-address buffers have no operands and use canonical
`UInt`. All lower to the same unpacked `{ T addrspace(1)*, i64 }` provider struct.

A buffer-data pointer descriptor recognizes only the core intrinsic's exact
`Ptr<UnsizedArray<T>, ReadWrite, UserPointer, DefaultLayout>` spelling whose `T` is selected. Shape
validation additionally requires its sole producer to be `getStructuredBufferPtr` or
`getUntypedBufferPtr`, requires the source buffer descriptor's physical element to match the
result, and requires exact result identity. The next `getElementPtr` must preserve element type,
pointer access, address space, and scalar-buffer layout. The raw-buffer descriptor independently
owns read-only/read-write resource semantics.

Emission extracts field zero from the lowered resource view. The canonical unsized-array pointer
is represented physically by that exact `T addrspace(1)*`. Element addressing uses the existing
generic provider pointer-offset GEP. The source qualifier remains a host validation contract because LLVM pointer
types do not encode Slang read-only access. Immutable-load metadata continues to come from the
shared root-address classifier.

## Interfaces and Dependencies

Update `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` to define the common raw-buffer and
buffer-data-pointer descriptors, replace the structured-only classifier, and lower both canonical
families through existing provider types. Update `source/slang/slang-emit-nvvm.cpp` to validate and
emit the two pointer producers plus their exact element-address relation.

Update `source/slang/slang-ir-util.cpp` only if the input-shape audit confirms that
`getUntypedBufferPtr` is the missing forwarding case for the existing immutable-location semantic
classifier. This is shared semantic ownership, not an NVVM-only guess.

The builder remains exact ABI revision 7. The real provider and facade should require no new
operation. Extend the fake provider only where it must distinguish or validate the already-generic
type/value topology, then add one focused emitter trace and one pre-provider adjacent-boundary test.

## Milestones

1. Consolidate the raw resource representation and add exact byte-address storage/value support
   without changing ABI revision 7.
2. Add exact buffer-data-pointer and unsized-array element-address validation, SSA availability,
   type lowering, and generic emission.
3. Prove fake-boundary topology and retain a direct byte-address operation as deterministic E52017
   before provider discovery.
4. Register `tests/cuda/get-buffer-ptr.slang` for direct GPU comparison and static PTX, inspect the
   output, and run CUDA 12.9 `ptxas`.
5. Run focused and complete NVVM tests, update durable design/ledger status, perform the helper and
   input-shape self-review, and commit this plan with the implementation.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox. Acceptance requires:

- structured and byte-address views share one exact provider representation and access model;
- data-pointer source/result element, canonical pointer access, layout, and address-space
  mismatches stop before
  provider discovery;
- fake evidence records field-zero extraction and generic pointer offsetting for both pointer
  producers, with no resource-specific builder callback;
- read-only data pointers reject stores and preserve immutable-location classification;
- direct ByteAddressBuffer load/store operations and other non-goals remain deterministic E52017;
- the existing shader passes CUDA/NVRTC and direct GPU comparison with all eight expected values;
- direct PTX contains a 48-byte conventional parameter block, three pointer loads, scalar global
  loads/stores, and passes CUDA 12.9 `ptxas -arch=sm_70`;
- standalone provider and Release host/test builds pass;
- focused tests and the complete `slang-unit-test-tool/nvvm` prefix pass;
- pinned clang-format 17 and `git diff --check` pass; and
- `external/slang-binaries/` and generated `build/` artifacts remain unstaged.

## Failure and Recovery

If libNVVM rejects the physical data pointer, serialize normal and LLVM 7-compatible text and
compare the extracted field-zero pointer plus GEP against the already-working structured-buffer
element path. Do not introduce an LLVM unsized-array value or a resource-shaped builder callback.
If runtime output differs, compare the exact source/result access and element-index graph before
changing layout or byte scaling: this source indexes `uint` elements after explicitly requesting
the byte buffer's `uint[]` pointer.

All type/emitter/fake/test/document changes are one forward-only slice and can be reverted together.
No builder ABI migration or generated dependency change is expected.

## Artifacts and Hand-Off

Retain final linked IR, normal and LLVM 7-compatible assembly, direct/NVRTC PTX, CUDA runtime
comparison output, and `ptxas` output under ignored `build/` paths. Distill the raw-buffer
representation, pointer producer/consumer contract, immutable-load decision, existing-file result,
remaining byte-address boundaries, and exact validation totals into `docs/design/nvvm-backend.md`
and `docs/design/nvvm-backend-capability-ledger.md` before committing Slice 82.
