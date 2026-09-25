# Preserve source-order FP64 masked min/max prefixes

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception.
Base: `29a47957226cf56767bd0f509a1ce56f65f7cea1`, branch `nvvm-backend`.
One fresh-context worker owns implementation; the parent owns acceptance and commit.

## Purpose and Observable Result

Execute canonical scalar/double2/double4 inclusive/exclusive masked min/max through NVVM O0/O3
with the exact CUDA source binary64 operand selections established by research 226. The new focused
fixture must fail before the change, then pass all three modes. Replay all 196 retained dynamic
research inputs/expected words in all three modes (588 executions).

## Progress

- [x] 2026-09-25: Read workflow, status, research 226, build skill and emitter; verify clean base.
- [x] 2026-09-25: Add unchanged final fixture; revert drill records source pass/direct rejection.
- [x] 2026-09-25: Extend typed recipe/shared graph and rebuild optimized tools.
- [x] 2026-09-25: All final gates pass; exact 633-cell fresh comparison has only four expected diagnostics.
- [x] 2026-09-25: Complete self-review, report, manifest, design and STATUS for parent acceptance.

## Surprises and Discoveries

Research 226 proves a universal ascending scan is wrong in 8,658 words across 32 cases. The existing recipe
already owns ordered compare/select and the precise low-contiguous-power-of-two mask classifier.
Exclusive prefixes require separate transmitted inclusive state and returned accumulator.

The first fixture run returned correct source output but failed the strict harness on E30081
(implicit uint-to-bool ternary). Change only the condition to `(lane & 1) != 0`; inputs and oracle
are unchanged. Reverse the exact production patch, rebuild original source, and rerun the final
fixture: NVRTC passes and both direct modes reject canonical double. Restore/rebuild the patch.
All 588 research replay executions now match unchanged 226 inputs/expectations, with 3 PTX assemblies.

## Decision Log

2026-09-25: Extend only FP64 prefixes; FP32 source min/max reductions and arithmetic prefixes keep
existing contracts. Reuse recipe operations and the shared scalar/aggregate graph, without changing
provider ABI 36. Material runtime lacks its application bindings/oracle; correctness-backed runtime
support remains the priority. Matrix capability and the next original-workload blocker are separate.

## Outcomes and Retrospective

Ready for parent acceptance. Fresh focused 3/3, smoke 4/4, units 479/479 plus one existing Windows-only skip,
toolkit 18/18, discovery contracts 6/6, material compile/assembly 6/6 and research replay 588/588.
Fresh frozen 321 cells and discovery 312 cells contain no missing/extra/duplicate inventory or
correctness loss. Exactly four diagnostics advance from double to int8_t exclusive min/max; stop
at that independent boundary. All 630 old fresh cells otherwise preserve all five outcome fields.
The new fixture adds three correct cells. Cumulative ledger: 1668 cells, 1617 correct, 51 failures,
six resolved histories, with 1035 cells explicitly inherited from full 225. All 551 old input hashes
are unchanged; 27 source, 12 artifact and 552 current input hashes are retained. Full checkpoint
225 remains current, cadence one after acceptance. No provider/ABI/system changes or push.

The proper revert drill and exact research replay establish this recipe boundary independently
of diagnostic advancement. The next narrow-integer prefix requires its own contract and evidence;
no second blocker is repaired here.

## Context and Current Pipeline

`WaveMultiPrefixExclusiveMin(value, uint4(mask,0,0,0))` specializes in hlsl.meta.slang to canonical
CUDA GenericAsm. `_initializeNVVMMaskedWaveScalarOperation` before this slice rejected the valid double
signature. `_emitNVVMMaskedWaveScalarValue` serves scalar and homogeneous aggregate leaves. The
correct boundary is the typed recipe/emission algorithm, not AST reconstruction or provider patching.

## Scope and Non-Goals

Scalar/vector FP64 min/max prefixes only. Preserve all reductions, integer/FP32 prefixes, FP64
sum/product, helper/context rules and ABI. No matrix capability, ordinary vector shuffle, quad
reconvergence, system changes, push, or independent next blocker repair.

## Architecture and Invariants

Inclusive seed is caller; exclusive seed is binary64 positive/negative infinity. Ordered compare
selects the second operand on ties/NaNs. Low masks with power-of-two population use ascending
shuffle-up offsets and separate inclusive transmitted state. Other masks scan original inputs in
ascending lane order, only sources strictly earlier than caller. Shuffle sources must be active.
Existing mask classification and typed operation requirement closure remain the single authority.

## Interfaces and Dependencies

Native Ubuntu, L4 SM89/target80, CUDA12.9.2/LLVM14, ABI 36. Source
`build/nvvm-loop/slice-203-env.sh`; build matching RelWithDebInfo tools via native
`cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
CMAKE_BUILD_PARALLEL_LEVEL=1; sequential suites and at most 4 CPU workers. Sandbox initialization
fails RTM_NEWADDR, so shell calls require escalation. No new external dependencies.

## Milestones

1. Add `tests/cuda/nvvm-fp64-prefix-minmax-order.slang`, independent bitwise expectations for tree,
   scan, singleton, NaNs, zeros, finite precision/subnormal boundaries. Run all 3 modes before changes.
2. Extend recipe, identities, mask handling and pending phi state. Register the fixture in discovery.
3. Replay research 226 unchanged inputs/oracle, with PTX assembly. Run final gates after formatting.
4. Generate durable per-cell summary/deltas and retain all 51 known failures and 6 resolved histories,
   resolving only cells proven correct; report original prefix advancement precisely.

## Validation and Acceptance

Targeted domain: frozen `selection.slice-208-frozen.tsv` contains 107 identities/321cells, including
all frozen wave/mask prefixes/reductions and original prefix min/max. All 103 existing discovery
identities plus the new fixture = 104 identities/312 cells cover source min/max, FP64 arithmetic, aggregates, context,
helpers and neighboring operations. Full 225 is the direct baseline, with no intervening compiler
changes. Remaining 1035 frozen cells inherit225 explicitly. Latest full 225 stays current; cadence
advances 0->1 on acceptance. If implementation touches broader shared lowering/provider contracts
or deltas reveal uncertain impact, run full frozen instead. Exact row/mode identity checks and five
outcome-field comparisons are required; no missing/duplicate cells. Unit 479+existing skip, smoke 4,
toolkit 18, discovery contracts 6, material compile/assembly 6; no material runtime claim. Scripts and
raw logs live under `build/nvvm-loop/slice-227-{before,after}`; all suite exits retained.

## Failure and Recovery

Stop GPU dispatch on device loss. Keep mismatches and before evidence unchanged; fix or revert
regressions before acceptance. Record next independent unsupported instruction without fixing it.
Raw research 226 stays untouched; replay uses a separate output directory. No commit by worker.

## Artifacts and Hand-Off

Completed plan/five-part report, `runtime-validation.slice-227.json`, cumulative census TSVs,
updated discovery/design/STATUS. Raw IR/logs/replay remain ignored. Record exact source/binary/input
hashes, fresh/inherited counts, failure history and acceptance deltas; compact parent handoff.

2026-09-25 parent acceptance: independently reviewed final source diff, fixture expectations,
accepted-data replay, all fresh five-field outcomes and full failure histories. Verified216 evidence
references,27 source hashes,12 artifact hashes and552 runtime inputs. Accept targeted633fresh plus
1035inherited cells,1617correct/51known failures/sixresolved histories; full225 remains, cadence1.
Completed plan/report and evidence are included with the local slice commit.
