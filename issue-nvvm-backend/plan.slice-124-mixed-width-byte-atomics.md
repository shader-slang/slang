# Support mixed-width byte-address atomics

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct NVVM path can compile and execute the existing
`tests/compute/byte-address-buffer-atomic-mixed-width-12265.slang` fixture. A writable byte-address
view crosses a helper boundary, is reinterpreted as a UInt64 structured view, and performs relaxed
global unsigned 64-bit max while the entry point independently performs the established UInt32
add at a different byte offset. The direct runtime result and PTX must preserve both widths and
locations exactly.

## Progress

- [x] (2026-08-29) Completed Slice 123 as `25a0318b2`; the final NVVM prefix passed 391/391.
- [x] (2026-08-29) Probed 36 compute fixtures with CUDA lanes, classified the next direct-NVVM
  boundaries, and selected the mixed-width byte-address fixture as the smallest coherent extension
  of the established raw-buffer helper ABI and relaxed global atomic catalog.
- [x] (2026-08-29) Captured the optimized IR and reproduced the first direct diagnostic at the
  UInt64 equivalent-structured-buffer conversion.
- [x] (2026-08-29) Admitted the exact UInt64 raw-buffer view by reconstructing its typed physical
  view, then added relaxed global UInt64 unsigned max through the existing generic atomic
  descriptor.
- [x] (2026-08-29) Added focused fake/real-provider coverage, including adjacent descriptor
  rejection and LLVM 14 plus NVVM IR 2.0 assembly checks.
- [x] (2026-08-29) Promoted all three fixture lanes; runtime passed 3/3, the 534-byte SM90 PTX
  contains `atom.global.max.u64` and `atom.global.add.u32`, and CUDA 12.9.86 `ptxas` produced a
  3,488-byte cubin.
- [x] (2026-08-29) Ran final Release provider/host builds and the complete 392/392 NVVM prefix;
  updated durable status, formatted with the pinned tool, and completed the input-shape self-review.

## Surprises and Discoveries

- The fixture contains two independent operations: helper `doMax` converts an
  RWByteAddressBuffer to an RWStructuredBuffer<UInt64> and emits `atomicMax` at element zero, while
  `computeMain` converts a separately loaded raw view to RWStructuredBuffer<UInt32> and emits the
  already-supported `atomicAdd` at element four (byte offset 16).
- Slang automatically upgrades the target capability to `cuda_sm_9_0` for the UInt64 atomic max.
  Runtime and `ptxas` validation therefore need the exact SM 90 lane rather than silently testing
  only the established SM 70 path.
- Selected 8/16/32/64-bit integer scalars already have a generic value/type classifier, and the
  resource-element classifier already delegates to it. The measured first rejection is the Slice
  123 byte-to-structured equivalence contract, which deliberately admitted UInt32 only.
- The UInt32 conversion is provider-handle identity because the byte view already contains an
  `i32 addrspace(1)*`. UInt64 cannot be identity: element scaling and provider validation require an
  `i64 addrspace(1)*`. The correct lowering extracts pointer/count, retags the same address with a
  zero-byte typed pointer operation, and constructs the exact UInt64 view while preserving count.
- The fake provider already modeled resource views passed directly as parameters, but this fixture
  exposed two equally canonical forms: a view loaded from a global-parameter field and a newly
  constructed typed view. Extending its exact value/type checks to those forms made the fake match
  the real provider contract without weakening type equality.
- The first full-prefix run exposed that a resource-view pointer extraction must remain pointer-like
  even when the resource element is itself a copyable struct. Classifying field zero by the
  resource-view contract, instead of the scalar-struct pointee classifier, restored both established
  aggregate-resource tests; the focused trio and final 392/392 prefix then passed.

## Decision Log

- Decision: keep UInt32 byte-to-structured conversion as handle identity, but reconstruct the
  physical view for UInt64 by extracting pointer/count, retagging the pointer at byte offset zero,
  and constructing the exact result struct.
  Rationale: the resource and count are preserved, but the provider pointer type must change from
  `i32 addrspace(1)*` to `i64 addrspace(1)*` so LLVM element scaling and exact call/atomic
  validation remain sound. Treating the entire aggregate as identity would be physically wrong.
  Date/author: 2026-08-29, Codex.
- Decision: add one atomic catalog row for relaxed global UInt64 unsigned max and map the generic
  operation descriptor to LLVM unsigned max.
  Rationale: the optimized IR explicitly carries unsigned UInt64 semantics. Adding signed max or a
  broader 64-bit atomic family without a fixture would weaken the evidence-driven boundary.
  Date/author: 2026-08-29, Codex.
- Decision: keep builder ABI revision 21.
  Rationale: `SLANG_NVVM_ATOMIC_OP_MAX` already exists in the forward-only generic enum. This slice
  adds one semantic catalog row and provider mapping without changing a table or structure layout.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

The relevant optimized IR is equivalent to:

    func doMax(RWByteAddressBuffer uav)
    {
        RWStructuredBuffer<UInt64> words = getEquivalentStructuredBuffer(uav);
        Ptr<UInt64> address = rwstructuredBufferGetElementPtr(words, 0);
        atomicMax(address, 5u64, Relaxed);
    }

    func computeMain()
    {
        doMax(buffer);
        RWStructuredBuffer<UInt32> words = getEquivalentStructuredBuffer(buffer);
        atomicAdd(rwstructuredBufferGetElementPtr(words, 4), 3u, Relaxed);
    }

Slice 123 already transports the writable byte view through the helper and lowers the exact
byte-to-UInt32 structured conversion as provider-handle identity. Slice 121 provides the generic
atomic descriptor and one relaxed global/shared 32-bit add catalog. Direct scalar lowering already
represents UInt64 as LLVM `i64`.

## Scope and Non-Goals

In scope are exact UInt64 writable structured interpretation of a writable byte-address view;
element-pointer validation and lowering through existing raw-buffer machinery; relaxed global
UInt64 unsigned max; correct eight-byte alignment; coexistence with the established relaxed global
UInt32 add; fake and real provider tests; promotion of the existing fixture; durable design status;
and this plan.

Out of scope are signed 64-bit max, min, exchange, compare-exchange, 64-bit add, shared UInt64
atomics, stronger memory orders, arbitrary resource casts, non-default layouts, access conversion,
64-bit raw-buffer loads/stores without an independently proven operation, source-name intrinsic
matching, builder table-layout changes, compatibility aliases, and unrelated corpus failures.

## Architecture and Invariants

- `getEquivalentStructuredBuffer` is resource-preserving only for a canonical byte-address operand
  and an admitted scalar unsigned-integer structured result with identical access and default
  layout. It may rebuild the physical aggregate when its typed data pointer must change.
- The structured element type remains the semantic source of truth for element pointer scaling,
  provider pointer type, atomic numeric class, bit width, and alignment.
- Atomic emission is selected by a complete operation descriptor: operation, numeric semantics,
  address space, and memory order. The provider must reject every tuple not present in the catalog.
- LLVM emission uses unsigned max for UInt64, a global address-space pointer, monotonic ordering,
  and alignment eight. The independent UInt32 add remains add/i32/alignment four.
- No producer shape is reconstructed from source syntax or intrinsic names; the direct emitter uses
  the canonical optimized IR operations and types.

## Interfaces and Dependencies

Expected committed areas are the generic builder atomic operation enum/catalog and LLVM mapping,
direct NVVM equivalence and atomic resolution, fake/real-provider tests, the existing compute
fixture, durable design status, and this plan. CUDA 12.9 runtime and `ptxas -arch=sm_90` provide
semantic and module-acceptance evidence.

## Milestones

1. Generalize the exact byte-address-to-structured resolver from UInt32 identity to the selected
   unsigned scalar width required by the fixture, rebuilding a typed physical view when necessary,
   then compile again to expose the next canonical boundary.
2. Add the UInt64 unsigned-max atomic descriptor row, provider mapping, alignment, validation, and
   LLVM 14-to-7 serialization support only where the real generated module proves it necessary.
3. Add focused fake-provider observations and real-builder assembly/bitcode checks, including
   rejections for neighboring operation/width/address-space/order tuples.
4. Promote the fixture, inspect and assemble PTX, validate runtime results, run Release builds and
   the full prefix, update docs/plan, format, perform the input-shape audit, and commit.

## Validation and Acceptance

Acceptance requires Release host/provider builds; focused fake and real builder tests proving one
UInt64 unsigned max and one UInt32 add with exact widths, address spaces, orders, and alignments;
adjacent unsupported descriptor rejection; the fixture's ordinary and direct CUDA runtime lanes;
direct PTX containing the expected `atom.global.max.u64` and `atom.global.add.u32`; CUDA 12.9
`ptxas -arch=sm_90`; the complete `slang-unit-test-tool/nvvm` prefix; pinned formatting; and
`git diff --check`.

The self-review inventories every new helper, fallback, catalog entry, serializer rule, and special
case. For each retained item, record the optimized IR producer and exact failing fixture. Remove any
source-name match, duplicated type classifier, inferred signedness, generic 64-bit atomic widening,
pointer reconstruction, access widening, provider-only fallback, or compatibility shim.

## Failure and Recovery

If UInt64 identity, atomic serialization, libNVVM compilation, PTX assembly, or runtime validation
fails, preserve optimized IR, LLVM text, PTX, cubin, and logs under ignored `build/slice124-*` and
trace the exact producer/type/descriptor through direct preflight and provider validation. Do not
patch expected fixture data, silently lower to compare-exchange, weaken capability, broaden the
atomic family, reset unrelated work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, LLVM text, PTX, cubin, and logs under ignored `build/slice124-*`. Distill the
selected UInt64 raw-view and atomic contract, exact CUDA evidence, and next measured corpus boundary
into `docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly
requested.

## Outcomes and Retrospective

The slice demonstrates that the generic resource and atomic interfaces scale to mixed widths
without adding width-specific callbacks. The critical correction was distinguishing semantic
resource identity from physical aggregate identity: the UInt64 view keeps the same allocation and
count, but must carry an exact UInt64 pointer. Runtime and PTX independently prove that the helper's
64-bit max stays at byte zero while the entry point's 32-bit add stays at byte 16. Final Release
provider/host builds pass, the promoted fixture passes 3/3, CUDA 12.9.86 assembles the 534-byte PTX
to a 3,488-byte cubin, and the complete NVVM prefix passes 392/392.
