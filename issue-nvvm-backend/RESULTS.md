# Refresh NVVM results

This is a results-only workflow. It does not restart the development loop. Run from the repository
root on native Linux, with matching optimized compiler, provider and test tools. Read STATUS and
WORKFLOW first. Rebuild through `slang-build` after source changes; never benchmark stale binaries.
The maintained entry point is `python3 issue-nvvm-backend/nvvm-results.py --help`.
When configuring after a revision change, clear cached `SLANG_VERSION_FULL` and
`SLANG_VERSION_NUMERIC` (`cmake --preset default -U SLANG_VERSION_FULL -U SLANG_VERSION_NUMERIC`
plus the selected build options on this native Linux host). Verify `slangc -version` identifies
the compiler source revision; source and binary hashes remain the acceptance identity.

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
export NVVM_BASELINE=issue-nvvm-backend/runtime-validation.slice-262.json
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

After all gates and exact deltas are reviewed, create one compact checked-in accepted record from
`outcomes.json`, set `status: accepted-full`, attach gate evidence, reviewed input/outcome transitions,
and update the failure histories. Preserve the original comparison and baseline reference. A manual
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
If a run has an infrastructure failure, retain the failed comparison. A supplemental closure needs
an explicit, predeclared scope and review: preserve every failed attempt, require all declared rounds
to pass, identify each substituted cell and its source evidence, and distinguish original counts from
composite accepted counts. Serial closure never proves concurrent reliability. Accepted262 demonstrates
this for one NVRTC automatic-PCH deletion incident; its open reliability limitation must remain visible
until separately resolved. Do not use retries to hide shader-output regressions or failed timing samples.

Set `NVVM_BASELINE` to this newly reviewed file before quality measurement (the setup example uses
the current accepted262 record; recheck STATUS in later sessions). It must retain `runtime_input_sha256`,
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

The fixed slice257 protocol runs six cells (two entries x NVRTC O3/NVVM O0/O3), two rounds in opposite
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

## Package and stop

`report` generates a readable `summary.md`, structured `summary.json`, and standalone SVG/PNG figures (`wall-time` for repeated material timing, `entry-registers` for quality)
from validated complete measurements. Copy compact summaries/charts and link the reviewed correctness
ledger from the presentation/results package; keep all raw logs/samples under ignored build. Add
links to exact source revision, manifests, binaries/toolkit/device, protocol, unresolved cells,
cache/math settings and limitations. A future refresh follows the same commands with a new output
root; avoid a new slice-specific benchmark script. Update STATUS with the package path and explicit
stopped-loop authority. The finite maintenance request ends here.

Use a new package directory for each accepted refresh. For the existing package layout:

```bash
export NVVM_PACKAGE=issue-nvvm-backend/results/2026-09-28-refresh1
mkdir -p "$NVVM_PACKAGE/material" "$NVVM_PACKAGE/quality"
cp "$NVVM_RESULTS/material-report/summary.md" "$NVVM_RESULTS/material-report/summary.json" \
  "$NVVM_RESULTS/material-report/wall-time.svg" "$NVVM_RESULTS/material-report/wall-time.png" \
  "$NVVM_PACKAGE/material/"
cp "$NVVM_RESULTS/quality-report/summary.md" "$NVVM_RESULTS/quality-report/summary.json" \
  "$NVVM_RESULTS/quality-report/entry-registers.svg" "$NVVM_RESULTS/quality-report/entry-registers.png" \
  "$NVVM_PACKAGE/quality/"
# Matplotlib SVG path lines can contain trailing spaces; retain all XML tokens.
python3 - "$NVVM_PACKAGE" <<'PY_SVG'
import sys
from pathlib import Path
for path in Path(sys.argv[1]).rglob("*.svg"):
    path.write_text("\n".join(line.rstrip() for line in path.read_text().splitlines()) + "\n")
PY_SVG
```

Copy the narrative/presentation structure from the latest package named in STATUS, then replace every
result from the new summary rows. Keep generated summaries/PNGs unchanged; SVG trailing-whitespace normalization above preserves all XML tokens. Derive each material
ratio as NVRTC O3 median divided by the matching NVVM O3 median, and report both entries. Compare
quality metrics only for the same fixture and mode; record entry versus whole-module scope. Inspect
the rendered charts. Update correctness, exact source/binary identities, sample inventory, protocol,
limitations and raw-root references. Local raw paths in JSON identify evidence on this host; they are
not portable download links. The summaries, hashes and accepted per-cell ledger remain durable.

If a future source change invalidates the fixed quality manifest's hashes, review that fixture's
semantics and runtime obligations first. Update the manifest only as part of an accepted change,
then rerun quality against the matching correctness ledger. Do not simply replace hashes to make a
measurement pass. If the selected subset or options change, describe it as a changed experiment.

A results-only refresh of unchanged source can use the existing accepted-full ledger after verifying
its compiler/provider/input identities. A compiler, toolkit, shared runner or configuration change
requires the correctness gates prescribed by WORKFLOW before new presentation claims. Preserve old
packages and all failed attempts. Commit the compact completed plan/report and package, update STATUS
and HANDOFF navigation, and stop unless further work has been explicitly authorized.
