# Measure complete six-cell session lifetimes

This ExecPlan follows `.agent/PLANS.md`. The NVVM loop explicitly commits completed plans and
reports; raw probes, binaries, generated PTX and logs stay ignored under `build/`.

## Purpose and Observable Result

Determine whether one global session per finite six-cell material batch reduces full API lifecycle
cost by at least 10% against one global per request, without a greater than 5% per-cell compile
median regression. Startup and teardown are inside every batch timer. This is research only.

## Progress

- [x] 2026-09-24: Read AGENTS, planning/workflow/status, 211 plan/report/measurements, 210 ledger,
      prior ignored probe and local build skill/environment. Parent grants worker checkout writes.
- [x] 2026-09-24: All 30 accepted source/artifact identities match; ignored probe prepared.
- [x] 2026-09-24: All 28 batches / 168 calls complete; exact PTX equals 210/211, six hashes assemble.
- [x] 2026-09-24: Recorded timing/order/memory verdict, five-part report and STATUS.
- [x] 2026-09-24: Formatting and identity/output/lifecycle checks complete; worker returns checkout
      write ownership to parent with handoff. Parent acceptance/local commit remain parent-owned.

## Surprises and Discoveries

Slice 211's 11.90% improvement excludes shared-session creation and final destruction. Its one-cell
result cannot establish finite-batch savings. Native steady-clock timing avoids timeout polling
quantization. GLSL must explicitly be enabled to match slangc.

Measured global creation/destruction savings dominate. Both policies have identical post-release
RSS boundary sequences; Linux process high-water and sampled RSS differ slightly (439432 versus
439656 KiB). Preserve both accounting observations rather than treating either as an exact
per-batch peak.

## Decision Log

- 2026-09-24, worker: predeclare two paired warmup batches and twelve paired measured batches.
  Twelve permits six forward rotations and six reverse rotations, each identity appearing twice
  at every position. Alternate fresh/shared policy order within each pair. Both policies use the
  same order per pair. No timing-based exclusion, retuning or early promotion.
- 2026-09-24, worker: run all pairs in one bounded process, with every global and request released
  before its batch ends. Process/allocator caches may persist; global sessions cannot. This
  isolates the specified API lifecycle, not process startup or a production-runner speedup.
- 2026-09-24, worker: unchanged binaries inherit full checkpoint 210 and its ledger, cadence 0.

## Outcomes and Retrospective

The candidate passes its bounded metric: 8848.207 to 7895.490 ms, 10.767% median batch
reduction. Every paired reduction exceeds 10%; every per-cell compile median improves. All
168 PTX outputs exactly match both baselines and six distinct hashes assemble. RSS grows 9000 KiB
at both policy boundaries then plateaus for the final three pairs; this is bounded evidence, not
a universal leak guarantee. No production change. Parent independently verified measurements, balanced sampling, lifecycle
boundaries, exact PTX and artifact hashes and accepted this research. Implementation/full checkpoint
remain 210, cadence 0.

## Context and Current Pipeline

The unchanged complex runner launches slangc for eval_buffer/sample_buffer at NVRTC O3 and NVVM
O0/O3. Each command creates builtins/global state, checks the material, generates and links IR,
then emits CUDA/NVRTC or NVVM/provider PTX. ptxas assembles separately. The probe uses
IGlobalSession::createCompileRequest, setCommandLineCompilerMode and processCommandLineArguments
with the exact accepted options, enableGLSL=true, and a fresh request for every cell.

## Scope and Non-Goals

Only ignored measurement apparatus and durable plan/report/JSON/STATUS may change. No production
compiler/provider/ABI/runner/shader changes, optimization implementation, module/IR caching,
material kernel dispatch, runtime claims, host changes, commits or pushes.

## Architecture and Invariants

Each batch contains exactly six identities. Fresh policy creates/releases six globals; shared
policy creates/releases exactly one global inside the timer. Requests never survive a cell.
Output artifacts and all source/options match 211 and 210 byte-for-byte; output filenames alone
vary. No canonicalization can hide PTX mismatches. Batch timing covers creation, setup, compilation,
diagnostic capture, request teardown, global teardown and endpoint cleanup identically.

## Interfaces and Dependencies

Native Ubuntu, optimized build/RelWithDebInfo, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, ABI 36,
L4 SM89 driver 580.126.09 and target SM80. Starting revision
ce8940d035e3fe036d7830d46d30f4f42a64e2a3. Compiler library SHA256
f8dc709857e70fbf7e4c0bbe2d94f9c1528f789ddf609a4c0db6f5bb90b776c7; provider
ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372.

## Milestones

1. Generate ignored batch-probe.cpp and run.py under build/nvvm-loop/slice-212-batch-lifetime;
   compile only that probe against the existing optimized library. Preserve accepted 211 raw data.
2. Source build/nvvm-loop/slice-203-env.sh; run the probe through run.py with a 900-second
   overall timeout and native 60-second per-cell alarm (global startup/teardown also bounded).
   Notify parent before/after the timed phase; no competing build/benchmark.
3. Validate 168 PTX outputs (24 warmup and 144 measured), exact expected entries/SM80 targets,
   byte equality to 211 and 210, and assemble at least one output for every distinct PTX hash.
4. Produce batch-lifetime.slice-212.json, report.slice-212-batch-lifetime.md and STATUS update.

## Validation and Acceptance

Compare medians of twelve total batch lifecycle samples per policy; accept the candidate metric
only at >=10% reduction and every per-cell compile median <=105% of fresh. Retain raw timings,
quartiles/ranges, paired differences, order dependence and policy-order effects. Record RSS before
and after each batch and cell, process high-water peak and post-warmup trend. RSS allocator caches
alone do not prove a leak. Check null request/global handles at endpoints; memory evidence remains
bounded, not a universal leak proof. Verify all accepted 210 source/artifact hashes before/after.
Exactly 1647 runtime cells (1594 correct, 53 open failures plus four resolved histories) are inherited
from 210; zero fresh runtime cells. No full suite for unchanged source/binaries. git diff --check.

## Failure and Recovery

Missing/mismatched output or nonzero exit invalidates the observation; diagnose without discarding
costs. A timeout kills the probe and remains incomplete. Keep partial evidence, use a new path for
reruns and disclose all exclusions. Do not change host perf policy or invent material inputs.

## Artifacts and Hand-Off

Raw root build/nvvm-loop/slice-212-batch-lifetime; durable four files above. A passing metric allows
only a concrete bounded implementation proposal with per-cell timeout/crash isolation/oracles and
fresh-session coverage, followed by the mandatory full checkpoint. Parent owns acceptance/commit;
worker explicitly returns write ownership after completing a compact handoff.
