# Slice 52: Prove public UInt wave shuffle composition

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route has durable evidence that the public unmasked UInt
`WaveReadLaneAt()` operation compiles and runs by composing the already-negotiated lane-index,
active-mask ballot, and UInt masked-shuffle capabilities. No new provider operation or LLVM wrapper
entry is added for a composition already expressed canonically in linked IR.

## Progress

- [x] (2026-08-28) Recorded the Slice 51 baseline: 340 names, SHA-256
  `7abb718be35a0e9ad61202e3c8776c718f22c43a790c22df960140164cf5ce2b`, Release 340/340,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 25,512 measured lines.
- [x] (2026-08-28) Audited the CUDA source definition, final active-mask-synthesized IR, and direct
  PTX for public UInt `WaveReadLaneAt()`.
- [x] (2026-08-28) Added direct topology and combined capability-negotiation evidence without
  changing the provider ABI or emitter operation set.
- [x] (2026-08-28) Added NVVM/NVRTC PTX, CUDA assembler, and RTX runtime evidence for source lanes
  0 and 7.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and audited the completed
  slice.

## Surprises and Discoveries

- Observation: the CUDA standard-library definition of scalar `WaveReadLaneAt(value, lane)` calls
  `WaveMaskReadLaneAt(WaveGetActiveMask(), value, lane)` rather than producing a new target
  intrinsic.
  Consequence: preserve that composition and require all of its existing feature bits. A new
  `WAVE_READ_LANE_AT_UNMASKED` provider operation would duplicate source-level structure.

- Observation: after active-mask synthesis, the entry computes
  `waveMaskBallot(0xffffffff, true)` and passes that value into a canonical
  `Func(UInt, UInt, Int, UInt)` public helper. The helper calls `WaveGetActiveMask(UInt)` and then
  the existing UInt masked-shuffle helper.
  Consequence: test the exact producer-created call graph and feature union; do not match public
  helper names in production code or rediscover the mask from arbitrary operands.

- Observation: the current direct route's kernel entry already emits exactly one
  `vote.sync.ballot.b32`, one `shfl.sync.idx.b32`, one lane-id read, one global store, and no global
  load for the audit kernel.
  Consequence: this slice can be evidence-only in production terms. Tests and durable documents
  should close the previously explicit unmasked-shuffle boundary without adding unused APIs.

## Decision Log

- Decision: make public unmasked UInt `WaveReadLaneAt()` a bounded composition slice.
  Rationale: Slice 51 deliberately unlocked this standard-library wrapper, and it is the smallest
  end-to-end proof that separately negotiated wave primitives compose through active-mask
  synthesis and direct helper calls.
  Date/author: 2026-08-28, Codex.

- Decision: add no feature, intrinsic operation, facade mapping, or provider implementation.
  Rationale: the canonical final graph contains only operation shapes already owned by features
  35, 36, and 39 plus established scalar/function/pointer transport. Adding another provider
  operation would mirror one source wrapper and make the API less economical.
  Date/author: 2026-08-28, Codex.

- Decision: use five independently registered evidence names rather than the seven-name primitive
  template.
  Rationale: negotiation/provider-construction tests already exist for every constituent
  primitive. This slice needs one direct composition test, one union-negotiation test, and the
  three established real PTX/assembler/runtime layers.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The public UInt wrapper works entirely through existing contracts. Its fake graph has five
functions, three intrinsic emissions, and four calls; the ballot result is the synthesized third
argument to the public helper, passes through `WaveGetActiveMask(UInt)`, and becomes the first
masked-shuffle argument. Clearing feature 35, 36, or 39 independently returns E52016 before module
creation. No production, facade, provider, or ABI file changed, and V3 remains 528/308 bytes.

The standalone Release provider and Release/Debug main targets build successfully. The five new
names pass 5/5, the complete Slice 46-52 wave matrix passes 47/47, Release passes 345/345, and Debug
preservation passes 10/10. The sorted LF-terminated registered name set hashes to
`d112ef187a1ff7999b55ed3222b51f0c5ad01416f04a63b46a70a9d25ccb1029`; removing exactly the five
Slice 52 names yields 340 names and the Slice 51 hash
`7abb718be35a0e9ad61202e3c8776c718f22c43a790c22df960140164cf5ce2b`. Five evidence names add 241
measured physical lines across the five test/support files, from 25,512 to 25,753.

NVVM and NVRTC agree on the `[64, 32]` launch ABI, one global 32-bit store, no load, and exactly one
ballot plus one shuffle in the entry. CUDA 12.9 `ptxas` accepts both, and one full RTX 5090 warp
selects source lanes 0 and 7 correctly through both routes. This validates the economical wrapper
strategy: public composition requires end-to-end evidence, not a duplicate low-level API.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneAt(laneIndex, sourceLane);
}
```

CUDA target selection expands the public wrapper to an active mask plus
`WaveMaskReadLaneAt`. Active-mask synthesis creates one ballot in the entry and threads its UInt
result through the reachable ordinary helpers. The final public helper therefore has its original
UInt value and Int lane parameters plus the synthesized UInt mask parameter. It obtains the mask
through `WaveGetActiveMask(UInt)` and passes that result, the value, and the lane to the already
recognized UInt shuffle helper.

Direct NVVM preflight walks the reachable graph, unions every required feature before module
creation, validates exact helper signatures and calls, and then lowers each constituent through
the existing value map. The facade and provider see only the established ballot, lane-index, and
UInt-shuffle operations.

## Scope and Non-Goals

In scope are scalar UInt public `WaveReadLaneAt`, exact composed fake topology, required-feature
union failures, matching NVVM/NVRTC PTX semantics, `ptxas` acceptance, one-warp runtime source-lane
selection, documents, and the completed plan.

Out of scope are Int/Float public wrappers, vectors, matrices, 64-bit or half values, divergent
control flow, invalid lane behavior, new provider ABI or operations, production emitter changes,
performance claims, and other vote/shuffle/reduction operations.

## Architecture and Invariants

The source library owns the public-to-masked wrapper. Active-mask synthesis owns mask creation and
threading. Direct NVVM consumes only the canonical resulting functions, calls, ballot, and exact
GenericAsm shuffle helper. Each layer retains one source of truth; the evidence must not add a
shadow representation of unmasked shuffle.

Preflight must require all constituent capabilities before provider module mutation. Clearing lane
index, UInt masked shuffle, or ballot independently must produce the established E52016
capability diagnostic with no provider emission. With all features present, fake topology must
show the ballot result flowing into the public helper, through the active-mask identity call, and
into the masked-shuffle call.

## Interfaces and Dependencies

Add one source fixture, one runtime parameter shape, and five tests across
`unit-test-nvvm-support.h`, `unit-test-nvvm-emitter.cpp`, and
`unit-test-nvvm-integration.cpp`. Update `docs/design/nvvm-backend.md`, the capability ledger, and
this plan. Do not change V3, exported symbols, facade/provider code, LLVM components, or NVVM
serialization.

## Milestones

1. Add the public UInt source fixture and fake direct graph assertions for ballot-plus-shuffle
   composition.
2. Prove preflight requires lane-index, UInt shuffle, and ballot features as one union before
   module construction.
3. Add differential PTX, `ptxas`, and RTX/NVRTC runtime evidence for source lanes 0 and 7.
4. Run focused and preservation matrices, full Release/Debug checks, hash/measure the registered
   names, document the result, audit the diff, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
five new names, the complete Slice 46-52 wave matrix, full Release NVVM prefix, and Debug 10/10
preservation.

Accept unchanged 528/308-byte V3 tables and no production/provider diff; exact fake call and
intrinsic topology; independent E52016 failure before module creation for each missing constituent
feature; matching `[64, 32]` PTX ABI; one ballot and one UInt shuffle; one global store and no
global load; CUDA 12.9 `ptxas` acceptance; correct lane 0 and 7 selection through NVVM and NVRTC on
one RTX 5090 warp; exact name/hash continuity; formatted code; clean diff checks; and a completed
input-shape audit.

## Self-Review and Input-Shape Audit

Inventory the new source fixture, direct graph assertions, feature-union loop, runtime parameter
shape, and real evidence. The final graph is intentionally produced by standard-library target
selection and active-mask synthesis. Tests observe it through existing fake callback records and
compiled output; no production helper, fallback, name matcher, custom equivalence, operand-graph
walk, syntax reconstruction, or new special case is justified for this slice.

The public helper's added UInt mask parameter is canonical and its repaired function type comes
from Slice 51's producer-side fix. Its call to the active-mask identity helper and then the masked
shuffle are valid ordinary direct calls. If testing exposes another shape, audit its producer
before changing the emitter.

## Failure and Recovery

If direct compilation stops composing, inspect CUDA target selection and active-mask synthesis
before adding any provider operation. If one missing feature is not detected during preflight,
trace which canonical instruction requires it rather than adding a public-wrapper feature. The
slice changes only tests and documentation, so removing its five names and fixture restores Slice
51. Never stage `external/slang-binaries/` or temporary audit artifacts.

## Artifacts and Hand-Off

Retain the final linked-IR call graph, feature-union evidence, NVVM/NVRTC PTX summaries,
`ptxas`/RTX results, counts, hashes, line growth, and completed audit in this plan and the durable
design/ledger entries. Remove `tmp-slice-52-*` audit files before staging.
