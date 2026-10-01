# L0 CPU shadow pilot

`shadow-l0-linux-release-cpu` runs on pull requests that already build Slang.
It downloads the same Linux GCC release artifact as the full CPU job, and uses
the same environment, expected-failure lists, CPU/LLVM API filter, and four test
servers. The extra selection is `-category quick`, which includes smoke and the
Slang unit-test module. This is a candidate boundary to evaluate, not an approved
L0 coverage contract. GPU hardware tiers are unchanged.

The reusable test workflow opts into `shadow: true`. Its job tolerates failures,
and the caller is absent from `check-ci.needs`. Existing required test jobs keep
their defaults. The shadow only observes slang-test; the separate slangc script
is still covered by regular CI.

## Observation artifacts

Each run uploads an `l0-shadow-linux-x86_64-gcc-release-ATTEMPT` artifact with:

- `result.json`: revision, run identity, exact commands, elapsed time, exit codes,
  final test counts, step outcomes, and observation status.
- `candidates.txt` and `dry-run.log`: discovery before runtime API filtering.
  Candidate count must not be interpreted as executed count.
- `test.log`: actual test results, including failing test names and per-test times.
- `summary.md`: the same advisory summary shown in the Actions job.

The helper limits discovery to two minutes and testing to ten minutes; the job
has a twenty-minute limit including setup. A timeout, missing summary, or empty
selection is not a successful observation. Failed setup gets an infrastructure
failure report when checkout succeeded. If checkout fails or GitHub cancels the
job, artifact publication is not guaranteed; inspect the Actions job itself.
Artifacts expire after fourteen days.

## Evaluate before promoting

Compare the shadow and full CPU jobs within each workflow run, using the same
revision. Record candidate/executed counts, test duration, time from workflow
start to result, setup failures, and tests that failed in full CI but were absent
or passed in the shadow. The run URL in result.json identifies the matching full
job. This draft does not automatically classify full-CI failures or compute
historical runtime percentiles; that comparison is a review step during the pilot.
There is no second build, but the shadow adds a hosted runner job and artifact
download. A small test selection cannot remove build or queue latency.

Before making L0 required or moving tests out of PR CI, agree on representative
feature coverage and review the missed-failure record. No full-CI coverage is
removed by this draft.

Local helper validation:

```sh
python3 -B -m unittest discover -s extras/tests -p test_l0_shadow.py
```
