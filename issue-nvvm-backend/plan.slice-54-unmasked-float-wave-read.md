# Slice 54: Add a thin public Float wave-read row

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public unmasked Float `WaveReadLaneAt()` by
composing the established active-mask ballot and Float masked shuffle. The parameterized
composition evidence from Slice 53 admits Float as a thin third row without backend or test-runner
architecture changes.

## Progress

- [x] (2026-08-28) Recorded the Slice 53 baseline: 350 names, SHA-256
  `003afec34f28ad32e84961b91f1c87fff1fa006f1da535cb10ab00d29cc727c7`, Release 350/350,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 25,874 measured lines.
- [x] (2026-08-28) Audited CUDA target selection and direct PTX for public Float
  `WaveReadLaneAt(source[lane], sourceLane)`.
- [x] (2026-08-28) Added the Float fixture/runtime row and five thin registered evidence wrappers.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, and audited the completed
  slice.

## Surprises and Discoveries

- Observation: Float has the exact shared five-function composition topology. Only the loaded
  value kind, feature/operation 38/4, and f32 PTX load/store spelling differ from Int.
  Consequence: supply a new data row to the existing direct, capability, and PTX runners; do not
  change their control flow or add another helper.

- Observation: direct PTX entry ABI is `[64, 64, 32]` with one ballot, one `ld.global.f32`, one
  `shfl.sync.idx.b32`, and one `st.global.f32`.
  Consequence: retain the common mechanism checks and use the existing bit-exact Float runtime
  values for source lanes 0 and 7.

## Decision Log

- Decision: complete the public scalar wave-read trio with Float.
  Rationale: it closes the natural wrapper set unlocked by masked UInt/Int/Float plus active mask,
  and proves the Slice 53 test abstraction scales without another refactor.
  Date/author: 2026-08-28, Codex.

- Decision: add exactly five thin evidence names and no new direct/capability/PTX runner fields.
  Rationale: the existing dimensions already describe Float canonically: source text, Float
  shuffle operation/feature, loaded value origin, two pointer offsets, one load, parameter widths,
  and expected global load.
  Date/author: 2026-08-28, Codex.

- Decision: add no backend feature, facade/provider code, or LLVM wrapper.
  Rationale: all final operations and types are established contracts.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Float fits the existing direct/capability/PTX composition runners without a new field or branch.
Its fake row proves the same five functions, three intrinsic emissions, four calls, and exact
ballot/mask flow while selecting a loaded value, Float shuffle operation 4, two pointer offsets,
and one load. Clearing feature 35, 38, or 39 independently returns E52016 before module creation.
No production or ABI file changes, and V3 remains 528/308 bytes.

The standalone Release provider and Release/Debug main targets build successfully. The five new
names pass 5/5, all fifteen scalar composition names pass 15/15, the complete Slice 46-54 wave
matrix passes 57/57, Release passes 355/355, and Debug preservation passes 10/10. The sorted
LF-terminated name set hashes to
`492d3838278e789e6b0ebabc8798653ba6fdccefc90dbe946b46f8d224453f9e`; removing exactly the five
Slice 54 names yields 350 names and the Slice 53 hash
`003afec34f28ad32e84961b91f1c87fff1fa006f1da535cb10ab00d29cc727c7`. The measured files grow by
only 83 lines, from 25,874 to 25,957, below Slice 53's 121-line second-row cost.

NVVM and NVRTC agree on `[64, 64, 32]`, one `ld.global.f32`/`st.global.f32` pair, and exactly one
ballot plus one shuffle in the entry. CUDA 12.9 `ptxas` accepts both, and RTX 5090 runtime selects
source lanes 0 and 7 bit-exactly through both routes. The third row confirms the shared evidence
keeps incremental growth bounded while preserving precise layer-local failures.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneAt(source[laneIndex], sourceLane);
}
```

CUDA target selection composes `WaveGetActiveMask()` with Float `WaveMaskReadLaneAt`. Active-mask
synthesis threads one ballot result into the public Float helper. The entry loads the Float source,
calls that helper with value/lane/mask, and stores the selected result. The helper routes the mask
through the active-mask identity to the already-supported Float masked-shuffle helper.

Direct preflight unions features 35, 38, and 39 plus established scalar Float/function/pointer
transport before provider module creation. Provider emission uses the existing lane-index, ballot,
and Float shuffle operations.

## Scope and Non-Goals

In scope are public scalar Float `WaveReadLaneAt`, five thin evidence wrappers, one fixture/runtime
mode, exact fake/feature/PTX/`ptxas`/RTX evidence, documents, and the completed plan.

Out of scope are additional runner abstraction, other scalar/vector/matrix types, new backend APIs,
production emitter changes, divergent control flow, invalid lanes, performance claims, and other
wave operations.

## Architecture and Invariants

Production ownership remains source wrapper, active-mask synthesis, direct canonical consumption,
and provider primitive emission. Slice 53's shared test runners remain the single source of truth
for scalar unmasked composition assertions. The Float wrappers must supply measured data only and
retain their own registered failure names.

The fake graph must prove the same exact ballot-to-public-helper and active-mask-to-shuffle flow,
with a loaded Float entry value, Float operation 4, two pointer offsets, and one load. Clearing
feature 35, 38, or 39 independently must fail before module creation.

## Interfaces and Dependencies

Add one Float source fixture and runtime mode in `unit-test-nvvm-support.h`; add thin wrappers in
`unit-test-nvvm-emitter.cpp` and `unit-test-nvvm-integration.cpp`; update the design, ledger, and
this plan. Do not change production sources, V3, exports, LLVM components, or serialization.

## Milestones

1. Add the public Float source and runtime row.
2. Register direct topology and capability wrappers through the existing shared runners.
3. Register differential PTX, `ptxas`, and bit-exact RTX/NVRTC runtime wrappers.
4. Format/build, run focused/full/Debug lanes, hash names, measure marginal growth, document,
   audit, remove temporary artifacts, and commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
five new Float names, all fifteen public scalar composition names, the complete Slice 46-54 wave
matrix, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3 tables and no production/provider diff; exact shared topology;
independent E52016 failures for features 35, 38, and 39 before module creation;
`[64, 64, 32]`; one f32 global load/store and exactly one ballot/shuffle per entry; `ptxas`
acceptance; bit-exact lane 0 and 7 Float values through NVVM and NVRTC; exact name/hash continuity;
smaller marginal line growth than Slice 53; formatted code; clean diff checks; and a completed
input-shape audit.

## Self-Review and Input-Shape Audit

Inventory the fixture, runtime mode, and five thin wrappers. Every supplied shared-runner field must
match the audited Float graph. No new helper, branch, fallback, custom equivalence, name matcher, or
production special case is expected.

The input shape is canonical: source-library selection and active-mask synthesis produce the same
valid public-helper graph with Float value and synthesized UInt mask parameters. The entry's Float
load and store are established direct IR. Tests observe existing callbacks and compiled output.

## Failure and Recovery

If Float requires another runner dimension, prove it from final linked IR before changing the
abstraction. If a new operation appears, audit its producer before expanding the backend. Removing
the Float row and five wrappers restores Slice 53. Never stage `external/slang-binaries/` or
`tmp-slice-54-*` artifacts.

## Artifacts and Hand-Off

Retain the final Float IR/PTX shape, shared-row data, feature failures, NVVM/NVRTC/`ptxas`/RTX
results, hashes, marginal lines, and completed audit. Distill durable evidence into the design and
ledger, then commit this completed plan with Slice 54.
