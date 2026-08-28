# Slice 50: Carry float32 through the typed wave shuffle

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs a CUDA kernel that loads a lane-varying
float32 value, applies `WaveMaskReadLaneAt()`, and stores the selected lane's exact value. Three
helpers with identical CUDA GenericAsm text but canonical UInt, Int, and Float signatures select
independent negotiated operations through the unchanged generic intrinsic callback.

## Progress

- [x] (2026-08-28) Recorded the Slice 49 baseline: 326 names, SHA-256
  `64c930268a8edb87cf2cfba3d12991e4ac66c2a4c9336399d1f03e54f5eda8f0`, Release 326/326,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 24,687 measured lines.
- [x] (2026-08-28) Audited the Float specialization's exact linked IR, existing typed
  pointer/load/call/store dependencies, and LLVM 7/14 intrinsic declarations.
- [x] (2026-08-28) Added feature 38/operation 4 and exact canonical
  `Func(Float, UInt, Float, Int)` descriptor selection without changing the callback or V3 table.
- [x] (2026-08-28) Added seven provider/direct/capability/PTX/assembler/runtime evidence names
  around a lane-varying float32 source while preserving Slices 48/49 and unsupported boundaries.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and audited the
  completed slice.

## Surprises and Discoveries

- Observation: final linked IR retains exact `Func(Float, UInt, Float, Int)` with parameters
  `(mask, value, lane)` and terminator `GenericAsm("__shfl_sync($0, $1, $2)")`.
  Consequence: append a third complete signature row to the established text-plus-canonical-type
  descriptor lookup; do not add helper-name or placeholder matching.

- Observation: the kernel reaches the new boundary using established Float read-only/read-write
  device pointers, Float load/store, UInt lane indexing, and generic scalar calls/returns.
  Consequence: reuse those typed paths and refactor the Int/Float provider fixture around their
  identical loaded-scalar graph instead of copying a third fixture.

- Observation: both checked-in LLVM 7 and built LLVM 14 expose
  `llvm.nvvm.shfl.sync.idx.f32` as `float(i32, float, i32, i32)` with inaccessible-memory and
  convergent semantics.
  Consequence: use the native Float shuffle rather than bitcasting through i32, and extend the
  audited legacy declaration branch without rewriting its spelling or attributes.

- Observation: the first direct Float kernel stopped at E52017 `device i32 pointer offset` even
  though entry parameters and type lowering already admitted Float device pointers.
  Consequence: replace that stale consumer classifier with one centralized established
  Int/UInt/Float scalar-pointer classifier. Preserve the separate i32-only array-element contract.

- Observation: the fake recorded every pointer-offset load as integer because the derived handle
  discarded its base pointer's scalar kind.
  Consequence: derive pointee kind recursively from the canonical fake base-pointer record for
  loads and stores, and reject value/pointer type mismatches rather than special-casing Float.

## Decision Log

- Decision: append feature 38 `WAVE_READ_LANE_AT_FLOAT` and operation 4.
  Rationale: Float support is a distinct Slang semantic/provider capability even though it reuses
  the same callback and PTX b32 shuffle mechanism. Exact Slice 49 providers must remain honest and
  loadable with feature 38 clear.
  Date/author: 2026-08-28, Codex.

- Decision: validate operation-defined provider argument types as an exact vector.
  Rationale: integer shuffles require `(i32, i32, i32)`, while Float requires
  `(i32, float, i32)`. One per-operation expected-type vector preserves pre-mutation validation and
  scales without casts or a separate callback.
  Date/author: 2026-08-28, Codex.

- Decision: share the loaded-scalar provider fixture and 32-bit runtime launcher across Int and
  Float modes.
  Rationale: their graph and launch ABI are representation-identical apart from the canonical
  scalar type and expected bit pattern. Sharing these mechanics prevents a third copied harness
  while keeping named evidence independent.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The direct route now accepts the canonical Float specialization through feature 38/operation 4 and
the unchanged generic callback/table. Descriptor lookup selects the third same-text row by exact
canonical result and parameter types. Provider emission validates `(i32, float, i32)`, appends i32
clamp 31, and emits native `llvm.nvvm.shfl.sync.idx.f32`; the legacy writer verifies the exact
LLVM-7-compatible declaration without rewriting it.

The Float source exposed two upstream assumptions and both were fixed at their source of truth.
Production pointer-offset validation now uses the centralized established scalar-pointer
classifier already owned by type lowering, rather than its old integer-only classifier. The fake
propagates a derived pointer's scalar kind from its recorded base pointer and enforces matching
load/store values. Int and Float provider fixtures share one loaded-scalar graph, and their runtime
modes share one 32-bit launcher while retaining type-specific host oracles.

The standalone Release provider and Release/Debug main targets build successfully. The seven new
names pass 7/7, the focused Slice 48/49/50 matrix passes 21/21, intrinsic compatibility plus the
unsupported boundary passes 2/2, Release passes 333/333, and Debug preservation passes 10/10.
The sorted LF-terminated name set hashes to
`57f52bd80e15eefb8a35bc51821d99a4b70c858f111535fde1fea3f90b2bb367`; removing exactly the
seven Slice 50 names yields 326 names and the Slice 49 hash
`64c930268a8edb87cf2cfba3d12991e4ac66c2a4c9336399d1f03e54f5eda8f0`. V3 remains 528/308
bytes. Seven evidence names add 433 measured physical lines across the five test/support files,
from 24,687 to 25,120. NVVM/NVRTC, CUDA 12.9 `ptxas`, and RTX 5090 runtime evidence all pass;
lanes 0 and 7 select -11.5 and -6.25 bit-exactly.

## Context and Current Pipeline

Slice 49 makes GenericAsm descriptor selection depend on exact text plus the owning function's
complete canonical signature. UInt and Int operations remain independently negotiated, then share
LLVM's signless i32 shuffle. The kernel-side signed fixture establishes a source load, two pointer
offsets, and generic scalar helper transport.

The Float specialization has the same selected text but returns Float and takes
`(UInt mask, Float value, Int lane)`. Unlike signed/unsigned i32, LLVM has a distinct native Float
shuffle intrinsic. Its first, third, and clamp arguments remain i32 while its payload/result are
float.

## Scope and Non-Goals

In scope are feature 38, operation 4, the exact Float descriptor signature, mixed typed provider
argument validation, native f32 shuffle construction/audit, shared loaded-scalar fixtures/runtime,
and seven evidence layers.

Out of scope are Float16/Float64, Bool, 64-bit integers, vectors/matrices, active-mask synthesis,
broadcast/read-first operations, arbitrary GenericAsm overload resolution, bitcasts, new callback
fields, convergence changes, and performance claims.

## Architecture and Invariants

Descriptor lookup continues to iterate exact same-text rows and returns only the complete matching
canonical signature. The Float row requires `Func(Float, UInt, Float, Int)`; UInt and Int rows stay
exact and table order remains unobservable.

The facade maps operation 4 to feature 38. The provider chooses native
`nvvm_shfl_sync_idx_f32`, requires exactly `(i32, float, i32)` available arguments, appends i32
clamp 31, and emits only after all checks pass. The legacy writer validates the exact Float result,
argument vector, and three semantic attributes already accepted by LLVM 7/NVVM.

## Interfaces and Dependencies

Append one feature, operation, and minimum-size alias to the unchanged callback suffix. Extend
facade/provider/direct/fake mappings, loaded-scalar fixtures, tests, design, ledger, and this plan.
Add no table field, ABI version, V2 change, export, LLVM component, cast operation, or text rewrite.

## Milestones

1. Add feature 38/operation 4 with exact Slice 49 compatibility and unchanged 528/308-byte tables.
2. Add the exact Float helper signature to same-text descriptor lookup.
3. Emit and audit the native mixed-signature Float shuffle with pre-mutation type checks.
4. Share Int/Float loaded-scalar fixtures and runtime mechanics, then add seven named evidence
   layers.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new names, Slices 48/49 preservation names, generic-intrinsic prefix/invalid tests, the
unsupported matrix, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged V3 sizes; exact Slice 49 compatibility; distinct UInt/Int/Float descriptor
selection; one Float load, native Float shuffle with clamp 31, and Float store in LLVM/NVVM text;
exact fake argument topology; matching `[64, 64, 32, 32]` PTX ABI; load/store and
`shfl.sync.idx.b32`; `ptxas` acceptance; bit-exact Float lane selection at runtime; name/hash
continuity; formatted code; completed input-shape audit; and clean diff checks.

## Self-Review and Input-Shape Audit

Inventory the Float descriptor signature, operation mappings, mixed provider type vector, legacy
declaration audit, fake result classification, shared loaded-scalar fixture/runtime modes, and new
evidence. Prove all selection consumes canonical types, provider checks precede mutation, native
Float transport avoids reconstructed bit representations, and shared test mechanics have genuinely
identical graphs/ABIs.

The inventory survives that audit:

- CUDA selection and generic specialization produce the valid exact
  `Func(Float, UInt, Float, Int)` helper ending in the measured GenericAsm spelling. The linked
  function's canonical result and parameters are the sole descriptor key beside exact text.
  Removing the row restores E52017 GenericAsm rejection; no helper-name match, placeholder parser,
  syntax reconstruction, fallback row, or structural equivalence relation was added.
- The facade owns operation-to-feature negotiation. The provider chooses an intrinsic plus exact
  expected argument-type vector, validates count/type/ownership/availability/insertion state, then
  appends clamp 31 and mutates the module. Native Float transport preserves the existing Float
  value; there is no bitcast to an integer shadow representation.
- The legacy declaration branch handles only LLVM's two established scalar shuffle intrinsics. It
  derives the expected payload/result type from the intrinsic ID, requires i32 mask/lane/clamp,
  and retains the exact three semantic attributes. LLVM 7's checked-in definition and real
  libNVVM/`ptxas` tests prove this is a canonical input declaration rather than a parser workaround.
- The exact shape reaching pointer-offset validation is a canonical Float device pointer produced
  as an entry parameter, followed by `getOffsetPtr(pointer, UInt laneIndex)`. That is intentionally
  valid: the generic provider GEP already accepts any established typed pointer and integer index.
  The accidental alternative was the consumer's stale call to the integer-only classifier, so the
  fix adds one scalar-pointer classifier in type lowering and makes preflight/value validation use
  it. Array element pointers remain deliberately fixed-i32 and unchanged.
- The fake pointer-offset handle already stores the base-pointer reference, which is its semantic
  source of truth. Recursively resolving that recorded reference preserves Float across offset
  chains and lets both load and store enforce exact scalar kind. This replaces the old hardcoded
  integer default; it does not walk production IR or invent a second compiler representation.
- The shared loaded-scalar fixture differs only by canonical payload handle and stable intrinsic
  operation. The Int/Float kernel graphs are otherwise identical. The shared launcher operates on
  their representation-identical 32-bit CUDA parameter/storage ABI but selects separate Int/Float
  source buffers and exact bit-pattern oracles. Named provider/direct/PTX/runtime tests remain
  independently registered.

## Failure and Recovery

If the Float helper shape changes, fix or document the producer rather than matching names. If the
LLVM 14 declaration differs from LLVM 7/NVVM acceptance, isolate and validate the exact mismatch
before any rewrite. Removing feature/operation 4 and its descriptor restores Slice 49. Never stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Retain exact Float linked IR, descriptor selection evidence, LLVM/NVVM declaration and calls, PTX
mechanisms, `ptxas`/RTX results, sizes, hashes, line growth, and the completed audit. Distill durable
results into design/ledger and ship this plan with Slice 50.
