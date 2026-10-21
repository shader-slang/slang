# Plan: recombine Windows GPU API test jobs

Status: proposed, 2026-10-21. This document plans a measured consolidation; it does not
change the CI workflow or its required checks.

## Motivation and evidence

[PR #12422](https://github.com/shader-slang/slang/pull/12422) merged on
2026-08-07 at 22:08 UTC. It increased Linux test-server concurrency and split each
Windows GPU configuration into DX, Vulkan/CPU/LLVM, and CUDA jobs. Recombining the
Windows jobs should reduce repeated discovery, compilation, unit tests, setup, and
runner demand. It can also lengthen the longest individual job, so reduced job count
alone is not a rollout criterion.

The October investigation used the monthly archives at
[`slang-ci-analytics` revision `8d87cd59`](https://github.com/shader-slang/slang-ci-analytics/tree/8d87cd59380720c83445a1458c95184aadb6df8d).
The following medians are minutes. Dates use Europe/Helsinki and the original workflow
trigger. Successful full runs require a successful `check-ci` and all Windows GPU
Debug/Release test jobs. The baseline uses combined jobs; later windows use all three
API partitions. PR and merge-queue turnaround remain separate populations.

| Metric                                                  | Jul 25–Aug 7 | Aug 10–23 | Oct 7–20 |
| ------------------------------------------------------- | -----------: | --------: | -------: |
| Required PR turnaround: trigger to `check-ci`           |        88.75 |    147.33 |    80.95 |
| Required merge-queue turnaround                         |        84.10 |    102.73 |    80.46 |
| Longest Windows Debug GPU test job, excluding queue     |        47.68 |     34.93 |    41.27 |
| Summed Windows GPU test-job minutes, Debug plus Release |        62.48 |    125.55 |   143.57 |
| Successful full PR runs                                 |          156 |       126 |      115 |
| Successful full merge-queue runs                        |           59 |        55 |       80 |

Execution includes setup and all steps in each `test-slang` leaf job, including its
via-GLSL invocation where applicable. Summed job-minutes exclude builds and RHI;
they are not CPU utilization or billed cost. Cancelled, failed, and incomplete runs
are outside this table. The archives lack `run_attempt`, so turnaround includes
retries within a workflow run; this is not a first-attempt benchmark.

These observations do not establish a causal turnaround improvement from the split.
The 90th-percentile PR turnaround rose from 202.08 to 249.77 minutes, and merge-queue
turnaround from 123.13 to 160.51 minutes. The
[Falcor approval gate](https://github.com/shader-slang/slang/pull/11915) landed earlier
on August 7 and was on the required path until
[PR #12775](https://github.com/shader-slang/slang/pull/12775) merged on August 27.
Falcor was the last completed build/test before `check-ci` in 127 of the 181
successful full runs in the August post-change window. Capacity, approvals, test
growth, retries, and other workflow changes must remain visible in comparisons.

## Proposed workflow shape

Use one combined main `slang-test` invocation per Windows x86_64 GPU configuration.
Keep Debug and Release separate and retain their existing build dependencies.

| Configuration | Replace these callers                             | Combined caller                      |
| ------------- | ------------------------------------------------- | ------------------------------------ |
| Debug         | `test-windows-debug-cl-x86_64-gpu-{dx,vk,cuda}`   | `test-windows-debug-cl-x86_64-gpu`   |
| Release       | `test-windows-release-cl-x86_64-gpu-{dx,vk,cuda}` | `test-windows-release-cl-x86_64-gpu` |

For both combined callers in [ci.yml](../../.github/workflows/ci.yml):

- Reuse `ci-slang-test.yml`, the existing build artifact, `full-gpu-tests: true`,
  `gpu-tier: t4`, and the `Windows,self-hosted,GCP-T4` runner selection.
- Set the explicit API union to `dx11+dx12+wgpu+vk+cuda+cpu+llvm` so consolidation
  preserves the current three masks. Retain eight test servers initially.
- Run the main invocation once. API-independent files and mixed-file preparation
  must run; do not apply an API-only filter or skip independent files in this job.
- Keep the T4 and Debug skip lists, expected-failure lists, validation settings,
  cleanliness checks, offline CUDA-prelude check, and GPU setup/health checks.
- Replace the six old `check-ci.needs` entries with the two new caller IDs in the
  same change. Preserve aggregate failure propagation and the docs-only skip path.
- Retain the independent Debug/Release RHI jobs and all other platform jobs.
  Preserve the Linux concurrency increases from #12422; do not revert that PR wholesale.

### Main and via-GLSL execution

Preserve exactly one via-GLSL invocation per Windows configuration on pull requests,
using its existing `-api vk` mask. Preserve the current omission on `merge_group`.
The actual reusable-workflow condition includes Debug despite the input description
saying "release pull requests"; the implementation must reconcile that description
without silently dropping Debug coverage.

Initially evaluate the main and via-GLSL invocations in the combined caller. The
reusable workflow currently has an 80-minute timeout. A roughly 48-minute historical
combined main job plus Debug via-GLSL can exceed that budget: do not infer safety by
adding medians, and do not raise the timeout without measuring the actual stages.
Require combined-job p95 below 64 minutes, leaving 20% of the current limit as
headroom. If that fails, keep one combined main job and move via-GLSL to a dedicated
required caller per configuration. That caller must invoke only via-GLSL, reuse the
same build, preserve its PR-only condition, and have its own cleanliness check.
It must not call the full reusable workflow and repeat the main suite.
Add any dedicated via-GLSL caller to the aggregate gate. Either keep its caller
successful with skipped test steps on non-PR events, or teach `check-ci` to accept
that specific event-dependent skip; a failed or unexpectedly skipped PR invocation
must still fail the gate.

### Relationship to API-independent-file filtering

[PR #13803](https://github.com/shader-slang/slang/pull/13803) is a separate, currently
open improvement to the split layout. At head `0ce184a9e`, matching baseline names
showed 5,102 Debug and 5,104 Release previously shared tests passing in Vulkan and
ignored in DX/CUDA. Another 780 file-test names and 652 unit-test names remained
shared per configuration; equal names do not prove equal API-dependent work.

That result is an interim split-layout baseline, not evidence that a combined job
will be faster. If #13803 lands first, omit its `skip-api-independent-tests` input
from the combined callers so the default remains false. Retain the runner option
and regression coverage unless a separate audit proves they have no other use.
If #13803 is still open, consolidate from master without copying its four opt-ins.
Rebase whichever change lands second and recheck the resolved commands.

## Analytics prerequisites and coordinated changes

[PR #13204](https://github.com/shader-slang/slang/pull/13204), "Track required CI gate
metrics", remains open at `b0ac054d7`. It changes the collector, visualization,
runner classification, and regression tests. Coordinate with that PR rather than
copying a second required-gate implementation into this consolidation.

1. Land or rebase #13204 independently before using the published dashboard to
   judge rollout. Until deployed, compute the same required-gate metrics directly
   from archived/live records and label the dashboard's last-job endpoint as a
   different metric. Optional Falcor work must not extend required turnaround or
   turn a successful required gate into a reported failure.
2. Preserve `run_attempt` when collecting new jobs. Select the latest completed
   exact-name `check-ci`, then restrict attempt-level timing, outcome, queue, and
   critical-path inputs to that gate's attempt. Keep original-trigger-to-final-gate
   developer wait separate from attempt duration. Legacy records without attempt
   identity must remain explicitly legacy; do not fabricate reliable attempt IDs.
3. Keep Falcor bridge label/name classification from #13204. Combined Windows jobs
   still use the existing Windows GPU group, so no new runner class is necessary.
4. In `ci_visualization.py`, aggregate both historical combined IDs and split IDs
   into a Windows configuration family while retaining their raw names. Report the
   layout as combined, split, or mixed; do not silently pool layouts or overwrite
   archives when a familiar combined job name reappears.
5. Show longest partition duration and sum of partition job-minutes separately.
   Distinguish main from via-GLSL timing; job-level archives alone cannot separate
   two invocations in one job. Collect step timing or explicit invocation timings
   prospectively rather than reconstructing unavailable historical measurements.
6. Report valid queue time separately from execution, count negative/missing queue
   timestamps as unknown, and keep approvals/dependency waits distinct from runner
   wait. Compare PR and merge-queue runs independently, with sample counts, p50,
   p90, failure/cancellation rates, and optional tail.
7. Treat the existing platform-level build-plus-test "speed of light" and
   `analyze-ci-parallelization.py` name-grouped chains as approximations. Verify
   actual matching build/configuration dependencies before using either as the
   acceptance metric; summing parallel partitions is not a critical path.

Extend `extras/ci/analytics/tests/test_ci_analytics.py` with historical combined,
split, transition/mixed, and recombined fixtures. Cover optional Falcor tails,
multiple attempts, unknown legacy attempts, invalid queue timestamps, and skipped
workflow wrappers. Confirm no Windows job-minutes disappear because the suffixes
change. Keep the current JSON/static-dashboard collection and publication pipeline.

## Validation and rollout

1. Record the base SHA, workflow SHA, actual API arguments, test-server count, runner
   image/driver, event, configuration, and all skip/expected-failure lists. Collect
   a fresh split baseline after #13803 if it lands; the undeduplicated August layout
   must not be the only control for a new consolidation.
2. On the same source revision and runner image, compare each split invocation's
   results with the combined invocation. Compare the union of test identities,
   including API labels, synthesized variants, and outcomes. An API-ineligible
   ignored occurrence in one partition does not require ignoring that test in the
   combined run when another partition passes it. Investigate missing/extra tests
   and outcome changes instead of relying on equal aggregate counts.
3. Verify independent files, mixed-file compile producers and their consumers,
   API-specific tests, and unit tests remain covered. Check for cross-API state
   interference when work that used to have separate processes shares a runner.
   Compare main and via-GLSL identities and results separately.
4. Validate both rendered configurations and PR/merge-queue conditions. Run
   actionlint, Bash syntax checks under Bash 3.2, and ShellCheck on changed shell
   code. Exercise `check-ci` success, failure, and expected-skipped paths; a failed
   combined job must fail the aggregate gate. Run the focused analytics regressions.
5. Trial Release first, then Debug after checking timeout headroom. Keep temporary
   comparison jobs explicitly experimental and outside the required gate; remove
   them before the final production switch. Do not claim capacity savings while
   both layouts run side by side.
6. Evaluate at least 30 successful full PR runs and 20 successful merge-queue runs
   over at least seven days, while also recording failures and cancellations.
   Compare with a recent baseline under comparable runner capacity. Proposed
   acceptance bounds are at least 20% fewer summed Windows main-test job-minutes,
   no coverage loss or new failures, and no increase above 5% in required-CI median
   or p90 turnaround. These are rollout criteria, not predicted results. Extend
   observation when uncertainty or workload differences prevent a conclusion.

Rollback if coverage, reliability, timeout headroom, or turnaround criteria fail.
Restore the six callers and their aggregate dependencies together, including
Vulkan-only via-GLSL ownership and the four independent-file-filter opt-ins if
#13803 is already present. Preserve the analytics corrections and both layout
classifications so the rollback remains measurable. Do not merge automatically.

## Implementation slices

1. Required-gate, optional-tail, retry identity, and runner classification: #13204.
2. Layout-aware Windows analytics, invocation timing, and regression fixtures.
3. Matched Release consolidation trial, coverage evidence, and timeout decision.
4. Debug consolidation trial, required-gate update, and final measured rollout.

At the time of writing, #13204 and #13803 are open. This plan does not close
[issue #13791](https://github.com/shader-slang/slang/issues/13791) or establish that
either runtime change has shipped. Update these dependency states before implementation.
