# Refresh NVVM results

This is a results-only workflow. It does not restart the development loop. Run from the repository
root on native Linux, with matching optimized compiler, provider and test tools. Read STATUS and
WORKFLOW first. Rebuild through `slang-build` after source changes; never benchmark stale binaries.
The maintained entry point is `python3 issue-nvvm-backend/nvvm-results.py --help`.

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
  --baseline issue-nvvm-backend/runtime-validation.slice-260.json \
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
  --baseline issue-nvvm-backend/runtime-validation.slice-260.json \
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
The next command examples call this reviewed file
`issue-nvvm-backend/results.baseline.json`; use its actual path. It must retain `runtime_input_sha256`,
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
  --correctness issue-nvvm-backend/results.baseline.json \
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
from validated complete measurements. Copy compact summaries/charts and the reviewed correctness
ledger into the presentation/results package; keep all raw logs/samples under ignored build. Add
links to exact source revision, manifests, binaries/toolkit/device, protocol, unresolved cells,
cache/math settings and limitations. A future refresh follows the same commands with a new output
root; avoid a new slice-specific benchmark script. Update STATUS with the package path and explicit
stopped-loop authority. The finite maintenance request ends here.
