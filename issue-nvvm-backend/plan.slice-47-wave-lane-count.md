# Slice 47: Add wave lane count through the generic intrinsic family

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs a CUDA kernel that combines
`WaveGetLaneIndex()` and `WaveGetLaneCount()`, storing the lane count at each lane-indexed UInt
destination element. The existing append-only intrinsic callback gains one stable operation and
feature without growing the V3 table. Direct classification uses one descriptor mapping from the
exact CUDA-selected GenericAsm spelling to provider operation/feature.

## Progress

- [x] (2026-08-28) Recorded Slice 46 baseline: 305 names, SHA-256
  `a5d99d25f4218d69bf938e171083e49c3826150873a58506c42e2b8bcbf98dbb`, Release 305/305,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 23,368 measured lines.
- [x] (2026-08-28) Audited `WaveGetLaneCount`, exact CUDA-selected linked IR, the current generic
  intrinsic path, LLVM 14/LLVM 7 NVPTX intrinsic catalogs, and shared declaration attributes.
- [x] (2026-08-28) Extended the generic intrinsic operation family with feature 35/operation 1;
  V3 remains 528 bytes on x64 and 308 bytes on x86.
- [x] (2026-08-28) Generalized exact GenericAsm classification and legacy declaration-attribute
  auditing for
  composed lane index/count modules.
- [x] (2026-08-28) Added seven independently named provider/direct/capability/PTX/assembler/runtime
  evidence layers; the unsupported matrix retains its Slice 46 boundaries.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and completed the
  input-shape audit; prepared the complete slice for commit.

## Surprises and Discoveries

- Observation: CUDA target selection produces a retained zero-parameter `Func(UInt)` helper whose
  sole block terminates with exact `GenericAsm("(warpSize)")`.
  Consequence: extend the same canonical terminator mapping established by Slice 46; do not search
  helper names or source syntax.

- Observation: LLVM 14 and LLVM 7 both catalog
  `llvm.nvvm.read.ptx.sreg.warpsize`, returning signless i32.
  Consequence: add operation 1 to the existing generic callback and map it directly to the LLVM
  NVPTX intrinsic.

- Observation: lane-id and warp-size declarations carry the same LLVM 14 optimization attribute
  set and LLVM assembly may share one numbered attribute group between them.
  Consequence: validate every semantic declaration but compare serialized rewrites against unique
  semantic attribute sets, not declaration count.

- Observation: libNVVM lowers the warp-size intrinsic to PTX `WARP_SZ`, not the `%warpsize`
  spelling suggested by the special-register intrinsic name.
  Consequence: assert the exact downstream token selected by libNVVM and retain runtime equality;
  do not prescribe a different legal PTX spelling.

- Observation: a full copied wave provider/runtime/differential harness would repeat most of Slice
  46 for one operation.
  Consequence: refactor provider population into one optional composed topology, runtime execution
  into one expected-value helper, and PTX comparison into one callback-driven runner. Seven new
  evidence names add 364 measured lines rather than another full Slice 46-sized layer.

## Decision Log

- Decision: add feature 35 `WAVE_LANE_COUNT` and intrinsic operation 1 through the unchanged
  callback suffix.
  Rationale: feature negotiation remains semantic while the generic operation vector avoids a
  lane-count-specific wrapper method or ABI field. Exact Slice 46 tables implement the callback
  and remain compatible when the new feature is clear.
  Date/author: 2026-08-28, Codex.

- Decision: make the end-to-end fixture use both lane index and lane count.
  Rationale: composition proves two retained UInt helpers, two operations through one callback,
  shared legacy attribute handling, lane-indexed UInt storage, and exact one-warp lane count in one
  bounded kernel.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Feature 35 and operation 1 extend the existing generic intrinsic callback. V3 stays 528 bytes on
x64 and 308 bytes on x86. An exact Slice 46 provider initializes successfully with feature 35
clear, retains lane index, and returns `SLANG_E_NOT_AVAILABLE` for lane count without dispatch.
Providers advertising lane count still require the complete intrinsic suffix; unknown operations
remain invalid.

The final linked IR has two canonical zero-parameter UInt helpers ending in exact
`GenericAsm("_getLaneId()")` and `GenericAsm("(warpSize)")`. The kernel calls both, uses the first
result for its device-pointer offset, and stores the second. One direct descriptor supplies text,
feature, operation, and diagnostic to preflight/emission. The fake graph records both helpers,
intrinsics, returns, calls, offset, and store without a lane-count-specific value representation.

LLVM and audited NVVM 2.0 text contain one lane-id and one warp-size intrinsic call, two helper
calls/returns, one GEP, and one store. Both declarations share one exact LLVM 14 attribute set, and
legacy text contains one rewritten `nounwind readnone` group. NVVM and NVRTC agree on `[64]`, one
global 32-bit store, and no load; direct PTX uses `%laneid` and `WARP_SZ`. CUDA 12.9 `ptxas` accepts
both routes, and one 32-thread RTX 5090 warp writes 32 to all 32 elements through both compilers.

The focused Slice 46/47 matrix passes 14/14; Release passes 312/312 with sorted LF-terminated
name-set SHA-256 `dbd8d587f633ab06ac2daaf086690a14fa3b9f4cab8c22332d0a75e562d65ab7`;
removing the seven new names reproduces Slice 46's 305-name hash exactly. Debug preservation passes
10/10. The five measured files grow by 364 physical lines, from 23,368 to 23,732.

## Context and Current Pipeline

Slice 46 admits sign-independent canonical Int/UInt transport, exact UInt device pointers, generic
UInt helper calls/returns, and one exact target intrinsic. Feature 34 appends
`emitIntrinsic(module, operation, arguments, count, outValue)` and operation 0 for lane index. The
provider emits LLVM's lane-id intrinsic, while the legacy writer narrows its exact six LLVM 14
optimization attributes for the LLVM-7-era NVVM 2.0 parser.

`WaveGetLaneCount()` is a target intrinsic in `hlsl.meta.slang`. CUDA selection materializes
`GenericAsm("(warpSize)")` in an otherwise identical UInt helper. The existing UInt transport is
already sufficient for its helper call and store.

## Scope and Non-Goals

In scope are feature 35, operation 1, one shared operation-to-feature map in the facade, one shared
GenericAsm descriptor map in direct lowering, LLVM warpsize intrinsic emission, unique legacy
attribute-group accounting, combined lane-index/lane-count fake and real topology, and seven
end-to-end evidence layers.

Out of scope are arbitrary GenericAsm, compile-time folding of warp size, wave index/num waves,
ballots, votes, shuffles, reductions, masks, convergence, unsigned constants/arithmetic/
comparisons, dynamic warp sizes outside CUDA's contract, vectors, and performance claims.

## Architecture and Invariants

The exact semantic helper shape remains a non-entry, one-block, zero-parameter `Func(UInt)` with a
one-operand GenericAsm terminator. A descriptor owns exact asm text, stable operation, feature, and
diagnostic. Preflight and emission consume that same descriptor so accepted semantics cannot drift
from dispatch.

The wrapper maps each known operation to its independent feature before invoking the unchanged
callback. The provider requires zero arguments and a valid unterminated insertion block, maps
operation 0/1 to the corresponding LLVM intrinsic ID, and returns its i32 call. Unknown operations
remain invalid.

The legacy writer validates each lane-id/warp-size declaration's exact signature and six LLVM 14
attributes. It counts unique semantic attribute sets because LLVM prints one shared numbered group
for identical sets, then requires exact equality with rewritten serialized groups.

## Interfaces and Dependencies

Append one feature bit, one intrinsic operation, and an alias minimum-size macro for the unchanged
callback suffix. Extend facade dispatch, provider mapping/writer audit, direct descriptor mapping,
fake state/fixtures, tests, design, ledger, and this plan. Add no callback, table field, ABI version,
V2 change, export, LLVM component, source rewrite, or raw-assembly transport.

## Milestones

1. Add feature 35/operation 1 with exact Slice 46 compatibility and unchanged table sizes.
2. Share exact GenericAsm descriptor and provider intrinsic mapping across lane index/count.
3. Make the legacy writer handle one shared attribute group for both validated declarations.
4. Add seven named negotiation, provider, direct, capability, differential, `ptxas`, and runtime
   tests around the composed kernel.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run all
new names, Slice 46 lane-index preservation, generic-intrinsic prefix/invalid tests, unsupported
matrix, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte table sizes; exact Slice 46 compatibility; two exact LLVM/NVVM
intrinsic declarations/calls with one shared rewritten attribute group; direct fake topology from
both helpers through one callback family to lane-indexed UInt storage; matching `[64]` PTX ABI and
store/no-load behavior; direct `%laneid` and `WARP_SZ` evidence; `ptxas` acceptance; one-warp
runtime output of 32 at all 32 lane-indexed elements; exact name continuity; formatted code;
completed input-shape audit; and clean diff checks.

## Self-Review and Input-Shape Audit

The new-helper/fallback/special-case inventory is the lane-count feature/operation mapping, exact
GenericAsm descriptor, provider operation switch, unique attribute-set accounting, composed fake
population, shared wave runtime, and shared PTX runner. All survive the audit:

- CUDA target selection produces the exact valid input shape: a reachable, defined,
  zero-parameter `Func(UInt)` helper whose one-block terminator is one-operand
  `GenericAsm("(warpSize)")`. The existing closure and terminator walk are the source of truth.
  Removing the descriptor row restores the E52017 GenericAsm boundary, proving direct target
  preflight owns this canonical spelling rather than a source-name or syntax reconstruction.
- The descriptor is one source of truth for exact assembly text, provider operation, feature, and
  diagnostic. Both capability collection and emission consume it. The shape guard remains outside
  the descriptor and requires the same non-entry/one-block/no-parameter/UInt contract for every
  admitted target intrinsic.
- The facade owns wire-operation-to-feature dispatch; the provider owns wire-operation-to-LLVM-ID
  dispatch. They are separate trust boundaries over the shared C ABI. Unknown operations fail in
  both. No operation-specific method or callback is introduced, and the Slice 46 callback/prefix
  remains the sole transport.
- LLVM 14 constructs and verifies both canonical declarations. Their identical valid attribute
  sets are context-uniqued and serialize as one numbered group. Counting declarations would reject
  a valid composed module; counting unique already-validated semantic sets matches LLVM's writer
  representation while the exact six-attribute/signature checks still reject any alternate shape.
  The combined real-provider test fails without this change and proves the dialect writer owns it.
- The fake population and runtime changes parameterize the established wave topology and expected
  value instead of introducing a second representation. Slice 46's lane-only builder/runtime tests
  remain unchanged and pass, while the composed path adds the second helper/call and pointer offset.
  The PTX runner centralizes only ABI/store/no-load mechanics and leaves semantic instruction checks
  in each thin registered test.

## Failure and Recovery

If LLVM 14 assigns different warp-size attributes, record and validate the exact declaration rather
than weakening the audit. If NVRTC folds warp size to a literal, retain route-specific PTX evidence
and runtime equality. Removing operation/feature 1 and the lane-count descriptor restores Slice 46.
Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Retain unchanged table sizes, exact linked-IR spelling, composed fake topology, generic/NVVM text,
attribute sharing, PTX mechanisms, `ptxas`, RTX/NVRTC results, counts/hashes, line growth, and the
completed audit. Distill durable evidence to design/ledger and ship this plan with Slice 47.
