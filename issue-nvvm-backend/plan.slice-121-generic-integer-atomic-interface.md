# Generalize integer atomics and promote the structured-buffer corpus fixture

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct-NVVM builder has one descriptor-driven atomic-operation interface
instead of the construction API's hard-coded relaxed global signed-i32 add callback. The first
admitted descriptor family is relaxed global Int32/UInt32 add. The existing
`tests/compute/atomics.slang` fixture should pass direct runtime with its unchanged four-thread
result and a direct PTX lane, proving that canonical atomics rooted in writable
`RWStructuredBuffer<uint>` elements compose with helpers and contention.

## Progress

- [x] (2026-08-29) Completed Slice 120 as `fc4185fa8`; the full NVVM prefix passed 387/387.
- [x] (2026-08-29) Reproduced E52017 for `atomics.slang` and captured its optimized final IR.
- [x] (2026-08-29) Added one exact, forward-only atomic operation descriptor/interface and removed the bespoke
  construction callback and wrapper.
- [x] (2026-08-29) Resolved canonical relaxed global Int32/UInt32 adds through one compiler catalog and validated
  provider availability, pointer/value dominance, and emission from that same descriptor.
- [x] (2026-08-29) Generalized the LLVM 14 provider, LLVM 7-era compatibility serializer, fake provider, and
  focused builder/emitter coverage for the admitted family and adjacent rejection boundaries.
- [x] (2026-08-29) Promoted the existing fixture, validated runtime/PTX/`ptxas` and the full NVVM prefix, updated
  durable status, format, self-review, and commit.

## Surprises and Discoveries

- The optimized corpus fixture contains one ordinary UInt helper parameter and three
  `atomicAdd(pointer, value, Relaxed)` instructions. Each pointer is produced directly by
  `rwstructuredBufferGetElementPtr` from the already accepted global parameter-group field. There
  is no new resource, helper, control-flow, or address calculation shape.
- The current failure text says signed i32 because direct preflight hard-codes the result type even
  though LLVM integer types are signless and the provider operation itself is equally valid for
  Int32 and UInt32 add.
- Groupshared atomics first stop at a separate shared-array sequential-pointer boundary. They do
  not belong in the same proof merely because the eventual leaf operation is also atomic.
- The existing builder API puts one atomic spelling in the construction table. Keeping that shape
  would require another callback for every operation/type/address-space/order combination and
  would repeat the bring-up pattern that the generic value, surface, and texture interfaces
  already replaced.
- Migrating the established signed-i32 device-pointer test exposed `IRParam` as another canonical
  actual-device-pointer producer alongside `IRGlobalVar`. This is part of the existing raw CUDA
  pointer ABI, so the resolver admits both exact producers rather than special-casing the test or
  accepting arbitrary global-address-space pointers.
- LLVM integer types are signless. Int32 and UInt32 require distinct semantic catalog rows, but
  both deliberately serialize as the same LLVM 7-compatible `atomicrmw add i32` instruction and
  the direct PTX/runtime evidence confirms their intended behavior.

## Decision Log

- Decision: add one `SlangNVVMAtomicOperationDesc` containing operation, integer value type,
  physical address space, and memory order, exposed through an atomic operation interface with
  `isOperationSupported` and `emitOperation`.
  Rationale: these are the independent semantic dimensions the compiler and provider must agree
  on. A queried interface gives preflight and emission one source of truth and can grow supported
  catalog rows without adding callbacks.
  Date/author: 2026-08-29, Codex.
- Decision: remove `emitRelaxedGlobalI32AtomicAdd` from the construction table and wrapper rather
  than retaining a compatibility shim.
  Rationale: the backend is forward-only, and two APIs for the same canonical operation would
  preserve unnecessary tests and create competing behavior.
  Date/author: 2026-08-29, Codex.
- Decision: admit only relaxed global scalar Int32/UInt32 add in this slice even though the
  descriptor enumerates the coherent RMW operation dimensions.
  Rationale: the measured real fixture proves this exact family. Wider types, min/max signedness,
  exchange, compare-exchange, shared memory, and stronger orders need their own runtime and
  compatibility evidence but will not need another interface redesign.
  Date/author: 2026-08-29, Codex.
- Decision: derive the physical global address space from an already accepted writable pointer
  producer, not from source resource syntax or a provider guess.
  Rationale: actual device globals and legalized raw-resource element pointers can have different
  IR spellings while both lower to LLVM global address space. Producer-aware pointer validation is
  the existing canonical boundary.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Builder ABI revision 21 now exposes a required queried atomic-operation interface. The old
`emitRelaxedGlobalI32AtomicAdd` callback and wrapper were deleted without an alias. One exact
descriptor flows through compiler requirement collection, provider support preflight,
availability checks, real/fake emission, and builder validation. The catalog currently accepts
only relaxed global scalar Int32/UInt32 add.

The established signed actual-device-pointer tests pass through both `IRParam` and `IRGlobalVar`
producers, and focused unsigned coverage records the direct writable structured-buffer element
pointer. Adjacent subtract, wide/vector/floating values, shared memory, stronger order, malformed
enum, mismatched pointer/value, foreign-module, unavailable-value, and provider-failure cases all
stop at their owning boundary.

The Release isolated provider and host unit-test tool build successfully. The unchanged
`tests/compute/atomics.slang` result passes direct runtime, its 880-byte PTX contains exactly three
`atom.global.add.u32` instructions, and CUDA 12.9.86 `ptxas -arch=sm_70` emits a 2,920-byte cubin.
The full NVVM unit-test prefix passes 388/388.

The self-review inventory covered the descriptor/catalog, queried interface, exact pointer
resolver, real/fake provider implementations, compatibility serializer, and migrated tests. Each
survives: all consume the same semantic descriptor or validate a canonical producer. No source
intrinsic matching, arbitrary pointer inference, signed physical LLVM type, duplicated support
table, provider-only fallback, unchecked text rewrite, compatibility shim, shared-memory leak, or
memory-order widening remains.

## Context and Current Pipeline

Optimized final IR for the helper is equivalent to:

    func test(UInt val)
    {
        atomicAdd(rwstructuredBufferGetElementPtr(outputBuffer, val), val, Relaxed);
        atomicAdd(rwstructuredBufferGetElementPtr(outputBuffer, val ^ 1), val * 16, Relaxed);
        atomicAdd(rwstructuredBufferGetElementPtr(outputBuffer, val ^ 2), val * 256, Relaxed);
    }

Four threads call this helper. Resource parameter-group loading, raw-view extraction, element
addressing, UInt arithmetic, calls, and launch indexing are already established. The only missing
semantic leaf is UInt32 atomic add through the same physical global-pointer family used by the
existing signed-i32 actual-global test.

## Scope and Non-Goals

In scope are a forward-only builder ABI revision; a generic atomic operation enum/descriptor and
queried interface; removal of the old callback/wrapper; selected relaxed global Int32/UInt32 add;
canonical writable pointer-producer resolution; provider query/emission; LLVM 7 textual
compatibility; fake/provider validation; migration of existing signed-i32 atomic tests; focused
UInt32 resource evidence; and runtime/PTX promotion of `atomics.slang`.

Out of scope are atomic load/store, subtract, bitwise, min/max, exchange, compare-exchange,
increment/decrement, Float atomics, 64-bit atomics, shared/local/generic/constant address spaces,
stronger memory orders, explicit scopes, volatile operations, resource values passed through
helpers, groupshared array addressing/barriers, byte-address helper ABI, surface atomics, a
compatibility alias for the old callback, and LLVM source-text recognition outside the isolated
compatibility serializer.

## Architecture and Invariants

- One descriptor completely identifies an atomic overload; invalid enum, type, lane count,
  address-space, or order values fail before provider mutation.
- The semantic catalog accepts exactly relaxed global scalar Int32/UInt32 add for this slice.
- Compiler structural preflight, provider capability preflight, availability validation, and
  emission resolve the same descriptor.
- The destination must be an exact writable pointer producer whose physical provider address
  space is global and whose pointee exactly equals the atomic result/value type.
- The memory-order operand is a canonical executable literal and is descriptor metadata, not a
  provider SSA operand.
- LLVM provider emission validates the descriptor against the actual typed pointer and value and
  emits a naturally aligned monotonic `atomicrmw add`.
- LLVM 14-to-LLVM 7 text normalization is semantic-instruction driven and verifies every rewritten
  atomic rather than matching arbitrary source text.

## Interfaces and Dependencies

Expected committed areas are the builder C ABI, wrapper, semantic catalog, direct emitter, LLVM
provider and serializer, fake provider, builder/emitter/integration tests, the existing compute
fixture, durable design status, and this plan. No source-language API or public Slang API changes.
CUDA 12.9 runtime and `ptxas` provide real semantic and module-acceptance evidence.

## Milestones

1. Define the exact atomic descriptor/interface, revise the ABI, remove the old callback, and
   migrate builder initialization/wrapping tests.
2. Add catalog validation and provider implementation for relaxed global Int32/UInt32 add,
   including strict handle/type/address-space/order checks and compatibility serialization.
3. Resolve canonical atomic IR and writable global pointer producers once in the emitter, then use
   that result for shape requirements, provider availability, dominance, and emission.
4. Migrate the signed-i32 actual-global coverage, add focused UInt32 structured-buffer evidence and
   adjacent invalid-descriptor/type/order/address-space tests.
5. Promote `atomics.slang`, run exact runtime/PTX lanes, inspect/assemble PTX, run full builds and
   tests, document, format, self-review, and commit.

## Validation and Acceptance

Acceptance requires Release provider and host builds; builder rejection tests for invalid atomic
enum/type/lane/address-space/order, wrong handles, mismatched pointer/value types, foreign modules,
null outputs, and provider failures; focused compiler evidence for both signed actual-global and
unsigned structured-buffer producers; preserved rejection of Float/wide/shared/strong-order and
adjacent atomic opcodes; direct runtime/PTX for `atomics.slang`; CUDA 12.9
`ptxas -arch=sm_70`; the complete `slang-unit-test-tool/nvvm` prefix; pinned formatting; and
`git diff --check`.

The self-review inventories the removed callback, descriptor/catalog, queried interface, pointer
resolver, serializer rewrite, fake/provider changes, and tests. For each new helper or branch,
record the exact final-IR producer, why it is canonical, and which test fails without it. Remove any
source intrinsic matching, arbitrary pointer acceptance, signedness-specific physical type,
duplicated support table, provider-only fallback, unchecked text rewrite, compatibility shim,
shared-memory leak, or order widening.

## Failure and Recovery

If libNVVM rejects the generalized but equivalent LLVM 7 textual add, preserve LLVM assembly,
rewritten NVVM IR, PTX, cubin, and logs under ignored `build/slice121-*`; compare with the existing
signed-i32 reference before narrowing the descriptor catalog. Do not restore the bespoke callback,
patch source shader text, infer a global pointer from arbitrary LLVM address-space coincidence,
widen shared memory or orders, reset unrelated work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, LLVM text, PTX, cubin, and logs under ignored `build/slice121-*`. Distill the
generic atomic-interface contract, exact supported rows, validation evidence, and next measured
corpus boundary into `docs/design/nvvm-backend.md`, then commit this plan with the implementation as
explicitly requested.
