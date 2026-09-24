# Admit FP64 implicit aggregate indexed shuffles

This ExecPlan follows `.agent/PLANS.md`. WORKFLOW requires completed plans/reports committed;
raw logs remain ignored. Worker exclusively owns writes; parent accepts and commits.

## Purpose and Observable Result

`WaveReadLaneAt(double2x2(...), lane)` executes through direct NVVM O0/O3 with exact binary64
transport, matching independently expected bit patterns and NVRTC O3. Slice 209 repaired mask
acquisition; this slice validates and removes only its deferred Float64 admission boundary.

## Progress

- [x] 2026-09-24: Read instructions, local slang-build skill, 208/209 history and full 210 ledger.
- [x] 2026-09-24: Native Linux clean base 8075158ca3631ba72cd11df810ffc29d2628e3f7; plan before edits.
- [x] Establish final fixture's original preflight failure and NVRTC expected output.
- [x] Remove admission guard; retain malformed and unrelated unsupported shapes.
- [x] Build, focused GPU/structural validation, bounded targeted acceptance and exact comparison.
- [x] Complete self-review, report, manifest and STATUS; return ownership without committing.

## Surprises and Discoveries

Hardware active masks are scheduling-dependent. Earlier separately sampled masks cannot prove
participation at a later implicit helper. Divergent cases use self-lane transport only.

## Decision Log

2026-09-24 worker: Select narrow FP64 admission over production batching protocol, whose fresh
sessions/deadlines/failure isolation still need separate design and full checkpoint. Materials
already compile/assemble; absent binding/input/oracle prevents runtime claims. Full-warp transport
uses straight-line code before divergence; partial/sparse/singleton cases read each caller itself.

## Outcomes and Retrospective

Implementation and all validation are complete; parent accepted this slice. Fresh 615 cells
contain 577 correct and 38 unchanged failures. Inherited 1035 frozen cells retain full 210;
cumulative 1650 cells contain 1597 correct and 53 open failures, plus four resolved histories.
No old outcome delta, missing/duplicate cell, GPU loss or system change occurred. Parent owns
local commit; accepted implementation cadence is 1 since full checkpoint 210.

## Context and Current Pipeline

`hlsl.meta.slang` specializes matrix WaveReadLaneAt into canonical GenericAsm
`_waveShuffleMultiple(_getActiveMask(), $0, $1)`. Matrix lowering gives Void/OutParam fixed arrays
of double vectors. `_resolveNVVMAggregateWaveOperation` validates exact helper/type/signature and
currently rejects Float64 implicit transport. CUDA prelude `_getActiveMask` reads hardware then
ballots true using the observed mask; slice 209 represents both steps. Existing aggregate recursion
emits scalar WAVE_READ_LANE_AT for every leaf; provider transports binary64 through two i32 shuffles.
The producer is valid; no representation repair is necessary.

## Scope and Non-Goals

One resolver guard, one focused source/manifest identity, existing unit boundary coverage and
slice evidence. No provider/ABI/catalog/library/general lowering changes; no vector-by-value,
FP64 min/max, KernelContext, reconvergence, runner batching, material runtime or system changes.

## Architecture and Invariants

Reuse typed scalar shuffle closure, homogeneous array/vector recursion and existing raw-mask ->
ballot(raw,true) composition. No arithmetic on transported payloads. Signed zero, signaling/quiet
NaN payloads and finite values with distinct high/low halves must preserve bits. Sparse divergence
must never name assumed bypassing lanes. Keep exact signature/mask/lane/out-pointer checks.

## Interfaces and Dependencies

ABI 36 unchanged; optimized matching bin/lib tools, CUDA12.9.2/NVRTC12.9.86, LLVM14, L4 SM89,
driver580.126.09, targetSM80. Source build/nvvm-loop/slice-203-env.sh. Max4 workers total,
CMAKE_BUILD_PARALLEL_LEVEL=1, unit servers2, suites sequential and timeout bounded.

## Milestones

1. Add focused fixture and run on unchanged compiler; retain original direct preflight failure.
2. Remove guard and obsolete rejection case; extend aggregate recipe structural unit to FP64.
3. Format explicit changed files; build optimized tools. Run smoke before broader GPU dispatch.
4. Validate exact inventories/deltas and write durable report/manifest/STATUS.

## Validation and Acceptance

Targeted domain: reuse explicit 107-identity selection.slice-208-frozen.tsv (all wave/quad plus
float64/double/helper/value/vector/matrix neighbors), all321 cells. Discovery full existing97 plus
one new source =294 cells (full discovery chosen for helper/control-flow/storage neighbors).
All other frozen1035 cells inherit full210 explicitly. Compare classification, return_code,
execution_counts, diagnostic and canonical_shape. Expected old deltas zero; three new correct cells.
Retain53 open failures and4 resolved histories. No frozenv1/source/oracle changes.
Focused GPU3 plus strict aggregate/unsupported units; NVVM/routing/reporter units; runtime4;
toolkit18; all6 complex compile/assembly. Actual PTX must show activemask then ballot(observed,true)
and binary64 paired shuffles. Last full210 cadence0; bounded targeted acceptance advances to1.
Provider/ABI/library/general lowering changes, unexplained deltas or uncertain impact trigger full.

Build: timeout --kill-after=30s 30m cmake --build --preset releaseWithDebugInfo --parallel 4
--target slangc slang-test render-test test-server. Test commands follow WORKFLOW, outputs below;
census --workload-ids-from selection.slice-208-frozen.tsv --jobs4, discovery full manifest --jobs4.

## Failure and Recovery

Stop dispatch immediately for GPU loss. Keep before/after artifacts separate, no indefinite retries.
If valid input exposes an independent blocker, record trace and ask parent before broadening scope.
Original final-fixture failure is the minimal revert proof; provider artifacts are never mixed.

## Artifacts and Hand-Off

Raw build/nvvm-loop/slice-213-{before,after}; durable plan, report.slice-213-fp64-implicit-shuffle.md,
runtime-validation.slice-213.json, census tables and STATUS. Record exact tested hashes, gates,
fresh/inherited outcomes, helper inventory/input-shape audit. Parent owns acceptance/local commit.

## Structural Fixture Discoveries

The real GPU fixture passed all three modes immediately after guard removal. The combined structural
fixture exceeded the fake's single array-element-type slot; isolate it in a fresh session rather
than expanding array infrastructure. A Ptr<double> kernel entry was outside the existing direct
entry contract; use established Ptr<int> destination after conversion. Finally the fake intrinsic
validator hardcoded floating width 32 despite receiving a Float64 descriptor. Parent approved using
that descriptor's bitWidth; existing validator already supports 64. This changes test infrastructure
only and preserves production/provider scope. Raw rejected logs remain excluded from acceptance.

## Final Gate Progress

Final focused 6/6, runtime smoke 4/4, units 477/477 with the existing Windows-only skip, and
all 18 toolkit cells pass. The final test-only change uses descriptor width in the fake validator;
source/binary provenance was frozen after these final fixtures passed. Frozen 321 cells match
full 210 in all five required fields: 313 correct, eight retained preflight stops. This diagnostic
subset returns zero because it has no infrastructure/output failures; that does not make its eight
preflight cells passes. At this intermediate milestone, full discovery and six complex cells were pending; both are now
complete as recorded below.

## Final Outcome and Handoff

All final gates pass their explicit preservation criteria: focused 6, smoke 4, units 477 with one
existing Windows-only skip, toolkit 18, frozen 321 and discovery 294 cells, complex 6. Exact five-field
comparison has zero old deltas; three additions pass; all 53 open and four resolved histories survive.
Final static PTX raw/ballot/shuffle counts by NVRTC O3/NVVM O0/NVVM O3 are 4/4/32, 1/1/8, 4/4/32.
O0 calls its shared helper four times; O3 retains all four inline sites and three divergent branches.
Every ballot consumes the raw snapshot with true; all eight word shuffles consume that ballot.
No self-shuffle site was folded. Scope remains the seven-line guard removal; typed fake validation
uses descriptor width. No new production helper, fallback, representation or provider change exists.

Final compiler SHA256 is 2775a5783a7dd1310ab9773d259bf1bdafc464a4f55a376c1fc22c472a6b1bf0;
provider remains ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372, ABI 36.
All final source/artifact hashes are rechecked in runtime-validation.slice-213.json. Parent reviews
then commits; worker returns exclusive write ownership after the final document/hash check.

Parent accepted targeted validation after independent diff, exact-result, hash and PTX review.
Last full checkpoint remains 210; implementation cadence is 1.
