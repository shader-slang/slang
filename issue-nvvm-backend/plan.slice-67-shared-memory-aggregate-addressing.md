# Slice 67: Add shared memory and aggregate addressing

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM represents canonical statically sized shared storage and the
aggregate addressing needed to use it. A multi-thread block-level workload writes, synchronizes,
reads, and reduces or transposes shared values with exact NVVM/NVRTC runtime parity.

## Progress

- [x] (2026-08-28) Placed this after V4/catalog cleanup and core execution because it exercises the
  first major type/address-space growth boundary and needs thread IDs plus synchronization.
- [x] (2026-08-28) Traced `groupshared int sharedValues[64]` through linked/final IR. The stable
  producer is a module-owned `IRGlobalVar` with `GroupShared` rate and `Ptr(Array(Int, 64))`; uses
  are ordinary `IRGetElementPtr` results in address space 3 with `ScalarLayout`.
- [x] (2026-08-28) Settled the representation: construction version 3 adds only a typed global
  storage declaration. Existing fixed-array, pointer, load/store, and array-element-addressing
  operations already express the remaining canonical shape.
- [x] (2026-08-28) Added construction version 3 with one generic typed global-storage declaration;
  reused existing fixed-array, pointer, GEP, load, store, atomic, and barrier operations.
- [x] (2026-08-28) Implemented a 64-thread cross-warp reverse-read workload with exact fake graph,
  differential PTX, `ptxas`, and RTX NVVM/NVRTC runtime evidence.
- [x] (2026-08-28) Validated Release 431/431, Debug preservation 8/8, compatible assembly,
  CUDA 12.9 `ptxas -v`, formatting, ledger/hash, and the final input-shape audit.

## Surprises and Discoveries

- Canonical static shared storage is explicit before emission: `IRGlobalVar` owns the fixed array,
  `IRGroupSharedRate` owns the storage class, and `IRGetElementPtr` owns each element address. No
  address-space cast or target-specific repair is needed.
- A natural `int(cudaThreadIdx().x)` probe stops at the separate canonical `IRIntCast`. Numeric
  conversion belongs to Slice 68, so this slice uses the established signed relaxed-atomic ticket
  fixture to assign all 64 shared indices. It still proves peer writes because each ticket reads
  the value written by ticket `63 - ticket` after the barrier.
- The current construction surface already has module-owned fixed arrays, typed pointers in shared
  address space, and structural array element addressing. Storage declaration is the only missing
  provider operation.
- `IRGlobalVar` is classified as hoistable by the generic IR metadata. Shared-storage preflight must
  therefore handle every global variable before its generic hoistable allowance, or an unused
  unsupported storage object could be silently dropped.
- The adjacent `groupshared float[64]` source retains a canonical i32 storage object after target
  legalization but produces a non-i32 shared element-pointer relation. The exact relation check
  rejects it before provider discovery, so this slice makes no floating-shared-storage claim.

## Decision Log

- Decision: require a representative shared-memory program, not isolated type/GEP unit success.
  Rationale: storage lifetime, layout, address space, synchronization, and addressing must compose
  to demonstrate the capability.
  Date/author: 2026-08-28, Codex.

- Decision: fix malformed or noncanonical storage upstream instead of teaching the emitter a
  target-specific alternative spelling.
  Rationale: shared-memory lowering is a high-risk representation boundary under the repository's
  problem-solving rules.
  Date/author: 2026-08-28, Codex.

- Decision: append a generic typed global-storage declaration to V4 construction version 3 and
  reuse all existing type/addressing operations.
  Rationale: the canonical source global owns storage identity while its already-lowered value type,
  address space, alignment, and linkage name are sufficient to declare it. Adding shared-specific
  array/GEP callbacks would duplicate facts and scale poorly.
  Date/author: 2026-08-28, Codex.

- Decision: defer `UInt` execution-index conversion and use signed atomic tickets in the workload.
  Rationale: the cast is a valid independent numeric operation planned for Slice 68. The ticket
  workload isolates this slice's storage/address/synchronization boundary and requires cross-warp
  peer visibility through reverse indexing.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Complete. The source-of-truth shape is one module-owned `IRGlobalVar` with `GroupShared` rate and
`Ptr(Array(Int, 64))`, consumed by exact address-space-3 scalar element pointers. Construction V4
version 3 adds only generic typed storage declaration; the provider owns LLVM global construction,
and the emitter rejects every other global variable or shared pointer relation before discovery.

The 64-thread workload proves peer visibility across two warps. Release passes 431/431 with
registered-name SHA-256 `3d3e5effec15efd6d8eec74752802df83fe21ffb89e9d9037b3abf0803d25c0b`;
removing the six new names reproduces Slice 66's hash exactly. Debug preservation passes 8/8.
CUDA 12.9 `ptxas -v` reports 14 registers, one barrier, 256 bytes shared memory, zero stack, and
zero spills, and RTX 5090 runtime results agree with NVRTC for all 64 tickets. Floating/nested
arrays, structs, dynamic shared storage, shared atomics, general globals, aggregate values/copies,
and address-space conversions remain explicit future boundaries.

## Context and Current Pipeline

The backend has typed device pointers, fixed arrays behind entry pointers, raw
`RWStructuredBuffer<int>`, loads/stores, pointer/array addressing, and scalar SSA. It has not proven
module/function storage declarations, shared address space, aggregate values, or address-space
transitions. Slice 66 supplies ordinary thread/block execution and synchronization.

## Scope and Non-Goals

In scope are one canonical static shared-storage representation, fixed scalar arrays or structs
needed by the workload, shared address-space pointers, canonical element/field addressing, loads,
stores, synchronization, provider validation, family/unit/integration/runtime evidence, design,
ledger, and this plan.

Out of scope are dynamically sized extern shared memory unless required by the canonical producer,
general global variables, arbitrary aggregate values/copies, matrices, recursive structs, unions,
local-memory spilling policy, bank-conflict optimization, and performance claims.

## Architecture and Invariants

The canonical producer owns storage class, element type, extent, layout, and address space. The
emitter maps that source of truth once to opaque V4 type/storage handles; it does not rebuild types
from use sites. Addressing preserves pointee/layout/access invariants and uses structural provider
operations rather than LLVM GEP indices crossing the ABI.

Provider validation completes before mutation and checks module ownership, sized types, legal NVVM
address spaces, exact pointer/result relationships, availability, alignment, and constant extents.
Synchronization must dominate shared reads that consume peer writes in the representative CFG.

## Interfaces and Dependencies

Potentially extend V4 construction with a versioned storage/type capability or generic descriptor;
modify final-IR validation/emission, provider, fake support, tests, runtime harness, design, ledger,
and this plan. Do not append V3 or expose LLVM aggregate/GEP objects.

## Milestones

1. Trace the representative source through canonical IR and identify the producer of each storage
   and address shape.
2. Freeze the minimal principled V4 storage/type/addressing representation.
3. Implement provider and emitter validation/emission with negative no-mutation tests.
4. Add the composed shared-memory workload and differential IR/PTX/`ptxas`/RTX evidence.
5. Run all regression lanes, document the shape audit and limits, format, audit, and commit.

## Validation and Acceptance

Run focused type/storage/addressing/barrier tests, malformed/unsupported/fallback tests, full
Release NVVM prefix, Debug preservation, compatible assembly, CUDA 12.9 `ptxas -v`, and multi-warp
RTX NVVM/NVRTC runtime parity outside the sandbox.

Accept if one canonical shared-storage producer flows without syntax reconstruction; exact address
spaces/layouts survive; invalid storage/address shapes fail before mutation; the workload would
fail without correct cross-thread synchronization and addressing; PTX reports expected shared
memory; all established tests pass; design/ledger/plan are current; formatting/audit pass.

## Self-Review and Input-Shape Audit

Inventory every new storage/type/address helper, address-space conversion, layout rule, GEP mapping,
barrier rule, and fallback. For each, record exact producer and consumer. Reject arbitrary operand
graph walks, magic type names, reconstructed syntax, use-site-derived duplicate types, silent
defaults, or emitter patches for a malformed producer shape.

## Failure and Recovery

If canonical shared storage is not represented explicitly enough, stop and fix its producer before
provider work. If the workload requires dynamic shared memory or unsupported aggregate copies,
narrow to a static representative that still proves the boundary and record the remaining gate.
Remove the new V4 queried interface/version as a unit to restore Slice 66. Never stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Retain source/AST/final-IR storage trace, type/layout/address-space inventory, normal/compatible IR,
NVVM/NVRTC PTX, `ptxas -v` resource data, runtime cases, suite counts/hash, and self-review. Commit
this completed plan with Slice 67.
