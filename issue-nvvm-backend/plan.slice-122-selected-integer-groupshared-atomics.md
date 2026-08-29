# Complete selected-integer groupshared storage and atomic-add composition

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM supports canonical uninitialized groupshared Int32/UInt32 scalars
and fixed arrays, including dynamic element addressing, ordinary loads/stores, and relaxed atomic
add. The unchanged `tests/compute/atomics-groupshared.slang` result should pass direct CUDA runtime
and PTX lanes, proving four contending threads, two group barriers, three shared atomic adds, old
value results, and a final structured-buffer write in one real suite fixture.

## Progress

- [x] (2026-08-29) Completed Slice 121 as `012130d3d`; the full NVVM prefix passed 388/388.
- [x] (2026-08-29) Reproduced the fixture's `sequential element pointer` stop and captured its
  optimized final IR.
- [x] (2026-08-29) Replaced signed-i32-specific groupshared classifiers with exact selected-integer scalar,
  fixed-array, and element-pointer contracts.
- [x] (2026-08-29) Extended the shared atomic descriptor catalog/provider/serializer/fakes and compiler resolver
  to relaxed Int32/UInt32 add through only those canonical shared producers.
- [x] (2026-08-29) Added focused positive and adjacent-boundary coverage, promoted the fixture,
  validated runtime/PTX/`ptxas` and the full prefix, and updated durable status.

## Surprises and Discoveries

- The backend already supports dynamic indexing, loads/stores, and synchronization for
  `groupshared int[N]`; an existing focused test also proves UInt indices. The storage and element
  pointer classifiers, however, hard-code the element type to signed Int32.
- The optimized fixture contains one uninitialized `groupshared uint[4]`, three direct
  `getElementPtr` instructions rooted in that global, and three relaxed `atomicAdd` operations.
  All arithmetic, helper transport, barriers, and the final RWStructuredBuffer store are already
  established.
- Slice 121 intentionally made address space an atomic descriptor dimension. Shared add therefore
  expands the catalog rather than changing the interface or adding another callback.
- LLVM represents both Int32 and UInt32 as signless `i32`. The material physical distinction is
  the pointer's shared address space, which must remain derived from its canonical producer.
- The fake provider had modeled `declareGlobalStorage` as returning a usable pointer only for its
  prior shared-array path, even though the builder contract returns a pointer for every admitted
  global declaration. Promoting the scalar probe exposed that inaccurate fake boundary. The fake
  now accepts selected shared scalar/array declarations and recognizes all declared global storage
  results as pointers, matching the real provider rather than special-casing atomic emission.
- FileCheck's `COUNT` cursor advances past its final match. Expressing both barriers as a count
  after the three atomics therefore missed the earlier barrier; ordered checks now state the actual
  barrier, three atomic operations, barrier sequence.

## Decision Log

- Decision: rename and generalize the shared-storage classifiers to selected scalar Int32/UInt32
  rather than adding unsigned variants beside signed-specific helpers.
  Rationale: signedness does not affect size, alignment, LLVM storage, GEP, load, or store. One
  classifier is the source of truth for the exact semantic family.
  Date/author: 2026-08-29, Codex.
- Decision: cover both scalar and fixed-array uninitialized groupshared globals in this slice.
  Rationale: they share the same declaration and atomic contract, and a pre-existing scalar atomic
  probe supplies focused evidence. Leaving scalar storage artificially excluded would preserve a
  producer-shape accident unrelated to the physical representation.
  Date/author: 2026-08-29, Codex.
- Decision: admit only relaxed add for selected integer shared pointers.
  Rationale: this is the operation/order family proven by the measured fixture. Other RMW
  operations and orders remain catalog rows to establish with their own semantic evidence.
  Date/author: 2026-08-29, Codex.
- Decision: resolve shared atomic address space only from the canonical shared scalar global or an
  exact array-element GEP rooted in an admitted shared array global.
  Rationale: a matching pointer type alone should not make arbitrary pointer construction valid;
  producer ownership keeps global/shared inference deterministic before provider discovery.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The direct compiler now has one selected-integer groupshared storage family covering canonical
uninitialized scalar and fixed-array Int32/UInt32 globals. Signed-specific classifier names and
branches were removed. Scalar declarations use the same internal shared global-storage operation;
array elements use the existing sequential-element-pointer operation after an exact base/result
relation check.

The atomic catalog admits relaxed scalar Int32/UInt32 add in global or shared address space without
an ABI change. Compiler resolution derives address space from an admitted scalar shared global or
direct shared-array GEP, and the real provider validates the matching LLVM address space. The
legacy serializer semantically accepts both address spaces while retaining its checked natural
alignment rewrite.

Focused fake tests prove signed scalar shared atomic production and UInt32 shared-array
load/store/indexing. The real builder emits one global and one shared `atomicrmw add` and preserves
invalid descriptor, pointer, type, dominance, and lifecycle rejections. The existing signed shared
memory runtime/PTX group remains green, and floating shared arrays plus other atomic families still
stop before provider mutation.

The unchanged `tests/compute/atomics-groupshared.slang` result `223, 322, 21, 120` passes all five
active fixture lanes including direct CUDA runtime and PTX. Its 1,333-byte PTX declares one 16-byte
shared object, contains three `atom.shared.add.u32` instructions and two ordered barriers, and CUDA
12.9.86 `ptxas -arch=sm_70` emits a 3,168-byte cubin. Release provider and host builds pass, and the
complete NVVM prefix passes 389/389.

The self-review inventory covered the generalized scalar/array/element classifiers, scalar global
declaration branch, availability and GEP relation changes, exact atomic-pointer resolver, catalog
row, serializer condition, fake-provider corrections, and tests. Each survives because it either
defines a canonical producer or consumes the same resolved descriptor. No parallel signed/unsigned
helper, arbitrary typed-pointer acceptance, source intrinsic matching, duplicate support table,
provider-only fallback, unchecked text rewrite, ABI callback, initializer discard, broad shared
aggregate admission, address-space guess, or compatibility shim remains.

## Context and Current Pipeline

Optimized final IR for the groupshared helper is equivalent to:

    groupshared uint shared[4];
    func test(UInt val) -> UInt
    {
        UInt* p0 = getElementPtr(shared, val);
        store(p0, 0);
        workgroupBarrier();
        UInt old0 = atomicAdd(p0, val, Relaxed);
        UInt old1 = atomicAdd(getElementPtr(shared, val ^ 1), val * 16, Relaxed);
        UInt old2 = atomicAdd(getElementPtr(shared, val ^ 2), val * 256, Relaxed);
        workgroupBarrier();
        return load(p0) ^ (old0 + old1 + old2);
    }

The direct pipeline already declares internal shared fixed-i32 arrays, lowers their dynamic GEPs
through the generic sequential-element pointer callback, loads/stores their elements, and emits
workgroup barriers. Slice 121's queried atomic interface already carries address space. This slice
joins those established pieces without expanding the construction API.

## Scope and Non-Goals

In scope are selected uninitialized groupshared Int32/UInt32 scalar and nonempty fixed-array
globals; exact array-element pointers; relaxed shared Int32/UInt32 add; descriptor-based compiler
preflight and provider emission; legacy textual serialization; real/fake provider tests; migration
of the focused scalar shared probe; runtime/PTX promotion of `atomics-groupshared.slang`; durable
design status; and this plan.

Out of scope are initialized or externally linked shared globals, floating/vector/aggregate/shared
storage, multidimensional shared arrays, pointer escape or helper pointer ABI, shared pointer phi,
atomic operations other than add, stronger orders, explicit scopes, volatile atomics, 64-bit and
floating atomics, local/generic/constant address spaces, compare-exchange, byte-address/surface
atomics, compatibility aliases, and source-text or intrinsic-name recognition.

## Architecture and Invariants

- A canonical shared global is uninitialized, module-owned, GroupShared-rate storage containing
  either scalar Int32/UInt32 or a nonempty fixed array of exactly that selected scalar.
- A shared array element pointer is accepted only when its GEP base is that global, its result is
  read-write GroupShared scalar-buffer-layout storage, and its pointee exactly equals the array
  element type.
- Shared scalar globals and exact shared array element GEPs are the only new atomic pointer
  producers. The compiler derives `SLANG_NVVM_ADDRESS_SPACE_SHARED` from that producer.
- The semantic catalog accepts relaxed add for scalar Int32/UInt32 in global or shared address
  space. Every other descriptor dimension remains rejected before module mutation.
- Real provider emission checks the descriptor against the actual LLVM pointer address space and
  pointee/value width, then emits naturally aligned monotonic `atomicrmw add`.
- The LLVM 7 compatibility serializer validates every global/shared atomic semantically before
  removing LLVM 14's explicit natural-alignment suffix.

## Interfaces and Dependencies

Expected committed areas are the direct type classifier/emitter, shared atomic semantic catalog,
LLVM provider/serializer, fake provider, builder/emitter/integration tests, the existing compute
fixture, durable design status, and this plan. Builder ABI revision 21 and the public Slang/source
APIs remain unchanged. CUDA 12.9 runtime and `ptxas` provide real evidence.

## Milestones

1. Generalize exact shared scalar/array/pointer classification and every associated name,
   availability, global validation, type-lowering, declaration, GEP, load, and store use.
2. Add the shared descriptor rows, derive address space from canonical atomic pointer producers,
   and extend provider and compatibility validation without changing the interface.
3. Move the focused shared scalar probe from negative to positive coverage; expand builder/fake
   tests for shared signed/unsigned rows and mismatched physical address space.
4. Promote `atomics-groupshared.slang`, inspect and assemble PTX, run Release builds and the full
   NVVM prefix, update docs/plan, format, perform the input-shape audit, and commit.

## Validation and Acceptance

Acceptance requires Release provider and host builds; focused classification/emitter tests for
signed and unsigned scalar/array shared storage; builder support/emission tests for both shared
descriptor signedness rows; preserved rejection of floating/wide/vector/initialized/non-shared
storage, invalid pointer relations, other operations/orders/address spaces, and pointer/value
mismatches; unchanged direct runtime/PTX for `atomics-groupshared.slang`; CUDA 12.9
`ptxas -arch=sm_70`; the complete `slang-unit-test-tool/nvvm` prefix; pinned formatting; and
`git diff --check`.

The self-review inventories every renamed/generalized classifier, scalar-global branch,
array-element relation, descriptor catalog row, serializer condition, fake/provider change, and
test migration. For each, record the exact producer and failing test. Remove any parallel signed
and unsigned helpers, arbitrary typed-pointer acceptance, source intrinsic matching, duplicated
support table, provider-only fallback, unchecked text rewrite, new ABI callback, initializer
discard, broad shared aggregate admission, address-space guess, or compatibility shim.

## Failure and Recovery

If libNVVM rejects semantically valid shared LLVM atomic IR, preserve LLVM 14/LLVM 7 assembly,
PTX, cubin, and logs under ignored `build/slice122-*`; compare the exact address-space spelling,
sync scope, and alignment with NVRTC before changing the catalog. Do not force global address
space, rewrite arbitrary text, patch the source fixture, restore signed-only helpers, widen other
operations/orders/types, reset unrelated work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, LLVM text, PTX, cubin, and logs under ignored `build/slice122-*`. Distill the
selected shared-storage/atomic contract, exact evidence, and next measured corpus boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly
requested.
