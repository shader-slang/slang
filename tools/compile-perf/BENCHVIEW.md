# BenchView reporting

`benchview.py` exports one compile-perf `results.json` to the **Slang** project
(`slang`), suite `compile-perf`. Test and metric IDs are prefixed to leave room
for future runtime suites in the same project. It uses only the Python standard
library. The first accepted submission creates the project if necessary.

## Nightly setup

Set the repository Actions secret `BENCHVIEW_WRITE_KEY` to the shared server's
write key. Optionally set the repository variable `BENCHVIEW_URL`; its default is
`http://rtrci.nvidia.com/benchview`. The dedicated Windows runner must reach it.
Never put the key in command-line arguments, result files or workflow YAML.

For `publish=true`, the nightly exports after the confirmation step and existing
results push. It exports the original `results.json`: confirmation measurements
are diagnostic and do not replace that point in the existing reporting either.
The observed timestamp records the start of the nightly sweep, not the commit date.
Without the secret, the job warns and exports without sending. With the secret,
it posts the batches to `/api/v1/submissions`. A submission failure warns without
blocking the existing reports. Payloads are archived as a workflow artifact even
when submission fails. `publish=false` performs no BenchView export or submission.
Backfills must select a suite ref containing this script.

The workflow submits the nightly point only. Newly swept releases and historical
data can be imported separately using the same CLI. Production authentication and
runner connectivity still need end-to-end validation after the secret is set.

## Export and import

From `tools/compile-perf`, export without a credential:

```sh
python3 benchview.py \
  --results /archive/daily/2026-10-14-8230cda/results.json \
  --meta /archive/daily/2026-10-14-8230cda/meta.json \
  --os Windows --observed-at 2026-10-14T05:00:00Z \
  --execution-id archive-2026-10-14-8230cda \
  --output /tmp/benchview-export
```

Add `--url http://rtrci.nvidia.com/benchview` to submit, with
`BENCHVIEW_WRITE_KEY` supplied through the environment. Use a fresh output
directory per execution. Rerun with identical inputs to replay safely: request
content determines the idempotency key, and transient failures retry the same
bytes up to three times. Redirects are rejected to protect the write credential.

The metadata file requires `commit` (the full compiler commit SHA) and `runner`
(the measured runner fingerprint). Optional `commit_time` and `corpus_sha` retain
source provenance. Supply the measured OS explicitly, even when importing from
another machine. `--branch` is optional; omit it when historical branch provenance
is unknown. Select all branches in the viewer to include those runs.

For a release, add `--release v2026.19` and optionally
`--released-at 2026-09-29T16:37:25Z` when the actual publication time is known.
The version identifies **Slang**, including when comparing other workloads using
different Slang versions. This uses BenchView !4's `run.metadata.release` support:
the same metadata accompanies every batch. No separate annotation operation is
needed. The server must include !4 for release filtering and labels. Daily runs
omit the release metadata. Prepare release metadata with the resolved compiler
commit and measured runner; do not substitute the importing checkout's HEAD.

The CLI requires a measurement timestamp. Old archives often have only a date;
if a midnight approximation is used, record that fact in the metadata file (for
example, `observed_at_basis`). Never use a clone's file modification time.

Logical runs are keyed by compiler commit, suite and configuration (daily/release
and corpus revision). Repeated measurements of the same logical run are execution
attempts, not extra graph points. In particular, days with an unchanged compiler
commit share a point. A changed compiler commit creates a new run.

## Data representation

| Source data                          | BenchView representation                                |
| ------------------------------------ | ------------------------------------------------------- |
| Timer and wall-clock samples         | Independent millisecond metrics with original names     |
| Peak RSS and API memory samples      | KiB metrics, preserving signed deltas                   |
| `compileInner` and `apiTotal` phases | Complete, flat per-sample breakdowns from `lib.buckets` |
| Workload, mode and size              | Namespaced test ID and case dimensions                  |
| Compiler commit, corpus, runner      | Run provenance and observation environment              |
| Failed workload                      | Failed observation and source error message             |
| Slang release tag                    | Native release metadata recognized by !4                |
| Entire source record                 | Inline JSON artifact, including non-graphable fields    |

Breakdown components are derived allocations: overlapping children are scaled to
their parent's budget, missing phases allocate zero, and residuals become self
components. They are not new exclusive timing measurements. Unequal sample counts
are rejected because phase alignment cannot be established.

BenchView allows 64 metrics per observation. Wall time and memory are retained
first, followed by tree/primary timers and remaining timers in name order.
Overflow timers are listed in `ungraphedTimers` and preserved in the source
artifact. BenchView does not reproduce the original nested timer tree, custom
HTML analysis, confirmation diagnostics or regression decisions. Old summary-only
records are rejected rather than inventing raw samples or missing maxima.
Requests are split at 500 observations or approximately 7 MiB (below the 8 MiB
server limit); an oversized individual observation or source artifact is rejected.

Run adapter tests with:

```sh
python3 -m unittest discover -s tools/compile-perf -p 'test_benchview.py'
```
