# Slice 53: Compose public Int wave reads without duplicating tests

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public unmasked Int `WaveReadLaneAt()` by composing
the established active-mask ballot with the signed masked shuffle. The composition tests introduced
for UInt become shared scalar runners, so Int and later scalar cases add thin evidence wrappers
instead of another large copied topology test.

## Progress

- [x] (2026-08-28) Recorded the Slice 52 baseline: 345 names, SHA-256
  `d112ef187a1ff7999b55ed3222b51f0c5ad01416f04a63b46a70a9d25ccb1029`, Release 345/345,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 25,753 measured lines.
- [x] (2026-08-28) Audited CUDA target selection and direct PTX for public Int
  `WaveReadLaneAt(source[lane], sourceLane)`.
- [x] (2026-08-28) Refactored UInt composition topology, negotiation, and PTX checks into
  parameterized runners.
- [x] (2026-08-28) Added five thin Int direct/capability/PTX/assembler/runtime evidence names.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and audited the completed
  slice.

## Surprises and Discoveries

- Observation: the final Int graph has the same ballot/public-helper/active-mask/masked-helper
  topology as UInt, but the entry loads the shuffled Int from a device pointer and requires signed
  shuffle feature 37.
  Consequence: parameterize the value-origin kind, load/pointer counts, and shuffle feature/
  operation while keeping one exact call-flow assertion.

- Observation: copying the 112-line Slice 52 direct assertion and 40-line capability test would
  grow the suite for facts that differ in only four fields.
  Consequence: extract runners now that a second row proves the abstraction. Keep independently
  registered wrappers so failures remain type- and layer-specific.

- Observation: direct PTX entry ABI is `[64, 64, 32]` and contains one ballot, one signed 32-bit
  load, one shuffle, and one signed 32-bit store.
  Consequence: reuse one composition PTX checker with parameter widths and expected-load data;
  retain exact one-ballot/one-shuffle assertions per route.

## Decision Log

- Decision: make public unmasked Int `WaveReadLaneAt()` the next composition slice.
  Rationale: it is the smallest new source-level behavior after UInt and proves active-mask
  composition with the already-supported loaded signed-value path.
  Date/author: 2026-08-28, Codex.

- Decision: parameterize the composition evidence at its second use.
  Rationale: the stable invariant is the mask call-flow and capability union; value type, origin,
  feature/operation, launch ABI, and load expectation are data. A shared runner makes those
  distinctions explicit and bounds future test growth without hiding registered evidence.
  Date/author: 2026-08-28, Codex.

- Decision: add no backend feature, facade operation, provider code, or LLVM wrapper.
  Rationale: final linked IR contains only established operation shapes and ordinary calls.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Public unmasked Int wave read composes without backend changes. The shared fake runner proves five
functions, three intrinsic emissions, four calls, a loaded entry value, and exact mask flow into
the signed masked shuffle. Clearing feature 35, 37, or 39 independently returns E52016 before
module creation. The existing UInt row passes unchanged through the same runners.

The standalone Release provider and Release/Debug main targets build successfully. The five new
names pass 5/5, all ten UInt/Int composition names pass 10/10, the complete Slice 46-53 wave matrix
passes 52/52, Release passes 350/350, and Debug preservation passes 10/10. The sorted LF-terminated
name set hashes to `003afec34f28ad32e84961b91f1c87fff1fa006f1da535cb10ab00d29cc727c7`;
removing exactly the five Slice 53 names yields 345 names and the Slice 52 hash
`d112ef187a1ff7999b55ed3222b51f0c5ad01416f04a63b46a70a9d25ccb1029`. The five measured files
grow by only 121 lines, from 25,753 to 25,874, compared with the first composition row's 241 lines.

NVVM and NVRTC agree on `[64, 64, 32]`, one global 32-bit load/store pair, and exactly one ballot
plus one shuffle in the entry. CUDA 12.9 `ptxas` accepts both, and RTX 5090 runtime selects the
negative lane-0 and lane-7 values bit-exactly through both routes. Parameterizing at the second use
keeps exact assertions while making the test suite's growth sublinear across scalar rows.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneAt(source[laneIndex], sourceLane);
}
```

The CUDA standard-library wrapper constructs
`WaveMaskReadLaneAt(WaveGetActiveMask(), value, lane)`. Active-mask synthesis computes one ballot
in the entry and threads its result into the specialized public Int helper. The entry also computes
lane index, loads one Int source element, calls that helper with value/lane/mask, and stores the
selected value. The helper passes its mask through `WaveGetActiveMask(UInt)` and into the exact Int
masked-shuffle helper.

Direct preflight unions lane-index feature 35, signed-shuffle feature 37, ballot feature 39, and
the established scalar/function/pointer capabilities before provider module creation. The provider
sees only the three established intrinsic operations.

## Scope and Non-Goals

In scope are public scalar Int `WaveReadLaneAt`, parameterized UInt/Int composition runners, exact
fake topology and feature-union failures, NVVM/NVRTC PTX, `ptxas`, RTX runtime lanes 0 and 7,
documents, and the completed plan.

Out of scope are Float and other public wrapper types, vectors, matrices, new provider operations,
production emitter changes, divergent control flow, invalid lanes, performance claims, and other
wave operations.

## Architecture and Invariants

The production ownership remains unchanged: source library owns the wrapper, active-mask synthesis
owns ballot creation/threading, direct NVVM consumes canonical final calls and operations, and the
provider emits existing intrinsics. Test runners describe only observable dimensions; they do not
become a second production mapping.

The shared fake runner must continue to prove the exact ballot-to-public-helper and active-mask-
identity-to-masked-shuffle flow. Its parameters may select source text, expected shuffle operation,
entry value origin, pointer/load counts, and shuffle feature. Each registered wrapper supplies one
complete row. The PTX runner similarly owns only ABI/load/mechanism assertions.

## Interfaces and Dependencies

Add one Int source fixture and runtime mode in `unit-test-nvvm-support.h`. Refactor and extend
composition tests in `unit-test-nvvm-emitter.cpp` and `unit-test-nvvm-integration.cpp`. Update the
design, capability ledger, and this plan. Do not change production sources, V3, exports, LLVM
components, or serialization.

## Milestones

1. Extract shared direct-topology and capability-union runners from the Slice 52 UInt tests, then
   preserve the existing registered UInt names as thin wrappers.
2. Add the Int source row and registered direct/capability wrappers with exact load and signed
   shuffle expectations.
3. Extract the common ballot-plus-shuffle PTX checker and add Int differential, `ptxas`, and RTX
   runtime wrappers.
4. Format/build, run focused/full/Debug lanes, hash names, measure line growth, document and audit,
   remove temporary artifacts, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
five new Int names, all ten public UInt/Int composition names, the complete Slice 46-53 wave matrix,
full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3 tables and no production/provider diff; preserved UInt behavior;
exact five-function/three-intrinsic/four-call Int topology; independent E52016 failures for features
35, 37, and 39 before module creation; `[64, 64, 32]`; one global load/store and exactly one ballot/
shuffle per entry; `ptxas` acceptance; bit-exact negative lane values for lanes 0 and 7 through
NVVM and NVRTC; exact name/hash continuity; bounded or reduced marginal test growth;
formatted code; clean diff checks; and a completed input-shape audit.

## Self-Review and Input-Shape Audit

Inventory the shared runners, their parameter fields, thin wrappers, new fixture, and runtime mode.
Every parameter must correspond to a measured canonical difference between UInt and Int. Reject a
generic abstraction that hides call-flow assertions or reconstructs IR by names.

The input shape is valid and producer-owned: standard-library target selection and active-mask
synthesis create the same canonical composition with an Int value parameter and a synthesized UInt
mask parameter. The entry's source load is ordinary established direct IR. Tests observe callback
records and output; production code needs no helper, fallback, equivalence relation, or special
case.

## Failure and Recovery

If the shared runner cannot express both exact graphs without branching on incidental function
ordering or names, keep separate assertions and record why. If compilation exposes a new operation,
audit its producer before expanding the backend. Removing the five Int wrappers/fixture and
reverting the test-only runner refactor restores Slice 52. Never stage `external/slang-binaries/`
or `tmp-slice-53-*` artifacts.

## Artifacts and Hand-Off

Retain the final Int IR/PTX shape, shared-runner dimensions, feature failures, NVVM/NVRTC/
`ptxas`/RTX results, hashes, line impact, and completed audit. Distill durable results into the
design and ledger, then commit this completed plan with Slice 53.
