# Supervise bounded opt-in complex compilation batches

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer requires completed plans and reports
in slice commits; raw logs, measurement apparatus and generated artifacts remain ignored in build/.

## Purpose and Observable Result

`run-complex-corpus.py --test-server build/RelWithDebInfo/bin/test-server` will compile at most six
cells per server process using its existing shared global session and a fresh request per cell.
The default fresh slangc path and schema remain unchanged. The opt-in report preserves one primary
result per identity, plus a separately costed mandatory fresh-process reference pass, exact PTX
comparisons, assembly and protocol evidence. No material kernels run.

## Progress

- [x] 2026-09-24: Read guidance, 210/213 checkpoint and 211/212 research, local build skill and server.
- [x] 2026-09-24: Implemented bounded supervisor and explicit opt-in; 15 direct tests pass, including real hooks and fake protocol faults.
- [x] 2026-09-24: Real six-cell shared smoke passed with six fresh references; 28 measured/warmup batches and all 168 exact PTX comparisons passed. Median lifecycle improvement18.02355%; six hashes assemble.
- [x] 2026-09-24: Full 1650-cell checkpoint matches210+213 across all five fields; all 546 runtime sources and selection/oracles unchanged.1597 correct / 53retained failures, four resolved histories.
- [x] 2026-09-24: Final 15 protocol tests,32 routing/reporter,4 discovery contracts and4 GPU smoke pass. Four external command observations complete; full report/manifest/tables/architecture/STATUS prepared.
- [x] 2026-09-24: Explicit-path formatting and final source/binary/hash audit pass; unrelated historical design formatting restored. Worker returns checkout write ownership to parent for acceptance/local commit.

## Surprises and Discoveries

The server already supports slangc internally, GLSL enabled, request-local command-line mode, and
DIE/KILL/GARBLE_ON_REQUEST hooks. `quit` has no reply; wait for process exit. Global profiler totals
can accumulate, so opt-in cell phase medians must remain null. TestToolUtil maps SLANG_E_INTERNAL_FAIL to signed returnCode=-1, unlike POSIX CLI exit 255;
the protocol preserves that distinction. Cleanup must kill the owned POSIX group even if its leader
already exited; a pipe-holding child test covers this. Default toolkit timers poll at about
50ms; new performance evidence must use equivalent precise lifecycle clocks on both policies.

## Decision Log

- 2026-09-24 worker: require explicit --test-server path; do not infer compiler compatibility.
  Fresh references and exact PTX equality reject incompatible outputs. Record executable/library hashes.
- 2026-09-24 worker: use a small Python standard-library supervisor, not a new compiler executable,
  ABI or provider. Drain raw stdout/stderr, cap headers/bodies, validate exact IDs and result types,
  include startup in first deadline and bound quit/exit/cleanup. One in-flight request only.
- 2026-09-24 worker: predeclare two paired warmups and twelve measured pairs, alternating policy
  order, six forward and six reverse cell rotations. Retain all observations; no threshold tuning.

## Outcomes and Retrospective

The paired batch lifecycle gate passes (9.991360s fresh,8.190561s shared;18.02355%). Each service
latency median improves 19.73–22.88%; these are observable CLI/request boundaries, not compiler-call
phase times. All 168 PTX comparisons match and all six hashes assemble. After timing, only a cold
RecursionError catch and its test changed; the exact timed/final identities are retained.

The mandatory reference work prevents overall savings in both unreplicated whole-command
observations: one-sample fresh 13.40s/shared 24.08s; actual defaults fresh 43.87s/shared 48.83s. Shared
reference work takes 12.815/12.866s. Retain explicit opt-in and do not recommend this as a routine
checkpoint accelerator. No threshold or observation was discarded or revised.

The final full checkpoint freshly preserves all 1650 cells,1597 correct,53 open failures and four
resolved histories, with zero five-field deltas and all 546 runtime sources matching accepted base.
Fresh protocol 15/15, routing/reporter 32/32, discovery contracts 4/4 and GPU smoke 4/4 pass. Unchanged
compiler units 477/477 + one skip and toolkit 18/18 explicitly inherit 213. No compiler/provider/ABI,
material source, oracle or host change occurred. Parent owns acceptance/commit; worker returns
write ownership after final artifact formatting and audit. No push or worker commit.

## Context and Current Pipeline

Two unchanged tiled-brass entries times NVRTC O3/NVVM O0/O3 give six identities. The runner currently
starts slangc for every attempt. `TestServer::_executeTool` calls `SlangCTool::innerMain` with the
GLSL-enabled `m_session`; innerMain creates/releases an ICompileRequest, processes identical CLI
arguments and compiles. A finite process lifetime bounds global/allocator/library retention.
Research 212's 10.767% API-lifetime improvement motivates measuring actual protocol/process costs.

## Scope and Non-Goals

Only complex runner/protocol/tests, plan/report/status/manifest and relevant architecture change.
No compiler/provider, source, oracle, ABI, backend options, new runtime inputs or runtime speed claim.
Default invocation/exit/schema remain compatible. No automatic retries or fallback on failure.

## Architecture and Invariants

Fresh references each compile and assemble before batch measurements; a failed reference blocks
acceptance. Every successful shared attempt requires byte-identical PTX for that identity, expected
entry and SM80 target; final shared outputs assemble separately. A protocol failure leaves the
completed prefix explicit, active cell failed and suffix incomplete; later samples cannot erase it.
One process owns at most six requests and exits before its total lifecycle timer ends. Full raw
frames and stderr survive errors. Timing does not interpret cumulative global profiler data.

## Interfaces and Dependencies

Explicit --test-server enables an additive opt-in schema with reference/protocol/timing evidence.
Existing compiler/toolkit options remain exact. Native Linux RelWithDebInfo, source
build/nvvm-loop/slice-203-env.sh, CUDA12.9.2/NVRTC86, LLVM14, L4 SM89 driver580.126.09, target SM80,
provider ABI 36. Base786a2452f576fc620c9eeb4019bb277a20f394cd. Compiler library SHA
2775a5783a7dd1310ab9773d259bf1bdafc464a4f55a376c1fc22c472a6b1bf0; provider SHA
ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372.

## Milestones

1. `complex-test-server.py` supervisor plus direct unittest contract scenarios and runner opt-in.
2. Real server hooks and controlled fake peer cover fragmented/truncated frames, timeout including
   first request, crash/signal, malformed/oversize frames, wrong IDs/types, shutdown cleanup and suffix.
3. Validate all six identities in old default and new shared paths; preserve reference failure/mismatch.
4. Measure batches using ignored apparatus in build/nvvm-loop/slice-214-performance, then full checkpoint.
5. Produce runtime-validation.slice-214.json, full census/discovery tables, five-part report and STATUS.

## Validation and Acceptance

Predeclared performance gate: >=10% median complete six-cell process-lifecycle reduction versus six
fresh CLI processes, startup and graceful teardown/exit included; no >5% median per-identity service
time regression where genuinely comparable. Per-request response latency includes different lifecycle
work and is not a compile-call metric. Use paired precise clocks, two warmups plus twelve balanced
measured pairs, exact options/PTX equality/entry/target checks, independent assembly, raw samples,
order analysis. Record mandatory fresh-reference cost and actual whole-command cost separately;
batch savings alone never establish overall runner savings. Notify parent before timing; host idle.
If threshold fails, record rejection and consult parent, never lower it or hide mandatory costs.

Focused protocol/default/shared tests precede small GPU smoke and full frozen 452x3=1356 plus
98x3=294 discovery cells, sequential suites at jobs 4 maximum. Compare all five stable fields
(classification, return_code, execution_counts, diagnostic, canonical_shape) against full 210 plus 213.
Expected 1597 correct / 53 open failures and four resolved histories, zero missing/duplicate/source deltas.
Full runners expected exit 2. Unchanged compiler units 477/477 + one Windows skip and toolkit 18/18 may
inherit 213 explicitly; run relevant routing/reporter tests fresh. All 6complex cells compile/assemble.

## Failure and Recovery

No retry. Preserve first error and all logs; blocked/unexecuted cells remain incomplete. Kill/wait on
timeout and bound shutdown, retaining cleanup status separately. A new output directory identifies
any manual rerun. Device loss stops dispatch without retry/reboot. Disable opt-in by omitting flag.

## Artifacts and Hand-Off

Raw root build/nvvm-loop/slice-214-*. Durable report and manifest record exact commands/hashes,
reference versus primary coverage, preservation ledger and measured verdict. Formatting uses explicit
changed paths. Parent reads/reviews, accepts and commits after worker returns checkout write ownership.

Parent accepted the full checkpoint after independent diff, exact-result, timing and artifact review.
Latest implementation and full checkpoint are 214; implementation cadence resets to 0. The default
remains unchanged because neither observed short/default command is faster overall.
