# Runtime performance prototype

The existing compile-perf nightly now collects this suite after compilation and
publishes a GPU runtime section on the same report site. See
[`../compile-perf/DESIGN.md`](../compile-perf/DESIGN.md#shared-runtime-reporting-and-schedule)
for scheduling, runner prerequisites, failure handling, and history compatibility.
The nightly also exports runtime, reference and paired compile samples with the
compile suite into the BenchView host-collection bundle. The host handles delivery;
export readiness does not imply successful upload.
The runtime landing page links each subtest/backend to its own daily and release
graphs, including paired compilation. Reference comparisons overlay the Slang run
measured at the reference's exact dimensions; full-suite history remains separately
labeled when sizes differ. Missing or incompatible measurements are never joined.
The manual prototype workflow below remains a separate build/smoke check.

## Optimization-sensitive workloads and alternative references

The suite also includes six fixed, versioned cases selected with `--cases sensitive`:

| Cases                                              | Structure being tested                                                                                         |
| -------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| `sensitive_registers_16`, `sensitive_registers_48` | Local arrays with runtime indexing and overlapping live values; array lowering and potential spills            |
| `sensitive_loops_8`, `sensitive_loops_32`          | Fixed outer loops, lane-dependent inner bounds, and indirect input loads                                       |
| `sensitive_material_8`, `sensitive_material_32`    | Nested generic layers constrained by interfaces, specialization/inlining, and mixed per-lane material branches |

The suffix fixes program complexity; `--complexity` does not change these cases.
Element count controls parallel work. The array cases exercise register/local-memory
pressure, but do not claim a particular register count or spill threshold without
profiling. These are initial compiler-sensitive structures, not coverage of all
optimization passes. AD, subgroup/shared-memory algorithms, and real application
kernels remain useful additions.

The normal `perf.py run`, `--slangc`, calibration reuse, and `compare` commands work
with these cases. This is the primary path for comparing ToT with previous daily
runs or releases. Build each runner against the selected compiler with identical
RHI and harness sources. Fixed case names and source hashes prevent changes to the
workload from being mistaken for compiler improvements.

`references.py` adds independently authored alternatives:

- Vulkan: GLSL compiled by glslang to SPIR-V.
- D3D12: HLSL compiled by DXC to DXIL.
- CUDA: CUDA C++ compiled directly by NVRTC to PTX. This is explicitly an NVRTC
  reference, not an NVCC measurement. It does not require CUDA host headers.

The material references flatten the Slang abstractions into arithmetic and branches.
The array and loop references express the same algorithm in each source language.
No reference source is translated from Slang-generated output. They are equivalent
algorithm references, not hand-tuned hardware ceilings. Every implementation checks
every output against a separately implemented CPU calculation, before and after
measurement, using the existing float tolerance. All use FP32, 64-thread groups,
identical inputs, immutable source buffers, and the same runner and timestamp regions.

RHI currently requires Slang reflection to construct resource bindings. The runner
loads the matching Slang module for its explicit binding layout, then provides the
external binary through a private, one-entry implementation of RHI's public shader
cache interface. It accepts only the exact reflected entry-point hash, requires a
cache hit, and rejects any cache miss. It does not alter a disk cache or RHI itself.
This prototype ABI has two structured buffers and one constant buffer. CUDA uses
RHI's 40-byte global block (two pointer/count buffer descriptors and a constant-buffer
pointer). Both CUDA PTX target architectures must match; a mismatch fails the run.
Layout changes require a new workload version and correctness validation.

For example, on an RTX 4090 with CUDA 13 installed (adjust all paths and driver ID):

```powershell
python tools/runtime-perf/references.py `
  --runner build/runtime-perf-sensitive/Release/slang-runtime-perf.exe `
  --backends vulkan,d3d12,cuda --driver-id YOUR-ACTUAL-DRIVER-VERSION `
  --nvrtc "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.0/bin/x64/nvrtc64_130_0.dll" `
  --cuda-arch compute_89 --output results/sensitive-baseline

# A later daily/release compiler, using the previous dimensions and reference toolchains.
python tools/runtime-perf/references.py `
  --runner build/runtime-perf-candidate/Release/slang-runtime-perf.exe `
  --backends vulkan,d3d12,cuda --driver-id YOUR-ACTUAL-DRIVER-VERSION `
  --nvrtc "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.0/bin/x64/nvrtc64_130_0.dll" `
  --cuda-arch compute_89 --baseline results/sensitive-baseline/results.json `
  --output results/sensitive-candidate
```

Enable `SLANG_RHI_ENABLE_CUDA=ON` when building a runner for CUDA. Vulkan/D3D12-only
runs need neither NVRTC nor `--cuda-arch`. The CUDA architecture must match the
actual GPU and Slang target; do not reuse the RTX 4090 value on RTX 5090.

Defaults are 4,194,304 elements, 20 samples per round, two rounds with reversed
implementation order, and a 25 ms batch target. Calibration measures both
implementations and freezes the larger required batch for the pair. If the target
cannot be reached within 1,024 iterations, increase `--elements` and establish a
new baseline. No result is silently resized during historical comparison.
Individual-run stability, pooled relative MAD, and between-round median range
must all pass the 5% policy. This is a measurement acceptance policy, not a 5%
regression threshold. Confirm suspected changes in additional interleaved runs.

Each output directory contains `report.html`, `results.json`, the source files,
external compiler logs, raw measurements, and the actual binary for each round.
The report shows per-case Slang/reference ratios and, with `--baseline`, changes
against the prior Slang run. Comparisons reject environment, source, reference
toolchain/binary, harness, dimensions, or coverage changes and incomplete/unstable
results. The shared nightly collects equivalent-reference bundles and publishes them
inside the compile-perf report. Standalone runs remain usable offline; preserve
their bundles when making daily or release comparisons.

Raw results retain one-time reference compilation observations and Slang API
preparation times. These have different boundaries and are not presented as
comparable compile-time benchmarks. Use the normal repeated `perf.py --slangc`
measurements for compiler-time tracking.

### Equivalent implementation versus tuned target

Keep these two questions separate. The equivalent reference measures how well
Slang handles the same algorithm and its abstractions. A tuned alternative gives
an empirical performance target. Slang beating the equivalent reference does
not establish that either implementation is optimal.

`tune_references.py` explores explicit optimization candidates separately, keeping
the equivalent implementation and Slang in the measurement rotation:

```powershell
python tools/runtime-perf/tune_references.py `
  --runner build/runtime-perf-sensitive/Release/slang-runtime-perf.exe `
  --baseline results/sensitive-baseline/results.json `
  --driver-id YOUR-ACTUAL-DRIVER-VERSION `
  --nvrtc "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.0/bin/x64/nvrtc64_130_0.dll" `
  --cuda-arch compute_89 --output results/sensitive-tuning
```

The initial tuning focus is `sensitive_material_32` on all three backends. Candidates
add explicit static-loop unrolling; CUDA also uses read-only loads. A second
material candidate folds the two affine layers into coefficients. This changes
FP32 operation order, so it is judged under the same documented numerical
tolerance, not bitwise equality. `--cases all` also explores static-loop unrolling
for the other cases. This is a small initial search space, not an exhaustive tuner.

The tuning report shows Slang, equivalent, and best measured reference times,
with every candidate's variability and raw results retained. It freezes the
baseline's dimensions and uses four rounds with rotated implementation order.
If an optimized candidate falls below the timing target, establish a larger
baseline before adopting its result. Selecting a minimum is statistically biased:
confirm a winning source independently, then freeze that source as a versioned
target. Do not reselect the fastest variant silently in each daily run. Historical
Slang regressions remain separate from reference-tuning improvements.

CUDA architecture checks are intentional: for example, a historical compiler that
emits `sm_80` cannot be compared with a reference targeting `sm_89` as if they used
identical compilation constraints. Align targets or establish a separate baseline.
Paired CLI compilation reads the native PTX architecture and requests the same
Slang capability, rather than compiling a different default target.

## Ray-tracing stability cases

Three fixed cases run on Vulkan and D3D12: `rayquery_visibility` (terminate on
first opaque hit), `rayquery_closest` (closest inline query), and
`raytrace_pipeline` (raygen, closest-hit, and miss). CUDA coverage explicitly
excludes these cases; there is no OptiX or software fallback.

The scene contains 2,048 opaque triangles in a 32-by-32 grid on two depth planes,
with the farther triangles preceding the near triangles in primitive order. One
identity instance references the BLAS. Deterministic rays produce 75% hits and
25% misses, away from triangle edges. Full output validation checks visibility,
nearest distance, primitive ID, and both barycentric coordinates against analytic
CPU intersections. IDs and flags are exact; distance and barycentrics use a
2e-5 absolute tolerance. Outputs are checked before and after sampling.

BLAS/TLAS construction, allocation, upload and synchronization are outside trace
sampling and calibration, recorded as one `scene_setup_ms` host observation.
`gpu_trace_ms` covers the instrumented trace dispatches; normal batch and host
boundaries still apply. Scene version/build flags are recorded and comparison
rejects changed scene contracts. These are controlled traversal stability cases,
not representative heavy material shaders or an acceleration-structure build benchmark.

The pipeline archives code for all three entry points. Paired CLI compilation
measures their shared source module; it is one compile module, not three
independent compile observations. Together with the original and sensitive cases,
the suite contains 23 runtime cases and 25 compile modules.

## Original stability workloads

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
Only)** with its `runtime-perf` input enabled. This also builds the runner on
hosted Windows and executes all workload families with paired compilation on
the upstream Linux NVIDIA GPU pool. The compiler package is pinned to 2026.17.1
with verified checksums. Windows CI validates the build and orchestration;
hardware runtime measurements come from the Linux job. Both jobs upload their
diagnostics, and the GPU job archives the full result bundle. Forks can run the
Windows check but do not have access to that upstream GPU pool.

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
are 16,777,216 elements and 1,024 executions. Image width is 256 and height scales
with element count. Each calibration step warms up and takes five pilot samples.
It aims for at least 1.25 times `target_ms` (25 ms by default) and relative
median absolute deviation (MAD/median) at or below `max_spread` for the GPU total.

Final measurement freezes those dimensions, warms up after CPU validation,
and takes 10–30 samples by default. The run is valid only if the total median
is at least `target_ms` (25 ms by default) and the GPU total meets the 5%
relative-MAD target. Individual GPU stage variability is reported, but does
not gate calibration or success by
default. This avoids increasing job sizes just to chase noisy phase timings
on a laptop in active use. Hardware activity, thermal drift, or reaching the size limit can leave
a run `unstable`; additional samples do not guarantee low variance. Calibration
history, raw samples, phase variability, and selected dimensions are archived.

### Sizing on fast GPUs

The default limits leave room for calibration on fast GPUs without changing
shader complexity. On a Windows RTX 4090 (driver 591.86, Slang 2026.17,
complexity 4), 8,388,608 elements at batch 128 left 12 of 14 cases below
10 ms on both Vulkan and D3D12. At 16,777,216 elements, all cases cleared
10 ms, but a 25 ms target provides more duration margin.

Calibration with that target selected 16,777,216 elements and batch 256 for
all single-pass cases, and batch 128 for the image pipeline. Three complete
runs per backend, with frozen dimensions and 30 samples per case, passed all
84 case runs. The largest within-run relative MAD was 1.30% on Vulkan and
0.10% on D3D12. The largest range of the three run medians, divided by their
median, was 3.30% and 0.18%, respectively. These are local observations, not
accuracy guarantees or regression thresholds.

The 1,024-execution ceiling leaves calibration room beyond those measured
sizes. An RTX 5090 CI runner must calibrate independently; its sizes and
stability have not been measured here. Do not reuse the RTX 4090 calibration
on another GPU. Larger batches remain serialized throughput measurements,
and resource-array inputs still have their fixed small working set.

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

Schema version 2 separates repeated execution samples from single host
observations. Version 1 bundles and calibrations must not be mixed with these
results; create a fresh baseline after updating the runner.

### Repeated execution samples

All values are milliseconds. Host values measure elapsed wall time, including
blocking and scheduling, rather than CPU execution cost.

| Field                          | Boundary                                                                                                                                                 |
| ------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `gpu_total_ms`                 | GPU timestamps around the entire serialized batch, including ordering and timestamp instrumentation overhead.                                            |
| `gpu_<stage>_ms`               | Sum of that stage's timestamp regions across the batch, including its ordering barriers. These are instrumented pass regions, not pure shader execution. |
| `gpu_iteration_ms`             | GPU batch total divided by batch count; throughput with reused resources, not isolated dispatch latency.                                                 |
| `host_record_ms`               | Command encoder creation through command-buffer finalization, including binding and recording.                                                           |
| `host_submit_ms`               | Wall time inside queue submission.                                                                                                                       |
| `host_wait_ms`                 | Wall time waiting for queue completion, including remaining GPU execution and host scheduling.                                                           |
| `host_record_to_completion_ms` | Encoder creation through completion of the host wait. Excludes query reset/result retrieval, setup, calibration, and correctness validation.             |

Host recording, submission, and waiting are contained in record-to-completion.
GPU intervals overlap host intervals; do not add them together. Per-stage GPU
regions need not sum to the batch total because there is work between regions.
Raw samples, summary statistics, and GPU relative MAD remain available.

### Single host observations

`host_observations` contains one wall-time observation of each named interval.
These are diagnostics, not repeated startup or transfer benchmarks:

- `case_ms`: adapter lookup through completion of the final correctness check.
  Includes device/shader preparation, calibration, both validations, final warmup,
  sampled execution, and timestamp result retrieval. Excludes argument parsing,
  result serialization, teardown, Python orchestration, and paired CLI compilation.
- `device_create_ms`: the device creation API call, including its internal setup.
- `resource_setup_ms`: the final calibration step's CPU input generation, buffer
  allocation/initialization/upload, and queue synchronization. Earlier allocation
  steps and destruction of the previous buffers are not included in this value.
- `calibration_ms`: the complete calibration loop, including all allocation steps,
  query-pool creation, pilot warmups/executions, and pilot timestamp retrieval.
  Frozen-calibration runs still perform their single pilot step.
- `validation_before_ms` and `validation_after_ms`: the complete correctness check
  before and after measurement, including CPU reference calculation, output
  readback, comparison, and temporary storage allocation/destruction.
- `validation_readback_before_ms` and `validation_readback_after_ms`: the readback
  API call inside the corresponding validation. Both observations are preserved.

These intervals are nested: resource setup is inside calibration, readback is
inside validation, and all are inside case time. Do not sum them as disjoint phases.

`preparation_observations` records one observation per shader stage: module-load
API time (`load_ms`), composition/link API time (`link_ms`), target-code API time
(`codegen_ms`), and combined RHI program/pipeline creation (`program_pipeline_ms`).
These are API boundaries rather than isolated compiler passes. Deferred work,
reflection, compiler caches, and driver caches can affect attribution; driver
caches are not flushed. They are included in case time, but not execution samples.

GPU upload/readback timing, detailed allocation/binding breakdowns, timestamp
and barrier overhead estimation, percentile/confidence-interval reports, automated
regression thresholds are not implemented. Shared nightly publication is described
in the compile-perf design.

## Paired compile benchmarks

`--slangc` reuses `compile-perf/bench.py` for repeated compilation of the exact
same generated sources, with the backend target, `sm_6_6`, row-major default
layout, and `-O3`. It checks the CLI compiler version against the runner's
loaded compiler. The CLI and RHI API are distinct compilation paths; their
timings should be labeled accordingly. Target-code hashes identify what the
runtime actually prepared. The image's three compile results remain separate;
a sum of medians is not a sampled whole-pipeline compilation distribution.

The original 16 entries are also available directly through compile-perf, with a
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

Each subtest/backend/cadence shows two graphs: GPU execution time (suite Slang,
matched-run Slang and equivalent reference) and compilation time (CLI wall and
compiler internal timings, with separate series for image stages). Runtime series
label their element counts; only matching dimensions support direct reference
comparisons. Changed measurement contracts remain separate curves with gaps.
