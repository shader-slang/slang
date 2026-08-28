# Slice 55: Add public UInt wave-read-lane-first

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public UInt `WaveReadLaneFirst()` by composing the
established active-mask ballot with a newly negotiated UInt read-first primitive. The provider
derives the first participating lane with LLVM's generic count-trailing-zeros intrinsic and emits
one synchronized indexed shuffle, matching NVRTC at LLVM/NVVM, PTX, assembler, and runtime layers.

## Progress

- [x] (2026-08-28) Recorded the Slice 54 baseline: 355 names, SHA-256
  `492d3838278e789e6b0ebabc8798653ba6fdccefc90dbe946b46f8d224453f9e`, Release 355/355,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 25,957 measured lines.
- [x] (2026-08-28) Audited CUDA source-library selection, final linked IR, CUDA prelude semantics,
  LLVM 7/14 `cttz` definitions, and NVRTC PTX for public UInt `WaveReadLaneFirst()`.
- [x] (2026-08-28) Appended feature 40/operation 6 and lowered the exact UInt
  `_waveReadFirst(mask, value)` helper through generic `cttz` plus indexed shuffle.
- [x] (2026-08-28) Added seven provider/direct/capability/PTX/assembler/runtime evidence names
  through the public Slang path and extended the shared public-wave runner by operation arity.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and audited the
  completed slice.

## Surprises and Discoveries

- Observation: public CUDA `WaveReadLaneFirst(value)` composes `WaveGetActiveMask()` with
  `WaveMaskReadLaneFirst(mask, value)`, whose exact helper ends in
  `_waveReadFirst($0, $1)`. Active-mask synthesis leaves one canonical `GenericAsm` helper with
  signature `UInt(UInt mask, UInt value)`.
  Consequence: add one semantic provider primitive for that final helper; do not bypass the source
  wrapper, match its function name, or duplicate active-mask synthesis.

- Observation: the CUDA prelude implements read-first as `__ffs(mask) - 1` followed by
  `__shfl_sync(mask, value, lane)`. NVRTC emits `brev.b32`, `bfind.shiftamt.u32`, and exactly one
  synchronized indexed shuffle without a zero-mask branch.
  Consequence: use `llvm.cttz.i32(mask, true)` for the intentionally nonzero participating mask and
  feed its result to the established i32 shuffle intrinsic with clamp 31.

- Observation: LLVM 7 and LLVM 14 expose the same overloaded `i32(i32, i1)` `cttz` signature, but
  LLVM 14 adds optimization and immediate-argument attributes unknown to the older reader.
  Consequence: validate the exact generated declaration and normalize only those proven
  version-specific attributes in the legacy writer.

- Observation: libNVVM reports a parse error exactly at LLVM 14's `immarg` marker. After the
  audited rewrite, it accepts the module and lowers `cttz` to a `popc` sequence; NVRTC instead
  emits `brev` plus `bfind.shiftamt` for the CUDA-prelude expression.
  Consequence: prove the exact LLVM semantic and each route's first-lane PTX mechanism separately,
  then use runtime parity as the cross-route semantic oracle.

## Decision Log

- Decision: make public UInt wave-read-first the next bounded wave primitive.
  Rationale: it is the next source-visible scalar operation rejected by one exact `GenericAsm`
  helper and exercises a reusable first-active-lane mechanism without widening type scope.
  Date/author: 2026-08-28, Codex.

- Decision: append one feature and one intrinsic operation to the existing generic V3 callback.
  Rationale: the callback already transports the two i32 operands and owns target-intrinsic
  semantics; a new function pointer would grow the wrapper surface without adding a new value kind.
  Date/author: 2026-08-28, Codex.

- Decision: model the participating mask as nonzero at the provider boundary.
  Rationale: synchronized shuffle requires the executing lane to participate in its mask, so the
  valid operation domain is nonzero. `cttz(mask, true)` matches NVRTC's branch-free mechanism and
  does not invent behavior for an out-of-contract zero mask.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The direct route now recognizes only the exact canonical UInt read-first helper and negotiates
feature 40 before module creation. Provider operation 6 performs all two-i32 validation before
emitting `llvm.cttz.i32(mask, true)` and the established synchronized indexed i32 shuffle. The
legacy writer verifies the complete LLVM 14 `cttz` declaration, removes its unsupported `immarg`
marker and newer optimization attributes with exact rewrite counts, and produces text accepted by
the LLVM 7-based libNVVM reader.

The public-path fake proves five functions, three intrinsic emissions, four calls, and ballot flow
through the active-mask identity into the two-argument masked operation. Generalizing the existing
public-wave runner by operation arity avoided a second topology body. Removing feature 34, 39, or
40 independently returns E52016 before provider module construction. V3 remains 528/308 bytes.

The standalone Release provider and Release/Debug main targets build successfully. The seven new
names pass 7/7, the complete Slice 46-55 wave matrix passes 64/64, Release passes 362/362, and Debug
preservation passes 10/10. The sorted LF-terminated name set hashes to
`652de9ad6905f2e885264851e4245cdc88e9119414a920111ee081b557ff786f`; removing exactly the seven
Slice 55 names yields 355 names and the Slice 54 hash
`492d3838278e789e6b0ebabc8798653ba6fdccefc90dbe946b46f8d224453f9e`. Seven evidence names add
347 measured physical lines across the five test/support files, from 25,957 to 26,304.

NVVM and NVRTC agree on `[64]`, one store/no load, one ballot, and one shuffle in the entry. NVVM
uses `popc`; NVRTC uses `brev` plus `bfind.shiftamt`; CUDA 12.9 `ptxas` accepts both, and every lane
in one full RTX 5090 warp reads lane zero's UInt value through both routes.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneFirst(laneIndex);
}
```

CUDA source selection composes the public call from `WaveGetActiveMask()` and
`WaveMaskReadLaneFirst`. Active-mask synthesis turns the former into
`waveMaskBallot(0xffffffff, true)` and threads its UInt result through the public helper. The exact
masked helper ends in `_waveReadFirst($0, $1)`, which is the only unsupported final instruction.

The provider already owns lane index, ballot, indexed i32 shuffle, functions, calls, and UInt
stores. The new operation must validate `(i32 mask, i32 value) -> i32`, derive the least set lane,
and reuse the native synchronized shuffle declaration.

## Scope and Non-Goals

In scope are public scalar UInt `WaveReadLaneFirst`, feature 40/operation 6, exact helper matching,
generic LLVM `cttz.i32`, legacy declaration validation/normalization, fake and real fixtures, and
provider/direct/capability/PTX/`ptxas`/RTX evidence.

Out of scope are Int/Float or aggregate rows, arbitrary explicit masks, zero-mask behavior,
divergent control flow, performance claims, additional callback fields, and other wave operations.

## Architecture and Invariants

Source-library selection and active-mask synthesis remain the producers of the canonical helper
graph. Direct NVVM recognizes only the exact assembly spelling plus complete UInt helper signature,
requires feature 40, and forwards existing parameter handles through the generic callback.

The facade maps operation 6 to feature 40. The provider performs all ownership, insertion-point,
arity, and exact-type checks before mutation; emits `cttz.i32(mask, true)`; then emits
`llvm.nvvm.shfl.sync.idx.i32(mask, value, firstLane, 31)`. The legacy writer validates each
declaration before applying narrowly counted compatibility rewrites.

## Interfaces and Dependencies

Append feature 40, intrinsic operation 6, and a minimum-size alias to V3. Extend facade, provider,
direct emitter, fake, fixtures, tests, design, ledger, and this plan. Do not change table layout,
ABI version, V2, exports, LLVM components, or serialization formats.

## Milestones

1. Append feature 40/operation 6 with exact Slice 54 compatibility and unchanged V3 sizes.
2. Match and lower canonical `UInt(UInt, UInt)` `_waveReadFirst($0, $1)` through direct NVVM.
3. Emit and legacy-audit generic `cttz.i32` plus the established synchronized i32 shuffle.
4. Add public-path provider/direct/capability/PTX/`ptxas`/RTX evidence for one full warp.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
new Slice 55 names, the complete prior wave matrix, generic-intrinsic compatibility/invalid tests,
the unsupported boundary, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3 tables; exact Slice 54 provider compatibility; one validated
`llvm.cttz.i32` call and one `llvm.nvvm.shfl.sync.idx.i32` call; public fake topology with ballot
flow into read-first; independent feature-40 failure before module creation; matching NVVM/NVRTC
entry ABI and ballot/shuffle mechanism; CUDA 12.9 `ptxas` acceptance; every lane reading UInt zero
from lane zero on the RTX 5090 through both routes; hash continuity; formatted code; completed
input-shape audit; temporary artifact removal; and clean diff checks.

## Self-Review and Input-Shape Audit

Inventory the appended mappings, one exact GenericAsm descriptor, provider compound operation,
legacy declaration rewrite, fake result classification, fixtures, and evidence. For each, prove
checks precede mutation, existing semantic operands remain the source of truth, and no function-name
matching, graph rediscovery, syntax reconstruction, fallback, or custom equivalence was added.

The input shape is canonical: the CUDA source library deliberately implements public read-first by
feeding the synthesized active mask and scalar value to its exact masked helper. The final helper's
assembly spelling and complete signature uniquely identify a real target semantic at the existing
direct-lowering boundary. The provider derives a lane index because that is the operation's CUDA
prelude definition, not to repair malformed upstream IR.

The valid synchronized operation requires a participating nonzero mask. Public execution supplies
the active mask of every executing lane, so the provider's zero-undefined `cttz` flag preserves the
contract and matches NVRTC's branch-free PTX. This slice does not claim or test explicit zero-mask
behavior.

## Failure and Recovery

If LLVM 14 declaration attributes differ from the audited form, stop before broad text rewriting
and isolate the exact LLVM 7 parser contract. If final linked IR differs, fix the plan before adding
alternate spellings. Removing feature 40, operation 6, the descriptor/provider case, and Slice 55
evidence restores Slice 54. Never stage `external/slang-binaries/` or `tmp-slice-55-*` artifacts.

## Artifacts and Hand-Off

Retain the final helper IR, LLVM/NVVM declaration and call, PTX mechanism, `ptxas`/RTX results,
sizes, hashes, line growth, and completed audit. Distill durable architecture and evidence into the
design and ledger, then commit this completed plan with Slice 55.
