# Slice 59: Add public wave-active-any-true with a Boolean helper parameter

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public `WaveActiveAnyTrue(bool)`. CUDA's canonical
`WaveMaskAnyTrue(activeMask, condition)` helper transports a real Bool parameter and result through
an ordinary call, and the provider lowers the exact masked vote to `llvm.nvvm.vote.any.sync`.

## Progress

- [x] (2026-08-28) Recorded the Slice 58 baseline: 383 names, SHA-256
  `ddb9139c2d89bafd5be199f9d299f3c85b6ca8cca82146b9466ddbaf7fb84335`, Release 383/383,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 26,982 measured lines.
- [x] (2026-08-28) Audited source selection, exact final linked IR, the pre-provider E52017
  boundary, LLVM 7/14 intrinsic signatures, and NVRTC PTX for public `WaveActiveAnyTrue()`.
- [x] (2026-08-28) Admitted canonical Bool helper parameters without admitting unsupported Bool
  entry-point parameters or block phis.
- [x] (2026-08-28) Appended feature 44/operation 10 and lowered the exact masked vote through the
  generic callback.
- [x] (2026-08-28) Added provider/direct/capability/PTX/`ptxas`/RTX evidence through the public
  source path.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, audited, and removed
  temporary probes for the completed slice.

## Surprises and Discoveries

- Observation: final linked IR contains exact `Func(Bool, UInt, Bool)` ending in
  `GenericAsm("(__any_sync($0, $1) != 0)")`; public `WaveActiveAnyTrue(condition)` passes the
  synthesized active mask and condition through that helper.
  Consequence: match the complete helper signature and exact assembly semantic. Do not match names,
  bypass active-mask synthesis, or reconstruct the expression graph in the Slang IR.

- Observation: initial direct preflight reports E52017 `helper function parameter`; Slice 58 already
  preserves Bool helper results but deliberately did not admit Bool parameters.
  Consequence: extend only the helper-parameter and direct-call-argument roles proven by this graph;
  retain the established signed-i32/Float block-phi contract.

- Observation: LLVM 7 and the provider LLVM agree that `llvm.nvvm.vote.any.sync` has signature
  `i1 (i32, i1)` with convergent/inaccessible-memory attributes, and NVRTC lowers the source to one
  `vote.sync.any.pred` between two synchronized ballots.
  Consequence: call the native intrinsic directly and audit its LLVM-7-compatible declaration;
  introduce a compatibility rewrite only if serialized text demonstrably differs.

## Decision Log

- Decision: make public `WaveActiveAnyTrue()` the next bounded wave operation.
  Rationale: it is a small source-visible vote, composes with established active-mask synthesis, and
  proves the canonical Bool helper-parameter path intentionally deferred by Slice 58.
  Date/author: 2026-08-28, Codex.

- Decision: append feature 44 `WAVE_MASK_ANY_TRUE` and intrinsic operation 10.
  Rationale: the exact masked helper is the semantic provider boundary, while source-library
  composition remains upstream. Independent negotiation preserves exact Slice 58 compatibility.
  Date/author: 2026-08-28, Codex.

- Decision: preserve Bool as i1 across helper parameter, call, intrinsic, and return positions.
  Rationale: Bool is the canonical semantic type and the provider APIs already carry opaque typed
  values. Coercing it to i32 would create a second representation and diverge from both LLVM
  intrinsic contracts.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Feature 44 and operation 10 now negotiate the exact canonical `Func(Bool, UInt, Bool)` masked
helper. Direct type lowering and preflight preserve Bool as native i1 in helper parameter/result,
call, intrinsic, and return positions; Bool entry-point parameters and block phis retain their
unsupported boundaries. V3 remains 528 bytes on x64 and 308 bytes on x86, and exact Slice 58
providers load with feature 44 clear.

The provider validates both operands before mutation and emits the native
`llvm.nvvm.vote.any.sync(i32, i1) -> i1`. LLVM 7 and the provider LLVM agree on its signature and
convergent/inaccessible-memory/nounwind attributes. Both normal LLVM assembly and LLVM-7-compatible
NVVM IR contain the same declaration and call, so the legacy writer needs only semantic auditing,
not a rewrite. The standalone provider and Release/Debug main targets build successfully.

All seven new names pass 7/7, the Slice 46-59 wave matrix passes 92/92, the Release NVVM prefix
passes 390/390, and Debug preservation passes 10/10. The complete sorted LF-terminated Release name
set hashes to `eaa8420ddbba56d34cb047211d872acd6ad2dc0dcdd0209059e307e9879e3186`; removing the seven
Slice 59 names gives 383 names and exactly Slice 58's
`ddb9139c2d89bafd5be199f9d299f3c85b6ca8cca82146b9466ddbaf7fb84335`. The five measured
test/support files grew by 368 physical lines, from 26,982 to 27,350.

NVVM and NVRTC agree on the `[64, 64]` entry ABI, one 32-bit global load/store pair, two
synchronized ballots, one synchronized any-vote, and signed inequality. CUDA 12.9 `ptxas` accepts
both. The RTX 5090 fixture makes only lane seven's condition true, and all 32 lanes store one through
both routes. The only initial test correction changed the shared differential harness's expected
global-load flag from false to true; generated code and runtime semantics were already correct.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAnyTrue(source[laneIndex] != 0) ? 1 : 0;
}
```

CUDA target selection implements the public operation as
`WaveMaskAnyTrue(WaveGetActiveMask(), condition)`. Active-mask synthesis produces the established
`waveMaskBallot(0xffffffff, true)`, threads its UInt result through the wrapper, and retains one
exact `Bool(UInt, Bool)` GenericAsm helper. Control-flow mask maintenance adds a second ballot after
the Bool call. Direct NVVM already owns lane index, ballot, integer load/compare, calls, Bool
conditions, signed-i32 phi/store, and all pointer/ABI shapes.

The initial failure is the helper-parameter whitelist, not malformed IR. Once Bool is legal in that
specific role, the exact GenericAsm semantic is the only new operation. The provider's generic
intrinsic callback can validate i32 mask plus i1 condition and call the native synchronized vote.

## Scope and Non-Goals

In scope are canonical Bool helper parameters, feature 44/operation 10, exact `Bool(UInt, Bool)`
descriptor selection, provider vote-any emission, fake Bool parameter/call classification, one
public fixture/runtime row, seven provider/direct/capability/PTX/`ptxas`/RTX evidence names, design,
ledger, and this plan.

Out of scope are Bool entry-point parameters, Bool block parameters/phis, Bool memory, arbitrary
public mask APIs, all-true/equal votes, ballot changes, reductions, divergence stress, new callback
fields, performance claims, or speculative text rewrites.

## Architecture and Invariants

The source library and active-mask synthesis remain the sole producers of the canonical graph.
Direct NVVM admits Bool as a helper parameter and ordinary call argument, recognizes the exact
masked helper by complete result/parameter shape plus assembly text, requires feature 44, and
forwards the existing UInt mask and Bool predicate through the generic callback.

The facade maps operation 10 only to feature 44. The provider validates two usable operands before
mutation and emits `llvm.nvvm.vote.any.sync(i32, i1) -> i1`. Its declaration must retain the exact
LLVM 7 signature and semantic attributes in compatible text.

Fake and real fixtures preserve Bool as its own scalar kind through function type, parameter,
integer comparison, call, intrinsic result, and value return. They must not reinterpret i1 as i32
or broaden block-phi acceptance.

## Interfaces and Dependencies

Append feature 44, operation 10, and a minimum-size alias to V3. Extend facade, provider, exact
descriptor, helper-parameter type lowering/preflight, fake Bool classification, provider/public
fixtures, tests, design, ledger, and this plan. Do not change table layout, ABI version, V2,
exports, LLVM components, or formats.

## Milestones

1. Append feature 44/operation 10 with unchanged V3 sizes and exact Slice 58 compatibility.
2. Admit canonical Bool helper parameters and preserve Bool through generic function calls.
3. Match exact `Bool(UInt, Bool)` GenericAsm and emit the native synchronized vote intrinsic.
4. Add provider/direct/capability/PTX/`ptxas`/RTX evidence through the public source path.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, remove probes, and
   commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new names, the complete wave matrix, generic-function and intrinsic compatibility/invalid
tests, unsupported boundary, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3; exact Slice 58 compatibility; exact i1 helper parameter/result,
call, intrinsic, and return; independent feature-44 E52016 before module construction; public
`[64]` ABI with two ballots, one vote-any, and one 32-bit store; CUDA 12.9 `ptxas`; mixed per-lane
conditions producing one in every active lane on an RTX 5090 through both routes; hash continuity;
bounded marginal growth; formatted code; completed audit; removed probes; clean diffs.

## Self-Review and Input-Shape Audit

Inventory the parameter-role change, feature/operation mappings, descriptor/provider case, LLVM-7
declaration handling, fake Bool classification, fixture/runtime row, and evidence. Prove every
provider check precedes mutation, semantic types remain authoritative, and no helper name matching,
graph rediscovery, syntax reconstruction, fallback, custom equivalence, or duplicate compatibility
bridge was added.

The Bool helper parameter is canonical and intentionally produced by target specialization; it is
not an alternate spelling of an integer. The generic function APIs already transport arbitrary
scalar LLVM handles, so admitting i1 in this role preserves the source type. The exact GenericAsm is
also canonical target-selected input. Implementing its target semantic at the provider boundary is
analogous to the established ballot operation and leaves upstream composition intact.

## Failure and Recovery

If Bool transport exposes a Bool block phi or entry-point ABI parameter, stop and audit the actual
linked graph rather than broadening roles. If libNVVM rejects the native intrinsic declaration,
compare exact normal and LLVM-7-compatible text before adding a narrowly justified rewrite.
Removing feature 44/operation 10, the Bool parameter-role additions, and Slice 59 evidence restores
Slice 58. Never stage `external/slang-binaries/` or `tmp-slice-59-*` artifacts.

## Artifacts and Hand-Off

Retain exact final IR, LLVM/NVVM vote declaration and call, NVVM/NVRTC PTX, `ptxas`/RTX results,
sizes, hashes, line growth, and audit. Distill durable evidence into design/ledger and commit this
completed plan with Slice 59.
