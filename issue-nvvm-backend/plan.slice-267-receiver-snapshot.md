# Test narrower receiver snapshots and one general optimization

This bounded ExecPlan follows `.agent/PLANS.md` and the NVVM exception requiring completed plans
and reports in local commits. On 2026-09-26 the user authorized the proposed experiment: narrow the
normal helper's inputs while retaining its branch, trace the relevant Slang IR boundary, and, only
if supported, prototype one general compiler improvement. Validate the reproducer and unchanged
original material, report tradeoffs, and stop. The general development loop remains stopped.

## Purpose and Observable Result

Determine whether passing only `hints` and `direction` instead of a whole Graph to the nonmutating
normal helper restores constant propagation. If the comparison identifies an existing optimization
boundary that owns a general solution, test one compiler prototype against the unchanged fixtures
and material. A negative result with a precise boundary is a valid research outcome; no promotion
is promised. A successful prototype must eliminate constant exponentials, preserve nonconstant
work and all independent outputs, and improve the original material as well as the reduced example.

## Progress

- [x] 2026-09-26: Read STATUS, WORKFLOW, PLANS and the native slang-build skill; clean start d937c9f5a.
- [x] 2026-09-26: Verify accepted artifact identities and preserve baseline source comparisons.
- [x] 2026-09-26: Explicit hints/direction/normal inputs preserve the branch and yield constant
      NVRTC/NVVM counts0/0 and runtime-control2/2;12 original/narrow GPU cells pass. A raw material
      copy passing SurfaceInteraction/hints to its helper removes NVVM's six exponentials in both entries.
- [x] Trace shared IR and review canonical snapshot, extraction and load-stability contracts.
- [x] Decide to test one bounded value-parameter decomposition prototype; gates below precede build.
- [x] Preserve accepted layout; lead rehashed all98 manifest entries with zero mismatches.
- [x] Corrected explicit-snapshot boundary passes3/3 baseline modes without warnings.
- [x] Implement one prototype in existing parameter-transform machinery, scheduled only on direct
      NVVM before `deferBufferLoad`; initial patch0696b4049d00f93849e09d6b8c4cea26832e47c7f99562221cacb6ad1980f8d9.
- [x] Corrected parameter order: unchanged266 masked/unmasked constants now0 exponentials,
      controls2; both original material entries0. All18 standalone compile/assembly cells pass.
      Material entry registers52/62, stack0 and spills0 reproduce the source-counterfactual benefit.
- [x] Ordered prototype138141681c89d414ebda6b031164cb3b6df68e28266f64eb1e62ec434e74fa3e
      passes27 focused GPU cells and independent signature/call/snapshot IR review.
- [x] First full checkpoint under `build/nvvm-receiver-snapshot267/full/checkpoint` completed
      review-required: frozen1356 exact; discovery355/357 preserved, with two new parameter-block
      compile regressions. Runtime4/material6 pass. Retain this rejected checkpoint.
- [x] Correct the same pass's resource-specialization boundary and qualify the failing fixture and
      selected/local resource neighbor. The final common-pass IR regression check is also qualified.
      Full correctness, units/semantics/toolkit/contracts, quality and paired timing remain below.
- [x] Field-policy correctionfe504217967549990a4310645c9903ce321f7b23b3af7f5f5af9b0d17e665635
      passes18 standalone compile/assembly cells and31 focused runtime/IR checks. Both unchanged
      material entries still remove their six exponentials. The local resource snapshot passes all
      three modes on baseline and corrected candidate. The common-`deferBufferLoad` IR revert drill
      fails baseline only for the expected narrower signature, while all three baseline GPU modes pass.
- [x] Final full checkpoint at `build/nvvm-receiver-snapshot267/full-final/checkpoint` preserves
      all 1356 frozen and 357 discovery outcomes exactly, with unchanged input hashes and 39/18 failure
      histories. Runtime4 and material6 pass. Units preserve 1086 pass and 13 skip exactly.
      Final library SHA256 is
      `1cba6a5119a4058b449f70ed79a242fc13ddccff382e77132e58bdcbad8d97ed`.
      Exact global nested-parameter-block baseline/candidate pairs also pass all three modes.
- [x] Semantics preserve 1170 pass and 78 skip exactly; toolkit18 and harness contracts
      1/6/15 (one skip)/16 pass. Lead and independent reviewer accept final correctness, including
      exact test identities and final compiler/input bindings. Raw correctness accepted for quality
      measurement only; optimization promotion remains conditional on the declared resource/time gates.
- [x] Execute prototype/revert, full correctness, fixed quality and paired measurement gates.
      All four NVVM O3 timing limits pass; medians decrease 1.45–2.35%. Quality resources/sizes are
      unchanged; no measured mode adds spills. O0 material entry stacks grow 320 bytes with larger
      modules and four additional existing helper blocks each; retain the raw symbol-review exit1.
- [x] Lead and independent reviewer accept the explicit O0 tradeoff and final compiler identity.
      Complete report/evidence/handoff and final formatting. Closing local commit and notification are
      recorded in the ignored closeout; stop after this bounded slice, with no push.

## Surprises and Discoveries

Research266 is the starting evidence: masked/unmasked constant NVRTC/NVVM O3 counts are0/3;
branchless gives0/0. Runtime controls pass and retain exponential work. No particular internal
libNVVM pass is identified. Whole-graph return, vector payload and index masking are not individually
necessary explanations. These facts constrain this experiment; do not restart the earlier reduction.

The narrower-input source experiment succeeds on both the small fixture and the material. Existing
CUDA source runs `transformParamsToConstRef`, whereas direct NVVM skips that scheduling case. That
pass narrows field reads and retains a snapshot temporary. However, an explicit copied-Graph constref
probe fails direct NVVM compilation with E52018, compact parameter-group vector element extraction.
A 16-line struct containing float3 reproduces that failure; float4 succeeds. The emitter classifies
a read-only helper field as compact storage even though its helper layout uses a native vector.
That separate storage-role correctness issue is recorded, not repaired here. Local/inout roots remain deliberately outside
buffer-load argument specialization; simply widening that contract would risk snapshot semantics.

The first mutation boundary passed one source object as both value and inout arguments. Its three
baseline tests failed diagnostic comparison with E30051. Review found that source shape explicitly
out of contract (`slang-check-expr.cpp` and the parameter nonalias rule); it is not a compiler
regression. The corrected test captures a separate local snapshot before the mutating call. Retain
the original attempt and its source, and qualify the corrected oracle on baseline before candidate.

The first configure/build succeeded. During review-guard rebuilding, the worker briefly overlapped
two incremental builds after misreading a running poll. No GPU suite or measurement overlapped.
Both attempts are retained but excluded from qualification; after every build subprocess exits,
force affected translation units stale and recompile/relink once serially before testing.

The first candidate gate rejected transformed calls with E52017 (`uint -> vector<float,3>`).
`IRBuilder::emitParam` appends parameters regardless of the insertion point, so parameter and
argument order differed. Correct the producer with `createParam` plus insertion before the original
parameter. This repairs the same prototype's IR construction, not the NVVM consumer or input-shape
contract. Preserve the failed gate and rerun under a new identity/output root.

The first full checkpoint finds two genuine regressions: `bindings/nested-parameter-block-3.slang`
in NVVM O0/O3 changes from correct to E52017 for a naked `ParameterBlock<MaterialSystem>` helper
parameter. The transformation introduced that parameter after `specializeResourceUsage` had already
run. That existing pass explicitly expects resource fields exposed as top-level parameters and
handles uniform parameter groups on all targets when their argument has a supported global-root
chain. Move the producer before this consumer as the first principled correction, and inspect the
actual scene.material chain. Also check a selected/local resource snapshot: resource-containing
struct helper parameters are supported more broadly than standalone parameter-block formals, and
the existing specializer deliberately excludes arbitrary phi/local roots. If that domain requires
a restriction, reuse an existing type policy rather than adding a type-name guard or a new
alias/provenance walk. No provider extension is authorized by this correction.

The selected local Scene probe passes baseline compilation/assembly and all three GPU modes with
outputs7/11, but fails the candidate in NVVM O0/O3. Moving the pass earlier fixes the global case
and still fails this local case, so scheduling alone is insufficient. Review also rejects
`isSimpleDataType` as the final bound: it accepts arbitrary pointers, while NVVM's resource aggregate
and standalone helper domains differ for parameter blocks, resource arrays, atomics and physical
pointers. Reuse `isNVVMSupportedHelperValueType`, the same recursively closed policy accepted by
NVVM helper parameters, via a required callback supplied at the NVVM scheduling boundary. Keep the
generic transformation free of backend classifiers. Restore its original before-`deferBufferLoad`
position because an earlier position is unnecessary once newly exposed fields obey that contract.
Keep original parameters intact whenever any selected field lies outside that domain.

## Decision Log

- 2026-09-26, lead: The user authorizes a conditional single-mechanism prototype, not unrelated
  layout/serializer work or an open-ended optimization loop. Source counterfactuals are evidence,
  not compiler improvements. Preserve the original material and research266 fixtures.
- 2026-09-26, lead: One fresh-context writer owns experiment fixtures and all compiler/build/GPU
  work. Independent read-only source and acceptance reviews may overlap, but tests/benchmarks/builds
  are serialized. Lead owns this plan and final integration/commit.
- 2026-09-26, lead and independent reviewer: Source narrowing succeeds in both material entries
  and the runnable fixture, supporting one used-field value-parameter decomposition prototype.
  Restrict it to direct internal callees and canonical field-extraction-only uses of struct value
  parameters, reducing to a strict subset of fields. Process callees first; use canonical field keys
  and existing builders/type repair. Extract from the original SSA argument, preserving the snapshot;
  let existing load narrowing enforce its own memory-stability proof. Exclude external identities,
  whole-value uses and unsupported call shapes. No pointer borrowing or new alias claims.

## Outcomes and Retrospective

Accepted267 retains one general internal value-parameter transformation. Source narrowing predicted
the benefit; the unchanged original fixtures and material establish the compiler result. Full1713-cell
correctness and all side gates preserve accepted262 exactly. Both material O3 entries remove six
exponentials and their784-byte stack allocation, with registers67→52 and86→62. Both reversed timing
rounds meet the predeclared limit, with medians1.45–2.35% lower. No measured mode introduces spills.

The fail-closed resource checker requires review because each O0 material module retains four more
helper functions. Independent LLVM/PTX comparison confirms existing internal helpers, not newly
invented behavior. O0 entry stacks grow320bytes and modules grow; all added helpers have zero
stack/spills. Lead and independent review accept that disclosed control tradeoff. The original
six-exponential/784-byte benefit baseline and timing gate concern O3; this is not a claim that every
optimization level reduces stack. Preserve raw exit1 and the symbol flags beside the separate
reviewed acceptance. No sample was removed and no measurement rerun was used to close that flag.

The qualified final source is d937c9f5a plus patchfe504217..., version2026.18.3-283-gd937c9f5a,
with loaded library1cba6a5119a4058b449f70ed79a242fc13ddccff382e77132e58bdcbad8d97ed.
The unchanged launcher alone cannot identify it. The first failed full checkpoint used ordered
patch13814168... and library6695bdd0...; its two resource regressions and all earlier attempts remain
recorded. The helper-value policy correction, canonical parameter ordering and independent snapshot
oracles were necessary to qualify the optimization. No provider/layout repair was added.

The bounded experiment is complete and the general loop remains stopped. The separate constref
float3 classification failure is queued for a new authorized correctness task. Material runtime
contracts remain unavailable. The completed report and one structured validation record own the
durable findings; raw artifacts and closing notification metadata stay under ignored build.

## Context and Current Pipeline

`experiments/material-reproducer/graph.slangh` initializes Graph and its counters. `populate` stores
absorption, prepares normal indices0/1, then layer0. `prepare` snapshots the whole Graph by value for
`adjust`, which branches on zero-initialized hints and otherwise reads direction. The final NVVM
PTX removes calls/branches yet reloads counters and payload around dynamic sibling-array stores.
The existing `canAddressesPotentiallyAlias` reasons by canonical field keys;
`canInstHaveSideEffectAtAddress` is conservative for whole-object pointer call arguments;
`tryRemoveRedundantLoad` forwards same-block exact-pointer stores; `isPromotableVar` excludes partial
aggregate stores. Load narrowing and argument specialization have their own root/stability contracts.
Locate the precise producer/consumer break before extending any of these contracts.

## Scope and Non-Goals

Narrowed helper inputs preserving the branch, shared IR tracing, and at most one evidence-supported
general optimization mechanism. No material-specific names/constants, blanket alias/inbounds flags,
new semantic representation, ABI/local-layout redesign, math relaxation, unrelated fixes, push or
material GPU-speed claim. If the hypothesis fails, investigate downstream ordering as bounded
research and report the result rather than adding another speculative compiler mechanism.

## Architecture and Invariants

The checked aggregate snapshot is canonical and must retain value semantics. Loads cannot be moved
past mutation without proof; fields supplied to a helper must reflect the original call's snapshot.
Partial writes and aliases, control-flow merges, escaping references and nonconstant predicates
must remain correct. Reuse the existing field-key alias and canonical builder APIs. Before retaining
any helper/special case, record the full AGENTS input-shape audit and a revert drill when practical.

## Interfaces and Dependencies

Native Ubuntu24.04, accepted262 compiler49593da72/providerABI42, LLVM14, CUDA12.9.2/NVRTC12.9.86,
SM80 on L4, RelWithDebInfo. WORKFLOW/RESULTS define identity and commands. Build skill is at
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`; use native cmake and existing configuration,
cap CPU workers at4, unit servers2, and refresh cached CMake version metadata for a real build.
Raw root: `build/nvvm-receiver-snapshot267/`. Preserve full accepted bin/lib/cache layout before any
compiler rebuild. No dependency or environment update is part of this task.

## Milestones

1. Verify unchanged source/artifact identities, submodules and loaded compiler/toolkit paths.
2. Add isolated source variants under the raw root; replace only the helper receiver with explicit
   field values while preserving its conditional. Compile/assemble O3 in both backends, run exact
   computeMain bodies in all three modes with research266's independent oracles, dump shared and
   downstream IR. Keep full failed/non-improving variants and commands.
3. Identify an existing transformation that can expose the same facts safely. Review source-to-IR
   trace and whether input is canonical. Record decision and precise promotion gates before building.
4. If justified, preserve baseline layout and implement one bounded general mechanism with positive
   and mutation/alias/control-flow/escape boundaries. Build through skill; first check unchanged
   research266 and material support. If required benefit fails, preserve patch/evidence then restore
   accepted source/build. If it succeeds, complete correctness and paired tradeoff measurement.
5. Retain compact reviewed research/prototype outcome, full applicable per-cell history, completed
   plan and five-part report; update navigation, commit, once-per-slice DM, stop.

## Validation and Acceptance

Counterfactual qualification requires exact independent integer outputs and three-mode GPU checks;
record the source and all include hashes, and use computeMain wrappers to match render-test.
Standalone compilation uses `slangc <source> -target ptx -stage compute -entry computeMain -O3
-capability cuda_sm_8_0 -o <unique.ptx>`, adding `-emit-cuda-via-nvvm` for NVVM. Assemble via
CUDA12.9 `ptxas -arch=sm_80 -v`. Use RESULTS' qualified dump options and the skill's before/after
pass IR options; inspect local help/source if unsure rather than assuming a flag works.

Prototype promotion requires constant exponentials removed in unchanged research266 while runtime
controls stay correct, improvement in both unchanged material entries, and boundary coverage that
proves the responsible layer. General shared-pass changes require full frozen/discovery/material
checkpoint plus units, semantic suites, toolkit and relevant harness contracts per RESULTS. Compare
all old per-cell obligations and39 known gaps; do not reset histories or turn missing execution into
success. Check resource tradeoffs on the fixed quality subset and material, with spills explicit.

Declared before prototype build: unchanged266 constant modules must contain zero exponentials,
runtime controls must retain required work and exact outputs, and both unchanged material entries
must remove their six exponentials and reduce stack usage. Fixed quality/material comparisons must
introduce no spills. Complete the full correctness obligations above before promoting the change.
For paired compile timing, neither material entry may have a median slowdown greater than5% in
either reversed-order round. Improved code quality with neutral compile time is sufficient; no
GPU-speed benefit is inferred from resources. Preserve and report all other resource deltas.
Use paired baseline/candidate RelWithDebInfo layouts with two reversed-order rounds, two warmups
and nine measured samples per identity; no concurrent work, retries or sample removal. Report fresh
wall times, material/vendor costs when directly available, assembly/resources separately. Any
retained optimization needs supported benefit with acceptable declared tradeoffs, not just a
source-level ablation. A discarded or source-only experiment needs proportionate checks, not a
new full correctness claim.

## Failure and Recovery

Bound individual long gates to30minutes, retain timeouts and failures with exact commands. Keep
accepted layouts separate; restore all matching compiler/module/cache bytes together after a
discarded prototype. Do not paper over malformed input if found: repair its producer within scope
or record the independent blocker. If narrower inputs do not help, do not force a source/API change;
retain the negative comparison and bounded downstream-ordering findings. Stop at this experiment.

## Artifacts and Hand-Off

Raw variants, dumps, prototypes, binaries, repeated measurements and exhaustive indexes stay under
ignored build. Retain only necessary reproducible fixtures, compact evidence with per-cell outcomes
and failures, plan/report267, and concise design/STATUS/HANDOFF/HISTORY changes. The lead owns final
review, local commit and the standing completion notification to Simon under WORKFLOW. Record its
permalink in task closeout. No subsequent implementation or general-loop continuation is implied.
