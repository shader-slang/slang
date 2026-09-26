# Locate one material semantic-checking hot path

This bounded research ExecPlan follows `.agent/PLANS.md` and the NVVM committed-plan exception.
The user-authorized development loop remains active. Root owns scope, records and local commits;
one fresh worker owns debugger execution and ignored experiment files, and an independent reviewer
checks method, source trace and evidence. Slack remains skipped; no push or host-setting changes.

## Purpose and Observable Result

Research272 measures semantic checking at385–397ms per fixed material compile, substantially larger
than target preparation/emission. Identify a repeatable responsible call path on accepted270 and,
if the evidence permits, a small source reduction that preserves its semantic work. Propose one
separately reviewed optimization gate. Do not implement an optimization or alter diagnostics here.
A truthful bounded negative result is acceptable if no stable actionable path can be established.

## Progress

- [x] 2026-09-26: Accepted research272 committed as4c03b2f7d; compiler/provider/layout exactly270.
      Perf owned-child task-clock probe denied by kernel policy4; owned-child GDB probe works with
      ASLR unchanged. Read-only preflight retained under raw272 next-profiling-preflight.json.
- [x] 2026-09-26: Worker verifies37runtime artifacts,576inputs,22pins,100layout entries and3config
      hashes exactly. Independent source review confirms the boundary includes TU and entry checking.
- [x] 2026-09-26: Pilot02 exits normally with identical PTX and40 semantic stacks; all stacks
      reach main on thread1 with zero unresolved frames. Lead/independent reviewer approve the fixed
      inventory after owned-child cleanup hardening and preserving cleanup failures as result records.
- [x] 2026-09-26: All16 declared profiles qualify in159.19s; exact PTX,40–84 stacks each,
      982total/912sensitivity, complete unwinds/module PCs and owned-child cleanup. No main retries.
- [x] 2026-09-26: Two paths recur in every profile: generic overload359/982 and inheritance338/982.
      Independent count/source audit passes. Existing canonical caches are present; concrete keys and
      miss reasons were not captured. Zero reduction variants; no optimization justified.
- [x] 2026-09-26: Independent review approves exact16 obligations/counts/source/identity and compact
      report/record. All28 referenced hashes verify. Reviewed candidate ff29f89f is retained under raw273.
- [x] 2026-09-26: Complete documentation/formatting; local commit closes273. Next bounded slice274
      observes existing inheritance cache keys/outcomes; authorized loop continues, Slack skipped.

## Context and Invariants

Accepted runtime/compiler baseline remains270:37runtime artifacts,22pins,576inputs,1,740cells,
1,703correct/37unresolved/20resolved histories, cadence zero. Latest material output hashes and
resources are in272, which exactly matches270. No main corpus, runner, production source, provider,
configuration, math flag or installed artifact changes are allowed. No rebuild is planned.

`FrontEndCompileRequest::executeActionsInner` surrounds `checkAllTranslationUnits()` with the
SemanticChecking timer (`source/slang/slang-compile-request.cpp` near838). The called function near702
checks unchecked translation units and then entry points. Its current source and named symbols are
the sampling boundary. Do not conflate all startup/link/backend frames with semantic-check work.

Host perf_event_paranoid is4 and ptrace_scope1. Do not change sysctl, permissions, capabilities,
ASLR or install tools. Use GDB only as parent of its own compiler child, never attach unrelated PIDs.
Disable debuginfod/network lookup, init scripts and automatic external scripts. Keep actual source,
binary/debug-file identity and environment explicit. Debugger run/stop overhead and wall sampling are
not CPU-clock percentages or benchmark timings. Existing272 timings retain their original identity.

## Milestones and Exact Execution

1. Raw root `build/nvvm-semantic-profile273`. Verify37runtime artifacts,22pins,576main inputs and the
   fixed material source. Use accepted native RelWithDebInfo CLI and source flags copied exactly from
   272 material commands. Capture debugger/tool versions and debug-symbol correspondence.
2. Pilot an ignored owned-child GDB/MI or equivalent controller. Break at the semantic-check entry;
   establish a reliable exit boundary, run for short active intervals, interrupt without delivering
   SIGINT to the compiler, collect source-qualified stacks, and resume to normal completion. Only
   stacks containing the verified semantic ancestor qualify. Record all stops, thread IDs, unresolved
   frames and actual run/stop scheduling intervals. Avoid evaluating compiler functions in GDB.
3. Qualify the pilot: source-boundary and symbol proof, at least one actual semantic stack, normal
   exit and exact accepted PTX output. Detect deadlocks, outside-boundary samples, stack truncation,
   thread confusion and debugger bias. Save failed pilots explicitly. Present method/evidence to
   lead and independent reviewer before the main sampling inventory. If debugger control is not
   reliable, stop this method and propose a separately reviewed temporary timer approach; do not
   silently mutate compiler sources or host settings.
4. Approved main inventory: two material entries x two O3 backends (NVRTC/NVVM) x two requested active
   sampling intervals (5ms/10ms) x two fresh-process repetitions =16profiles. Serial jobs1, unique
   outputs; per-profile timeout180seconds and whole batch1800seconds. Keep all requested outcomes.
   Require normal exit, exact baseline PTX hashes and at least10 usable semantic stacks per profile;
   otherwise preserve the insufficient cell as such and review adequacy before interpreting ranks.
   Preserve source/commands/caches and both interval/repetition groups. Do not present profiled wall
   times as normal compile latency. No concurrent build/GPU/performance work.
5. Aggregate leaf counts and inclusive stack-presence counts separately, deduplicating repeated
   function names within a single stack for inclusive presence. Preserve inlined-frame and recursion
   interpretation, module-relative PCs, source paths/lines and unknown/truncated counts. Counts are
   observations under this sampler, not disjoint CPU percentages. A useful hot path must recur across
   intervals/repetitions and both material entries; document backend differences rather than erase them.
6. Trace the selected path through named producer/consumer functions and existing canonical helpers.
   Identify which checked source shape invokes the work and whether repetition is required. Bound
   reduction to one hypothesis and at most8 variants; record every variant/outcome, independent
   expected semantics and why it represents the path. A smaller source compiling faster is not a
   compiler improvement. Do not claim a reproducible bottleneck from an unrelated microbenchmark.
   If no canonical trace/reduction is established, close research with that explicit limitation and
   a narrower next observation rather than speculative caching or an alternate equivalence relation.
7. Verify unchanged source/config/runtime/layout identities. Lead and independent review retain one
   compact evidence record with16per-profile obligations, method qualification, rank summaries and
   all failures/limits; raw stacks, debug transcripts, snapshots and exhaustive indices stay ignored.
   Complete five-part report, plan/navigation and local commit. Full correctness remains inherited270.

## Decision Log

- 2026-09-26, lead: Choose material-driven shared semantic work based on272 measurements. Use an
  owned-child debugger fallback because perf monitoring is unavailable under current host policy.
  Qualify the sampler before trusting its attribution; require a source trace before optimization.
- 2026-09-26, lead and independent reviewer: Qualify pilot02 for bounded stack observations only.
  Require Linux owned-PID/starttime proof before timeout cleanup, cleanup failure retention, module
  offsets and thread events, and all-data versus delayed-notification sensitivity ranks. No extra
  performance pilot is needed for these controller-only cleanup changes.
- No provider ABI/serialization change or broad semantic cache is selected merely from aggregate
  timing. Any retained compiler change needs a separate input-shape audit, tests and paired timings.

## Surprises and Discoveries

Pilot01 reaches the correct semantic entry and caller return address but fails before sampling:
GDB cannot resolve the unquoted overloaded-method disassembly expression. Preserve this attempt;
correct the debugger query without modifying compiler code. Pilot02 uses numeric disassembly and
qualifies40 stacks. Its semantic scope takes5.255s under debugging, including4.179s of recorded
stopped work. Resume-to-stop notifications range10.46–266.59ms at a requested10ms interval; these
are controller wall intervals, not directly measured inferior active time. Retain all samples and
compare ranks with/without intervals greater than twice the request to expose delayed-notification
bias. No debugger duration is a compile-time or CPU-percentage claim. Perf denial is a host policy restriction,
not a reason to alter the system. GDB's
successful trivial child proves only basic debugger availability, not method accuracy on Slang.

## Validation, Recovery and Stopping Conditions

Keep every failed/insufficient profile and failed reduction. A debugger child that times out must be
terminated without affecting other processes; no residual inferiors may remain. Existing diagnostics,
source oracles and accepted artifact identities remain authoritative. No optimization support or
full-checkpoint cadence change follows from a research profile. Unavailable essential resources or
an external semantic-policy decision can require human input; routine local research is authorized.

## Outcomes and Retrospective

The bounded result establishes two recurring semantic paths, without identifying repeated canonical
keys or cache-miss causes. No representative reduction or optimization is claimed. All37 runtime
artifacts,576 inputs,22 pins,100 layout entries and3config hashes remain unchanged; no rebuild or
production change. Preserve accepted270 full correctness/cadence and272 timing identity. Next is a
separate bounded canonical-key/cache-outcome observation gate, followed by a reduction only if the
evidence supports it. Independent final record/report review passes. Two review-script schema assumptions (symlink mode
and absent Counter key) and a lead assembly field assumption are corrected without workload reruns.
Accepted research273 is ready for its closing local commit; no remaining validation obligation.
