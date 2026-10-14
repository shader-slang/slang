# Runtime performance prototype

Six compute workload families share deterministic shader generators with
`tools/compile-perf`. This is a standalone, headless C++/slang-rhi runner with
standard-library-only Python orchestration. Measurement requires no network,
CI service, dashboard, Python packages, or credentials.

## Workloads

| Family                    | Cases                                                                          | Complexity axis                                          |
| ------------------------- | ------------------------------------------------------------------------------ | -------------------------------------------------------- |
| Arithmetic                | `arithmetic_basic`, `arithmetic_transcendental`                                | Generated arithmetic-chain length                        |
| Matrix transforms         | `matrix_row`, `matrix_column`                                                  | Matrix/vector multiplication-chain length                |
| Resource arrays           | `resources_static`, `resources_dynamic`                                        | Number of buffer descriptors and accesses                |
| Interface dispatch        | `dispatch_direct`, `dispatch_generic`, `dispatch_dynamic`                      | Material implementations and evaluations                 |
| Automatic differentiation | `autodiff_primal`, `autodiff_forward`, `autodiff_reverse`, `autodiff_analytic` | Differentiable function-chain length                     |
| Image pipeline            | `image_pipeline`                                                               | Horizontal/vertical filter radius; tone mapping is fixed |

There are 14 execution cases and 16 compile entries (the image pipeline has
three shaders). `complexity` is 1–64, default 4. Element count is a runtime
uniform and does not change shader bytes. Resource arrays have fixed 256-float
buffers and uniform runtime descriptor indexing: increasing element count
does not enlarge their cache working set. Nonuniform indexing is a future
capability-gated case. Matrix payloads use nonsymmetric, bounded matrices;
row/column variants receive the corresponding physical storage layouts.

All variants validate every output element against independently implemented
CPU references, both before and after measurement. Inputs are immutable;
outputs and intermediate image buffers are separate. The image prototype is
buffer-backed, so it does not yet measure texture sampling. The dispatch case
uses mixed material IDs. Uniform IDs, parameter-block resource variants,
texture workloads, and a separate streaming control are future extensions.

## Build once on the measurement platform

Use a Slang package or a build configuration directory containing `bin/` and
`lib/`, and matching public headers. A compiler source build is unnecessary.
Initialize `external/slang-rhi` and its submodules first. CMake may fetch RHI
build dependencies; an offline build needs those dependencies already cached.

```sh
cmake -S tools/runtime-perf -B build/runtime-perf -DCMAKE_BUILD_TYPE=Release \
  -DSLANG_ROOT=/path/to/slang-package \
  -DSLANG_INCLUDE_DIR=/path/to/matching/slang/include
cmake --build build/runtime-perf --config Release --parallel
```

The executable is `build/runtime-perf/slang-runtime-perf` with a single-config
generator, or typically `build/runtime-perf/Release/slang-runtime-perf.exe` with
a Windows multi-config generator. Keep the selected compiler libraries and
downstream compilers available. RHI's `SLANG_RHI_ENABLE_*` options select built
backends; `SLANG_RHI_SOURCE_DIR` can select an explicit RHI checkout. The
prototype links its compiler at build time: build separate runners against
baseline and candidate compiler packages, holding RHI and build settings fixed.
It does not yet provide a relocatable binary distribution or ABI compatibility
validation for arbitrary historical compiler releases.

### Windows quick start (PowerShell)

Use an x64 Slang package with matching headers and an installed GPU driver.
Run from the repository root with CMake, Python, and a Visual Studio C++
toolchain available. Replace the package path and driver version below.

```powershell
git submodule update --init --recursive external/slang-rhi
$slangPackage = "C:/sdk/slang"
$env:PATH = "$slangPackage/bin;$env:PATH"
cmake -S tools/runtime-perf -B build/runtime-perf -A x64 "-DSLANG_ROOT=$slangPackage" "-DSLANG_INCLUDE_DIR=$slangPackage/include" -DSLANG_RHI_ENABLE_VULKAN=ON
cmake --build build/runtime-perf --config Release --parallel
$runner = "build/runtime-perf/Release/slang-runtime-perf.exe"
python tools/runtime-perf/perf.py devices --runner $runner
python tools/runtime-perf/perf.py run --runner $runner --backend vulkan --adapter 0 --cases all --driver-id YOUR-ACTUAL-DRIVER-VERSION --slangc "$slangPackage/bin/slangc.exe" --output results/windows-vulkan
```

Select the adapter index from `devices`. To test D3D12, use `--backend d3d12`
and a separate output directory. The build must enable that RHI backend and
the DXC runtime must be available. Both backends use the same 5% total-spread
default; per-pass noise is advisory. Keep the complete output directory when
reporting a failure, including the per-case logs.

## Local and CI usage

For an explicit CI test, manually dispatch **Check Python Scripts (Core Python
Only)** with its `runtime-perf` input enabled. The `runtime-perf-target-ci`
branch defaults to the same Windows performance pool used by compile-perf:
`["Windows", "X64", "nvrgfx-perf-kernelvm-bridge"]`. It builds the native
runner and executes all 14 runtime cases plus their 16 paired compile entries
on that machine. The compiler package is pinned to 2026.17.1 with matching
headers and verified checksums. This tests the new benchmark workloads and
runner, using a released compiler.

```sh
gh workflow run check-python-core.yml --repo shader-slang/slang \
  --ref runtime-perf-target-ci \
  -f runtime-perf=true -f runtime-target=windows-perf \
  -f runtime-backend=both -f runtime-max-spread=0.05
```

`runtime-backend` selects `vulkan` (the default), `d3d12`, or `both`; both APIs
run sequentially into separate result bundles. Set `windows-runner-labels`
to a JSON array to select another Windows machine. `runtime-cases` accepts
`all` or a comma-separated subset, and `runtime-adapter` selects the backend's
adapter index. For example, add `-f runtime-cases=arithmetic,image` for a focused
run or `-f runtime-max-spread=0.03` for 3%. The artifact
`runtime-perf-windows-results` includes machine/driver identity, device
enumeration, full samples, calibration, sources, target code, and logs. A run
with unsupported hardware or unstable total measurements fails and retains
its diagnostics.

Use `-f runtime-target=linux-gpu -f runtime-backend=vulkan` to select the
upstream NVIDIA GPU pool instead. That mode also checks the hosted Windows
build. Forks do not have access to the upstream performance pools. Workflows
are dispatched explicitly; pushing this branch does not launch a benchmark.

Run these commands from the Slang repository root. All paths and the backend
are explicit; CI can invoke exactly the same commands. Use an empty output
directory for each run.

```sh
python3 tools/runtime-perf/perf.py list
python3 tools/runtime-perf/perf.py devices --runner build/runtime-perf/slang-runtime-perf

# A focused development run; automatically calibrates execution size.
python3 tools/runtime-perf/perf.py run \
  --runner build/runtime-perf/slang-runtime-perf \
  --backend vulkan --adapter 0 --cases matrix \
  --driver-id YOUR-ACTUAL-DRIVER-VERSION --output results/matrix

# All six families, with paired compiler measurements.
python3 tools/runtime-perf/perf.py run \
  --runner build/runtime-perf/slang-runtime-perf \
  --config tools/runtime-perf/ci-example.json \
  --driver-id YOUR-ACTUAL-DRIVER-VERSION \
  --slangc /path/to/the-same-compiler/bin/slangc --output results/baseline

# Candidate: reuse the baseline's dimensions and batch counts exactly.
python3 tools/runtime-perf/perf.py run \
  --runner /path/to/candidate/slang-runtime-perf \
  --config tools/runtime-perf/ci-example.json \
  --driver-id YOUR-ACTUAL-DRIVER-VERSION \
  --calibration results/baseline/calibration.json --output results/candidate

python3 tools/runtime-perf/perf.py compare \
  results/baseline/results.json results/candidate/results.json
```

CLI settings override JSON settings. `--cases` accepts comma-separated family
or case names. `--environment-id` records a caller-defined machine/power policy
identifier. Record the actual driver version using `--driver-id`; comparison
refuses an unknown driver. Host and OS identity are recorded automatically.
For native Metal, the OS build identifies the driver, but the currently checked
RHI revision lacks Metal timestamp readback: Metal is reported as unsupported.
Vulkan on macOS requires an installed Vulkan implementation and its driver ID.
No backend or timing-method fallback is performed.

`--allow-unsupported` is useful for exploratory runs. CI should leave it false
to enforce the selected coverage. `--allow-unstable` retains exploratory runs
without a failing process status, but their measurements remain marked unstable
and cannot be compared. Compilation, execution, and correctness failures always
fail. The subprocess timeout is per runtime case. Compile-perf's own compiler
timeouts apply to optional CLI compilation measurements.

## Calibration and measurement contract

Calibration starts at 65,536 elements, doubles up to `max_elements`, then doubles
the number of ordered executions per batch up to `max_batch`. Initial limits
are 4,194,304 elements and 128 executions. Image width is 256 and height scales
with element count. Each calibration step warms up and takes five pilot samples.
It aims for at least 1.25 times `target_ms` and relative median absolute deviation
(MAD/median) at or below `max_spread` for the GPU total.

Final measurement freezes those dimensions, warms up after CPU validation,
and takes 10–30 samples by default. The run is valid only if the total median
is at least 10 ms and the GPU total meets the 5% relative-MAD target. Individual
GPU stage variability is reported, but does not gate calibration or success by
default. This avoids increasing job sizes just to chase noisy phase timings
on a laptop in active use. Hardware activity, thermal drift, or reaching the size limit can leave
a run `unstable`; additional samples do not guarantee low variance. Calibration
history, raw samples, phase variability, and selected dimensions are archived.

`--max-spread 0.05` is a development starting point, not a guarantee of 5%
accuracy or a regression threshold. MAD measures within-run dispersion; it
does not bound run-to-run drift. For dedicated CI runners, choose a tighter
target based on repeated baseline runs. `--strict-phases` also requires every
GPU stage to meet that target. The CI example uses the same defaults as local
development so shared runners do not implicitly require dedicated-machine
stability. Comparisons retain phase ratios and flag phases above the spread
target; treat those ratios as advisory. Changing either policy setting requires
a fresh baseline and calibration.

`elements` and `batch` can both be specified to disable calibration. Saved
calibration freezes both automatically and verifies source, environment,
measurement policy, adapter, and RHI identity. Use a fresh calibration series
after changing these. Calibration never changes shader complexity.

`batch=1` measures a single execution. Larger batches are explicitly labeled
`batched_throughput`; `gpu_iteration_ms` is total batch time divided by batch
count, not isolated dispatch latency. Repeated executions reuse the same
working set. Global barriers order storage reuse and timestamp boundaries;
this prototype measures serialized execution including those barriers and
timestamp instrumentation, not an application's overlapped scheduling.

## Phases and artifacts

Every run writes `results.json`, `calibration.json`, exact shader sources, target
code blobs and hashes, commands, and per-case stdout/stderr logs. Results retain
compile-perf's summary/raw-sample statistics shape under `timers`, plus richer
runtime metadata. Runtime bundles have a versioned envelope and must not be
passed directly to the existing compile-only dashboard or trend script.

Measured phases:

- Device initialization and resource initialization/upload/synchronization
  as separately labeled host setup intervals.
- Per-stage module loading, link, target-code generation, and RHI program/pipeline
  preparation. These are one-time preparation observations, not repeated cold
  startup benchmarks. Driver caches are not flushed.
- GPU total and each dispatch/pass, plus normalized time per pipeline iteration.
- Host recording, submission, waiting, and recording-through-completion latency.
- Validation readback as a separate host interval.

CPU and GPU intervals overlap and must not be added together. GPU pass timings
exclude resource setup and CPU reference checks, but include ordering barriers.
GPU upload/readback timing, detailed allocation/binding breakdowns, timestamp
overhead estimation, percentile/confidence-interval reports, automated regression
thresholds, and publishing adapters are not implemented in this prototype.

## Paired compile benchmarks

`--slangc` reuses `compile-perf/bench.py` for repeated compilation of the exact
same generated sources, with the backend target, `sm_6_6`, row-major default
layout, and `-O3`. It checks the CLI compiler version against the runner's
loaded compiler. The CLI and RHI API are distinct compilation paths; their
timings should be labeled accordingly. Target-code hashes identify what the
runtime actually prepared. The image's three compile results remain separate;
a sum of medians is not a sampled whole-pipeline compilation distribution.

All 16 entries are also available directly through compile-perf, with a
complexity ladder of 2, 4, 8, and 16. They are opt-in so existing nightly and
release-sweep workloads remain unchanged:

```sh
python3 tools/compile-perf/bench.py --slangc /path/to/slangc --label prototype \
  --only runtime_dispatch_dynamic,runtime_autodiff_reverse --sweep
```

These are useful compile benchmarks, particularly dispatch and autodiff.
Increasing runtime element count never increases compilation work. The tone
mapping shader is a fixed-cost compile baseline; its compile complexity does
not grow with the image filter radius.

## Tests

```sh
python3 -m unittest discover -s tools/runtime-perf -p 'test_*.py'
```

GPU-free tests check comparison compatibility, raw sample validity, frozen
calibration, and byte identity between all compile/runtime shader pairs.
The existing Python workflow runs them on Linux and Windows. GPU validation
requires running the suite on the selected backend; unsupported backends are
not treated as successful GPU coverage.
