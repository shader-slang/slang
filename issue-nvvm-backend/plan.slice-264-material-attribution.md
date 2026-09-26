# Attribute material compilation and generated resources

This bounded ExecPlan follows `.agent/PLANS.md` and is committed on completion under the NVVM
maintainer exception. The user authorized a finite follow-up: understand timing/resources, test a
strong general optimization if supported, refresh presentation evidence, notify and stop. The general
slice loop stays stopped. This plan owns research264 only; a promoted optimization needs its own
bounded plan after this evidence selects it. At most one optimization mechanism is selected initially.

## Purpose and Observable Result

Explain the material's NVVM O3 compilation cost relative to NVRTC and identify concrete differences
behind its larger registers/stack. Produce directly measured stage attribution and a source-to-IR/PTX
trace or clearly bounded uncertainty, plus one general candidate and a small representative fixture
when justified. Research is useful even if no optimization qualifies.

## Progress

- [x] Read WORKFLOW/STATUS/PLANS and build skill; clean starting revision f9532f06e.
- [x] Assign read-only timing-boundary and resource-trace reviews; parent owns all writes/execution.
- [x] Preserve accepted262 binary/cache identities and the existing263 package before experiments.
- [x] Add minimal measurement scopes or use existing tools to separate backend work; prove output preservation.
- [x] Measure both material entries/NVRTC O3/NVVM O0/O3 with maintained warmup/repetition protocol.
- [x] Trace observed resource differences through representative helpers and boundary fixtures.
- [x] Independent closeout review passed: identities, outputs, evidence hashes, units and probe oracles.
- [x] Complete research artifacts and local commit; send the completion DM before starting265.

## Surprises and Discoveries

Measured shared work accounts for72–76% of NVVM O3 wall time, vendor compilation13–18%.
Duplicate serialization is real but below1%wall. Vector layout explains evaluation's192 extra
stack bytes. General grouped probe loses constants in both compilers; separated storage removes
the constant exponential path and passes all three runtime modes. No differential reproducer yet. All material
O3 spill counts are zero; stack growth cannot simply be labeled register spilling. The accepted262
parallel PCH incident remains open and serial measurement must keep normal cache behavior.

## Decision Log

- Preserve the original263 package. Any updated results use a new directory and explicit identities.
- No material-specific symbol checks, fixture edits, forced register caps or weakened math settings.
- Separate repeated timing from intrusive dumps/debugger inspection. Do not time while building/testing.
- Qualify instrumentation against unchanged PTX/cubins before using it for attribution. Timers are
  inclusive unless a scope explicitly measures a disjoint boundary; no sum of nested phase medians.
- No driver/profiling-permission changes, dependency updates, push or open-ended development loop.

## Outcomes and Retrospective

Research found no narrow production change to promote. Timing output preservation and139units
pass; grouped/separated probes pass6runtime modes total. Exact accepted262 source/build is restored
(27identities). The next authorized slice265 refreshes reporting/presentation only, then stops.
Final handoff must explain what is measured, what is inferred and what remains unresolved.

## Context and Current Pipeline

Accepted262 covers1713 cells/1674 correct/39 gaps and18 histories with explicit serial closure of
one NVRTC PCH incident. Compiler49593da72, providerABI42, LLVM14, CUDA12.9.2/NVRTC12.9.86, L4SM89,
SM80 target, RelWithDebInfo. Package263 uses workspace201cea6c9. Fresh material O3 medians are
NVRTC1362.76/1390.16ms versus NVVM1355.04/1460.18ms (evaluation/sampling). Registers48/63 versus67/86;
stack592/624 versus784bytes. Source is tests/cuda/complex/tiled_brass_material.slang, both buffer entries.

Slang's shared front end and link/optimization feed CUDA-source/NVRTC or direct NVVM emission/provider
and libNVVM. source/slang/slang-emit-nvvm.cpp owns direct emission; source/compiler-core/slang-nvvm-compiler.cpp
owns libNVVM calls; slang-nvrtc-compiler.cpp owns NVRTC calls. Existing profiler scopes are defined by
core profiler infrastructure and report-perf-benchmark. Resolve exact boundaries before adding scopes.

## Scope and Non-Goals

Timing attribution, observational resource analysis, a general minimized candidate, and reviewed
research artifacts. No broad semantic refactor, new supported feature, material GPU performance claim,
PCH mitigation or speculative optimization bundle. The material application runtime contract remains
missing. Compiler improvements are a subsequent bounded slice only if this research supports them.

## Architecture and Invariants

Keep shader sources, options, numeric behavior, provider ABI, library selection and oracles unchanged.
Use established profiler APIs; do not invent a parallel timing subsystem. Classify internal buffers,
function arguments and address-taken aggregates by canonical roles, tracing upstream producers.
Per AGENTS, inventory any new helper/fallback and perform the input-shape audit before promotion.

## Interfaces and Dependencies

Follow RESULTS.md and the native slang-build skill. Four CPU workers total, two unit servers; no
competing builds/GPU/profilers during timing. Bound each long process to30minutes and retain logs.
Use build/nvvm-material-followup for raw source snapshots, preserved layouts, dumps and inspection.
Use nvvm-results.py material/report for repeated measurements; record instrumentation separately.

## Milestones

1. Verify baseline and audit current boundaries/PTX without changing compiler semantics.
2. Establish directly timed API/emission scopes and output-identical attribution evidence.
3. Explain representative resource differences, select one mechanism and a reproducer if supported.
4. Review research report and compact evidence, commit, Slack notification; prepare the next bounded
   optimization plan only if justified. Otherwise refresh the explanatory package and stop.

## Validation and Acceptance

All selected material cells compile/assemble; instrumented outputs match accepted263 exactly.
Any measurement-only compiler changes require helper/scope review, relevant profiler/downstream
coverage and proportionate preservation checks, with fresh/inherited correctness clearly separated.
A retained compiler optimization requires the separate plan's full required correctness gates and a
paired reversed-order baseline/candidate experiment. Declare numeric benefit/regression criteria
before prototype measurements; no retries/outlier removal or weakened thresholds after seeing data.
Accept research with direct stage evidence, concrete resource trace where possible and honest limits.
Never relabel serial success as repaired concurrent PCH reliability or code size as GPU speed.

## Failure and Recovery

Retain failed attempts and instrumentation patch. Preserve accepted compiler/cache bytes together;
restore exact layout if a measurement experiment changes outputs or its mechanism is not qualified.
If profiling is unavailable, use explicit API timing and inspect saved IR/PTX without system changes.
If no general mechanism is justified, close research with findings and no optimization rather than
manufacturing a material-only improvement. Independent blockers stay follow-up items.

## Artifacts and Hand-Off

Completed plan264, five-part report264 and compact timing/resource evidence; raw files remain ignored.
Distill durable findings into docs/design/nvvm-material-compile-time.md and a refreshed package after
any accepted optimization. Keep STATUS current. Lead sends one DM at slice completion and a detailed
final notification at the finite sequence boundary, then stops for the user's discussion.

## Completion evidence

See report264 and timing-evidence264 for reviewed statistics, output hashes and probe results.
All132 repeated PTX/66cubins match263;42qualification cells preserve output/resources.
No benchmark ran alongside build/test work. Source instrumentation is reversed; no accepted compiler
change or new full checkpoint. All raw failures and negative probes remain under ignored build.
