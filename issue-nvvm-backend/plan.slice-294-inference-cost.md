# Measure material generic-inference CPU cost

This bounded ExecPlan follows `.agent/PLANS.md` and the committed NVVM-plan exception. The authorized
loop remains active; skip Slack, no push/system changes. Root owns builds, runs, acceptance and
restoration; one fresh author owns temporary observer source and raw preparation. Separate review
must approve the method/source before build and the frozen experiment before timing. No compiler
optimization or observer source survives this research slice.

## Purpose and Observable Result

Determine whether failed bitwise-OR generic candidates consume meaningful semantic CPU time in one
unchanged material compile.286 found 1,314 such failures among 7,995 inferences, but frequency is not
cost and does not justify pruning. Report measured thread-CPU spans with observer-overhead limits,
or a bounded inconclusive result. The older aggregate-memory gap is already fixed by267.

## Progress

- [x] 2026-09-27: Full293 accepted/committed da796bb41; WORKFLOW/STATUS read. No active workloads.
- [x] Fresh author prepared bounded observer/parser; root audit and independent source/method review
      approved temporary build (source-review.json SHA5c66a445383d8d9d34b414b326198cf36bc40ed2113b0984cb6167efbf392ec9).
      Raw runner preparation/review remains separate from source approval.
- [x] Root built temporary observer after approved direct-pointer accessor repair;112.14s build-v2
      passed. observer-identity.json captures100layout/37runtime/3source and refreshed
      version2026.18.3-310-gda796bb41; provider unchanged. Untouched control identities remain separate.
- [x] Three modes/all6material artifacts preserved; native1110+1248IDs exact293. Reviewed
      relative-path guard amendment passed all6cells/48words with1/1/0native summaries.
      Independent qualification/timing review accepted; amended original failure remains retained.
- [x] Independent timing approval accepted freeze-v4; all8warmups+32measured passed in67.05s.
      Raw independent timing review passed; no sample exclusion or accounting-method repair.
- [x] Restore exact accepted source/layout/config; independent identity-after check passed.
- [x] Final independent review accepted research/restoration with no findings; final formatting
      and research-only commit close this slice. Review SHA149526eb687b2cc854d023a8dc30d5513e782012ff48719ca7a993e2b7e182e1.

Configuration passed with refreshed metadata. Build v1 failed after62.40s because the observer
accessed private m_shared. Existing getShared() returns that exact pointer without semantic work;
review approved the narrow access repair. Failed build/source freeze are retained.
Build-v2 and identity capture passed. Loader2 and pilot3 passed with exact293PTX/cubin/resources.
Actual accepted/experimentalcompiler hashes62469125/43596d05 loaded with identicalprovideraf1661de.
Count/timed pilots match7995inferences/810arity/1965success/5220null, OR1350/1314null,
75declarations/18sites and60nested calls. MappedORsource inventories equal286.
Independent pilot-review.json approved qualification. All6material cells passed20.25s with
exact293PTX/cubin/resources and valid7995inventory. Units passed153.03s with exact1110IDs (1097pass/13skip); semantics passed42.96s with
exact1248IDs (1170pass/78skip). Firstguard v3 command exited0 with no tests run; strict
identity validation rejected it and remaining5cells stayed unrun. That attempt produced no guard output.
options.cpp normalizes positional selectors with NoRoot while preserving absolute test-dir;
review approved using repository-relative test-dir/selector/expectedIDs. Versioned guard-only
repair/freeze amendment preserves all passed qualifications and the failed zero-execution attempt.
Guard-v4 passed20.76s; root qualification audit passed2358nativeIDs+6guards/48words.
Root read-only audit initially expected numeric0 instead of null for the failed no-summary log;
v2 explicitly checks null/no-tests marker/no result lines. No workload repeated.
Timing completed67.05s; all40passed. Experimental layout/source/config archived, accepted
layout restored, and independent identity-after check passed. Final documentation/restoration review accepted; commit-ready.
Runner review strengthened guards to six fresh complete buffers and per-cell180s execution.

## Surprises and Discoveries

Read-only proposal: build/nvvm-diagnostic-colors293/material-next-proposal.md, from reused trace_aggregate.
It corrected a stale lead:267 already removed the material constant-exponential/aggregate-memory gap.
Untouched accepted binaries cannot provide newly instrumented semantic-root CPU spans; mark those
unavailable, keeping existing wall-phase and external process CPU observations distinct.

## Decision Log

- 2026-09-27, root: Measure only eval_buffer/NVVM O3 initially.286 had identical inference inventories
  across both entries/backends; preserve this as historical context, not a new four-way cost claim.
- Three experimental modes share one binary: disabled (root clock envelope only), count-only
  (primitive records, no per-inference clocks), timed (same records plus clocks). Also use the untouched
  accepted binary. Disabled-vs-untouched includes binary/build/layout effects, not pure clock overhead.
- Temporary observation changes no semantic operations. Full293 corpus evidence stays inherited;
  all material outputs and native identities must qualify the observer before timing. Restore293's
  exact compiler285 bytes at closeout; no new compiler baseline or implementation cadence increment.
- At most one reviewed method repair is permitted for accounting or overhead; retain the initial
  attempt. Otherwise report inconclusive and restore. No expanding into a candidate-pruning prototype.

## Outcomes and Retrospective

Measured OR failure median14.368ms (14.093–14.632), root share3.593%; global7995-call
timed-minus-count-only median18.594ms (16.062–21.570). Stable instrumented attribution does
not establish precise uninstrumented/removable cost. Deprioritize OR pruning as a prioritization
decision, not a zero-cost conclusion. All40measurements passed; restoration100layout/37runtime/
11source/2config/576inputs/22pins exact. Method/accounting repair allowance unused.

Accepted293 is full baseline:580 cases/576 inputs/1740 cells,1703 correct/37 unresolved,
20 histories;1110 unit/1248 semantic identities, runtime4/toolkit18/material6,46 runner passes/1skip.
Lastfull293/targeted233/cadence0. Compiler source8fbf0f84e+patch12f503e9, unchanged by293.

## Context and Current Pipeline

ResolveInvoke and AddOverloadCandidates enumerate generic candidates. In slang-check-overload.cpp,
SemanticsVisitor::inferGenericArguments begins around2895, before ensureDecl; it checks arity,
obtains existing parameter/argument types, unifies and calls trySolveGenericArguments around3030.
addOverloadCandidatesForCallToGeneric later handles applicability/ranking. Inference success does not
mean a candidate was selected. FrontEndCompileRequest::checkAllTranslationUnits in
slang-compile-request.cpp owns the semantic root, including checkEntryPoints.

286 raw events and patch under build/nvvm-generic-inference286 explain lifetimes and canonical inputs.
Its temporary compilercc6b5823 is historical, not the current baseline. The observed inputs are valid
semantic data; this is observation at the owning phase, not a representation repair.

## Scope and Non-Goals

Temporary observer may touch source/slang/slang-check-impl.h, slang-check-overload.cpp and
slang-compile-request.cpp. Raw scripts/artifacts live under build/nvvm-inference-cost294. No retained
production source, workload/input/oracle, provider, shared runner or corpus changes. No cache, pruning,
new equivalence relation, repeated semantic getter, reconstructed syntax or speculative optimization.
No material GPU runtime claim; its binding/texture/LUT/input/output contracts remain unavailable.

## Architecture and Invariants

Use native Linux CLOCK_THREAD_CPUTIME_ID for semantic-root and inference entry/exit spans. Verify all
observed inference belongs to the root's owning thread before mutating collector state. Stop the
method on mixed threads rather than invent whole-process CPU attribution. Existing SLANG_PROFILE
wall durations stay wall durations. Untouched root CPU is unavailable.

Capture at most16,384 primitive inference rows per observed root, with stable lifetime/callsite/raw
canonical-reference identity, parent ID, existing return outcome and CPU timestamps when enabled.
Overflow, missing exit, unsupported scope or invalid nesting invalidates observation. Reuse286's
reviewed lifetime/return cleanup where suitable, without its95,295-row semantic event stream.
Read existing values only: no extra getArgTypeForInference, resolution, substitution, equality,
signature computation or semantic getters. Format names/locations and write output after the measured
root; if existing wall profiling includes dumping, label that distinction explicitly.

Measure inclusive spans and subtract nested inference spans to obtain exclusive costs. Root minus
sum of exclusive inference costs is residual; retain per-sample values/percentages and nesting proof.
Do not sum overlapping wall phases or label ensureDecl/first-use work as solver arithmetic. Group
bitwise-OR by recorded declaration/source identities after checking, not as a semantic equivalence key.

Count-only and timed modes must perform the same primitive capture/classification, differing only in
per-inference timing/accounting. Record external fresh-process wall/user/system CPU for every treatment;
experimental modes also record root thread CPU. Calibrate paired thread-clock read cost after the root
as an overhead diagnostic. Do not subtract a calibration median from every call and claim exact cost.

## Interfaces and Dependencies

Raw root build/nvvm-inference-cost294. Untouched control layout may reuse verified
build/nvvm-generic-inference286/accepted285-layout, byte/link/mode identical to installed293's compiler.
Capture original three source files and both CMake caches before any edit/configure; preserve accepted
layout100, runtime37, qualifiedsource11, config2, inputs576 and pins22. Never identify a build by HEAD
or cached version alone. Native Ubuntu24.04/L4SM89, driver580.126.09, CUDA12.9.2/NVRTC12.9.86, LLVM14.

Use the previously read slang-build skill with native commands, max4CPU:
`cmake --preset default -U SLANG_VERSION_FULL -U SLANG_VERSION_NUMERIC -DSLANG_EMBED_CORE_MODULE=OFF`
then `cmake --build --preset releaseWithDebugInfo --target slangc slang-test slang-unit-test
slang-numerics-modules --parallel 4`. This rebuilds restored286 files and matching serialized modules.
Retain configure/build logs and all temporary compiler/provider/module/library identities separately.
Provider ABI42/bytesaf1661de must remain unchanged unless the method is rejected and re-scoped.

Each process uses its own layout's bin/lib/module paths and explicit provider; verify actual loaded
compiler/provider identities for untouched and experimental pilots. Avoid accidentally loading the
experimental compiler into the untouched control through LD_LIBRARY_PATH. No simultaneous builds,
GPU/native suites or measurements. Native suites use2servers; shader cells180s; long gates1800s.

## Milestones

1. Preserve sources/config/layout, prepare observer and exact return/scope/input-shape audit. Root and
   independent reviewer approve source/method before build. Archive patch/source/runner versions.
2. Build through the skill, record actual identities, then freeze three pilots (experimental disabled,
   count-only, timed) for the original tests/cuda/complex/tiled_brass_material.slang, entry eval_buffer:
   `slangc SOURCE -entry eval_buffer -stage compute -target ptx -capability cuda_sm_8_0 -O3
-emit-cuda-via-nvvm -report-perf-benchmark -o UNIQUE.ptx`. Only layout, explicit observer mode/sink
   and output path differ. Compare each PTX/cubin/parsed resource set with293's same cell.
3. Reconcile counts:7,995 total;810 arity failure/1,965 solver success/5,220 solver-null;
   OR1,350 calls/1,314 solver-null. Any inventory difference stops for separate attribution. Check
   nonnegative nested/exclusive/root accounting, owner thread, row cap and complete scope closure.
4. Qualify all six registered material cells once in timed mode, preserving293 PTX/cubin/resources.
   Run native unit and semantic commands from RESULTS with observer disabled and compare all2358
   IDs/statuses to293. Six focused GPU cells (two existing267 material-reproducer guards at NVRTC O3,
   NVVM O0/O3) independently expect masked-constant[24,1,1,1,0,0,0,0] and
   masked-control[12,1,1,1,6,1,1,1]. Confirm exact original paths/directives before freezing commands;
   do not alter these oracles. Guards check267 preservation, not OR cost or material runtime.
5. Independent pilot/qualification review must accept before timing. Freeze four treatment environments,
   exact inputs/outputs and balanced order: two warmups per treatment, then eight rounds of all four,
   with a rotated order and its reverse so each treatment occupies each position twice. Exactly40
   timing compiles (8warmups+32measured), jobs1, no retries/outlier removal. Retain all per-process CPU,
   wall phases, rows/root clocks, output hashes and cache observations. Recheck output for every run.
6. Summarize OR-failure exclusive CPU milliseconds/share, other inference and residual per sample;
   paired timed-minus-count-only/disabled costs, medians/spread and calibration. Require repeatability
   above overhead/noise for a material cost lead; otherwise explicitly deprioritize or call inconclusive.
   No cost result establishes avoidable work, a safe cache key or a pruning rule.
7. Stop all owned processes; archive the experimental layout. Restore original source and both caches,
   exact accepted layout/links/modes and full identity maps. Restored source mtimes must be newer than
   experimental objects for the next build. Independent final review, compact research record,
   five-part report/completed plan/navigation, formatting and local commit of research only.

## Validation and Acceptance

All three pilots, six material cells, native identities and six small GPU cells must preserve the
accepted outputs before measurement. Count/accounting/thread/overflow checks are mandatory. Keep
warmups separate; retain every requested row and failure/unrun disposition. CPU observation remains
limited to the measured root/thread and unchanged eval_buffer/NVVM O3 workload. Compare paired samples;
old286 frequency and old timings are historical, not fresh controls. No shader/kernel speed claim.

A useful positive result is a measurable cost lead with explicit overhead. A useful negative result
bounds or deprioritizes the lead. Either requires restoration and independent review; failed, altered,
missing or overhead-dominated evidence cannot be presented as a performance opportunity.

## Failure and Recovery

On altered output, overflow, mixed threads, missing exit/negative exclusive time, changed inventory
or dominant overhead, stop dependent work and preserve every attempted/unrun cell. At most one reviewed
method repair; no silent replacement or retries. If the observer cannot be qualified, discard it and
close with evidence. Never overwrite a frozen source/runner/result; version amendments. Restore the
verified accepted layout/config/source before leaving the slice. User stop takes precedence.

## Artifacts and Hand-Off

Keep raw source/patches, binaries, events, samples, logs and exhaustive refs under ignored build.
Commit only report.slice-294-inference-cost.md, research-evidence.slice-294.json, completed plan and
navigation. Keep293 full baseline/compiler285 identity/cadence unchanged after restoration. Next
choice must follow the measured result: investigate a justified producer cost or choose another
language-surface gap; do not reopen267's closed aggregate-memory problem.
