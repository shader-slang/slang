# Slice 49: Select a signed-i32 wave shuffle by canonical type

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs a CUDA kernel that loads a lane-varying
signed-i32 value, applies `WaveMaskReadLaneAt()`, and stores the selected lane's value. Two helpers
with the same CUDA GenericAsm spelling but different canonical Slang signatures select independent
negotiated operations, proving descriptor lookup scales by semantic type rather than text alone.

## Progress

- [x] (2026-08-28) Recorded the Slice 48 baseline: 319 names, SHA-256
  `6c97ed4746f5a67237d642f180e69984ec4bdc0f5ae23e5eecb540bd7d51d83c`, Release 319/319,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 24,211 measured lines.
- [x] (2026-08-28) Audited the signed-i32 specialization's exact linked IR and its existing
  pointer/load/call/store dependencies.
- [x] (2026-08-28) Added feature 37/operation 3 and selected duplicate GenericAsm text by exact
  canonical helper signature without changing the callback or V3 table.
- [x] (2026-08-28) Added seven provider/direct/capability/PTX/assembler/runtime evidence names
  around a lane-varying signed source while preserving Slice 48 and unsupported boundaries.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and audited the
  completed slice.

## Surprises and Discoveries

- Observation: specialization retains exact `Func(Int, UInt, Int, Int)` with parameters
  `(mask, value, lane)` and terminator `GenericAsm("__shfl_sync($0, $1, $2)")`.
  Consequence: text alone is no longer a unique descriptor key; combine exact text with the
  complete canonical helper signature.

- Observation: the kernel reaches the new boundary using only established operations: UInt lane
  index, signed-i32 read-only/read-write device pointers, UInt offsets, signed load/store, generic
  scalar calls/returns, and the shuffle helper.
  Consequence: use a lane-varying source buffer for semantic runtime evidence without adding casts
  or arithmetic.

- Observation: LLVM/NVVM use the same signless i32 shuffle intrinsic for Int and UInt payloads.
  Consequence: negotiate a distinct Slang semantic operation/feature at the facade boundary, then
  map both wire operations to the same provider intrinsic after exact argument validation.

- Observation: both direct routes preserve the signed kernel's `[64, 64, 32, 32]` launch ABI and
  global load/store, and the RTX 5090 returns the expected negative values for source lanes 0 and
  7.
  Consequence: the shared signless provider representation preserves the distinct signed Slang
  semantic end to end rather than merely producing acceptable text.

## Decision Log

- Decision: append feature 37 `WAVE_READ_LANE_AT_INT` and operation 3 rather than widening the
  existing UInt feature silently.
  Rationale: providers may implement only the already-shipped Slice 48 semantic. Independent bits
  preserve exact compatibility and make the supported Slang type explicit even though LLVM's
  representation is signless.
  Date/author: 2026-08-28, Codex.

- Decision: make descriptor lookup take both the GenericAsm terminator and its owning function.
  Rationale: linked IR type/signature is the canonical source of truth. A text-first lookup that
  returns the first row cannot distinguish valid generic specializations and would make table
  ordering semantic.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The direct route now accepts the canonical signed-i32 specialization without weakening the Slice
48 UInt boundary. Feature 37 and operation 3 are independently negotiated through the unchanged
callback/table. Descriptor lookup examines all rows with matching exact assembly and selects only
the one whose complete result/parameter signature matches the owning linked-IR function, so table
order cannot silently select UInt for Int or vice versa.

The provider maps operations 2 and 3 to the same signless LLVM i32 shuffle, adds clamp 31, and
retains the audited declaration. The signed fixture proves the complete lane-index/call/load/store
graph. Direct and NVRTC PTX agree on `[64, 64, 32, 32]`, one global 32-bit load and store, and
`shfl.sync.idx.b32`; CUDA 12.9 `ptxas` accepts both, and the RTX 5090 selects lanes 0 and 7 from a
varying signed source buffer through both compilers.

The standalone Release provider and Release/Debug main targets build successfully. The focused
Slice 48/49 matrix passes 14/14, the intrinsic compatibility plus unsupported boundary passes 2/2,
the Release prefix passes 326/326, and Debug preservation passes 10/10. The sorted LF-terminated
name set hashes to `64c930268a8edb87cf2cfba3d12991e4ac66c2a4c9336399d1f03e54f5eda8f0`;
removing exactly the seven Slice 49 names yields 319 names and the Slice 48 hash
`6c97ed4746f5a67237d642f180e69984ec4bdc0f5ae23e5eecb540bd7d51d83c`. V3 remains 528/308
bytes. Seven evidence names add 476 physical lines across the five measured test/support files,
from 24,211 to 24,687.

## Context and Current Pipeline

Slice 48 admits a canonical UInt shuffle helper through feature 36/operation 2. Its descriptor
contains exact CUDA-selected text and a `Func(UInt, UInt, UInt, Int)` signature. Direct emission
passes the helper's canonical parameters to the existing generic intrinsic callback, and the
provider adds CUDA's clamp 31 before emitting `llvm.nvvm.shfl.sync.idx.i32`.

The signed specialization has the same GenericAsm text but returns Int and takes
`(UInt mask, Int value, Int lane)`. The test kernel obtains its lane-varying Int value from a
read-only device pointer indexed by the established UInt lane index.

## Scope and Non-Goals

In scope are feature 37, intrinsic operation 3, signature-aware duplicate-text descriptor lookup,
the provider's shared signless i32 mapping, a signed source-buffer fixture/runtime path, and seven
evidence layers.

Out of scope are Float/Bool/64-bit/vector/matrix shuffles, active-mask synthesis, broadcast/read-
first operations, arbitrary GenericAsm overload resolution, casts, new integer arithmetic,
convergence changes, new callback fields, and performance claims.

## Architecture and Invariants

Descriptor lookup iterates rows whose exact assembly text matches and returns only the row whose
structural signature validates the owning helper. The UInt row remains exact; the Int row requires
`Func(Int, UInt, Int, Int)`. Table order is not observable for valid distinct signatures.

The facade maps operation 3 to feature 37. The provider maps operations 2 and 3 to the same LLVM
intrinsic only after requiring exactly three available i32 arguments. Sign is Slang semantic policy;
provider transport remains signless. The audited shuffle declaration and clamp contract remain
unchanged.

## Interfaces and Dependencies

Append one feature, operation, and minimum-size alias to the unchanged callback suffix. Extend
facade/provider/direct/fake mappings, signed fixtures, tests, design, ledger, and this plan. Add no
table field, ABI version, V2 change, export, LLVM component, or text rewrite.

## Milestones

1. Add feature 37/operation 3 with exact Slice 48 compatibility and unchanged 528/308-byte tables.
2. Make exact GenericAsm descriptor lookup signature-aware and add the signed specialization row.
3. Share provider emission/declaration auditing across signless UInt/Int operations.
4. Add seven negotiation, provider, direct, capability, differential, `ptxas`, and runtime names.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new names, seven Slice 48 preservation names, generic-intrinsic invalid/prefix tests, the
unsupported matrix, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged V3 sizes; exact Slice 48 compatibility; distinct UInt/Int descriptor selection;
one signed load, shuffle call with clamp 31, and signed store in LLVM/NVVM text; exact fake argument
topology; matching `[64, 64, 32, 32]` PTX ABI; load/store and `shfl.sync.idx.b32`; `ptxas`
acceptance; signed lane selection at runtime; name/hash continuity; formatted code; completed
input-shape audit; and clean diff checks.

## Self-Review and Input-Shape Audit

Inventory descriptor lookup/signatures, operation mappings, provider sharing, fake result/argument
recording, signed fixture, PTX load expectation, and runtime source-buffer mode. Prove signature-
aware lookup consumes canonical linked types, no table-order fallback remains, sign policy stays in
Slang, and the provider shares only representation-identical LLVM construction.

The inventory survives that audit:

- CUDA target selection and generic specialization produce the exact valid input shape: a
  reachable, defined, non-entry, one-block `Func(Int, UInt, Int, Int)` helper ending in
  `GenericAsm("__shfl_sync($0, $1, $2)")`. Its result and `(mask, value, lane)` parameter types are
  the canonical semantic source of truth. Removing the signed descriptor restores the E52017
  GenericAsm rejection; the direct preflight layer therefore owns recognition of this selected
  target semantic.
- Signature-aware lookup does not add an alternative equivalence relation. It reuses the existing
  exact Int/UInt i32 predicates on canonical linked types and checks every same-text descriptor.
  There is no fallback row, helper-name match, placeholder parser, syntax reconstruction, or
  operand-graph search. The formerly text-only first-row return was removed because it became
  non-canonical once two valid specializations shared the CUDA spelling.
- The facade's operation-to-feature switch owns negotiated semantic availability. The provider's
  shared branch owns the representation-identical LLVM construction only after operation 3 has
  passed that boundary and all arguments have passed established i32/ownership/availability
  checks. LLVM i32 is signless; keeping distinct wire operations prevents provider support for the
  older UInt semantic from being mistaken for signed Slang support.
- Fake recording extends the existing intrinsic-result classification and three-argument record;
  it does not add another IR representation. The signed provider fixture composes established
  pointer offsets, load, generic calls/returns, and store around the new intrinsic. The direct fake
  test proves the kernel passes the existing mask parameter, signed load result, and lane
  parameter, while the real provider text and PTX/runtime layers prove that topology survives.
- The shared one-warp runtime launcher now has an explicit signed-source mode. It allocates a
  lane-varying Int buffer only for that valid four-parameter kernel shape and validates the selected
  lane before indexing the host oracle. Existing lane-index, lane-count, and UInt shuffle callers
  retain their original ABI and pass unchanged.

## Failure and Recovery

If the signed helper shape changes, fix or document the producer rather than matching names. If
NVVM/PTX treats the payload differently despite signless i32, retain independent feature/runtime
evidence. Removing feature/operation 3 and its descriptor restores Slice 48. Never stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Retain exact signed linked IR, descriptor selection evidence, LLVM/NVVM text, PTX mechanisms,
`ptxas`/RTX results, sizes, hashes, line growth, and the completed audit. Distill durable results
into design/ledger and ship this plan with Slice 49.
