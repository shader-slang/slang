# Slice 56: Add public Int wave-read-lane-first as a thin typed row

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public Int `WaveReadLaneFirst()` by composing the
established active-mask ballot with an independently negotiated signed read-first primitive. The
provider reuses Slice 55's generic count-trailing-zeros plus synchronized i32 shuffle lowering,
while tests add a thin typed row around a lane-varying signed source.

## Progress

- [x] (2026-08-28) Recorded the Slice 55 baseline: 362 names, SHA-256
  `652de9ad6905f2e885264851e4245cdc88e9119414a920111ee081b557ff786f`, Release 362/362,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 26,304 measured lines.
- [x] (2026-08-28) Audited CUDA target selection, exact signed linked IR, current E52017 boundary,
  and NVRTC PTX for public Int `WaveReadLaneFirst()`.
- [x] (2026-08-28) Appended feature 41/operation 7 and selected the exact signed helper by full
  canonical signature.
- [x] (2026-08-28) Reused the validated provider lowering and added seven thin provider/public/
  PTX/assembler/runtime evidence names.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, audited, and removed the
  completed slice's probes.

## Surprises and Discoveries

- Observation: final linked IR retains exact `Func(Int, UInt, Int)` with parameters `(mask, value)`
  and terminator `GenericAsm("_waveReadFirst($0, $1)")`; the public wrapper keeps the same ballot,
  active-mask identity, and call topology as UInt.
  Consequence: distinguish UInt and Int by complete canonical signature at the existing descriptor
  boundary; do not add name matching or alter the producer graph.

- Observation: the signed entry loads one lane-varying Int from a read-only device pointer before
  calling the public wrapper. NVRTC's `[64, 64]` entry contains one global load/store, ballot,
  `brev`, `bfind.shiftamt`, and synchronized indexed shuffle.
  Consequence: extend the shared public-wave topology and PTX assertions with the already-supported
  loaded-value dimensions instead of copying Slice 55's bodies.

- Observation: LLVM represents both UInt and Int payloads as signless i32, and the first-lane
  derivation is independent of payload signedness.
  Consequence: give the Slang semantic an independent feature/operation, then route it through the
  exact same validated provider implementation and legacy `cttz` bridge.

## Decision Log

- Decision: make public Int wave-read-first the next bounded scalar row.
  Rationale: it is the next canonical specialization of the newly established primitive and proves
  typed descriptor selection plus a loaded signed payload without widening to float or aggregates.
  Date/author: 2026-08-28, Codex.

- Decision: append feature 41 and operation 7 rather than treating Int as covered by the UInt bit.
  Rationale: exact Slice 55 providers implement only the already-shipped UInt semantic; independent
  negotiation preserves append-only compatibility even though LLVM's payload representation is
  signless.
  Date/author: 2026-08-28, Codex.

- Decision: share provider and test mechanisms at the semantic dimensions already proven stable.
  Rationale: first-lane derivation, synchronized i32 shuffle, legacy rewrite, public composition,
  PTX mechanisms, and runtime launcher are common; source type, feature/operation, value origin,
  ABI/load expectation, and expected lane-zero bits are row data.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Direct NVVM now accepts only the canonical signed read-first specialization through feature 41 and
operation 7. Exact Slice 55 providers remain valid with the new feature clear and the V3 table
remains 528/308 bytes. The provider shares the established validation, `cttz(mask, true)`, i32
shuffle, and exact LLVM 14-to-7 compatibility path; no declaration rewrite or wrapper field was
duplicated.

The second typed row extracted shared fake-provider negotiation/build evidence and a shared
read-first PTX runner. The existing public topology, capability, and runtime runners gained only
the signed row's measured dimensions. Seven evidence names therefore add 178 physical lines across
the five measured files, from 26,304 to 26,482, compared with the first row's 347-line cost.

The standalone Release provider and Release/Debug main targets build successfully. The seven new
names pass 7/7, paired UInt/Int read-first evidence passes 14/14, the complete Slice 46-56 wave
matrix passes 71/71, Release passes 369/369, and Debug preservation passes 10/10. The sorted
LF-terminated name set hashes to
`b8e9cc1b10ae6094dd3771696bc8ffa9f8c9a4fde60837c7b259c904097a8366`; removing exactly the seven
Slice 56 names yields 362 names and the Slice 55 hash
`652de9ad6905f2e885264851e4245cdc88e9119414a920111ee081b557ff786f`.

NVVM and NVRTC agree on `[64, 64]`, one global 32-bit load/store pair, one ballot, and one shuffle
in the entry. NVVM uses `popc`; NVRTC uses `brev` plus `bfind.shiftamt`; CUDA 12.9 `ptxas` accepts
both, and every lane of one full RTX 5090 warp reads the bit-exact lane-zero Int value `-40` through
both routes.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneFirst(source[laneIndex]);
}
```

CUDA source selection composes `WaveReadLaneFirst(value)` from `WaveGetActiveMask()` and
`WaveMaskReadLaneFirst(mask, value)`. Active-mask synthesis emits one ballot and threads it through
the public helper. The signed specialization's final helper is `Func(Int, UInt, Int)` ending in
exact `_waveReadFirst($0, $1)`; that unsupported GenericAsm was the pre-slice E52017 boundary.

The entry otherwise uses established lane index, signed AS1 pointers, pointer offsets, signed load
and store, scalar functions/calls, and ballot. The provider already owns the same signless i32
first-lane implementation for UInt.

## Scope and Non-Goals

In scope are public scalar Int `WaveReadLaneFirst`, feature 41/operation 7, exact signature-aware
descriptor selection, shared provider lowering, signed fixture/runtime data, thin
provider/direct/capability/PTX/`ptxas`/RTX evidence, design notes, ledger, and this plan.

Out of scope are Float and other scalar types, vectors/matrices, arbitrary explicit masks,
zero-mask behavior, divergent control flow, new callbacks, a second `cttz` rewrite, performance
claims, and other wave operations.

## Architecture and Invariants

Source-library target selection and active-mask synthesis remain the sole producers of the
canonical helper graph. Direct NVVM recognizes exact assembly plus complete `Func(Int, UInt, Int)`
shape, requires the new feature, and forwards the two existing handles through the generic
callback.

The facade maps operation 7 to feature 41. The provider validates the two available i32 arguments
before any mutation, then shares the Slice 55 `cttz(mask, true)` and indexed-shuffle path. Signedness
is a Slang semantic distinction; LLVM and PTX preserve its bits through signless i32 operations.
The existing exact legacy declaration validation and rewrite remain the single compatibility path.

Shared test runners may expose only measured row dimensions. They must keep exact call-flow,
feature-union, ABI, load/store, ballot/shuffle, mechanism, and runtime assertions visible.

## Interfaces and Dependencies

Append feature 41, intrinsic operation 7, and a minimum-size alias to V3. Extend the facade,
provider operation switch, direct descriptor signatures, fake, fixtures, tests, design, ledger, and
this plan. Do not change table layout, ABI version, V2, exports, LLVM components, serialization
formats, or the legacy text markers introduced by Slice 55.

## Milestones

1. Append feature 41/operation 7 with exact Slice 55 compatibility and unchanged V3 sizes.
2. Match canonical `Int(UInt, Int)` `_waveReadFirst($0, $1)` and share the provider's validated
   first-active-lane lowering.
3. Add isolated provider negotiation/build evidence and a signed public source/runtime fixture.
4. Add thin direct/capability/PTX/`ptxas`/RTX wrappers using shared typed-row mechanisms.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, remove probes, and
   commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new Slice 56 names, both UInt/Int read-first rows, the complete wave matrix, generic-intrinsic
compatibility/invalid tests, unsupported boundary, full Release NVVM prefix, and Debug 10/10
preservation.

Accept unchanged 528/308-byte V3 tables; exact Slice 55 provider compatibility; one shared
`llvm.cttz.i32(mask, true)` and i32 shuffle call for operation 7; exact signed fake topology and
independent feature-41 failure before module creation; matching `[64, 64]` NVVM/NVRTC ABI with one
load/store, ballot, and shuffle; CUDA 12.9 `ptxas` acceptance; every RTX 5090 lane reading the
negative lane-zero source value through both routes; hash continuity; bounded marginal test growth;
formatted code; completed input-shape audit; removed probes; and clean diff checks.

## Self-Review and Input-Shape Audit

Inventory the appended mappings, signed descriptor row, shared provider case, fake result mapping,
shared-runner dimensions, fixture, and evidence. For each, prove validation precedes mutation,
canonical semantic operands remain the source of truth, and no function-name matching, graph
rediscovery, syntax reconstruction, fallback, custom equivalence, or duplicate compatibility
rewrite was added.

The input shape is canonical and intentionally produced: the generic source library specializes
the exact scalar helper to Int, while active-mask synthesis supplies its UInt mask and the entry's
ordinary load supplies its Int value. Sharing a signless provider implementation preserves, rather
than erases, the source distinction because independent negotiation and descriptor selection occur
before LLVM type lowering.

## Failure and Recovery

If operation 7 requires any LLVM or legacy-text difference from UInt, stop and audit why signless
i32 no longer preserves the semantic. If shared runners require incidental function names/order,
keep thin separate assertions and record the reason. Removing feature 41, operation 7, descriptor/
provider cases, and Slice 56 evidence restores Slice 55. Never stage `external/slang-binaries/` or
`tmp-slice-56-*` artifacts.

## Artifacts and Hand-Off

Retain the signed final IR, provider LLVM/NVVM declaration/calls, NVVM/NVRTC PTX, `ptxas`/RTX
results, sizes, hashes, marginal line growth, and completed audit. Distill durable architecture and
evidence into the design and ledger, then commit this completed plan with Slice 56.
