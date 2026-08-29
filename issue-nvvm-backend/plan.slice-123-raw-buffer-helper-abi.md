# Carry selected raw-buffer views through helper calls

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, every already-supported canonical StructuredBuffer, RWStructuredBuffer,
ByteAddressBuffer, and RWByteAddressBuffer view can cross an ordinary helper parameter by value
without changing its physical pointer/count representation or access policy. The existing
`tests/compute/func-resource-param.slang` and
`tests/compute/byte-address-buffer-atomic-via-helper-12265.slang` fixtures should pass direct CUDA
runtime and PTX lanes, proving structured loads plus byte loads and atomic access through helpers.

## Progress

- [x] (2026-08-29) Completed Slice 122 as `55f7e06e0`; the final NVVM prefix passed 389/389.
- [x] (2026-08-29) Reproduced `helper function parameter` for both existing fixtures and captured
  the optimized byte-address helper IR.
- [x] (2026-08-29) Admitted exact selected raw-buffer views in helper signatures, call
  validation, availability, type lowering, and emission using the existing generic function ABI.
- [x] (2026-08-29) Resolved canonical ByteAddressBuffer-to-UInt StructuredBuffer equivalence as a
  representation-preserving identity, with exact kind/access/element validation.
- [x] (2026-08-29) Added focused real/fake-provider coverage, promoted both fixtures, validated
  runtime/PTX, `ptxas`, Release builds, and the full prefix, updated durable status, formatted, and
  completed the self-review.

## Surprises and Discoveries

- Both measured fixtures stop before provider discovery at the same helper-parameter allowlist.
  Raw buffer types already lower in entry/value/storage roles to the generic provider struct used by
  global parameter groups.
- The byte-address atomic helper contains one `getEquivalentStructuredBuffer` from writable byte
  view to writable UInt structured view. Both types deliberately represent the same underlying
  pointer/count pair; the conversion changes the typed interpretation but not the provider value.
- The rest of the byte fixture is already supported: immutable byte load at offset 12, UInt divide
  by four, structured element pointer, relaxed global UInt32 atomic add, direct helper call, and
  final structured-buffer store.
- The LLVM provider's generic call boundary originally accepted scalar aggregates or direct generic
  pointers, but not the raw view's `{global pointer, count}` aggregate. Recursive validation of the
  declared parameter representation is the generic missing invariant; the function type and exact
  argument type remain the source of truth.
- The fake provider similarly classified resource-view parameter types but could not validate a
  resource-view parameter value at a generic call. Exact parameter type-handle comparison fixes the
  test model without inventing resource-result or loaded-view transport.
- The adjacent `func-resource-param.slang` fixture needs no conversion. It passes an established
  RWStructuredBuffer<Int32> value to a helper and performs an ordinary element load.

## Decision Log

- Decision: admit all raw buffer views already accepted by `getNVVMSupportedRawBufferType`, while
  preserving exact canonical source type equality at calls.
  Rationale: structured/byte kind and read/read-write access already determine one complete view
  contract and physical representation. Four parallel helper special cases would duplicate that
  source of truth.
  Date/author: 2026-08-29, Codex.
- Decision: keep resource views parameter-only in this slice.
  Rationale: the measured corpus passes loaded views into ordinary helpers. Helper results,
  block-parameter/phi transport, nested aggregates, and pointer escape require separate ownership
  evidence and are not necessary to prove the ABI.
  Date/author: 2026-08-29, Codex.
- Decision: lower exact byte-to-structured equivalence as an identity on the already-lowered raw
  buffer value.
  Rationale: adding a provider callback or reconstructing the struct would invent work and risk
  changing the pointer/count provenance. Exact semantic validation makes identity reuse safe.
  Date/author: 2026-08-29, Codex.
- Decision: accept only byte-address input and selected scalar UInt structured output with matching
  read/read-write access for `getEquivalentStructuredBuffer`.
  Rationale: this is the canonical producer emitted by byte-address legalization. Broader casts,
  element types, and access changes are not representation-neutral contracts proven here.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

All four selected raw-buffer view kinds now cross helper parameters through the existing generic
call ABI. The compiler reuses the raw-buffer classifier in signature, availability, and lowering
gates. The LLVM provider recursively validates aggregate parameter representations containing
NVVM address-space pointers, and focused real-provider coverage serializes a call carrying the
physical `{i32 addrspace(1)*, i64}` view. Builder ABI remains revision 21.

The canonical writable byte-address atomic conversion is validated as ByteAddress to scalar UInt32
StructuredBuffer with identical access, then emitted as an identity on the existing provider
handle. Fake coverage carries read-only and read-write structured and byte views through four
helper calls, observes the writable byte atomic, and proves there is no aggregate reconstruction.

`func-resource-param.slang` passes its exact NVVM CUDA runtime and PTX lanes with the unchanged
`0, 11, 22, 33` result. Its whole prefix is 6/7 plus one ignored DX12 lane; the sole failure is the
pre-existing Dawn/WebGPU bind-group validation error. Its 758-byte PTX contains one global load and
one global store and assembles with CUDA 12.9.86 `ptxas -arch=sm_70` to a 2,792-byte cubin.
`byte-address-buffer-atomic-via-helper-12265.slang` passes 5/5, including both CUDA runtime lanes
and direct PTX, preserving result `103`. Its 577-byte PTX contains one global load, one
`atom.global.add.u32`, and one global store and assembles to a 2,920-byte cubin. Release host and
provider builds pass, and the complete NVVM unit-test prefix passes 391/391.

## Context and Current Pipeline

The optimized byte-address helper is equivalent to:

    func incrementAtomic(RWByteAddressBuffer buf, UInt offset, UInt value)
    {
        RWStructuredBuffer<UInt> words = getEquivalentStructuredBuffer(buf);
        UInt wordIndex = offset / 4;
        atomicAdd(rwstructuredBufferGetElementPtr(words, wordIndex), value, Relaxed);
    }

The entry loads `inputBuffer` from the collected conventional parameter block twice, performs
`byteAddressBufferLoad(inputBuffer, 12)`, passes the second raw view into the helper, and stores the
loaded value through an established RWStructuredBuffer<UInt>. Raw views already lower to a provider
struct containing the typed global data pointer and element count. Generic helper declaration,
parameter retrieval, direct calls, and aggregate by-value parameters already exist.

## Scope and Non-Goals

In scope are exact selected raw-buffer helper parameters; value availability and direct call
transport; type lowering through the existing raw-buffer representation; exact byte-address to
UInt structured-view identity conversion; fake-provider observations; focused adjacent rejections;
runtime/PTX promotion of the two existing fixtures; durable design status; and this plan.

Out of scope are raw-buffer helper results, block parameters or phi transport, raw-buffer fields in
helper aggregates, helper pointer/reference forms, noncanonical layouts, RasterizerOrdered views,
access-qualifier conversion, arbitrary element reinterpretation, surface/texture provenance
changes, resource arrays, structured aggregates not already admitted as elements, byte atomic
families beyond relaxed UInt32 add, 64-bit atomics, stronger orders, provider ABI changes, source
intrinsic matching, and compatibility aliases.

## Architecture and Invariants

- Helper admission calls `getNVVMSupportedRawBufferType`; no second kind/access/element allowlist is
  introduced.
- A call argument must have the helper parameter's exact canonical raw-buffer type. Passing by
  value preserves read versus read-write access and structured element type.
- Raw-buffer parameters are ordinary available first-class provider values, validated for module
  ownership and dominance like established surface/texture/sampler values.
- Type lowering uses `_lowerRawBufferType` in helper-parameter role and therefore emits the same
  pointer/count provider struct as entry/value/storage roles.
- `getEquivalentStructuredBuffer` has one canonical admitted shape: one ByteAddressBuffer operand,
  a UInt32 StructuredBuffer result, identical access, and default layout. It reuses the lowered
  operand handle without provider mutation.
- Structured/byte operations continue resolving their source view and element/access contract from
  existing helpers; helper ABI admission does not bypass consumer validation.

## Interfaces and Dependencies

Expected committed areas are direct type lowering/emission, fake-provider observation and focused
tests, the two existing compute fixtures, durable design status, and this plan. Builder ABI remains
revision 21; the real LLVM provider needs no new callback. CUDA 12.9 runtime and `ptxas` supply real
semantic and module-acceptance evidence.

## Milestones

1. Extend exact helper signature/type-lowering and call-availability gates to the existing raw
   buffer classifier; keep results and block parameters excluded.
2. Add one resolver for the canonical byte-to-UInt structured equivalence and map its output to the
   existing lowered view handle through preflight, availability, and emission.
3. Add focused fake coverage for structured and byte raw-view parameters/conversion, plus adjacent
   type/access/result/conversion rejection boundaries.
4. Promote both fixtures, inspect and assemble PTX, run Release builds and the full prefix, update
   docs/plan, format, perform the input-shape audit, and commit.

## Validation and Acceptance

Acceptance requires Release host/provider builds; focused compiler/fake tests covering read-only
and read-write structured/byte helper parameters or the exact exercised subset plus adjacent
rejections; proof that the equivalence conversion adds no provider construction call; preserved
formatted-surface provenance and unsupported pointer/helper-result boundaries; unchanged direct
runtime/PTX for both fixtures; CUDA 12.9 `ptxas -arch=sm_70`; the complete
`slang-unit-test-tool/nvvm` prefix; pinned formatting; and `git diff --check`.

The self-review inventories the raw-buffer helper admission, type-role addition, call-availability
branch, equivalence resolver, identity emission, fake observations, and test migrations. For each,
record the exact producer and failing fixture. Remove any duplicate buffer classification,
source-name matching, arbitrary resource cast, access widening, provider callback, struct
reconstruction, buffer pointer/count rediscovery, helper-result leak, block-parameter leak,
provider-only fallback, or compatibility shim.

## Failure and Recovery

If the by-value provider struct or identity conversion fails, preserve optimized IR, LLVM
assembly, PTX, cubin, and logs under ignored `build/slice123-*`; compare the entry-loaded and
helper-parameter type handles before changing the representation. Do not patch source fixtures,
split byte and structured helper callbacks, rebuild the view struct, infer access from a consumer,
widen helper results or block parameters, reset unrelated work, or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, LLVM text, PTX, cubin, and logs under ignored `build/slice123-*`. Distill the
raw-buffer helper ABI, identity conversion contract, exact evidence, and next measured corpus
boundary into `docs/design/nvvm-backend.md`, then commit this plan with the implementation as
explicitly requested.
