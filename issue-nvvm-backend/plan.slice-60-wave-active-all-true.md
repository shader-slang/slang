# Slice 60: Add public wave-active-all-true through shared vote infrastructure

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public `WaveActiveAllTrue(bool)`. CUDA's canonical
`WaveMaskAllTrue(activeMask, condition)` helper lowers to `llvm.nvvm.vote.all.sync`, and the any/all
vote fixtures and evidence share one economical implementation rather than duplicating a full row.

## Progress

- [x] (2026-08-28) Recorded the Slice 59 baseline: 390 names, SHA-256
  `eaa8420ddbba56d34cb047211d872acd6ad2dc0dcdd0209059e307e9879e3186`, Release 390/390,
  wave 92/92, Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 27,350 measured lines.
- [x] (2026-08-28) Audited exact CUDA specialization, pre-provider E52017 `GenericAsm`, LLVM 7
  intrinsic signature, and NVRTC PTX for public `WaveActiveAllTrue()`.
- [x] (2026-08-28) Appended feature 45/operation 11 and lowered the exact masked all-vote through
  the generic callback.
- [x] (2026-08-28) Refactored any/all provider, direct, and PTX evidence through shared vote helpers.
- [x] (2026-08-28) Added provider/direct/capability/PTX/`ptxas`/RTX evidence through the public
  source path.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, audited, and removed
  temporary probes for the completed slice.

## Surprises and Discoveries

- Observation: final CUDA source contains exact `Func(Bool, UInt, Bool)` semantics
  `GenericAsm("(__all_sync($0, $1) != 0)")`; public `WaveActiveAllTrue(condition)` passes the
  synthesized active mask and condition through that helper.
  Consequence: reuse the Slice 59 Bool helper signature contract and distinguish the operation by
  exact assembly plus complete signature, without matching helper names.

- Observation: direct NVVM now reaches E52017 `GenericAsm`; every surrounding Bool parameter,
  call, comparison, load, phi, and store shape is already supported.
  Consequence: this slice needs one new semantic row, not another type or IR representation change.

- Observation: LLVM 7 and the provider LLVM agree that `llvm.nvvm.vote.all.sync` has signature
  `i1 (i32, i1)` and synchronized-vote attributes, while NVRTC emits one `vote.sync.all.pred`
  between the established two ballots.
  Consequence: add the all-vote to the existing synchronized-vote declaration audit without a text
  rewrite.

- Observation: cloning Slice 59's provider fixture and direct/PTX assertions would repeat almost
  identical setup for operations differing only in feature, operation, names, source, and mnemonic.
  Consequence: parameterize those existing helpers now, keeping the independently registered test
  names while making future vote rows marginal rather than cumulative boilerplate.

## Decision Log

- Decision: make public `WaveActiveAllTrue()` the next bounded wave operation.
  Rationale: it is the adjacent source-visible synchronized vote, reuses the established Bool
  transport contract, and tests a distinct native NVVM semantic.
  Date/author: 2026-08-28, Codex.

- Decision: append feature 45 `WAVE_MASK_ALL_TRUE` and intrinsic operation 11.
  Rationale: the exact masked helper is the semantic provider boundary, while source-library
  composition remains upstream. Independent negotiation preserves exact Slice 59 compatibility.
  Date/author: 2026-08-28, Codex.

- Decision: share any/all fixture and evidence implementations while retaining seven separate names.
  Rationale: the operations have the same canonical type/graph shape and differ only in negotiated
  semantic identity. Parameterization keeps one source of truth without weakening observability.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Feature 45 and operation 11 now negotiate the exact canonical `Func(Bool, UInt, Bool)` masked
all-vote helper. No type role changed: Slice 59's native i1 helper parameter/result, call, intrinsic,
and return transport already covers the graph, while Bool entry-point parameters and block phis
remain unsupported. V3 remains 528 bytes on x64 and 308 bytes on x86, and exact Slice 59 providers
load with feature 45 clear.

The provider validates both operands before mutation and emits
`llvm.nvvm.vote.all.sync(i32, i1) -> i1`. LLVM 7 and the provider LLVM agree on its declaration and
convergent/inaccessible-memory/nounwind attributes. Both assembly forms contain the same declaration
and call, so the shared synchronized-vote audit needs no rewrite. The standalone provider and
Release/Debug main targets build successfully.

Any/all provider fixtures, negotiation checks, real assembly checks, direct fake-graph checks, and
differential PTX setup now share one implementation per graph concern. Each row still asserts its
own semantic dimensions. All 15 new/refactored/ABI focused names pass 15/15, the Slice 46-60 wave
matrix passes 99/99, the Release NVVM prefix passes 397/397, and Debug preservation passes 10/10.
The complete sorted LF-terminated Release name set hashes to
`d5daef5d6db4caa82e5dd8039a8b0f5e095d13cdb819a81f7ea69a30ab873b0d`; removing the seven Slice 60
names gives 390 names and exactly Slice 59's
`eaa8420ddbba56d34cb047211d872acd6ad2dc0dcdd0209059e307e9879e3186`. The five measured
test/support files grew by only 150 physical lines, from 27,350 to 27,500.

NVVM and NVRTC agree on the `[64, 64]` entry ABI, one 32-bit global load/store pair, two
synchronized ballots, one synchronized all-vote, and signed inequality. CUDA 12.9 `ptxas` accepts
both. The RTX 5090 fixture makes lane seven false and the other 31 lanes true; all 32 lanes store
zero through both routes. The refactor reduced marginal growth by 218 lines relative to Slice 59
without losing any separately registered evidence.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllTrue(source[laneIndex] != 0) ? 1 : 0;
}
```

CUDA target selection implements the public operation as
`WaveMaskAllTrue(WaveGetActiveMask(), condition)`. Active-mask synthesis produces the established
`waveMaskBallot(0xffffffff, true)`, threads its UInt result through the wrapper, and retains one
exact `Bool(UInt, Bool)` GenericAsm helper. Control-flow mask maintenance adds a second ballot after
the Bool call. Slice 59 already owns every type and graph shape around that helper.

The remaining failure is exact GenericAsm classification. Once selected, the provider can validate
i32 mask plus i1 condition and call the native synchronized all-vote.

## Scope and Non-Goals

In scope are feature 45/operation 11, exact `Bool(UInt, Bool)` descriptor selection, provider
vote-all emission and declaration audit, shared any/all fixture/test helpers, one public runtime row,
seven provider/direct/capability/PTX/`ptxas`/RTX evidence names, design, ledger, and this plan.

Out of scope are new type roles, Bool entry-point parameters/phis/memory, arbitrary public mask APIs,
equal/unique votes, ballot changes, reductions, divergence stress, new callback fields, performance
claims, or text rewrites.

## Architecture and Invariants

The source library and active-mask synthesis remain the sole producers of the canonical graph.
Direct NVVM recognizes the exact masked helper by complete result/parameter shape plus assembly
text, requires feature 45, and forwards the existing UInt mask and Bool predicate.

The facade maps operation 11 only to feature 45. The provider validates two usable operands before
mutation and emits `llvm.nvvm.vote.all.sync(i32, i1) -> i1`. The legacy writer accepts only the
exact LLVM 7-compatible declaration signature and semantic attributes.

Shared test helpers parameterize only genuine vote dimensions. Each independently registered test
still names its operation, asserts its negotiated feature/op pair, checks its own PTX mnemonic, and
runs its own public source/runtime contract.

## Interfaces and Dependencies

Append feature 45, operation 11, and a minimum-size alias to V3. Extend facade, provider, exact
descriptor, fake Bool intrinsic classification, shared provider/public fixtures, tests, design,
ledger, and this plan. Do not change table layout, ABI version, type roles, V2, exports, LLVM
components, or formats.

## Milestones

1. Append feature 45/operation 11 with unchanged V3 sizes and exact Slice 59 compatibility.
2. Match exact `Bool(UInt, Bool)` all-vote GenericAsm and emit the native synchronized intrinsic.
3. Refactor any/all fixture and evidence setup through vote-parameterized helpers.
4. Add provider/direct/capability/PTX/`ptxas`/RTX evidence through the public source path.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, remove probes, and
   commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new names, complete vote/wave matrix, generic-function and intrinsic compatibility/invalid
tests, unsupported boundary, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3; exact Slice 59 compatibility; one exact native all-vote call and
declaration in both assembly forms; independent feature-45 E52016 before module construction;
public `[64, 64]` ABI with one load/store, two ballots, and one all-vote; CUDA 12.9 `ptxas`; one
false lane making every RTX 5090 lane store zero through both routes; hash continuity; economical
marginal growth; formatted code; completed audit; removed probes; clean diffs.

## Self-Review and Input-Shape Audit

Inventory the feature/operation mappings, descriptor/provider case, synchronized-vote declaration
audit, fake classification, shared fixture/test refactors, runtime row, and evidence. Prove every
provider check precedes mutation, parameterization does not erase operation-specific assertions,
semantic types remain authoritative, and no helper name matching, graph rediscovery, syntax
reconstruction, fallback, custom equivalence, or duplicate compatibility bridge was added.

The `Bool(UInt, Bool)` helper is the same canonical target-specialized shape audited in Slice 59;
only its exact assembly semantic differs. Implementing that semantic at the provider boundary and
sharing test scaffolding by type/graph shape preserves the upstream source of truth without merging
the independently negotiated operations.

## Failure and Recovery

If final IR differs from the audited helper or exposes another type role, stop rather than broaden
the slice. If libNVVM rejects the native declaration, compare exact normal and compatible text
before adding a rewrite. If parameterization hides an operation-specific failure, retain separate
assertions or undo that refactor. Removing feature 45/operation 11 and Slice 60 evidence restores
Slice 59. Never stage `external/slang-binaries/` or `tmp-slice-60-*` artifacts.

## Artifacts and Hand-Off

Retain exact helper semantics, LLVM/NVVM vote declaration and call, NVVM/NVRTC PTX,
`ptxas`/RTX results, sizes, hashes, line growth, and audit. Distill durable evidence into
design/ledger and commit this completed plan with Slice 60.
