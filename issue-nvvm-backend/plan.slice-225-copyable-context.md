# Admit supported copyable values in canonical thread-local contexts

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception. Fresh-context
workers remain unavailable; local parent execution uses WORKFLOW's fallback.

## Purpose and Observable Result

Execute per-invocation Boolean and nested numeric state through generated KernelContext helpers.
Research224 proves the producer shape is canonical and the old scalar-only pointee guard rejects
already-supported Boolean struct storage. Reuse the existing recursive copyable-value classifier,
preserve pointer qualifiers/layout, and prove runtime state through helpers and a full checkpoint.

## Progress

- [x] 2026-09-25: Select on accepted research224 base `5cd4c301896960cf4b834e67dfaf7ad92194d41e`.
- [x] Read existing context unit, copyable algebra and helper-parameter generic-pointer lowering.
- [x] Add dynamic scalar/nested/value context fixture and retain before evidence.
- [x] Replace scalar-only classifier, update rationale, add context-lifetime structural coverage.
- [x] Format/build and run focused/research/smoke/units/toolkit/contracts plus full checkpoint.
- [x] Review exact deltas, final hashes and failure histories; finish report/STATUS and local commit.

## Surprises and Discoveries

Research224: context containing one Bool rejects before PTX in both direct modes, while local Bool
struct mutation and integer contexts pass. The original two prefix contexts contain Bool plus uints.
The existing provider lowers admitted context helper parameters to typed generic pointers using the
same value representation as entry-local storage. No new address-space cast or storage ABI is needed.

## Decision Log

2026-09-25: Replace `asNVVMSupportedScalarStructType` with existing
`asNVVMSupportedCopyableStructType` only in the canonical ThreadLocal pointer branch. Resource-bearing
contexts remain outside scope; exact four operands/read-write/ThreadLocal/default-layout checks stay.
Full checkpoint is required for helper-type admission, irrespective of cadence0 since full223.
Do not change front-end context generation, generic pointer matching, provider types or ABI.

## Outcomes and Retrospective

Accepted full checkpoint: all 1,665 cells fresh, 1,614 correct and 51 known failures. All 1,611 old
correct cells are preserved; three new context cells pass. Exactly four old prefix cells advance
only diagnostic/canonical shape to FP64 exclusive min/max, with first-known failure histories
retained. The other 1,658 old cells are exact across all five outcome fields. Six resolved histories
remain intact. Research replay passes nine executions and 864 words. Focused6, units479 plus one
existing skip, smoke4, toolkit18, contracts6 and material compile/assembly6 all pass. Parent audit
verified 203 evidence references, 26 sources, 12 artifacts and 551 runtime inputs before recording
acceptance. Latest full is225, cadence0. Next bounded work is FP64 prefix semantics research.

## Context and Current Pipeline

`introduceExplicitGlobalContext` creates one entry-local initialized struct and threads an explicit
Slang ThreadLocal pointer through helpers. `_isSupportedNVVMHelperParameterType` asks
`asNVVMSupportedLocalResourceStructPointerType`; its context branch still restricts the pointee to
flat integer/Float32 fields. Copyable-value support already admits Bool, numeric vectors, fixed arrays
and nested structs. Reuse that source of truth while retaining canonical pointer spelling. Existing
`_isSupportedNVVMHelperArgument` handles compact-local to ThreadLocal call matching by exact pointee,
and type lowering uses the provider's generic address space. Input shape is intentional and valid.

## Scope and Non-Goals

One pointee-classifier replacement/comment, one structural unit, one dynamic discovery fixture,
design/plan/report/evidence/STATUS. No new helper, fallback, name-based KernelContext exception,
provider/ABI/qualifier change, resource-bearing context admission, unrelated Float64/narrow prefix
support, quad reconvergence, vector-by-value shuffle or material runtime claim.

## Architecture and Invariants

Selected context values use the same finite recursive copyable representation as ordinary locals.
Each invocation initializes and owns one context; helpers mutate that same storage. Pointer access,
source address space, layout and operand count remain exact. Resource values are not admitted by
this copyable-only branch. Existing source/oracle contracts are preserved without rewriting inputs.

## Interfaces and Dependencies

Native Ubuntu L4 SM89 target80 CUDA12.9.2/NVRTC12.9.86 LLVM14 ABI36; env203 and local slang-build skill.
Before compiler01e06def851b6228dea63d2bbb18cb4c3167ea89542d542623ea79e9d6f3258d, provider
ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372. Matching RelWithDebInfo tools.
Sequential suites with four workers maximum and two unit servers; no pushes or system changes.

## Milestones and Validation

1. New `tests/cuda/nvvm-copyable-kernel-context.slang` checks initial state, dynamic writes, subsequent
   helper reads/mutation and per-invocation independence for Bool, nested structs, double, float2 and
   fixed uint arrays. NVRTC should pass before; both NVVM modes should reproduce context preflight.
2. Add a structural Bool-context unit checking local storage, zero provider globals, Bool field and
   entry-local call argument. Keep the old integer-context unit and negative preflight suite intact.
3. Build slangc/slang-test/render-test/test-server using native releaseWithDebugInfo preset/four workers.
   Run smoke4, focused new fixture/context units/negative unit, all NVVM/routing/reporter units,
   toolkit18 and discovery contracts. Replay exact research224 sources/inputs/expectations:9executions
   should now pass, with the two former Boolean-global preflight cells executing rather than rejecting.
4. Reassess the two original prefix workloads; record their next independent blocker without fixing
   it in this slice. Full frozen452identities/1356cells using explicit census195 selector, full
   discovery103identities/309cells and six material compile/assembly cells are mandatory.

## Preservation and Acceptance

All1662 prior runtime cells remain obligations; three additions counted separately. Expect old correct
cells preserved, new3correct and the51open failure identities/sixresolved histories retained. Only
four original prefix preflight cells may advance diagnostics after context admission; preserve their
first-known records and failed classification and review exact changes before acceptance. Any other
outcome delta requires investigation. Do not count diagnostic advancement as repaired runtime support.
Full acceptance means1665fresh cells/no inherited rows, likely1614correct51known failures. Reset cadence
only after review. Material remains compile/assembly-only without application bindings/output oracle.

## Failure and Recovery

New wrong output or lost support blocks acceptance. Trace canonical producer/consumer break, fix or
revert the bounded change without resetting baseline. Capture independent next blockers only. Stop
GPU dispatch on device loss; no driver change/reboot. Bound owned runs and retain apparatus errors.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-225-{before,after}` for before/final fixture, research replay, original-prefix
traces, full results and hashes. Commit completed plan/report, implementation/unit/fixture/manifest,
full result/census and STATUS after local parent acceptance. No independent worker review is implied.

2026-09-25 before proof: new nested dynamic state fixture passes NVRTC and rejects both NVVM modes
at the exact context parameter diagnostic. Production changes only the existing pointee classifier
and rationale. One new Bool-context unit asserts local lifetime/zero provider globals. Formatter
introduced unrelated type-lowering/unit hunks; restored11type and4unit hunks precisely, then rebuilt
final source. No unrelated changes remain. Final gate preparation uses full census195 selection.

2026-09-25 apparatus discovery: the new runtime fixture passes all three modes, but the structural
unit's fake `getStructType` omits Bool from its copyable scalar leaf list. Real builder/context GPU
execution already supports it. Add the existing Boolean type handle to that fake leaf list; pointer,
field, load/store helpers already understand it. Retain initial failure/provenance separately under
`initial-fake-struct-check`, rebuild units and rerun final gates. No additional production change.

2026-09-25 second fake gap: after admitting the Bool struct leaf, its loaded Bool reaches the unary
NOT checker. `_isFakeNVVMBuilderBooleanValue` lacked the Load branch already used by integer/float
checkers. Recognize only a bounds-checked existing load record with Boolean result kind; no fabricated
value or production change. Preserve this failure under `initial-fake-load-check`. Real nested GPU
fixture still passes all modes; rerun final structural/full gates after the mock correction.

The third structural attempt reaches Boolean NOT result storage. The fake emitter already records
BooleanUnary under its typed Unary family, but its Boolean value checker omitted that family.
Recognize Unary alongside Binary only for a recorded scalar Boolean result; retain the failed
store trace under `initial-fake-store-check`. The real provider/runtime fixture still passes.

Final focused6/6 and exact research224 replay9/9 now pass (864 words). Both original prefix
min/max sources advance to GenericAsm `_wavePrefixExclusiveMin/Max(($1).x, $0)`, signature
`double(double, vector<uint,4>)`, at both direct optimization levels. This independent FP64 prefix
boundary remains deferred; full corpus verification and acceptance are still running.

2026-09-25 acceptance: full gates and exact preservation review passed. Three fake-provider gaps
were fixed at existing typed records; final source/binary hashes are unchanged. Completed plan/report
and durable evidence are included in the authorized local slice commit.
