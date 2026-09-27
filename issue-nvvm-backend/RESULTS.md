# Refresh NVVM results

This is a results-only workflow. It does not restart the development loop. Run from the repository
root on native Linux, with matching optimized compiler, provider and test tools. Read STATUS and
WORKFLOW first. Rebuild through `slang-build` after source changes; never benchmark stale binaries.
The maintained entry point is `python3 issue-nvvm-backend/nvvm-results.py --help`.
When configuring after a revision change, clear cached `SLANG_VERSION_FULL` and
`SLANG_VERSION_NUMERIC` (`cmake --preset default -U SLANG_VERSION_FULL -U SLANG_VERSION_NUMERIC`
plus the selected build options on this native Linux host). Verify `slangc -version` identifies
the compiler source revision; source and binary hashes remain the acceptance identity.

The expanded discovery corpus also requires the standard numerics modules from the same build.
Ensure the existing `slang-numerics-modules` target is included when building through `slang-build`;
it produces `slang/numerics.slang-module` and the three serialized modules under
`slang/numerics/` in the configured standard-module directory. Do not copy serialized modules from
a different compiler build. For a results-only refresh, verify these artifacts against the accepted
ledger along with the compiler/core module. Missing modules are a packaging failure, not evidence
that the shader or backend is unsupported. Preserve such failed attempts before restoring the
matching layout or rebuilding and requalifying it.

## Evidence and outputs

Every harness command requires a **new** `--output` directory and refuses to overwrite old evidence.
Use a unique ignored root, for example `build/nvvm-results/2026-09-28-refresh1`. Failed attempts stay
there. Measurements retain every command, exit, timeout, log, phase, PTX/cubin and content hash.
Provenance records revision, working diff, submodules, compiler/provider/cache/toolkit bytes, sources,
inputs and device query. Review the actual loaded libraries as well when changing environments.
Do not publish the full environment/raw trees inadvertently; only compact reviewed results belong
in the repository. Commands default to SM80 and matching CUDA12.9 on this host.

```bash
export NVVM_RESULTS=build/nvvm-results/2026-09-28-refresh1
export NVVM_BASELINE=issue-nvvm-backend/accepted-baseline.json
export CUDA_PATH=/usr/local/cuda-12.9
export CUDA_HOME="$CUDA_PATH"
export LIBNVVM_HOME="$CUDA_PATH"
export SLANG_NVVM_TEST_ARCH=80
export SLANG_NVVM_BUILDER_PATH="$PWD/build/RelWithDebInfo/bin/libslang-llvm-nvvm.so"
export LD_LIBRARY_PATH="$PWD/build/RelWithDebInfo/lib:$PWD/build/RelWithDebInfo/bin:$CUDA_PATH/nvvm/lib64:$CUDA_PATH/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
```

The harness selects that toolkit's NVVM/NVRTC libraries and exact provider. A command failure returns
nonzero and keeps partial evidence. Census exit2 can describe established gaps; exact structured
comparison decides preservation. Running processes are killed as a group after their timeout.

## Correctness baseline and comparison

After a master/toolchain merge, first run the small GPU gate and full corpus wrapper:

```bash
python3 issue-nvvm-backend/nvvm-results.py checkpoint \
  --baseline "$NVVM_BASELINE" \
  --slangc build/RelWithDebInfo/bin/slangc --build-label RelWithDebInfo \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --jobs 4 --output "$NVVM_RESULTS/checkpoint"
```

This runs the established four-fixture runtime validator, frozen1356 inventory, discovery manifest
and every material support cell sequentially. It writes `checkpoint.json`, `comparison.json` and
`outcomes.json`. Full compiler acceptance **also** needs these gates, sequentially with no benchmark:

```bash
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries slang-unit-test-tool/
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
  tests/language-feature/generics tests/language-feature/overload \
  tests/language-feature/operator-overload tests/diagnostics tests/serialization
python3 extras/validate-nvvm-toolkit.py --slangc build/RelWithDebInfo/bin/slangc \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --expected-toolkit 12.9 --architectures 80 --output "$NVVM_RESULTS/toolkit"
python3 issue-nvvm-backend/test-run-compute-census.py
python3 issue-nvvm-backend/test-run-compute-discovery.py
python3 issue-nvvm-backend/test-run-complex-corpus.py
python3 issue-nvvm-backend/test-nvvm-results.py
```

Capture each gate's command, exit and log under the results root; use `timeout --kill-after=30s 30m`
for long gates. Compare unit/semantic identities and statuses with the last accepted ledger, including
skips and upstream additions; totals alone do not prove preservation. Toolkit and smoke validators
require real completed cells. Keep expected unresolved/runtime failure histories visible.

For CPU-only comparison against a durable old compact baseline, even without the old raw directory:

```bash
python3 issue-nvvm-backend/nvvm-results.py compare \
  --baseline "$NVVM_BASELINE" \
  --frozen "$NVVM_RESULTS/checkpoint/frozen/results.json" \
  --discovery "$NVVM_RESULTS/checkpoint/discovery/results.json" \
  --output "$NVVM_RESULTS/comparison"
```

The baseline must be explicitly `accepted-full`. Missing/duplicate cells, changed five-field outcomes,
incomplete mode inventories and false passes are rejected. Additions require `--allow-additions`
and all three correct modes. Comparisons retain unresolved/resolved histories. Input hashes can
legitimately change upstream: checkpoint writes each old/new hash delta and stops at `review-required`.
Do not overwrite old evidence or feed the rejected outcomes back as an accepted baseline.

After all gates and exact deltas are reviewed, replace `accepted-baseline.json` with the complete current record from
`outcomes.json`, set `status: accepted-full`, attach gate evidence, reviewed input/outcome transitions,
and update the failure histories. Keep the original comparison under the ignored results root and identify the previous baseline by Git revision, path and hash. A manual
status edit without that review is not acceptance. The compact schema produced by checkpoint already supplies all required fields:

```json
{
  "schema": 1,
  "status": "accepted-full",
  "baseline": { "path": "previous accepted JSON", "sha256": "..." },
  "provenance": {
    "revision": "tested revision",
    "artifact_sha256": { "absolute path": "sha256" }
  },
  "runtime_input_sha256": { "tests/path.slang": "sha256" },
  "corpora": {
    "frozen": { "fresh_cell_outcomes": ["all id/mode/five-field rows"] },
    "discovery": { "fresh_cell_outcomes": ["all id/mode/five-field rows"] }
  },
  "unresolved_failures": [],
  "resolved_failure_history": [],
  "acceptance": {
    "gate_evidence": [],
    "reviewed_transitions": [],
    "reviewed_input_deltas": []
  }
}
```

The sample shows field structure only; preserve actual complete rows and histories. Copy the complete
checkpoint outcomes, add reviewed gate/transition evidence, and change status only after acceptance.
The harness deliberately has no automatic promotion command: a matching count is not a review.
Update the current record in place; Git retains the old accepted snapshot. Do not add another
slice-numbered validation file. Retain exact cells and failure transitions, not nested old baselines.
If a run has an infrastructure failure, retain the failed comparison. A supplemental closure needs
an explicit, predeclared scope and review: preserve every failed attempt, require all declared rounds
to pass, identify each substituted cell and its source evidence, and distinguish original counts from
composite accepted counts. Serial closure never proves concurrent reliability. Historical NVRTC PCH incidents and their later
per-owner-directory resolution remain in the accepted evidence; do not reintroduce an already resolved
limitation as current. Do not use retries to hide shader-output regressions or failed timing samples.

Use the reviewed current `NVVM_BASELINE` identified by STATUS before quality measurement. It must
retain `runtime_input_sha256`,
`provenance.artifact_sha256`, and per-corpus `fresh_cell_outcomes` for the next refresh.

## Report environment

Execution and comparison use the Python standard library. Shareable SVG/PNG charts use Matplotlib:

```bash
python3 -m venv build/nvvm-results-tools
build/nvvm-results-tools/bin/python -m pip install -r issue-nvvm-backend/requirements-results.txt
```

The report records the installed Matplotlib version. Use that environment only for report commands.

## Material compilation benchmark

```bash
python3 issue-nvvm-backend/nvvm-results.py material \
  --slangc build/RelWithDebInfo/bin/slangc --build-label RelWithDebInfo \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --output "$NVVM_RESULTS/material"
build/nvvm-results-tools/bin/python issue-nvvm-backend/nvvm-results.py report \
  --measurements "$NVVM_RESULTS/material/measurements.json" --output "$NVVM_RESULTS/material-report"
```

The fixed material protocol runs six cells (two entries x NVRTC O3/NVVM O0/O3), two rounds in opposite
cell order, each with two warmups and nine measured fresh processes:132 compiles (108 measured,
24 warmup). Assembly is a separate eleven attempts per cell:66 assemblies (54 measured,12 warmup).
Every PTX/cubin hash must agree within its cell. No sample removal, automatic retries or cherry-picking.
Reports include median/IQR/range, per-round wall statistics and individual nested compiler phase
statistics. Keep raw diagnostics, including NVRTC PCH status when emitted. Warmups intentionally warm
filesystem/toolkit caches; a fresh process does not imply cold filesystem/PCH state. Record effective
math options and compiler/toolkit versions when comparing revisions. Unpaired historical sessions
are context, not a causal speedup claim. Run no build, GPU suite or profiler concurrently.

Automatic NVRTC PCH status is not necessarily visible in CLI logs: the driver appends its marker
to raw artifact diagnostics, while the ordinary CLI forwards parsed entries. An absent marker means
no directly observed PCH state; it does not prove the cache was disabled. Even a `not-created`
marker alone cannot distinguish reuse from a decision not to create. Label cache evidence as
observed, source-inferred or unavailable; keep normal cache behavior.

The material command does not explicitly select a floating-point mode. The current source maps this
to Slang Default, requesting neither `--fmad=false` nor `--use_fast_math`. Upstream adds
`--fmad=false` only for explicit Precise mode. Record actual commands and distinguish source-inferred
downstream options from an observed option trace. Changing math flags changes the experiment and
requires matching correctness evidence.

The existing bounded shared-session runner is a **separate experiment**, not interchangeable timing:

```bash
python3 issue-nvvm-backend/run-complex-corpus.py \
  --slangc build/RelWithDebInfo/bin/slangc --test-server build/RelWithDebInfo/bin/test-server \
  --build-label RelWithDebInfo --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so \
  --cuda-root "$CUDA_PATH" --warmup 2 --samples 9 --output "$NVVM_RESULTS/shared-material"
```

Retain `fresh_references`, `fresh_reference_seconds`, every batch and complete lifetime fields,
`runner_work_seconds`, and whole-command wall time. Request timers omit startup/reference/validation
cost; never present them as the end-to-end shared workload cost. The normal report command accepts
the maintained material/quality measurement schema, not this distinct shared-runner schema.

## Fixed simple-shader quality subset

```bash
python3 issue-nvvm-backend/nvvm-results.py quality \
  --correctness "$NVVM_BASELINE" \
  --slangc build/RelWithDebInfo/bin/slangc --build-label RelWithDebInfo \
  --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
  --output "$NVVM_RESULTS/quality"
build/nvvm-results-tools/bin/python issue-nvvm-backend/nvvm-results.py report \
  --measurements "$NVVM_RESULTS/quality/measurements.json" --output "$NVVM_RESULTS/quality-report"
```

The checked-in `quality-corpus.manifest.json` fixes12 representative existing shaders/36 modes and
source hashes. It includes integer/vector arithmetic, half/matrix values, helper transport, multiple
resources, row-major structured matrices, groupshared memory, switch/loop flow and inout mutation.
Its runtime IDs must have accepted three-mode correctness with identical source and compiler/provider
bytes. Explicit compiler options preserve row-major fixture selection. This is not the whole corpus
and does not assert runtime correctness of material shaders.

Compile each quality cell once, assemble with `ptxas -v`, record **named-function** register/stack/spill
resources and exact entry resources, retain optional `cuobjdump --dump-sass` and ELF section reports.
Absent metrics are null/unavailable, not zero. SASS instruction counts and executable `.text` sizes
cover the whole module. Cubin bytes include metadata; PTX bytes include formatting and symbol text.
Quality command latencies are single observations, not a repeated speed benchmark. None of these
metrics proves GPU speed, occupancy, numerical equivalence or a particular optimization's presence.

## Reporting and optional compiler-stage attribution

For a qualified instrumented material run, use the same reporter with an explicit option:

```bash
build/nvvm-results-tools/bin/python issue-nvvm-backend/nvvm-results.py report \
  --stage-attribution --measurements "$NVVM_RESULTS/material/measurements.json" \
  --output "$NVVM_RESULTS/attribution-report"
```

This adds `stage-attribution.json`, `.md`, `.svg` and `.png`. Keep the raw scope logs: the reporter
requires matching names, invocation counts and values, plus nested and outer interval containment.
It forms disjoint durations, remaining wall time and percentages per sample before computing medians
and inclusive IQRs; warmups are excluded and both rounds retained. Do not stack or sum marginal
medians. NVRTC exposes no separate verification call; its zero in that category is not proof of no
internal verification. The opaque vendor compile API is not pure optimization time.

Temporary attribution requires independently checked scope boundaries. Historical profiler patches
and packages are available through [HISTORY](HISTORY.md); they are not part of the current compiler.
Revalidate boundaries after source changes, preserve the accepted binary/module/cache layout, and
prove output preservation before interpreting timings. Restore the accepted layout at closeout.
Discarded instrumentation does not create a new full correctness baseline.

`report` produces readable/structured summaries and SVG/PNG figures under the new ignored output
root. Inspect generated charts and retain every sample there. Publish or export a presentation only
when requested; do not commit a new dated result package for every refresh. If a performance result
supports a current architectural decision, retain its compact reviewed metrics, provenance and limits
in the current evidence and matrix. Replace superseded summaries; Git owns earlier snapshots.

If a future source change invalidates the fixed quality manifest's hashes, review that fixture's
semantics and runtime obligations first. Update the manifest only as part of an accepted change,
then rerun quality against the matching correctness ledger. Do not simply replace hashes to make a
measurement pass. If the selected subset or options change, describe it as a changed experiment.

A results-only refresh of unchanged source can use the existing accepted-full ledger after verifying
its compiler/provider/input identities. A compiler, toolkit, shared runner or configuration change
requires the correctness gates prescribed by WORKFLOW before new claims. Preserve failed attempts
under the ignored results root. Update the current baseline/identity/focused evidence as applicable,
STATUS and feature matrix; keep completed working plans/report drafts uncommitted. Make the reviewed
local commit and stop unless further work was explicitly authorized.

## Documentation and evidence maintenance

[accepted-baseline.json](accepted-baseline.json) owns full comparison outcomes and native identities.
[accepted-identity.json](accepted-identity.json) preserves the current verified runtime/layout/config
snapshot and dependency pins. [focused-evidence.json](focused-evidence.json) owns selected later
qualifications and open failures outside the full corpus; it does not change full-checkpoint cadence.
Their historical provenance/loop fields never override STATUS/WORKFLOW authority.

Replace the accepted baseline only after reviewed full validation, identifying its predecessor by
Git revision, path and hash. Older repository paths in retained failure histories are archival
citations resolvable through HISTORY; `compare` reads embedded rows without opening those references.
Local `build/` evidence may be absent in a fresh checkout. Current runner inputs must exist
independently of both raw evidence and Git archives.

Retain `census.slice-195.tsv`, `census.slice-146.tsv` and `census.slice-146-clusters.json`: their old
names identify live frozen selection/overlap/summary inputs, not disposable reports. Preserve their
bytes unless a separately reviewed inventory change explicitly authorizes otherwise. Discovery,
material and quality manifests remain current input authorities.

For summary utilities, set output paths under `build/` explicitly (in particular the discovery
summarizer's `--table` and `--clusters` options); legacy defaults can generate numbered files in the
issue directory. Ignore guards prevent accidental snapshot commits. Keep source/tool behavior changes
separate from documentation cleanup. A documentation-only migration needs exact data/input checks,
link/reference checks and relevant CPU contracts; it does not require a fresh GPU checkpoint.
