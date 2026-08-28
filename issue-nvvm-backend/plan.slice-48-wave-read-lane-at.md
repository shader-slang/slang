# Slice 48: Carry scalar arguments through a wave lane shuffle

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs a CUDA kernel that uses
`WaveMaskReadLaneAt()` on a canonical UInt value and stores the selected lane's value at each
lane-indexed destination element. The existing append-only generic intrinsic callback carries its
first argument vector and maps one stable operation to LLVM/NVVM's integer shuffle intrinsic.

## Progress

- [x] (2026-08-28) Recorded the Slice 47 baseline: 312 names, SHA-256
  `dbd8d587f633ab06ac2daaf086690a14fa3b9f4cab8c22332d0a75e562d65ab7`, Release 312/312,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 23,732 measured lines.
- [x] (2026-08-28) Audited adjacent scalar wave builtins, CUDA-selected linked IR, LLVM 14/LLVM 7
  NVPTX intrinsic catalogs, and CUDA's `__shfl_sync` lowering contract.
- [x] (2026-08-28) Added feature 36/operation 2 without changing the V3 table and carried three
  canonical scalar helper parameters through preflight, direct emission, facade dispatch, and the
  provider.
- [x] (2026-08-28) Added seven independently named provider/direct/capability/PTX/assembler/runtime
  evidence layers while preserving the existing unsupported boundary.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and audited the completed
  slice; prepared the complete plan and implementation for commit.

## Surprises and Discoveries

- Observation: `WaveGetWaveIndex()` and `WaveGetNumWaves()` expand to CUDA thread/block builtins,
  UInt arithmetic, division, and vector component access.
  Consequence: they are not the smallest scalar wave boundary after Slice 47.

- Observation: CUDA target selection retains a specialized one-block
  `Func(UInt, UInt, UInt, Int)` helper ending in exact
  `GenericAsm("__shfl_sync($0, $1, $2)")`; the three values are function parameters, not operands
  on the GenericAsm terminator.
  Consequence: validate the complete helper signature and lower its canonical parameters through
  the existing value map rather than parsing placeholder text or reconstructing source syntax.

- Observation: LLVM 7 and LLVM 14 both expose
  `llvm.nvvm.shfl.sync.idx.i32(i32 mask, i32 value, i32 lane, i32 clamp)`, while CUDA's default
  width supplies constant clamp `0x1f`.
  Consequence: operation 2 carries the three semantic Slang arguments; the provider owns the
  CUDA/NVVM-specific fourth constant.

- Observation: LLVM 14 declares the integer shuffle as
  `i32(i32, i32, i32, i32)` with exactly `convergent inaccessiblememonly nounwind`, and libNVVM's
  LLVM-7-era reader accepts that group unchanged.
  Consequence: audit the declaration in the legacy writer but do not introduce a needless text
  rewrite.

- Observation: direct NVVM and NVRTC both lower the source operation to `shfl.sync.idx.b32`, while
  direct NVVM retains `%laneid` and NVRTC computes the lane from `%tid.x`.
  Consequence: compare stable operation/ABI/memory semantics and retain route-specific lane-index
  evidence rather than requiring identical PTX.

- Observation: the standalone provider project's Debug configuration expects a Debug LLVM library
  that is not provisioned in this clone; its supported Release configuration builds successfully.
  Consequence: keep the established validation contract: standalone Release provider plus Release
  and Debug main builds using that provider.

## Decision Log

- Decision: make the first argument-bearing intrinsic operation canonical UInt
  `WaveMaskReadLaneAt`, not an active-mask synthesis or a computed wave index.
  Rationale: it composes the established UInt/helper/lane-index subset and tests the generic
  callback's existing argument vector without adding arithmetic, vectors, Boolean values, or a
  second callback.
  Date/author: 2026-08-28, Codex.

- Decision: validate the exact helper signature structurally and treat its parameters as the
  GenericAsm semantic arguments in declaration order.
  Rationale: the linked IR, rather than source names or placeholder parsing, is the canonical
  producer. The exact spelling remains a descriptor key, while signature validation prevents the
  same spelling from admitting a different instantiation.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Feature 36 and operation 2 extend the existing callback without adding a table field. V3 remains
528 bytes on x64 and 308 bytes on x86. Exact Slice 47 feature sets initialize with feature 36
clear and return `SLANG_E_NOT_AVAILABLE` without dispatch; providers advertising the feature still
require the complete intrinsic suffix. Unknown operations remain invalid.

The final linked IR has a canonical `Func(UInt, UInt, UInt, Int)` shuffle helper ending in exact
`GenericAsm("__shfl_sync($0, $1, $2)")` and the established zero-parameter lane-index helper. The
kernel passes its mask, lane-index result, and source lane to the shuffle helper, uses lane index
for the device-pointer offset, and stores the shuffle result. The fake graph records those exact
helper and call arguments through the generic intrinsic/call families.

LLVM and audited NVVM 2.0 text contain one lane-id call, one
`llvm.nvvm.shfl.sync.idx.i32` call with clamp 31, two helper calls/returns, one GEP, and one store.
The shuffle declaration retains its exact LLVM-7-compatible convergent/inaccessible-memory/
nounwind attributes. NVVM and NVRTC agree on `[64, 32, 32]`, one global 32-bit store, no load, and
`shfl.sync.idx.b32`; CUDA 12.9 `ptxas` accepts both. A 32-thread RTX 5090 warp selects source lanes
0 and 7 correctly through both compilers.

The focused Slice 47/48 matrix passes 14/14; the unsupported boundary passes 1/1; Release passes
319/319 with sorted LF-terminated name-set SHA-256
`6c97ed4746f5a67237d642f180e69984ec4bdc0f5ae23e5eecb540bd7d51d83c`; removing the seven new
names reproduces Slice 47's 312-name hash exactly. Debug preservation passes 10/10. The five
measured files grow by 479 physical lines, from 23,732 to 24,211.

## Context and Current Pipeline

Slices 46 and 47 established canonical sign-independent Int/UInt transport, generic scalar helper
calls and returns, a single `emitIntrinsic` callback, and two zero-argument target operations.
Direct preflight recognizes exact CUDA-selected GenericAsm helpers; direct emission calls the
negotiated provider; the LLVM 14 provider emits NVPTX intrinsics and serializes audited LLVM-7-era
NVVM 2.0 text for libNVVM.

For a kernel that computes `laneIndex = WaveGetLaneIndex()` and then calls
`WaveMaskReadLaneAt(mask, laneIndex, sourceLane)`, specialization retains the exact UInt helper
described above. The kernel calls lane index for both its output address and shuffle value, calls
the shuffle helper with raw scalar parameters, and stores the returned UInt.

## Scope and Non-Goals

In scope are feature 36, intrinsic operation 2, exact scalar shuffle-helper recognition, lowering
three available helper parameters, the provider's i32 shuffle intrinsic call and constant clamp,
and seven evidence layers around the composed lane-index/shuffle kernel.

Out of scope are other `WaveMaskReadLaneAt` specializations, vectors, matrices, Float, 64-bit
values, `WaveReadLaneAt` active-mask construction, arbitrary GenericAsm placeholders, other shuffle
modes or widths, active-mask synthesis, ballot/vote/reduction operations, convergence modeling
beyond LLVM's intrinsic contract, new arithmetic, and performance claims.

## Architecture and Invariants

One descriptor maps exact GenericAsm text to feature, operation, diagnostic, and expected helper
shape. The admitted shuffle helper is non-entry, defined, one-block, returns UInt, and has exactly
`(UInt, UInt, Int)` parameters. Its parameters already have provider values in the helper's value
map; emission passes those handles in order to `emitIntrinsic` and returns the result through the
existing generic value-return callback.

The facade maps operation 2 to feature 36 before dispatch. The provider validates exactly three
same-module, available i32 arguments and an unterminated insertion block before mutation, then
emits `llvm.nvvm.shfl.sync.idx.i32(mask, value, lane, 31)`. Unknown operations and wrong argument
vectors remain invalid. Older exact tables advertise neither feature and remain compatible.

## Interfaces and Dependencies

Append one feature bit, one intrinsic operation, and one minimum-size alias for the unchanged V3
callback suffix. Extend facade dispatch, provider emission/legacy declaration audit, exact direct
descriptor validation/emission, fake argument recording, fixtures, tests, design, ledger, and this
plan. Add no ABI field, export, LLVM component, raw-assembly transport, or source rewrite.

## Milestones

1. Add feature 36/operation 2 with exact Slice 47 compatibility and unchanged table sizes.
2. Generalize exact GenericAsm helper validation and emission for the measured three-parameter
   UInt shuffle shape.
3. Emit and serialize the LLVM/NVVM shuffle declaration and call with strict pre-mutation checks.
4. Add seven named negotiation, provider, direct, capability, differential, `ptxas`, and runtime
   tests around one composed kernel.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run all
new names, Slice 47 preservation, generic-intrinsic prefix/invalid tests, unsupported matrix, full
Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3 tables; exact Slice 47 compatibility; one shuffle declaration and
call with three dynamic i32 operands plus exact clamp 31; direct fake topology from helper params
through the callback to a returned call result; matching `[64, 32, 32]` PTX ABI; direct
`shfl.sync.idx.b32` evidence; `ptxas` acceptance; one-warp runtime output selecting lane zero; exact
name continuity; formatted code; completed input-shape audit; and clean diff checks.

## Self-Review and Input-Shape Audit

The new-helper/fallback/special-case inventory is the feature/operation mapping, descriptor
signature enum and validator, declaration-order parameter lowering, provider operation branch and
clamp constant, shuffle declaration audit, fake argument recording, provider fixture, and shared
wave runtime mode. All survive the audit:

- CUDA target selection produces the exact valid input shape: a reachable, defined, non-entry,
  one-block `Func(UInt, UInt, UInt, Int)` helper whose sole terminator is one-operand
  `GenericAsm("__shfl_sync($0, $1, $2)")`. The generic specialization pass is the producer. The
  function's `(UInt mask, UInt value, Int lane)` parameters are the existing semantic source of
  truth; placeholders describe CUDA source rendering but are not IR operands. Removing the
  descriptor row restores the E52017 GenericAsm boundary, proving direct preflight owns this exact
  selected target semantic.
- One descriptor row owns exact text, structural signature, feature, operation, and diagnostic.
  The signature validator rejects other scalar/vector/matrix instantiations and parameter orders.
  Emission walks only the already-validated function parameters and asks the established value map
  for their provider handles. No helper-name match, placeholder parser, syntax reconstruction, or
  alternate argument representation remains.
- The facade owns wire-operation-to-feature dispatch; the provider owns
  wire-operation-to-LLVM-intrinsic dispatch. They are separate C-ABI trust boundaries. The
  provider checks count, nullability, exact i32 type, same-module/function ownership, availability,
  dominance, and insertion state before adding the declaration or call. The constant 31 is the
  fourth NVVM operand required by CUDA's default warp-width contract, not a missing Slang value.
- LLVM 14 constructs and verifies the canonical shuffle declaration. Its three attributes and
  four-i32 signature are already valid LLVM 7/NVVM text. The legacy writer's new branch validates
  that semantic declaration without rewriting it; a rewrite would add complexity without fixing a
  measured incompatibility. The real-provider and libNVVM tests fail if the declaration/call shape
  is removed or malformed.
- Fake argument lists extend the existing intrinsic value record and preserve exact parameter
  identity. The wave runtime enum extends the established one-warp launcher with the two additional
  kernel arguments and expected selected-lane value instead of copying another launcher. Existing
  lane-index and lane-count tests remain unchanged at their call sites and pass.

## Failure and Recovery

If LLVM 14's shuffle declaration attributes or NVVM 2.0 spelling differ, record and validate the
exact declaration rather than weakening the audit. If libNVVM rejects the LLVM 14 form, isolate the
smallest legacy-dialect difference and rewrite only after semantic validation. Removing feature/
operation 2 and its descriptor restores Slice 47. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Retain the exact linked-IR signature/spelling, provider LLVM and negotiated NVVM text, direct and
NVRTC PTX mechanisms, `ptxas`/RTX results, table sizes, name count/hash, measured line growth, and
the completed audit. Distill durable evidence into design/ledger and ship this completed plan with
Slice 48.
