# Slice 57: Complete the Float wave-read-lane-first scalar row

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public Float `WaveReadLaneFirst()` through an
independently negotiated typed helper. The provider combines Slice 55's first-active-lane
derivation with the established native f32 synchronized shuffle, and the third scalar row remains
thin through the shared provider and evidence runners introduced by Slice 56.

## Progress

- [x] (2026-08-28) Recorded the Slice 56 baseline: 369 names, SHA-256
  `b8e9cc1b10ae6094dd3771696bc8ffa9f8c9a4fde60837c7b259c904097a8366`, Release 369/369,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 26,482 measured lines.
- [x] (2026-08-28) Audited CUDA target selection, exact Float linked IR, the E52017 boundary, and
  NVRTC PTX for public Float `WaveReadLaneFirst()`.
- [x] (2026-08-28) Appended feature 42/operation 8 and selected the exact typed Float helper while
  preserving the V3 table size and exact Slice 56 compatibility.
- [x] (2026-08-28) Combined shared first-lane derivation with the established f32 shuffle and
  added typed provider, direct, capability, PTX, `ptxas`, and RTX/NVRTC runtime evidence.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, audited, and removed
  temporary probes for the completed slice.

## Surprises and Discoveries

- Observation: final linked IR retains exact `Func(Float, UInt, Float)` with `(mask, value)` and
  terminator `GenericAsm("_waveReadFirst($0, $1)")`; its public topology matches UInt/Int.
  Consequence: append one exact signature row and keep source selection/active-mask synthesis as
  the producers rather than adding name matching or a Float-specific graph rewrite.

- Observation: the provider already exposes and legacy-audits
  `llvm.nvvm.shfl.sync.idx.f32` for Float read-lane-at, while Slice 55 owns the only `cttz.i32`
  declaration and rewrite needed to derive the lane.
  Consequence: operation 8 selects the f32 shuffle and Float value type while sharing the existing
  two-step compound implementation.

- Observation: NVRTC's `[64, 64]` entry has one 32-bit global load/store pair, ballot, `brev`,
  `bfind.shiftamt`, and `shfl.sync.idx.b32`, identical in PTX shape to Int despite Float semantics.
  Consequence: use typed source/runtime bits as the semantic oracle and reuse the differential PTX
  runner with the same ABI/load dimensions.

- Observation: a brace-less `if`/`else` around `SLANG_RETURN_ON_FAIL` failed with C2181 because the
  macro expands to multiple statements.
  Consequence: the shared typed fixture uses explicit braces around both branches; no production
  behavior or interface changed.

## Decision Log

- Decision: make public Float wave-read-first the next bounded scalar row.
  Rationale: it completes the three established 32-bit scalar payloads and proves compound generic
  plus native Float intrinsic emission without introducing a new operation family.
  Date/author: 2026-08-28, Codex.

- Decision: append feature 42 and operation 8 rather than widening either prior row.
  Rationale: exact Slice 56 providers do not claim Float; independent negotiation preserves the
  append-only semantic contract even though final PTX shuffle payloads are raw 32-bit bits.
  Date/author: 2026-08-28, Codex.

- Decision: extend the shared read-first provider/test helpers with a Float payload dimension.
  Rationale: mask type, lane derivation, topology, feature union, ABI, and PTX mechanism are stable;
  value type, native shuffle ID, fake value kind, and expected lane-zero bits are measured row data.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Feature 42 and operation 8 now negotiate public Float `WaveReadLaneFirst()` independently through
the exact `Func(Float, UInt, Float)` helper. V3 remains 528 bytes on x64 and 308 bytes on x86, and
all exact Slice 56 compatibility fixtures still load with feature 42 clear. The provider validates
`(i32 mask, float value)` before mutation, shares `llvm.cttz.i32(mask, true)`, and emits the existing
native `llvm.nvvm.shfl.sync.idx.f32`; it required no callback, text marker, or compatibility rewrite.

All seven new names pass 7/7, the UInt/Int/Float read-first rows pass 21/21, the Slice 46-57 wave
matrix passes 78/78, the Release NVVM prefix passes 376/376, and Debug preservation passes 10/10.
The complete sorted LF-terminated Release name set hashes to
`e345e4b4ef33f3a7fe6426c95d461fd46cfb6de8e183be59c2db77ecfa78b4e9`; removing the seven Slice 57
names gives 369 names and exactly Slice 56's
`b8e9cc1b10ae6094dd3771696bc8ffa9f8c9a4fde60837c7b259c904097a8366`. The five measured
test/support files grew by 151 physical lines, from 26,482 to 26,633.

NVVM and NVRTC both emit `[64,64]`, one Float load/store pair, one ballot, and one shuffle. NVVM
uses `popc.b32` while NVRTC uses `brev.b32` plus `bfind.shiftamt.u32`; CUDA 12.9 `ptxas` accepts
both, and every lane of one RTX 5090 warp receives lane zero's bit-exact `-11.5f` value through
both routes. The row therefore completes the established 32-bit scalar read-first family while
keeping marginal evidence growth smaller than Slice 56's cleanup-enabled row.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneFirst(source[laneIndex]);
}
```

CUDA selection composes the public call from the synthesized active mask and
`WaveMaskReadLaneFirst`. The Float specialization's final helper is `Func(Float, UInt, Float)`
ending in exact `_waveReadFirst($0, $1)`; it is the sole pre-slice E52017 boundary. The entry
otherwise uses established Float AS1 pointers, load/store, lane index, ballot, scalar functions,
and calls.

## Scope and Non-Goals

In scope are public scalar Float `WaveReadLaneFirst`, feature 42/operation 8, exact descriptor
selection, combined generic-cttz/native-f32 lowering, Float fixture/runtime bits, seven thin
provider/direct/capability/PTX/`ptxas`/RTX evidence names, design, ledger, and this plan.

Out of scope are half/double/Bool/64-bit and aggregate rows, explicit masks, zero-mask behavior,
divergence, new callbacks, additional LLVM text rewrites, performance claims, and other wave
operations.

## Architecture and Invariants

The source library and active-mask synthesis own the canonical graph. Direct NVVM recognizes exact
assembly plus complete `Func(Float, UInt, Float)` shape, requires feature 42, and forwards the UInt
mask and Float value through the generic callback.

The facade maps operation 8 to feature 42. The provider validates `(i32 mask, float value)` before
mutation, emits the established `cttz.i32(mask, true)`, and calls the established f32 indexed
shuffle with lane and clamp 31. Existing exact legacy validation owns both declarations and their
attribute normalization; this slice adds no text marker.

Shared test helpers expose a Float-payload dimension without weakening exact type, topology,
feature, LLVM declaration/call, PTX, or runtime-bit assertions.

## Interfaces and Dependencies

Append feature 42, operation 8, and a minimum-size alias to V3. Extend facade, provider, descriptor,
fake, shared provider fixture/checks, public fixture, runtime row, evidence, design, ledger, and
this plan. Do not change table layout, ABI version, V2, exports, LLVM components, or formats.

## Milestones

1. Append feature 42/operation 8 with exact Slice 56 compatibility and unchanged V3 sizes.
2. Match `Float(UInt, Float)` and combine the existing cttz lane with native f32 shuffle emission.
3. Extend shared provider checks/fixtures for typed payloads and add Float public/runtime data.
4. Add thin direct/capability/PTX/`ptxas`/RTX wrappers.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, remove probes, and
   commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new names, all three read-first rows, complete wave matrix, generic-intrinsic compatibility/
invalid tests, unsupported boundary, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3; exact Slice 56 compatibility; one `cttz.i32(mask, true)` plus one
f32 shuffle for operation 8; exact Float fake topology and independent feature-42 E52016 before
module creation; `[64,64]`, one load/store, ballot, and shuffle per entry; CUDA 12.9 `ptxas`;
every RTX 5090 lane reading lane zero's `-11.5f` bits through both routes; hash continuity; bounded
marginal growth; formatted code; completed audit; removed probes; and clean diffs.

## Self-Review and Input-Shape Audit

Inventory appended mappings, Float descriptor/provider cases, typed shared-helper dimensions, fake
classification, fixture/runtime row, and evidence. Prove validation precedes mutation, source
types/operands remain authoritative, and no names, graph rediscovery, syntax reconstruction,
fallback, custom equivalence, or duplicate compatibility bridge was added.

The input is canonical: generic specialization deliberately produces a Float value helper while
active-mask synthesis supplies its UInt mask. Combining a generic integer lane calculation with a
native Float shuffle implements the source semantic at the provider boundary; it does not repair
malformed IR or reinterpret Float as integer in the Slang type system.

## Failure and Recovery

If f32 shuffle plus cttz produces a new LLVM 7 incompatibility, stop and audit the exact declaration
rather than broadening text rewriting. If shared helpers obscure typed checks, keep an explicit
Float assertion. Removing feature 42/operation 8, descriptor/provider cases, and Slice 57 evidence
restores Slice 56. Never stage `external/slang-binaries/` or `tmp-slice-57-*` artifacts.

## Artifacts and Hand-Off

Retain final Float IR, LLVM/NVVM calls, NVVM/NVRTC PTX, `ptxas`/RTX results, sizes, hashes, line
growth, and audit. Distill durable evidence into design/ledger and commit this completed plan with
Slice 57.
