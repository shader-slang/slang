# Slice 212: include complete material batch lifetimes

## Motivation

Slice 211 measured an 11.90% steady-state improvement when requests reused a global session,
but excluded that session's startup and final destruction. That does not establish savings for
a finite workload. This gate measures complete six-cell batches of the unchanged tiled-brass
material: `eval_buffer` and `sample_buffer`, each at NVRTC O3 and NVVM O0/O3. All six cells already
compile and assemble; no material bindings, texture/LUT inputs or expected runtime outputs exist.

## Proposed solution

The bounded API experiment meets its predeclared metric. Reusing one global session within a
six-cell batch reduces the median full batch lifecycle from **8848.207 ms to 7895.490 ms**, a
**10.767% reduction**, including global startup and teardown. Every per-cell compile median
improves by 1.694% to 2.567%; none violates the maximum 5% regression condition. All 168 PTX
outputs exactly match both accepted baselines, and all six distinct hashes assemble successfully.
This supports a bounded harness implementation proposal, not a compiler optimization or an
already-achieved production-runner speedup. No production code changes in this research gate.

One concrete next slice could add an opt-in, maximum-six-cell batch to the complex corpus runner,
backed by a supervised API worker. Each batch process would own one global session and six fresh
requests, then exit; the existing fresh-process lane would remain mandatory. The supervisor would
retain cell identities and exact options, enforce per-cell deadlines, and isolate a crash to one
bounded batch. An active failed cell stays failed; an unexecuted suffix stays incomplete and is
never counted as a pass. Each artifact still receives the existing target/entry/hash/assembly
checks. Any recovery attempt must remain separately recorded, preserving the initial failure.
Use explicit per-cell timer deltas; global profiler accumulation must not become per-cell data.

That implementation must test timeout/crash handling, order independence, bounded memory and
fresh-session behavior; include process startup/exit in its own performance comparison; and pass
the full frozen/discovery checkpoint plus all six complex cells before adoption. The parent selects
the next slice. The ignored research probe is measurement apparatus, not production code to copy.

## Change summary

- `batch-lifetime.slice-212.json` retains every batch/cell timing, identity, output hash, lifecycle
  count, RSS checkpoint, assembly result, order analysis and inherited runtime ledger reference.
- This five-part report, the completed plan and STATUS record the result and bounded next proposal.
- Ignored `batch-probe.cpp`, generated option header, supervisor/analyzer, native executable,
  diagnostics, PTX and cubins remain under `build/nvvm-loop/slice-212-batch-lifetime`.

## Concepts and vocabulary

A **batch** is exactly six compile requests and all of their global-session lifecycle costs.
The **fresh policy** creates and destroys a global for each request. The **shared policy** creates
one global for the batch and destroys it before the batch timer stops. Both policies create a new
compile request for every cell; neither caches material modules or reuses generated IR.

A **paired sample** gives both policies the same six-cell order. A **process high-water mark** is
cumulative peak resident memory since process startup, not an isolated per-batch peak. Resident
pages may remain in allocators or downstream libraries after API objects are released.

## Process report

### Fixed identities and complete boundaries

Starting checkout revision is `ce8940d035e3fe036d7830d46d30f4f42a64e2a3`, branch `nvvm-backend`.
The compiler library remains SHA256 `f8dc709857e70fbf7e4c0bbe2d94f9c1528f789ddf609a4c0db6f5bb90b776c7`;
provider remains `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36.
All 30 recorded 210 source/artifact hashes match before and after. The source, runner and exact
material options match the accepted 211 records. `ldd` confirms the ignored probe resolves the
optimized compiler library. Native Ubuntu, L4 SM89, driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86,
LLVM 14 and target SM80 remain unchanged. No competing builds or benchmarks ran during timing;
ordinary host services remained active. No host policy, driver or toolchain change occurred.

Consider this API ownership pattern for the shared policy:

```cpp
startBatchTimer();
createGlobalSession(/* enableGLSL = true */);
for (auto cell : sixMaterialCells)
{
    createCompileRequest();
    setCommandLineCompilerMode();
    processCommandLineArguments(cell.exactAcceptedOptions);
    compile();
    captureDiagnostics();
    releaseCompileRequest();
}
releaseGlobalSession();
stopBatchTimer();
```

The fresh policy moves global creation/release inside the loop, with identical instrumentation.
The probe passes the accepted CLI argument arrays through `ICompileRequest`, changes only the
output filenames, adds the optimized binary search path and explicitly enables GLSL. Request
creation/setup, compile calls, request destruction and global destruction receive native
`std::chrono::steady_clock` timestamps. The enclosing timer also covers diagnostic capture,
cell RSS sampling, temporary argument cleanup and the final global handle's scope exit. Batch
JSON serialization and pre/post-batch RSS reads occur outside it. No profiler object retains the
request or global. Counters verify 168 requests and 98 global sessions are created and released;
all handles are empty at their intended endpoints. These counters establish explicit API handle
releases, not destruction of every internal object.

`IGlobalSession::createCompileRequest` produces independent requests. Command-line argument
processing selects the unchanged source/entry/target/backend/optimization. `compile()` still
runs material semantic checking, IR generation and target compilation for every request. Only
global ownership changes in the probe. This is intentionally valid API input, not an accidental
AST/IR/Val/witness spelling, and no producer or compiler consumer is patched.

### Balanced sampling and timing evidence

Before running, the plan selected two paired warmups and twelve measured pairs. Each measured
policy sees six forward rotations and six reverse rotations: every identity appears exactly twice
at every position. The policy running first alternates each pair. One serial probe process runs
all 28 batches, retaining process/allocator caches but never a global session across batches.
A native 60-second alarm bounds each cell and final teardown; a PIPE/communicate supervisor
bounds the process at 900 seconds. The completed process exits 0 after 235.358 seconds, with no
failures, timeouts, missing cells or exclusions. Process loading and final process exit are outside
the API batch metric; they are included only in this separate process-duration observation.

Reproduction, from repository root using the ignored retained apparatus:

```bash
source build/nvvm-loop/slice-203-env.sh
python3 build/nvvm-loop/slice-212-batch-lifetime/run.py
python3 build/nvvm-loop/slice-212-batch-lifetime/analyze.py
```

Use a new raw directory on a real rerun; these commands identify the retained experiment and must
not overwrite its accepted data. The local build skill was read; no compiler build was needed.
Only the probe was built, using `g++ -O2 -std=c++17 -Iinclude`, the existing RelWithDebInfo library
and an explicit runtime library path. Its deprecated compile-request API matches the prior probe.

| Full batch lifecycle, ms | Fresh global per request | Shared global per batch |
| ------------------------ | ------------------------ | ----------------------- |
| Median                   | 8848.207                 | 7895.490                |
| Interquartile range      | 8806.455–8874.651        | 7882.390–7902.497       |
| Minimum–maximum          | 8796.371–8943.641        | 7867.261–7920.069       |

The ratio of policy medians improves 10.767%. The median paired improvement is 10.658%, with
range 10.322%–11.714%; all twelve pairs exceed 10%. Fresh-first and shared-first median paired
improvements are 10.658% and 10.703%. Forward and reverse median paired improvements are 10.799%
and 10.604%. The finite sample shows no order-specific failure, while leaving cross-host/build
and long-running behavior unproved. The margin over the threshold is modest; broader conclusions
or removing the fresh lane would exceed this evidence.

| Cell compile-call median, ms | Fresh    | Shared   | Improvement |
| ---------------------------- | -------- | -------- | ----------- |
| eval / NVRTC O3              | 1238.352 | 1215.407 | 1.853%      |
| eval / NVVM O0               | 1245.500 | 1213.529 | 2.567%      |
| eval / NVVM O3               | 1342.084 | 1319.346 | 1.694%      |
| sample / NVRTC O3            | 1262.184 | 1238.467 | 1.879%      |
| sample / NVVM O0             | 1282.318 | 1250.229 | 2.502%      |
| sample / NVVM O3             | 1442.831 | 1417.746 | 1.739%      |

Median summed global creation decreases from 778.629 to 129.626 ms; median summed destruction
from 183.269 to 32.359 ms. Median summed compile time decreases from 7827.140 to 7678.255 ms.
These independently calculated medians need not sum to the median total. Creation/destruction
savings dominate; the smaller compile difference can reflect global/cache history and does not
prove material semantic checking was skipped or identify a backend-local redundant pass.
No nested phase timers are added or interpreted as exclusive costs.

### Exact outputs, memory and preservation

All 168 output files are byte-for-byte equal to both 210 and 211 artifacts for their exact identity,
with expected entry and actual `.target sm_80`. No canonicalization or text normalization is used.
Six distinct PTX hashes cover all six identities; one artifact per hash was freshly assembled with
CUDA 12.9 ptxas at SM80, all exit 0. Assembly is separate from the batch timing and no kernels run.

The process high-water RSS is 439432 KiB; the maximum sampled `/proc/self/statm` RSS is 439656 KiB.
These Linux accounting interfaces differ slightly and are reported separately. At post-warmup batch
boundaries both policies show the same sequence: 320216 KiB initially, 329216 KiB finally, a
9000 KiB increase (linear descriptive slope 921.259 KiB per measured pair). The last three pairs
remain at 329216 KiB. Every request/global handle is explicitly released. Allocator/library cache
retention is consistent with these observations; they do not identify a leak, establish that all
memory is reclaimed, or prove unbounded operation safe. No allocator trimming hides retained pages.
A future worker exiting after six requests would bound process-held state; that protocol remains
unimplemented and its actual timing still needs measurement.

The production helper/fallback/special-case inventory is empty. No AST/IR/Val/witness change or
consumer-side repair exists to audit. No rejection was timed as a successful compile. No metric
was retuned, costly startup omitted, or failed observation excluded to meet the threshold.

Full checkpoint 210 remains the inherited preservation source: 1647 runtime cells, 1594 correct,
53 open failures and four resolved histories. Their full identity/mode, first-known evidence and
reproductions remain in `runtime-validation.slice-210.json`. Slice 212 executes zero fresh runtime
cells and does not alter the ledger or healthy denominators. Latest implementation/full checkpoint
remain 210, cadence 0. A full runtime rerun is unnecessary for unchanged compiler/runner binaries;
a production runner change would require one. The raw index hashes all retained apparatus, logs and artifacts; the durable JSON links that index
and the unchanged source/binary checks. Parent independently verified the measurements, balanced positions, lifecycle boundaries/counters,
168 PTX hashes and referenced artifacts and accepted the research. The parent selected FP64 implicit
aggregate-shuffle admission as the next bounded compiler slice; production batching remains a
measured candidate with the acceptance obligations above. No commit or push was made by the worker.
