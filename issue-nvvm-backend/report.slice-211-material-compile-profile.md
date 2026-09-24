# Slice 211: measure successful material compilation

## Motivation

The unchanged tiled-brass material now compiles and assembles for both `eval_buffer` and
`sample_buffer` in NVRTC O3 and direct NVVM O0/O3. Earlier single-attempt support checks cannot
establish a performance baseline. Before choosing an optimization, this research gate measures
successful compilation repeatedly and asks which layer owns the observed cost. Application
bindings, texture/LUT inputs and expected outputs remain absent; there is no material execution,
GPU speed or register-count performance claim.

## Proposed solution

Use the accepted RelWithDebInfo compiler with the unchanged corpus runner for two warmups and
seven measured fresh-process compilations per cell. Supplement that baseline with separate
high-resolution process measurements, existing detailed-pass/downstream reporting, repeated
standalone assembly, and an ignored API lifecycle probe. Keep the observations separate because
profiling flags, process lifetime and output capture differ. No compiler, provider, ABI, runner
or shader change is proposed or implemented in this gate.

The evidence supports **one candidate: amortize global-session lifetime in a bounded serial host
batch**. Its responsible layer is harness/session ownership. The API probe provides a feasibility
signal; further six-cell/order/lifetime evidence is needed before implementation. No backend-local
optimization is justified by this profile. Dominant frontend costs and inclusive shared-pass
measurements do not identify redundant work inside NVVM lowering or the provider.

## Change summary

- `compile-time.slice-211.json` records six-cell samples/distributions, commands and hashes,
  source/toolchain identity checks, API lifecycle results and the exact inherited runtime ledger.
- This report, the completed bounded plan and STATUS record the next research gate and its metric.
- Ignored scripts, the API probe, generated PTX/cubins and full diagnostics remain under
  `build/nvvm-loop/slice-211-profile`. No production or shader files changed.

## Concepts and vocabulary

A **fresh process** invokes slangc independently and recreates its global session; warmups can warm
OS caches but cannot share that session. A **global session** owns builtins and downstream compiler
state. The probe always makes a new compile request, including semantic checking and IR generation,
while optionally retaining its global session. It introduces no cached material module.

An **inclusive timer** includes calls nested inside its scope. `SemanticChecking` and
`checkAllTranslationUnits` describe the same work; both lie within `frontEndExecute`. Downstream
compilation produces PTX and is separate from the standalone ptxas process that assembles it.
Neither operation executes a material kernel.

## Process report

### Reproducible identities and successful measurements

The tested source is clean base `0270fd7b555feb0447da1fb3b2df0bca4a92b604` on `nvvm-backend`.
Compiler library SHA256 is `f8dc709857e70fbf7e4c0bbe2d94f9c1528f789ddf609a4c0db6f5bb90b776c7`;
provider SHA256 is `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36.
The distinct slangc executable hash is recorded in the manifest. Every recorded 210 source and
artifact hash still matches. Host remains native Ubuntu/L4 SM89, driver 580.126.09, CUDA 12.9.2,
NVRTC/NVCC 12.9.86, LLVM 14, target SM80. Benchmarks ran sequentially without competing builds or
benchmarks. Only the ignored API probe was built, between measurement suites.

The primary command, after sourcing the inspected optimized environment helper, is:

```bash
source build/nvvm-loop/slice-203-env.sh
python3 issue-nvvm-backend/run-complex-corpus.py \
    --slangc build/RelWithDebInfo/bin/slangc --build-label RelWithDebInfo \
    --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so \
    --cuda-root /usr/local/cuda-12.9 --warmup 2 --samples 7 \
    --output build/nvvm-loop/slice-211-profile/complex
```

All 54 primary compile attempts and all six final assemblies passed. The exact six identities have
no duplicates or omissions. Each process includes `-report-perf-benchmark`. The runner's helper
uses `subprocess.run` with file output and a timeout; Python's POSIX wait loop polls up to every
50 ms, and the helper rounds to 1 ms. The repeated 1.617/1.667/1.717 second observations expose that
granularity. They are valid successful elapsed observations, but cannot resolve small differences.

The separate supplementary script uses PIPE/communicate plus `perf_counter` and adds
`-report-detailed-perf-benchmark -report-downstream-time`. It runs one warmup and three measured
compilations per cell, followed by two warmups and seven measured standalone ptxas runs. These
precise samples are the better evidence for small process differences, with the limitation of only
three samples and changed reporting/output capture. PTX hashes match the primary baseline exactly.

All values below are milliseconds, shown as median [minimum–maximum]. Downstream values have
10 ms reporting granularity and are nested within compiler work. Columns must not be summed as
an exclusive phase breakdown. Assembly is a separate process on a fixed PTX artifact.

| Entry / mode      | Primary process, n=7 | Supplement process, n=3 | Downstream, n=3 | Assembly process, n=7 |
| ----------------- | -------------------- | ----------------------- | --------------- | --------------------- |
| eval / NVRTC O3   | 1667 [1617–1717]     | 1620.2 [1612.2–1621.0]  | 130 [130–130]   | 176.5 [171.2–177.1]   |
| eval / NVVM O0    | 1617 [1617–1668]     | 1591.9 [1590.8–1595.3]  | 90 [90–90]      | 699.4 [690.8–704.5]   |
| eval / NVVM O3    | 1717 [1717–1718]     | 1688.5 [1670.4–1692.3]  | 190 [190–190]   | 208.6 [206.9–217.9]   |
| sample / NVRTC O3 | 1667 [1667–1668]     | 1655.2 [1640.7–1656.2]  | 140 [140–140]   | 255.0 [251.9–262.0]   |
| sample / NVVM O0  | 1667 [1617–1717]     | 1656.6 [1615.3–1663.6]  | 110 [100–130]   | 879.4 [876.1–885.5]   |
| sample / NVVM O3  | 1817 [1817–1869]     | 1790.9 [1783.0–1802.0]  | 270 [270–270]   | 296.7 [293.7–297.6]   |

The optimized versus unoptimized shader modes intentionally request different downstream work.
The O0 PTX taking longer to assemble does not establish a GPU performance difference or justify
silently substituting O3. No experiment changes those mode contracts.

### What the timers actually measure

`source/slangc/main.cpp::innerMain` creates a global session with GLSL enabled before invoking the
compile request. `slang_createGlobalSessionImpl` loads builtins through
`Session::loadBuiltinModule`. The primary builtin-loading medians are 205.25–210.74 ms. This is
session initialization, not per-material NVVM emission. Builtin AST/IR deserialization timers are
nested within it.

`FrontEndCompileRequest::executeActionsInner` parses and checks the complete translation unit,
then generates IR and layout. Its primary medians are 855.67–859.63 ms; enclosed semantic-checking
medians are 649.49–652.82 ms and enclosed IR-generation medians 185.00–186.12 ms. Both entries and
all three modes pay broadly similar shared frontend costs. The source remains valid and unchanged;
this experiment supplies no evidence of an accidental AST/IR representation requiring repair.

`linkAndOptimizeIR` specializes linked Slang IR for the selected target. Its medians are
321.01–342.94 ms. Baseline `specializeModule` medians are 138.89–145.00 ms, and `simplifyIR`
120.67–127.79 ms; they are inclusive shared-pass measurements, not backend-local diagnoses.
`PassHooksRAII` adds detailed timers around functions already using `SLANG_PROFILE`. In this build
those same-name timers accumulate together: detailed `specializeModule` has count two and about
twice the baseline duration, while `simplifyIR` has count ten and overlapping wrapper/function
costs. This is instrumentation overlap, not proof that the compiler performed double the work.
Do not rank those detailed totals as exclusive hotspots or add them to parent timers.

`CodeGenContext::emitNVVMForEntryPoints` runs common linking, then NVVM legalization, admission and
IR construction, then `emitWithDownstreamForEntryPoints`. The downstream counter wraps
`compiler->compile` in `slang-code-gen.cpp`. It distinguishes library compilation from the shared
frontend, but does not individually isolate direct NVVM legalizer/emitter/provider-building work.
`EndToEndCompileRequest::compile` clears the profiler when downstream reporting is requested, so
supplementary phase logs omit earlier builtin loading. The original baseline retains it.
Unmeasured residual process time must not be labeled backend cost.

`perf stat -e task-clock -- true` was denied at `perf_event_paranoid=4`. No tools were installed and
no host policy changed. Existing timers resolve broad responsibility; they do not identify a
specific wasteful semantic-checking or NVVM call site. This is why no compiler patch follows.

### Bounded session experiment and the one candidate

The ignored `session-probe.cpp` uses the same linked compiler library, command-line compile API,
`enableGLSL=true`, a fresh request each iteration and the exact eval/NVVM O3 material options.
One process recreates the global session each iteration; another retains it. Each policy has two
warmups and seven measured requests. All 18 final PTX outputs are byte-identical to baseline.
An initial probe used the API default GLSL setting; its logs remain separately named and are
excluded from comparable evidence after the setup mismatch was found and corrected.

| API lifecycle observation             | Fresh global per request | Shared global               |
| ------------------------------------- | ------------------------ | --------------------------- |
| Measured iteration median [range], ms | 1499.0 [1487.3–1508.2]   | 1320.7 [1315.2–1330.7]      |
| Compile call median [range], ms       | 1352.1 [1344.7–1366.5]   | 1313.2 [1307.7–1323.3]      |
| Global creation median, ms            | 111.4                    | 0 in measured iterations    |
| Global destruction median, ms         | 26.9                     | Deferred until process exit |
| Semantic-checking median, ms          | 617                      | 591                         |

The measured iteration median decreases 11.90%. Shared-session startup and final destruction are
outside steady-state measured iterations, so this is not an end-to-end finite-batch saving. First
global creation costs 225.9–233.9 ms, versus about 111.4 ms for repeated fresh globals in an already
warmed process. Different process/allocator/library histories affect these values; the 178.4 ms
iteration difference cannot all be attributed to builtin loading. The probe does not measure a
corpus-runner speedup. Reusing a global session still redoes material semantic checking and IR
production; no new module cache or compiler pipeline is implemented.

The next bounded gate for this candidate is an equivalent API experiment across all six cells,
with request-order permutations and bounded-batch lifetime/memory checks. Compare a fresh global
per request against one global per six-cell batch, retaining the independent CLI baseline. An
implementation should
be considered only if a serial six-cell batch, including startup and teardown, shows at least a
10% median lifecycle reduction with two warmup batches and at least seven measured batches,
without a greater than 5% per-cell compile-call median regression. Inputs, mode flags, PTX and
assembly outcomes must remain identical. A production runner would also need per-cell identity,
timeouts, crash isolation and oracle preservation plus a fresh-session lane; WORKFLOW requires a
full frozen/discovery checkpoint before adopting any runner change. This gate neither implements
nor authorizes bypassing those conditions.

Alternatives rejected here: attributing frontend time to NVVM, optimizing an unnamed semantic
checker hotspot, removing passes from inclusive timing totals, deriving an O3 speed claim from
PTX/register statistics, or inventing material runtime inputs. None has the required evidence.

### Preservation and self-review

New production helper/fallback/special-case inventory: empty. The only helper code is the ignored
measurement apparatus; no AST/IR/Val/witness representation or consumer behavior changed. All
recorded source/artifact hashes from accepted 210 match after measurements. There is no reason to
rerun full runtime corpora for this research-only slice.

Accepted 210 supplies inherited preservation evidence: 1647 runtime cells, 1594 correct and all
53 retained failures, including complete first-known history/reproduction/evidence, plus four
resolved 208 histories. Slice 211 has zero fresh runtime cells. The ledger and historical healthy
denominators are unchanged. Latest implementation and full checkpoint remain 210, cadence 0.
The new compile/assembly cells are fresh performance/support observations only.

Raw commands, samples, logs, scripts and artifact hashes are indexed by
`build/nvvm-loop/slice-211-profile/raw-index.json`; durable JSON contains its hash and all primary
sample values/distributions. The parent accepted the completed research. No compiler/provider/
runner/shader changes, GPU dispatch, driver change, reboot, commit or push occurred.

Parent accepted this research evidence after independent counts, medians, PTX and hash review.
Implementation and full checkpoint remain 210; implementation cadence remains 0.
