# Admit FP64 masked min/max through the typed source algorithm

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception. A fresh-context
worker attempt again failed at the app's agent-thread limit; the parent uses WORKFLOW's local fallback.

## Purpose and Observable Result

Make scalar and aggregate double WaveMultiMin/Max execute with the CUDA helper's input-word behavior,
including NaN payloads, signed zeros and low-word precision. Research 219 establishes an independent
integer oracle for 112 cases; both direct modes currently reject before PTX. Reuse the admitted FP32
source algorithm instead of widening numeric min/max semantics.

## Progress

- [x] 2026-09-25: Select slice on clean accepted full 220 base `40f19e54124e1b1d88625cfcc94e4e75b98bdec0`.
- [x] Inspect scalar admission, identity guard, singleton handling and research oracle.
- [x] Add focused dynamic fixture and retain failure before compiler changes.
- [x] Admit FP64 reduction leaves, extend structural checks, format and build optimized targets.
- [x] Run focused, research, runtime, units, toolkit, selected frozen, discovery and material gates.
- [x] Review exact deltas, provenance and input shapes; complete report/STATUS and local commit.

## Surprises and Discoveries

The first new fixture used firstbithigh(mask) in its expected bounds. Existing CUDA U32_firstbithigh
complements words with bit31 set, producing the wrong expected partition endpoint. The diagnostic
fixture confirmed only positive finite/subnormal families failed. Replaced that independent helper
dependency with explicit partition bounds before registering the fixture; raw initial evidence is
retained. Final before run: NVRTC passes, both NVVM modes reject the expected double min helper.
This pre-existing bit-index helper behavior is separate from min/max admission and is queued for a
bounded follow-up; no established runtime oracle was changed.

## Decision Log

2026-09-25: FP64 admission follows correctness fixes 216/218 and explicit capacity prerequisite220.
Other candidates (quad reconvergence, pointer prefixes, vector-by-value shuffles) require independent
representation work. Material runtime requires its missing bindings/oracle; no speculative execution.
Use targeted acceptance for this scalar admission-only change; no provider/library/ABI/shared lowering
change. The source algorithm, graph and deferred phi protocol remain identical.

## Outcomes and Retrospective

Accepted locally on 2026-09-25: scalar/aggregate FP64 min/max preserves the independently validated
CUDA source algorithm. All 336 research executions pass;624 fresh runtime cells have588 correct/36 known
failures, including two old fixes and three additions. Cumulative 1659cells1608correct51 open failures;
1035 frozen cells explicitly inherit220. Six resolved histories retain original evidence.

## Context and Current Pipeline

WaveMultiMin(doubleValue, members) lowers to canonical `_waveMin($1, $0)` GenericAsm; aggregate helpers
recursively use scalar leaves. `_initializeNVVMMaskedWaveScalarOperation` currently enables source
min/max only for Float32, then asks `_getNVVMMaskedWaveScalarIdentity`, which rejects Float64 min/max.
Source reductions need no numeric identity: the caller value seeds the existing butterfly/scan loop.
Typed Float64 ordered comparisons, select and two-word indexed shuffle already exist in ABI36.

## Scope and Non-Goals

Change scalar source-algorithm admission and structural coverage, add one explicit discovery fixture,
and retain plan/report/evidence/design/STATUS. No new loop, ABI/provider operation, numeric min/max
change, prefix admission, oracle weakening, corpus removal, material runtime or performance claim.

## Architecture and Invariants

Only one-lane Float32/Float64 reduction leaves with MIN/MAX use the source algorithm. Do not bypass
the scalar lane-count/width invariant when avoiding the identity helper. All other operations keep
existing identity validation. Floating min/max singleton handling follows the source loop itself;
FP64 sum/product retain explicit original-word singleton selection and existing sum seed handling.
The CUDA helper uses caller-seeded XOR stages for low contiguous power-of-two masks and ascending
original-input scans otherwise. Ordered comparison chooses the first operand only when true; selection
preserves raw words. Input shape is canonical; the admission consumer owns support validation.

## Interfaces and Dependencies

Native Ubuntu L4 SM89 target80, CUDA12.9.2/NVRTC12.9.86 LLVM14 ABI36. Source env
`build/nvvm-loop/slice-203-env.sh`; matching RelWithDebInfo bin/lib. Local slang-build skill applies.
Before compiler SHA256 a13354a47acefc8684cbfb5ae52b84bcf0331a83e5310e9675f53a0e030107d7.
Provider SHA256 ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372 stays unchanged.

## Milestones

1. Add `tests/cuda/nvvm-fp64-minmax-order.slang` with dynamic words, independent closed-form expected
   lane/value selection and three modes. Capture NVRTC pass and both direct preflight failures.
2. Extend source min/max admission for scalar64, avoid identity lookup for this caller-seeded recipe,
   exclude it from arithmetic singleton passthrough. Extend existing typed graph unit and preserve
   negative prefix coverage. Format explicit paths and build slangc/slang-test/render-test/test-server
   with `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4`.
3. Run final source gates sequentially, maximum four workers, two unit servers. Replay exact research 219
   source/inputs/oracles at all three modes (336 executions), preserving inactive sentinels.

## Validation and Acceptance

Focused new fixture plus FP32 algorithm/singletons, FP64 arithmetic and structural negative units.
GPU smoke4, all NVVM/routing/reporter units, toolkit18. Frozen selection
`selection.slice-208-frozen.tsv` (wave/quad plus affected scalar/vector/matrix/helper neighbors), all
101 discovery identities, six material compile/assembly cells. Frozen remainder inherits full 220
explicitly; compare exact IDs and five stable fields to220. Expected two frozen FP64 preflight cells
become correct and three new discovery cells pass; investigate any other delta before accepting.
Preserve53 first-known failures, moving only demonstrated fixes to resolved histories; no reset.
Latest full 220; acceptance advances implementation cadence to one.

## Failure and Recovery

A new correctness regression blocks acceptance. Fix responsible admission or revert bounded change;
record independent next blockers without expanding scope. Stop GPU dispatch on device loss; no driver
change/reboot. Raw expected before failures and apparatus corrections remain separate from acceptance.
No push. Sandbox workaround uses reviewed escalated commands for bwrap network-namespace failure.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-221-{before,after}`; exact research replay, gate logs, hashes and PTX retained.
Durable manifest/census, fixture/manifest addition, completed plan/five-part report/design/STATUS.
Parent locally audits the diff, field deltas, evidence references and final source/binary identity.

2026-09-25 implementation gates: runtime smoke4/4, focused16/16, research 336/336 exact integer-oracle
cases, units478/478 plus existing skip, toolkit18/18 and discovery contracts6/6 pass. The first
structural seed-count assertion conflated two source shuffle-input selects with the sum identity;
corrected the test to assert all three kinds independently, rebuilt units and reran final gates.
Compiler SHA256 55bd12f280ee51def87219c767557e198cbd07b9b99f06d118a20209dfb46598; providerABI36
unchanged. Targeted corpus and material checks subsequently completed; final acceptance is recorded below.

2026-09-25 final acceptance: frozen321cells315correct6 retained preflight, discovery303cells273 correct
30 retained failures, material6/6. All five stable fields preserve old outcomes except the two intended
FP64 min/max fixes; three additions pass. Parent verified 132 evidence references, 23 tested sources,
12 artifacts, 549 runtime inputs. All 53 first-known records remain open or resolved and four older resolved
histories are exact. No source/oracle weakening, missing/extra/duplicate cell or baseline reset.
Latest full 220, cadence 1. Commit this accepted slice before researching unsigned firstbithigh behavior.
