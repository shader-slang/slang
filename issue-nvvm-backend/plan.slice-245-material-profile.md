# Profile unchanged material compilation

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow requires this completed plan and report
in the slice commit; raw scripts and measurements remain ignored under `build/`.

## Purpose and Observable Result

Measure the intact tiled-brass material's six entry/backend/optimization cells on the accepted244
optimized compiler. Select one evidence-grounded next optimization boundary, or explicitly report
that finer measurement is required. This research implements no compiler or runner optimization.

## Progress

- [x] 2026-09-25: Read workflow, accepted244 and batching214; inspected runner and environment helper.
- [x] 2026-09-25: Found `perf stat -e task-clock true` denied at perf_event_paranoid=4.
- [x] Verify43 source,12 artifact and561 runtime-input hashes against244 before measurements.
- [x] Run two serial rounds, each with2 warmups and9 measured samples per identity; reverse round2.
- [x] Verify all PTX against244; measure independent assembly and inspect timer ownership.
- [x] Write compact evidence, durable design note, report and draft STATUS; recheck hashes and format.

## Surprises and Discoveries

The helper transitively selects Debug before correcting the builder and PATH to RelWithDebInfo.
All compiler/tool paths will be explicit. Existing runner wall timing uses a polling helper; the
local research driver reuses `compile_command` and measures piped `Popen.communicate` through exit.
An initial polling-driver pilot was interrupted and retained under `excluded-polling-pilot`; every
pilot observation is excluded because its measurement method was wrong, regardless of timing value.
Existing214 already established bounded session reuse; its profiler accumulates across requests.
Do not infer per-request phase medians from shared sessions or repeat that optimization study.

Three completed GDB runs collect358 semantic snapshots;63 top frames are the existing subtype
range check and20 are getClass. These are qualitative debugger observations, not CPU fractions.
One auxiliary trial stopped after semantic checking on a pending SIGINT; its117 stacks/log remain,
with no PTX success claim. Corrected local cleanup and independent replacement complete normally.

## Decision Log

- 2026-09-25, worker: Use fresh processes for phase attribution and two opposite-order rounds to
  expose drift. No timing exclusions or retries. Existing shared-session results remain historical.
- 2026-09-25, worker: Do not change system profiling permissions. Existing timers plus source traces
  can prioritize a boundary; an unmeasured function-level hypothesis is not a demonstrated speedup.

- 2026-09-25, worker: Select the canonical NodeBase cast boundary, not generic-solving caches or
  new hierarchy metadata. Existing class tests already use constant-time generated ranges. Future
  prototype must compare inlining with eliminating the tag-to-metadata roundtrip and run exhaustive
  all-tag/class equivalence, semantic tests and a full checkpoint because this boundary is broad.

## Outcomes and Retrospective

Measurements complete and independently accepted by parent. All132 definitive compiler
attempts and66 assemblies pass, matching244 PTX/cubins exactly. Before/after43/12/561 identity matches.
Semantic medians651.5–654.9ms and three completed qualitative GDB runs select canonical NodeBase
subtype checks as the next hypothesis. No optimization or runtime claim. Accepted full244,
targeted233 and cadence0 remain authoritative.

## Context and Current Pipeline

`tests/cuda/complex/tiled_brass_material.slang` exposes `eval_buffer` and `sample_buffer`. Existing
`run-complex-corpus.py::compile_command` selects compute/PTX/SM80, NVRTC O3 or NVVM O0/O3 and
`-report-perf-benchmark`. Front-end checking builds checked AST, IR generation lowers it, then
link/specialize/simplify and backend output produce PTX. `ptxas` assembles that artifact separately.
All six cells pass244; runtime bindings, textures/LUTs, input and output oracle remain absent.

## Scope and Non-Goals

Only research documents and compact evidence change. No compiler/provider/runner/source edits,
builds, GPU execution, kernel claims, FP8 or texture investigations, commit, push or system change.
Parent owns acceptance and commit. One worker owns mutations; no other agents.

## Architecture and Invariants

Preserve exact source and artifact identity, canonical six-cell inventory and PTX bytes versus244.
Timer totals may nest or alias; retain each total but never sum arbitrary phases. Wall residuals are
unattributed; separate compile wall, measured phases and independent assembly. Do not call residual
backend time or claim correctness beyond existing byte preservation and assembly support checks.

## Interfaces and Dependencies

Native Linux base8d53504112617efda0e3446f7b3117bcc2f77fd2, branch nvvm-backend, RelWithDebInfo,
CUDA12.9, providerABI40, L4SM89 targetSM80. Inspect `slice-203-env.sh` before source. Use explicit
build/RelWithDebInfo paths. No build planned. At most4 CPU workers total; serial measurements,
no competing build/benchmarks, per-command timeout180s and outer30-minute bound.

## Milestones

1. Save identity-before.json and local measurement script in `build/nvvm-loop/slice-245-material-profile`.
2. Reuse existing manifest/command builder;2 rounds ×6 identities ×11 attempts. Reverse cell order
   in round2, keep all attempts and logs. Independently assemble each identity after warmup and
   measured repeats; hash every output and require expected entry/SM80.
3. Inspect exact timer scopes and responsible code. Report medians, inclusive quartiles, ranges and
   per-round medians. Distinguish evidence from the next hypothesis and specify preservation tests.
4. Write `timing-evidence.slice-245.json`, `report.slice-245-material-profile.md`,
   `docs/design/nvvm-material-compile-time.md`, completed plan and draft STATUS. Format explicit files.

## Validation and Acceptance

Before/after verify43 source hashes,12 artifacts and561 runtime inputs from validation244. Exact
six identities, all attempts successful, PTX hashes equal244, expected entries/SM80 and assembly
pass. Record reproducible command arrays, fixed tool versions/hashes and all timing samples.
Inherit full244:1695 cells,1654 correct,41 unresolved,16 resolved histories; targeted233/cadence0.
Research unchanged source needs no full corpus or GPU gate. Perf unavailable is an explicit limit.

## Failure and Recovery

Retain failed attempts and stop timing if tool/source identity changes, output differs, a command
times out or infrastructure fails. No retries/exclusions to improve statistics. Use new evidence
paths for reruns. If attribution is insufficient, retain the useful measurements and specify the
smallest next measurement instead of inventing an optimization. No production path needs rollback.

## Artifacts and Hand-Off

Raw logs/PTX/cubins, executable local driver, sample JSON, identity checks and profiler denial remain
under the new ignored raw root. Durable evidence links and hashes refer to these files; old evidence
is immutable. Parent receives a <=500word handoff and explicit checkout release.

Parent acceptance independently verifies all132 compiler and66 assembly attempts, exact outputs,
all timing distributions and current identities,36 compact references,528 indexed artifacts and15
source snapshots. Research245 is accepted for the authorized local commit; full244 remains current.
