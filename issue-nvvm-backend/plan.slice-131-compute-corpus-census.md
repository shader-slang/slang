# Establish the direct-NVVM compute census and Pareto baseline

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM backend has a reproducible, bounded coverage denominator over
the repository's CUDA compute-runtime corpus. Every eligible test is classified at O0 and O3 as a
correct runtime result, runtime mismatch, compiler-side preflight rejection, provider/libNVVM
failure, or infrastructure/toolchain failure. Failures are grouped by the first canonical IR shape
and its producer, with a Pareto table that identifies the reusable changes likely to unlock the
largest and most important groups.

This slice adds measurement infrastructure and durable status only. It does not widen the backend,
promote fixtures, add expected-failure entries, or revise builder ABI 24.

## Progress

- [x] (2026-08-30) Completed and committed Slice 130 as `a5e1ad5f9`; Release host/provider builds,
  promoted runtime/PTX lanes, assembled PTX, and the complete 399/399 selected NVVM regression
  prefix pass.
- [x] (2026-08-30) Defined and documented the CUDA-candidate universe, explicit MVP
  eligibility/exclusion rules, and representative-workload selection criteria.
- [x] (2026-08-30) Added a repeatable census runner that derives direct O0/O3 lanes from active CUDA
  compute directives in an ignored mirror, preserves the source and test-input contract, captures
  raw evidence, and never edits the canonical test corpus.
- [x] (2026-08-30) Ran the complete eligible census, audited every failure classification and first
  canonical producer/diagnostic, and published per-test plus Pareto results with explicit
  denominators.
- [x] (2026-08-30) Recorded timing/PTX/runtime/toolkit/architecture measurements and honest gaps,
  defined the bounded usable-compute MVP and three representative release-gate workloads, and
  ranked the next root-cause slices.
- [x] (2026-08-30) Validated script parsing, discovery, table regeneration, and exact result
  reconciliation; reran the selected 399/399 regression, passed staged `git diff --check`, and
  completed the measurement-only self-review.

## Surprises and Discoveries

- The active CUDA runtime-directive universe is much larger than `tests/compute`: it spans
  compute, CUDA, HLSL intrinsics, language features, autodiff, cooperative matrix, neural, and
  several smaller directories. The denominator must therefore distinguish all CUDA candidates
  from the conventional-compute MVP subset rather than silently treating one directory as the
  whole backend corpus.
- The 399/399 `slang-unit-test-tool/nvvm` prefix is selected positive and adjacent-negative
  regression coverage. It supplies no denominator for the broader CUDA corpus and will be reported
  separately from census coverage.
- The first runner probe falsely passed because `slang-test` exits successfully when a prefix
  matches no tests. The classifier now treats `no tests run` as infrastructure failure and passes
  the exact repository-relative generated filename, including its extension.
- Renaming a generated source disconnects source-indexed `.expected.txt` sidecars. The runner now
  records each original test ordinal and copies the exact sidecar to the generated workload. The
  corrected native reference rises from a misleading 234/451 to 448/451.
- Direct O0 and O3 have the same 244 E52017 preflight failures, but their successful sets differ:
  four unoptimized Half workloads fail libNVVM only at O0, while eight narrow integer-conversion
  workloads are correct at O0 and mismatch only at O3.
- CUDA 12.9 NVRTC emits SM75 PTX for the representative sources on the physical SM120 RTX 5090.
  Direct PTX remains explicitly targeted and assembles for SM70, SM80, and SM90. The installed CUDA
  13 directory contains no binaries, so CUDA 13 is an infrastructure gap rather than backend data.

## Decision Log

- Decision: make Slice 131 measurement-only.
  Rationale: changing the backend while discovering the denominator would make the baseline move
  during measurement and bias prioritization toward whichever fixture was inspected first.
  Date/author: 2026-08-30, Codex.
- Decision: derive census lanes from each test's active native-CUDA `COMPARE_COMPUTE` directive in
  a generated mirror under `build/`.
  Rationale: the directive already owns entry-point selection, specialization, compiler arguments,
  and runtime input/output bindings. Reusing it preserves the workload contract; injecting only
  direct-NVVM selection, architecture, and optimization avoids source reconstruction and leaves the
  checked-in corpus untouched.
  Date/author: 2026-08-30, Codex.
- Decision: report a candidate-universe denominator and a narrower usable-compute MVP denominator.
  Rationale: advanced autodiff, cooperative-matrix/FP8, neural, ray-tracing/OptiX, RDC/device-LTO,
  dynamic-parallelism, device-syscall, and debugging workloads remain valuable future evidence but
  are explicitly outside the initial MVP unless a chosen representative application requires one.
  Date/author: 2026-08-30, Codex.
- Decision: retain advanced wave/quad and device-clock workloads in the classified corpus but label
  them `extension` rather than remove their evidence.
  Rationale: the complete eligible denominator remains 451 while the explicitly bounded MVP is
  429. This keeps long-term evidence visible without allowing out-of-scope operations to control
  initial product readiness.
  Date/author: 2026-08-30, Codex.
- Decision: select ordinary intrinsic semantics, helper ABI types, and aggregate/pointer/layout
  transport as the next three clusters.
  Rationale: they block 62, 51, and 39 MVP workloads respectively, account for 64.1% of current MVP
  failures, and each names a reusable compiler-owned representation/semantic boundary. Common wave
  semantics remain fourth because 14 neighboring cases are explicitly extension-tier.
  Date/author: 2026-08-30, Codex.
- Decision: define the measurable MVP gate as at least 80% differential correctness at both O0 and
  O3 over healthy native MVP references, no unexplained provider/runtime failure in the supported
  subset, all three representative gates, and the packaging/toolkit/architecture matrix.
  Rationale: feature prose alone cannot show whether combinations work, while requiring 100% of
  the broad suite would silently pull excluded research families into the initial product.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The census produces 683 candidate sources, 447 eligible sources, 451 eligible workloads, 429 MVP
workloads, and 22 classified extension workloads. Native CUDA is correct for 448/451. Direct O0 is
correct for 196, mismatches seven, stops 244 in preflight, and fails four in libNVVM. Direct O3 is
correct for 192, mismatches 15, and stops the same 244 in preflight. Healthy-reference differential
correctness is 195/448 at O0 and 190/448 at O3; 187 workloads are correct through all three routes
at both direct optimization levels.

The committed report and 451-row TSV make every numerator, first shape, producer/owner, and
diagnostic auditable. No compiler, provider, builder ABI, or checked-in fixture was changed. Three
representative multi-feature workloads pass native/direct O0/O3; their direct O3 PTX assembles for
SM70/SM80/SM90. Standalone direct O3 compile medians are 251--268 ms versus 373--382 ms for NVRTC
O3 on this host, but kernel-only timing and CUDA 13 remain explicit productionization gaps.

## Context and Current Pipeline

An active CUDA `COMPARE_COMPUTE` directive proves that the repository harness can compile and run
the source with a concrete CUDA entry point, specialization contract, resource bindings, and
expected output. The direct route is selected by appending `-Xslang -emit-cuda-via-nvvm` plus an
explicit CUDA capability. The direct compiler performs specialization, legalization, optimization,
canonical NVVM preflight, generic builder emission, LLVM verification/serialization, libNVVM
compilation, and CUDA execution.

Diagnostics E52016 identify provider discovery/ABI setup, E52017 identifies compiler-owned
preflight shapes before provider mutation, and E52018 or libNVVM diagnostics identify provider
verification/compilation. The runner must preserve complete diagnostics so classifications can be
audited instead of inferred from exit code alone.

## Scope and Non-Goals

In scope are corpus discovery; explicit eligibility/exclusion reasons; native-CUDA, direct O0, and
direct O3 outcomes; stable raw logs and machine-readable results under ignored `build/`; committed
per-test and Pareto summaries; first canonical IR shape, exact producer, and diagnostic for every
failure cluster; selected timing, PTX-size, runtime, CUDA-toolkit, and SM70/SM80/SM90 measurements;
representative workload selection; MVP definition; and ranked next slices.

Out of scope are compiler/provider semantic changes, fixture promotion, fixture-name checks,
expected-failure lists, syntax reconstruction, compatibility fallbacks, permanent generated test
copies, ABI revision 25, OptiX, RDC/device LTO, dynamic parallelism, device syscalls, FP8, advanced
wave operations, and source-level debugging.

## Architecture and Invariants

- Eligibility and exclusions are mechanical and reviewable from checked-in active test directives
  plus documented MVP-family rules; passing tests are not selected by prior knowledge.
- The generated mirror preserves source text and all `TEST_INPUT`/type/specialization metadata. It
  replaces only execution directives and is disposable.
- Every direct result records O0 and O3 independently. Native CUDA is the reference contract, not a
  fallback path for a failed direct compilation.
- Classification follows phase evidence: result match, result mismatch, E52017 preflight,
  E52018/E52019 provider/libNVVM, or setup/toolchain. Unknown failures stay explicitly unclassified
  until inspected.
- Pareto clusters use canonical IR operation/type plus producer and owning compiler layer. Fixture
  names are examples, never semantic keys.
- The selected 399/399 regression remains a separate regression score and is never divided by the
  census denominator.

## Interfaces and Dependencies

The census runner is a development tool under `issue-nvvm-backend/`; generated mirrors, logs, PTX,
and tables live below ignored `build/nvvm-census/`. Durable architecture and summary metrics live in
`docs/design/nvvm-backend.md` and the capability ledger. The machine uses the isolated LLVM 14
provider at `build/nvvm-builder-deps/slang-llvm-nvvm-build/Release`, CUDA 12.9 tooling, and the
Release `slang-test`/`slangc` binaries.

## Milestones

1. Enumerate active CUDA compute-runtime directives across `tests/`, document the candidate universe
   and exclusions, and prove the generated-mirror method on one passing and one failing source.
2. Run native CUDA, direct O0, and direct O3 over every eligible test. Capture machine-readable
   status, wall time, raw logs, and environment metadata without changing backend code.
3. Inspect final linked IR for one representative of each failure signature, trace the exact
   producer and owner, merge equivalent signatures, and create the Pareto table and per-test matrix.
4. Measure PTX size and compiler/runtime comparison where the harness contract permits it; report
   missing instrumentation explicitly rather than fabricating values. Validate SM70 compilation
   broadly and sample SM80/SM90/toolkit coverage according to locally available hardware/tooling.
5. Define the usable-compute MVP, select two or three multi-feature release-gate workloads, rank the
   next two or three reusable root-cause slices, validate documents and tooling, rerun regression,
   and commit.

## Validation and Acceptance

Acceptance requires a deterministic re-run to reproduce the candidate and eligible denominators;
one classified record for every eligible O0/O3 lane; totals that reconcile exactly with the
per-test matrix; complete raw evidence for each failure; producer traces for every Pareto cluster;
separate native-reference and selected-regression results; honest environment/toolkit/GPU fields;
no source-corpus or backend changes; the complete 399/399 regression prefix; `git diff --check`;
and no staged generated artifacts or `external/slang-binaries/` content.

## Failure and Recovery

If the generated mirror changes module/resource lookup, compare it with the original native CUDA
lane and repair the runner's preservation of test context; classify the case as infrastructure only
after reproducing the same failure independently of direct NVVM. If a diagnostic lacks enough final
IR detail to identify a producer, use an ignored one-test `slangc`/harness probe and record the
named producing pass/function. Do not add backend code to make census collection easier.

## Self-Review

The change inventory contains only measurement/reporting code and documents. Discovery uses an
active CUDA compare-directive grammar; exclusions use documented directory/family rules; the
extension tier uses explicit path families; no rule examines whether direct NVVM passed. Native,
O0, and O3 directives preserve the original command and inputs while changing only direct selection
and optimization. Direct-only fixtures derive a native lane by removing that selector and remain
marked in the matrix.

Two runner bugs were caught by negative probes. `no tests run` can no longer become `correct`, and
renamed expected-output fixtures copy the original ordinal's sidecar. Classification is phase
ordered: E52017 wins before result mismatches, provider/libNVVM diagnostics win before harness
output, generated discovery/provider/toolchain signatures are infrastructure, and FileCheck or
expected/actual output differences are runtime mismatches. The final matrix has no unclassified
row, and every count reconciles to 451 per mode.

Producer mapping is based on named compiler creation and validation functions, not fixture names.
The `GenericAsm` trace was confirmed with an optimized `frexp` final-IR dump; other rows name their
canonical linked IR operation/role and owning validator. Path classification divides only planning
families after the canonical phase/shape is known. The scripts add no AST/IR equivalence, syntax
reconstruction, emitter guard, compatibility fallback, source special case, or backend mutation.

## Artifacts and Hand-Off

Keep generated mirrors, raw logs, machine-readable run data, LLVM/final-IR probes, PTX, and timing
samples below ignored `build/nvvm-census/`. Commit the reusable runner, census/Pareto report, durable
MVP/status updates, and this completed plan. The report must name the next two or three root-cause
clusters and the coverage numerator each is expected to unlock.
