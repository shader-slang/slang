# Slice 58: Add public wave-is-first-lane with a Boolean helper result

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs public `WaveIsFirstLane()`. CUDA's canonical
`WaveMaskIsFirstLane(activeMask)` helper returns a real Bool through an ordinary helper call, and
the provider implements its exact least-participating-lane predicate with generic LLVM integer
operations plus the established lane-id intrinsic.

## Progress

- [x] (2026-08-28) Recorded the Slice 57 baseline: 376 names, SHA-256
  `e345e4b4ef33f3a7fe6426c95d461fd46cfb6de8e183be59c2db77ecfa78b4e9`, Release 376/376,
  Debug 10/10, 528-byte x64/308-byte x86 V3 table, and 26,633 measured lines.
- [x] (2026-08-28) Audited source selection, exact final linked IR, the pre-provider E52017
  boundary, Bool helper-result plumbing, and NVRTC PTX for public `WaveIsFirstLane()`.
- [x] (2026-08-28) Admitted canonical Bool helper results without admitting unsupported Bool
  parameters or phis.
- [x] (2026-08-28) Appended feature 43/operation 9 and lowered the exact masked predicate through
  the generic callback with no new compatibility rewrite.
- [x] (2026-08-28) Added provider/direct/capability/PTX/`ptxas`/RTX evidence through the public
  source path.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, audited, and removed
  temporary probes for the completed slice.

## Surprises and Discoveries

- Observation: final linked IR contains exact `Func(Bool, UInt)` ending in
  `GenericAsm("(($0 & -$0) == (WarpMask(1) << _getLaneId()))")`; public
  `WaveIsFirstLane()` passes the synthesized active mask through that helper.
  Consequence: match the complete helper signature and exact assembly semantic. Do not match
  names, bypass active-mask synthesis, or reconstruct the expression graph in the Slang IR.

- Observation: initial direct preflight reported E52017 `helper function result type` before it
  reaches the otherwise canonical GenericAsm because helper signatures admit only i32/float.
  Consequence: extend the result role specifically to Bool and route Bool returns/calls through the
  existing generic value APIs; keep helper parameters and block phis at their currently proven
  types.

- Observation: NVRTC emits two synchronized ballots around the public predicate and implements the
  predicate with `neg.s32`, `and.b32`, `shl.b32`, and `setp.eq.s32` using the lane id.
  Consequence: implement the exact source formula in the provider with ordinary LLVM `sub`, `and`,
  `shl`, and `icmp eq`; reuse the established lane-id intrinsic and compare runtime semantics.

- Observation: the initial UInt observation sink made the conditional expression produce an
  unrelated unsigned-i32 block phi, which is outside the established signed-i32/Float phi contract.
  Consequence: use an Int `1/0` sink to observe the Bool result through the existing signed-i32
  join; do not broaden phi types as a side effect of this wave slice.

- Observation: adding one more feature otherwise required another manual clear in 12 historical
  provider fixtures.
  Consequence: centralize append-only suffix clearing in one test helper; each fixture keeps its
  exact boundary while future features stop multiplying boilerplate.

## Decision Log

- Decision: make public `WaveIsFirstLane()` the next bounded wave operation.
  Rationale: it is the next small source-visible operation, exercises the already-established
  active-mask/lane-id graph, and introduces the missing canonical Bool helper-result type needed by
  later vote operations.
  Date/author: 2026-08-28, Codex.

- Decision: append feature 43 `WAVE_MASK_IS_FIRST_LANE` and intrinsic operation 9.
  Rationale: the exact masked helper is the semantic provider boundary, while public active-mask
  composition remains owned by the source library and synthesis pass. Independent negotiation
  keeps exact Slice 57 providers honest.
  Date/author: 2026-08-28, Codex.

- Decision: keep Bool support result-only in helper signatures for this slice.
  Rationale: the observed graph needs a Bool result but no Bool parameter or Bool phi. Broadening
  all scalar-signature positions would claim untested call and phi paths and obscure the exact
  representation contract.
  Date/author: 2026-08-28, Codex.

- Decision: replace repeated late-feature clears with `_clearNVVMBuilderFeaturesFrom`.
  Rationale: the suffix boundary is the actual compatibility contract; one implementation is less
  error-prone and scales independently of the number of later features.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Feature 43 and operation 9 now negotiate the exact canonical `Func(Bool, UInt)` masked helper.
Direct type lowering preserves Bool as native i1 specifically for helper results, calls, and value
returns; helper parameters and block phis retain their narrower established contracts. V3 remains
528 bytes on x64 and 308 bytes on x86, and exact Slice 57 providers load with feature 43 clear.

The provider validates the mask before mutation and emits the exact least-set-bit predicate with
ordinary LLVM `sub`, `and`, `shl`, and `icmp eq` plus the existing lane-id intrinsic. Both normal
LLVM assembly and LLVM-7-compatible NVVM IR contain the i1 helper definition/call/return and need no
new rewrite. The standalone provider and Release/Debug main targets build successfully.

All seven new names pass 7/7, the Slice 46-58 wave matrix passes 85/85, the Release NVVM prefix
passes 383/383, and Debug preservation passes 10/10. The complete sorted LF-terminated Release name
set hashes to `ddb9139c2d89bafd5be199f9d299f3c85b6ca8cca82146b9466ddbaf7fb84335`; removing the seven
Slice 58 names gives 376 names and exactly Slice 57's
`e345e4b4ef33f3a7fe6426c95d461fd46cfb6de8e183be59c2db77ecfa78b4e9`. The five measured
test/support files grew by 349 physical lines, from 26,633 to 26,982.

NVVM and NVRTC agree on the `[64]` entry ABI, two synchronized ballots, least-bit `neg`/`and`,
lane-bit `shl`, equality predicate, and one 32-bit global store. CUDA 12.9 `ptxas` accepts both, and
one RTX 5090 warp stores one only in lane zero through both routes. The initial UInt sink usefully
confirmed that unrelated unsigned phis should remain a separate future contract; the final Int
sink keeps this slice focused while fully observing the Bool result.

## Context and Current Pipeline

Consider this kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveIsFirstLane() ? 1 : 0;
}
```

CUDA target selection implements the public operation as
`WaveMaskIsFirstLane(WaveGetActiveMask())`. Active-mask synthesis produces the established
`waveMaskBallot(0xffffffff, true)`, threads the mask through the wrapper, and retains one exact
`Bool(UInt)` GenericAsm helper. Control-flow mask maintenance adds a second ballot after the Bool
call. Direct NVVM already owns lane index, ballot, calls, Bool conditions, branches, signed-i32
phi/store, and all pointer/ABI shapes.

The initial failure was the helper-result whitelist, not malformed IR. Once Bool is legal in that
specific role, the exact GenericAsm semantic is the only new operation. The provider's generic
intrinsic callback can validate its i32 mask, call `llvm.nvvm.read.ptx.sreg.laneid`, build the
least-set-bit expression, and return i1.

## Scope and Non-Goals

In scope are canonical Bool helper results, feature 43/operation 9, exact `Bool(UInt)` descriptor
selection, provider least-set-bit predicate emission, fake Bool result/call/return classification,
one public fixture/runtime row, seven provider/direct/capability/PTX/`ptxas`/RTX evidence names,
design, ledger, and this plan.

Out of scope are Bool helper parameters, Bool block parameters/phis, Bool memory, explicit arbitrary
mask source APIs, divergence stress, zero active-mask behavior, votes, reductions, new callback
fields, text rewrites, and performance claims.

## Architecture and Invariants

The source library and active-mask synthesis remain the sole producers of the canonical graph.
Direct NVVM admits Bool only as a helper result and call/return value, recognizes the exact masked
helper by complete result/parameter shape plus assembly text, requires feature 43, and forwards the
existing UInt mask through the generic callback.

The facade maps operation 9 only to feature 43. The provider validates one usable i32 operand before
mutation, emits `mask & -mask`, emits `1 << laneId`, compares them for equality, and returns i1.
Its only intrinsic declaration is the already-established lane-id declaration, so LLVM 14-to-7
serialization requires no new normalization.

Fake and real fixtures preserve Bool as its own scalar kind through function type, call result,
intrinsic result, and value return. They must not reinterpret i1 as i32 or broaden parameter/phi
acceptance.

## Interfaces and Dependencies

Append feature 43, operation 9, and a minimum-size alias to V3. Extend facade, provider, exact
descriptor, helper-result type lowering/preflight, fake Bool classification, provider/public
fixtures, tests, design, ledger, and this plan. Do not change table layout, ABI version, V2,
exports, LLVM components, or formats.

## Milestones

1. Append feature 43/operation 9 with unchanged V3 sizes and exact Slice 57 compatibility.
2. Admit canonical Bool helper results and preserve Bool through generic function calls/returns.
3. Match exact `Bool(UInt)` GenericAsm and emit the source predicate in the provider.
4. Add provider/direct/capability/PTX/`ptxas`/RTX evidence through the public source path.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, remove probes, and
   commit.

## Validation and Acceptance

Build the standalone Release provider and Release/Debug main targets outside the sandbox. Run the
seven new names, the complete wave matrix, generic-function and intrinsic compatibility/invalid
tests, unsupported boundary, full Release NVVM prefix, and Debug 10/10 preservation.

Accept unchanged 528/308-byte V3; exact Slice 57 compatibility; exact i1 helper definition/call/
return; one provider lane-id call plus `sub`/`and`/`shl`/`icmp eq`; independent feature-43 E52016
before module construction; public `[64]` ABI with two ballots and one 32-bit store; CUDA 12.9
`ptxas`; lane zero storing one and lanes 1-31 storing zero on an RTX 5090 through both routes; hash
continuity; bounded marginal growth; formatted code; completed audit; removed probes; clean diffs.

## Self-Review and Input-Shape Audit

Inventory the result-role change, feature/operation mappings, descriptor/provider case, fake Bool
classification, fixture/runtime row, and evidence. Prove every provider check precedes mutation,
semantic types remain authoritative, and no helper name matching, graph rediscovery, syntax
reconstruction, fallback, custom equivalence, or duplicate compatibility bridge was added.

The Bool helper result is canonical and intentionally produced by target specialization; it is not
an alternate spelling of an integer. The generic function APIs already transport arbitrary scalar
LLVM handles, so admitting i1 in the result role preserves the source type rather than coercing it.
The exact GenericAsm is also canonical target-selected input. Implementing its target semantic at
the provider boundary is analogous to prior shuffle rows and leaves upstream composition intact.

## Failure and Recovery

If Bool transport exposes a Bool parameter/phi, stop and audit the actual linked graph rather than
broadening roles. If libNVVM rejects ordinary LLVM predicate instructions or the established lane-id
declaration in this composition, isolate the exact text before adding any rewrite. Removing feature
43/operation 9, the Bool result-role additions, and Slice 58 evidence restores Slice 57. Never stage
`external/slang-binaries/` or `tmp-slice-58-*` artifacts.

## Artifacts and Hand-Off

Retain exact final IR, LLVM/NVVM predicate instructions, NVVM/NVRTC PTX, `ptxas`/RTX results, sizes,
hashes, line growth, and audit. Distill durable evidence into design/ledger and commit this completed
plan with Slice 58.
