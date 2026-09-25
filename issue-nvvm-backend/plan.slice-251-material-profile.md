# Refresh material compilation profiling after AST predicate inlining

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow explicitly requires the completed plan
and report in the slice commit; raw evidence stays in ignored `build/`. Parent owns acceptance and
commit; this worker owns the checkout until it explicitly releases it.

## Purpose and Observable Result

Measure the six intact tiled-brass compile identities on accepted250 and select one bounded next
optimization hypothesis from fresh evidence. Produce reliable phase/wall distributions, independent
assembly measurements, qualitative profiles and explicit future promotion/discard gates. Implement
no optimization and make no material runtime or kernel performance claim.

## Progress

- [x] 2026-09-25: Read repository instructions, workflow, STATUS, reports245/246/250, measurement
      design, native slang-build skill and both environment helper layers.
- [x] 2026-09-25: Declare protocol and scope before sampling.
- [x] 2026-09-25: Capture starting source, submodules, binary/input identity and current environment.
- [x] 2026-09-25: Complete two opposite-order timing rounds and independent assembly study.
- [x] 2026-09-25: Profile the dominant phase with at least three distinct seeded completed compiles.
- [x] 2026-09-25: Trace one bounded candidate through primary sources and record its input-shape audit.
- [x] 2026-09-25: Complete report/evidence/design/STATUS and verify final identities; hand off
      completed checkout to parent for independent acceptance.

- [x] 2026-09-25: Independent parent acceptance passed; debugger table consumer recorded for the next prototype.

## Surprises and Discoveries

The default sandbox fails with `bwrap: loopback: Failed RTM_NEWADDR`; native commands use approved
escalation. The inspected203 helper overrides202's stale Debug compiler/builder paths. Earlier245
`subprocess.run(timeout)` file-redirection timings are invalid; its definitive piped implementation
is reusable. Its final GDB sampler drains pending interrupts until actual process exit.

## Decision Log

- 2026-09-25, worker: Reuse245's definitive protocol unchanged: two rounds in opposite six-identity
  order, two warmups and nine measured fresh processes per cell per round, then one independent
  assembly round of two warmups and nine samples per cell. This is132 compiles and66 assemblies.
  All logs/commands/samples remain; no timing-based retries or outlier exclusions.
- 2026-09-25, worker: Profile separately after measurements, beginning with the dominant measured
  phase. Reuse semantic-window GDB sampling if semantic checking remains dominant. At least three
  distinct seeds must complete with exact PTX. Interrupt counts are qualitative, never CPU fractions.
- 2026-09-25, worker: Preserve current119 source/generated/test,12 artifact and563 runtime-input
  hashes against250. Reuse full250's1701 cells/1662 correct/39 unresolved/18 resolved histories;
  no fresh runtime cells for unchanged-source research. Latest implementation/full250, targeted233,
  cadence0 remain unchanged. Historical246 timing is not a paired baseline for this session.

## Outcomes and Retrospective

Research is complete:132 benchmark compiles,66 assemblies and three profile compiles preserve
accepted250 output. One bounded constructor-visibility hypothesis has explicit semantic/timing
gates; no implementation or runtime behavior changed. Final identity and evidence checks pass; independent parent acceptance passed. Full250/targeted233/cadence0 are unchanged.

## Context and Current Pipeline

Both `eval_buffer` and `sample_buffer` in `tests/cuda/complex/tiled_brass_material.slang` compile at
NVRTC O3 and NVVM O0/O3 with SM80 output. Their many overloaded/generic calls reach semantic
checking, then IR generation, linking/optimization and backend output. Slice246 inlined the existing
`SyntaxClassBase::isSubClassOf` predicate, with exact hierarchy/cast preservation. Its former hotspot
must not be presumed to remain. Inclusive timer relationships are documented in
`docs/design/nvvm-material-compile-time.md`; parent/child timers cannot be summed.

## Scope and Non-Goals

Research scripts, raw outputs and primary-source snapshots under
`build/nvvm-loop/slice-251-material-profile`; durable plan, five-part report, timing evidence, design
note and STATUS only. No compiler/provider/runner/test/material/build configuration changes, no new
runtime contract, no GPU dispatch or full-corpus rerun, no system permissions/driver/reboot/push.
Stop after selecting one defensible candidate or explaining why none is defensible.

## Architecture and Invariants

Preserve the exact accepted250 source/entry/stage/target/capability/optimization/backend/performance
options via `run-complex-corpus.py::compile_command`, changing only output paths. Check entry and
SM80 target on every PTX and all PTX/cubin hashes against250. Investigate any mismatch without
silently dropping it. Fresh process elapsed time uses `perf_counter` around piped Popen.communicate
through exit; log writes occur afterward. Assembly is measured separately. Downstream/lifecycle
residuals remain explicitly unattributed; neither is a direct backend/startup measurement.

## Interfaces and Dependencies

Native Ubuntu24.04, branch nvvm-backend, accepted250 commit
`5916b57fb1ed95c33b1f7b1730b099a05496add4`; matching RelWithDebInfo, CUDA12.9.2/NVRTC12.9.86,
LLVM14, providerABI41, NVIDIA L4 SM89 driver580.126.09 targetingSM80. Compiler library hash
`ae6fe92965ed066a02ff9eab550093294b916c6b29f122f02e8a3f2ab8b74f60`; provider hash
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`.
No API changes. No perf permissions change at the recorded perf_event_paranoid4 restriction.

## Milestones

1. Save starting git/submodule/environment/configuration evidence and verify250 identities.
2. Adapt inspected245 local driver into a fresh raw root, save exact commands, run serially with
   no competing owned build/benchmark/GPU suite. Require132 compile and66 assembly passes.
3. Inspect phase statistics, then run three seeded completed profiles with180-second child bounds.
   Inspect local primary source for one repeated path; do not implement or explore unrelated fixes.
4. Write bounded future prototype gates and audited source trace; preserve raw index and snapshots.
5. Verify final hashes, exact inventory/statistics and docs diff, then hand off to parent.

## Validation and Acceptance

From repository root, inspect/source `build/nvvm-loop/slice-203-env.sh`; run the recorded local
measurement driver under `timeout --kill-after=30s 30m`. Each child has180 seconds. Benchmark serial;
at most4 active CPU workers across owned work. At least three distinct completed profile compiles
must preserve accepted250 PTX. Recompute medians, inclusive quartiles, min/max, per-round stats and
all phase distributions from108 measured compiles/54 assemblies, retaining24/12 warmups. No GPU
runtime evidence is claimed. Exact119/12/563 identities before/after and immutable historical250
references justify inheriting the full checkpoint. Future implementation requires separately declared
paired optimized-build timing and correctness gates appropriate to its actual shared-code scope.

## Failure and Recovery

Retain every failed/interrupted attempt. A timeout/mismatch/crash is not a pass and requires diagnosis.
Do not overwrite245/246/250 evidence or rerun for favorable timings. If profiling cannot support a
candidate, record that outcome. A method correction must be documented before replacement samples,
with the entire invalid batch excluded for the method, independent of values. Never modify system
profiling permissions. Stop for unavailable resources or consequential scope uncertainty.

## Artifacts and Hand-Off

Durable `timing-evidence.slice-251.json`, this plan, `report.slice-251-material-profile.md`, appended
design findings and STATUS handoff. Compact evidence links commands, all raw samples, identities,
primary snapshots and a closed raw artifact index. Parent independently audits and commits; worker
returns the observable findings, exact counts, identity, limitations and next bounded hypothesis.

## Recorded Research Results

All132 compiles (108 measured,24 warmups) and66 independent assemblies (54 measured,12 warmups)
pass with exact250 PTX/cubin bytes. Fresh wall medians are1407.13–1601.66ms; SemanticChecking
484.74–488.73ms. No failures, retries, timeouts or excluded benchmark attempts. Semantic checking
remains the largest localized front-end phase. `generateOutput` is larger in some modes but includes
322.88–346.06ms link/optimize plus downstream/output work; it is not a competing exclusive stage.

The perf probe still fails at perf_event_paranoid4. Seeds251/252/253 each compile through normal
GDB exit and produce exact250 PTX, collecting89/91/93 snapshots. getClass leaves6/6/4 plus the
constructor3/3/9 identify a repeated narrow metadata path. The old predicate appears1/5/3 times,
while `_int_malloc` appears6/10/4 across unrelated consumers. This does not establish one uniquely
dominant function or any CPU fraction. Sixteen stacks hit the100-frame cap; five end before the checker frame,
while268 retain that frame. The window also spans post-check component construction before generateIR, although every
fully captured stack is inside checkAllTranslationUnits. Preserve this boundary caveat.

The selected future hypothesis is inlining the existing tag-to-metadata constructor. Unlike246,
this requires exposing one declaration of TU-static `kAllSyntaxClasses` while retaining one generated
definition and compile-time agreement with `ASTNodeType::CountOf`. Preserve metadata pointer identity,
node initialization, all casts/predicate, default/null state and invalid-tag assertion policy. A
private static table member is one possible internal declaration, not an implementation chosen here.
No second mapping or public ABI change is acceptable. Fresh seed253 stack81 reaches the constructor
through `inferGenericArguments -> as<CallableDecl> -> NodeBase::getClass`; concrete canonical input
is the generic declaration's inner node. No producer repair or syntax reconstruction is indicated.

The future prototype must independently prove all702 tags/492804 class pairs,66 abstract classes,
636 typed objects and all cast/null/const/DeclRef policies, plus exact table/pointer/count identity,
serialization and assertion behavior. Shared-AST units/semantic regressions and full1701-cell corpus
are required. Paired optimized timing must retain both orders/builds and all samples:5% lower semantic
median in every cell,2% lower sum of six wall medians, no wall regression above2% in either round.
Discard on failed semantic/performance gates; do not bundle direct-tag/dispatcher or allocator work.
`hypothesis.json` and the five-part report provide the complete handoff. No optimization is implemented.

2026-09-25 decision: prefer the narrow repeated metadata construction path over dispersed allocation
and Val work; the fresh profile does not justify changing generic solving or preserving the old claim
that `isSubClassOf` is the dominant leaf. Historical246 timings remain unpaired historical evidence.

2026-09-25 parent acceptance: independently recomputed all distributions and exact sample inventories,
verified132 PTX/66 cubin identities, all3 completed profiles, source trace,119/12/563 identity,
579 raw artifacts and134 worker snapshots. Research251 is accepted for local commit. Parent review
also records `source/slang/slang.natvis`: six NodeBase conditions use the existing table name.
The next prototype must preserve that lookup or explicitly adapt and validate those consumers;
preserving its name/scope is preferable to an unnecessary debugger change. A separate parent
snapshot retains this additional consumer without changing the closed worker artifact index.
