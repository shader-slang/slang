# Establish a successful material compilation baseline

This ExecPlan follows `.agent/PLANS.md`. The NVVM loop explicitly commits completed plans and
reports; raw profiling scripts, binaries and logs remain ignored under `build/`.

## Purpose and Observable Result

Measure unchanged tiled-brass `eval_buffer` and `sample_buffer` compilation in NVRTC O3 and
NVVM O0/O3 with the accepted optimized compiler. Produce repeated successful process timings,
phase distributions, distinct assembly evidence, and one justified next candidate or an explicit
finding that backend-local optimization is not justified. This gate implements no optimization.

## Progress

- [x] 2026-09-24: Read repository/workflow/build guidance, slice 210 report and manifest, runner,
      material manifest and environment helper. Confirmed native Linux and initially clean checkout.
- [x] 2026-09-24: Verified all accepted 210 recorded source/artifact hashes and host/toolkit identity.
- [x] 2026-09-24: Completed 54/54 primary compiles and 6/6 assemblies, sequentially.
- [x] 2026-09-24: Completed 24 supplementary compiles, 54 assembly timings and 18 final API
      compiles. Traced inclusive timer boundaries and overlap; perf access denied without host changes.
- [x] 2026-09-24: Wrote measurement manifest, five-part report and STATUS; worker complete,
      accepted by the parent after ownership return.

## Surprises and Discoveries

The existing runner enables `-report-perf-benchmark` for every attempt, times the complete slangc
process, and assembles only the last PTX once per cell. Assembly is therefore initially a support
observation, not a repeated timing baseline. Timers may nest and cannot be summed.

Primary process observations show Python timeout-wait polling at approximately 50 ms granularity.
A separate PIPE/communicate supplement provides precise wall time with changed reporting flags.
Detailed wrappers add overlapping same-name timings to `specializeModule` and `simplifyIR`;
`-report-downstream-time` clears profiler state before compile, excluding builtin setup.
An initial API probe used default GLSL setup; it is excluded, with a corrected enableGLSL=true
probe retained. Both fresh/shared corrected outputs exactly match baseline PTX. Perf is restricted.

## Decision Log

- 2026-09-24, worker: use the unchanged runner for the six-cell baseline; retain exact commands,
  hashes and raw samples. Do not change source, provider, runner, shader or ABI.
- 2026-09-24, worker: preserve accepted full checkpoint 210, implementation cadence 0 and its
  complete failure ledger. This research-only slice needs no full runtime rerun on identical tools.

- 2026-09-24, worker: do not force a compiler patch. Session lifetime is the measured candidate;
  require a further six-cell lifecycle proof and preservation contracts before adopting it.

## Outcomes and Retrospective

All six cells pass with exact unchanged PTX identities. Primary process medians span 1.617–1.817 s;
more precise supplementary process medians span 1.592–1.791 s. Shared semantic checking remains
about 650 ms, and builtin load about 205–211 ms. No backend-local optimization is justified.
The one candidate is bounded global-session amortization at harness scope: final eval/NVVM O3 API
probe steady-state lifecycle median decreases 11.90% (1499.0 to 1320.7 ms), with identical PTX.
This excludes shared startup/final destruction and is not a corpus-runner or finite-batch speedup.
Before implementation, measure all six cells with request-order and lifecycle checks, including
startup/teardown, targeting >=10% batch reduction and <=5% per-cell compile-call regression.
Latest implementation/full checkpoint remain 210, cadence 0; all 53 failure histories remain
inherited from the unchanged accepted ledger. No runtime material contract is supplied.

## Context and Current Pipeline

`run-complex-corpus.py` invokes slangc independently for each unchanged entry and mode. Session
builtin loading precedes material semantic checking and IR lowering. Shared linking/optimization
feeds either CUDA source/NVRTC or direct NVVM/provider compilation; ptxas validates resulting PTX
separately. Resolve exact timer scopes in source before assigning costs to these stages.

## Scope and Non-Goals

Only documentation, a compact measurement JSON and ignored probes/scripts may change. No production
compiler/provider/ABI/runner edits, shader edits, optimization implementation, host setup changes,
GPU dispatch, material runtime assumptions or GPU performance claims. Do not start another feature.

## Architecture and Invariants

Keep canonical input source and all requested `(entry, backend, optimization)` identities intact.
Successful compilation requires fresh target-SM80 PTX with the expected entry; assembly must succeed.
Do not count rejection latency. Report nested timers independently. Fresh processes may benefit from
OS file cache warmups but do not reuse global or module sessions. Keep that distinction explicit.

## Interfaces and Dependencies

Native Ubuntu, RelWithDebInfo `build/RelWithDebInfo/{bin,lib}`, LLVM 14 provider ABI 36,
CUDA 12.9.2, target SM80. Source inspected `build/nvvm-loop/slice-203-env.sh` (overrides old Debug
builder selection). Verify source base `0270fd7b555feb0447da1fb3b2df0bca4a92b604`, compiler-library SHA256
`f8dc709857e70fbf7e4c0bbe2d94f9c1528f789ddf609a4c0db6f5bb90b776c7` and provider SHA256
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.

## Milestones

1. Capture identity, tool paths, accepted ledger reference and concurrency evidence under
   `build/nvvm-loop/slice-211-profile`.
2. Run `python3 issue-nvvm-backend/run-complex-corpus.py --slangc
build/RelWithDebInfo/bin/slangc --build-label RelWithDebInfo --provider
build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root /usr/local/cuda-12.9 --warmup 2
--samples 7 --output build/nvvm-loop/slice-211-profile/complex` after sourcing the helper.
3. Inspect timing scopes and available profiler. If needed, use bounded ignored session/profiling
   probes on fixed material input to distinguish startup/shared frontend/downstream costs.
4. Retain medians, min/max and quartiles, per-cell outcomes and evidence hashes in
   `compile-time.slice-211.json`; explain candidate, responsible layer and acceptance metric in
   `report.slice-211-material-compile-profile.md`; update STATUS without changing checkpoint cadence.

## Validation and Acceptance

Require exactly six unique cells, 54 successful compile attempts (12 warmups, 42 measured), and
six assemblies. Ensure no overlapping build/benchmark and matching inputs/toolchain. Record all
phase samples and boundaries, with no sum of nested timers. Supplementary probes must remain
separate from baseline and disclose timing granularity/limits. Full 210 results are inherited
preservation evidence (1647 runtime cells, 1594 correct, 53 retained failures), never fresh 211 tests.
Compare identities/hashes and run `git diff --check`; no compiler test rerun for unchanged sources.

## Failure and Recovery

Timeout or missing tool is an incomplete observation. Stop a failed cell's repeated timings.
Do not install profiling tools or alter host policy; record restrictions and use existing timers.
Use new evidence directories on reruns. No GPU dispatch is planned; stop on any device-loss signal.
Discard probes without promoting architecture if findings do not justify an optimization.

## Artifacts and Hand-Off

Raw root `build/nvvm-loop/slice-211-profile`. Durable plan/report/measurements/STATUS only. Parent
owns independent acceptance and local commit; worker returns explicit checkout ownership without
committing or pushing. Next implementation must be selected from measured evidence, not forced.

Parent acceptance: research 211 is accepted after independent measurement, probe and hash review.
Latest implementation/full checkpoint remains 210 with cadence 0. The next research gate includes
global-session creation/destruction inside each six-cell batch measurement.
