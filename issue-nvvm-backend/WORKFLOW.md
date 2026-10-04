# NVVM development workflow

**Bounded performance isolation complete (2026-10-04):** the maintainer authorized causal
investigation of the corrected Vulkan/CUDA gap. Graph specialization and fast-math policy are
confirmed substantial leads, with reusable Falcor/portable controls and current evidence. No
production optimization or global driver change was made. Future graph specialization must
preserve the general HitObject path; fast math requires numerical qualification. This finite
diagnosis does not restart the general loop; full OptiX profiling remains permission-limited.

**Bounded Vulkan hardware/clock audit (2026-10-04):** L4 native hardware ray tracing is verified.
The prior short Vulkan timing was confounded by GPU clock ramp-up. Sustained warmup reverses
the result: Vulkan default/control are 0.2007/0.2657 ms versus NVVM 0.697 ms. Current evidence
records the correction and preserves the old observations with their limitation. No production
code or driver settings changed; this finite audit does not resume the general loop.

**Bounded Vulkan comparison/code inspection (2026-10-04):** the maintainer requested matched
Vulkan timings and review of generated NVVM PTX/SASS. The four-mode comparison and PTX/isolated
SASS inspection are complete and reviewed; full OptiX native-code profiling is blocked by GPU
counter permissions. No compiler/runtime optimization or global profiling-setting change was
made. Retain the sincos lead, loader collision and exact capture boundary in current evidence.
This finite analysis does not restart the general development loop.

**Falcor milestone complete (2026-10-03):** standard-library GeometryIndex and the necessary
application fixes are accepted. Original smoke and lit path tracing render on both backends; the
matched iteration/compilation comparison is recorded in STATUS, RESULTS and falcor2-status.json.
The bounded request is complete. Rare cross-backend pixel outliers and four pre-existing static
expectation failures remain recorded; unrelated feature work and the general loop stay stopped.

**Falcor milestone continuation authorized (2026-10-03):** implement `GeometryIndex()` in the
standard library for NVRTC and NVVM, remove Falcor's CUDA workaround, then continue resolving
application blockers until successful NVVM rendering or a decision requiring human input. Once
rendering succeeds, compare matched per-iteration GPU performance and separately capture compile
times. This supersedes earlier per-failure stops for work necessary to that application milestone;
use bounded reviewed local commits and existing test infrastructure. Unrelated feature work and
the legacy HitObject semantic decision remain outside this authorization.

**Bounded empty trace-payload fix complete (2026-10-03):** the maintainer authorized repairing
Falcor's `optixTraceRay` rejection. Canonical Void field placeholders now agree with existing
payload serialization; focused execution/static checks pass and the original application rejection
is gone. Falcor now rejects its CUDA-text `optixGetSbtGASIndex()` compatibility helper. This
supersedes the earlier trace-shape stop only for zero-storage payload fields. Compatibility-shim
changes, broader packing/OptiX work and the general loop remain stopped.

**Bounded 64-bit wave arithmetic fix complete (2026-10-03):** the maintainer authorized repairing
the newly exposed Falcor wave-sum restriction. Core signed/unsigned64 sum/product reductions and
prefixes are now qualified; scene setup completes. NVVM reaches the path tracer render call and
rejects `optixTraceRay`. This supersedes the earlier wave stop only for this arithmetic family.
OptiX trace-shape repair, wave performance work, unrelated fixes and the general loop remain stopped.

**Bounded scalar-select fix complete (2026-10-03):** the maintainer authorized fixing the
reduced Falcor issue. Scalar-predicate selection for the existing numeric/Boolean vector family
is now accepted and runtime-tested. The original application rejection is gone; the next scene-update
kernel rejects `WaveActiveSum(uint64_t3)`. This supersedes the earlier no-fix scope only for scalar
select. Further wave implementation, unrelated fixes and the general loop remain stopped.

**Bounded Falcor2 reduction completed (2026-10-03):** the maintainer requested diagnosis and a
standalone shader if useful. The scalar-Bool/float3 selection is captured faithfully in the existing
application corpus, with constant/dynamic failures and a passing vector-condition control. No
compiler fixes were requested or made; general development stays stopped.

**Bounded Falcor2 qualification completed (2026-10-03):** the maintainer redirected work from
Torch fixes to running one existing Falcor2 path tracer test with local dependencies on both
compiler routes. The existing environment and Release build were reused; local SlangPy changes
were merged while preserving the application's required upstream TRS API. NVRTC passes; NVVM
stops during scene update with an unsupported `select` shape, before path tracing. Exact results
are in [the application manifest](falcor2-status.json). No compiler repair or general development
loop is started by this comparison.

**Bounded easy Torch fixes (2026-10-03):** the maintainer authorized repairing the easy cases after
corpus creation. This slice covers CUDAKernel direct-call roles and packed-tensor pointer provenance.
The five mapped application nodes now pass; local/neighboring checks and review establish the bounded
acceptance recorded in STATUS. Polynomial AD layout, softplus optimizer investigation and the general
feature loop remain outside this request. The earlier corpus-only restriction below is superseded
for these two families.

**Bounded application-reproducer work (2026-10-03):** the maintainer requested portable shaders
for the four newly exposed Torch failure families and a reusable application capture workflow.
This authorizes fixtures, provenance, replay and documentation; compiler fixes and the general
implementation loop remain stopped. The earlier green SlangPy checkpoint excluded Torch.

**Compact-vector consolidation complete and stopped (2026-10-03):** explicit and default
uniform groups now share the existing CUDA storage lowering. Both duplicate compact-vector load
conversions are removed. Host layout, Half role separation, snapshot semantics and material
performance are preserved. Full working **1,735/1,735** retains all 1,726 prior passes; units
**570/570** and smoke **16/16** pass. Material PTX is byte-identical to the accepted fast code;
no fresh timing is claimed. This bounded request is complete; no general loop or wave work resumes.

**Bounded material investigation completed and stopped (2026-10-03):** the small shared-storage
producer fix restores material performance using existing unpack helpers, without new provider
machinery or ABI changes. Focused, static, AD, RHI and full working validation are accepted; exact
before/after timing and preserved failed experiments are recorded in STATUS and focused evidence.
The later compact-group consolidation is completed above; further storage/ABI audits need a new
bounded request. Wave work, unrelated features and the legacy OptiX8 HitObject decision remain parked.

Start with [STATUS](STATUS.md), the [architecture](../docs/design/nvvm-backend.md), and the
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md). Use [RESULTS](RESULTS.md) for
commands and [AGENTS](../AGENTS.md) for compiler methodology. [HISTORY](HISTORY.md) explains archive
recovery; current design must be understandable without reading completed slice narratives.

## Authority and ownership

**Bounded work completed and stopped (2026-10-03):** fresh working/unit/RHI checkpoint,
ordinary-IR bitfield simplification and compilation/GPU performance refresh are accepted. The
material regression is resolved by the later bounded request above; wave/min/max weaknesses remain
recorded in STATUS.
No further implementation is started by these recommendations. Unrelated feature work and the legacy
OptiX8 HitObject decision remain parked; the three-item authorization is complete.

**Stopped after completion (2026-10-03):** all 29 remaining SlangPy failures resolve. The fresh
CUDA-selected checkpoint passes 1,591 with zero failures,807 skips and3 expected failures across2,401 nodes,
plus14 module skips. All 1,561 previous passes are preserved; one RGB metadata test is new. No failure was
demoted or test removed. Current exact evidence and smoke/working outcomes are in STATUS and the
maintained manifests. The maintainer requested a stop after resolution; do not resume unrelated
feature work. The legacy OptiX8 HitObject semantic decision remains parked independently.

**Application checkpoint authorized on 2026-10-02:** qualify the sibling SlangPy checkout
against local Slang and RHI, verify explicit NVVM execution, run its complete CUDA-selected Python
suite, and group failures into complete feature families. This work proceeds independently of the
parked legacy HitObject semantic decision below. Preserve the existing SlangPy build and package.

**Resumed on 2026-10-02:** the maintainer approved explicit OptiX8.0/8.1/9.0 targeting,
common raygen/trace/callable qualification, then the older-version HitObject family. Version targeting
and the common family are accepted. **Current decision boundary:** await the maintainer's choice
between modern flag visibility with a private cross-stage ABI preserving32payloadwords and an
explicitly documented native8Invoke semantic difference. Do not enable legacy HitObjects before
that contract is settled. Keep each version's capabilities and tested semantics explicit.

The maintainer approved the corpus-tier and OptiX proposal and explicitly resumed development on
2026-10-01. The earlier stop after four bounded items is superseded. First organize smoke, working
and exploratory selection using existing inventories and runners. Then establish executable OptiX
ray generation, triangle hit/miss with payloads, and representative material execution in bounded,
reviewed local commits. Expand exploratory coverage around feature combinations and applications;
use the findings to choose subsequent work. Continue until a concrete blocker needs human input or
the maintainer sets another stopping point.

The accelerated workflow remains in effect: eight build jobs, related feature batches, economical
validation and no routine semantic-module version bumps. CUDA is comparison evidence, not the
universal oracle; preserve Slang's intended Vulkan/D3D12 semantics and document differences.
The CUDA-text migration, maintenance sequence, native Half batch and requested full checkpoint are
complete. Their accepted results and failure histories remain preserved. No push, publication,
system installation, driver change, reboot or external communication is authorized.

Use one implementation owner and an independent reviewer when available. The lead owns scope,
acceptance and commits. Read-only analysis may overlap; serialize builds and GPU runs. Use **eight
build jobs** on the upgraded eight-CPU host. Use focused test selections and existing runners;
do not build a new orchestration, approval or evidence framework for each batch.

## Select and implement

Read STATUS and inspect the working tree and actual compiler/provider identity. Preserve unrelated
user files. Follow the platform-specific slang-build skill and compiler methodology in AGENTS.
For substantial batches, keep a short uncommitted ExecPlan with scope, input-shape audit and actual
validation results. Do not turn the plan into the program backlog.

Select work from exploratory failures grouped by their underlying missing representation or
operation, weighted by application relevance. Implement complete feature families: identify every
related operation, variant and shared prerequisite before coding, and qualify the whole family
before claiming it complete. This maintainer direction supersedes isolated-operation slices. Use
related batches that share lowering and calling conventions; intermediate dependency commits do
not constitute completion of the family. Isolate genuinely new ABI, resource, control-flow or
synchronization contracts into bounded slices with executable milestones. Reuse canonical shared
IR and checked plan records. Express core behavior through explicit NVVM branches, typed named
calls and ordinary Slang composition; genuine primitive PTX remains an intentional interface.
Do not restore interpretation of CUDA target-switch strings or duplicate a mapping that already
has an owner. Delete fallback branches made unreachable by a principled change.

## Corpus tiers and development cadence

Use one inventory of test configurations; frozen/discovery remain provenance metadata. Smoke is a
subset of working, not a duplicate shader collection. A configuration identifies its source/test
ordinal, backend/optimization and validation level. Keep compile-only qualification distinct from
runtime correctness and retain intentional CUDA semantic differences.

- **Smoke:** run after each implementation iteration alongside affected tests. Target one to two
  minutes excluding builds; measure and trim redundant cases. Default to NVVM O3 with selected O0
  lowering controls, broad compute/ABI/memory/resource coverage, and a few rejection contracts.
- **Working:** protect all admitted passing configurations in the maintained inventory. Run every
  three to five implementation iterations, or sooner after a broad representation/cache/ABI change.
  A new failure remains a regression; never automatically demote it to exploratory.
- **Exploratory:** include known failures and unevaluated candidates. Exercise bounded batches when
  choosing work and between iterations, prioritizing feature combinations and application relevance.
  Distinguish backend gaps, invalid inputs, harness limitations and external failures. Group related
  failures by root cause. Promote only after the stated oracle and execution level pass.

Use existing runners and compact selection metadata. Do not copy sources, duplicate baselines or
build a second reporting framework. Eight-job incremental builds remain standard. NVRTC comparisons
are selected evidence, not a universal oracle. PTX inspection can validate instruction selection;
memory, synchronization, ABI and new OptiX launch contracts require execution. Preserve prior full
results and exact failures. Full integration checkpoints remain deliberate milestones.

The initial OptiX path uses PTX and existing runtime infrastructure. First qualify module/pipeline
creation and raygen buffer output, launch index/dimensions, launch parameters/SBT data and updated
bindings across launches. Then qualify triangle hit/miss with payload transport, followed by a
representative material. Keep stage/ABI ownership explicit and use typed OptiX operations or genuine
primitive calls; do not restore CUDA-text recognition. Advanced OptiX features follow actual demand.

## Application-derived reproducers

Application testing supplies realistic feature combinations. Keep a portable shader reproducer
for each distinct failing shape so local compiler work does not require the application runtime.
Application provenance is independent of smoke/working/exploratory qualification. The initial
[Torch corpus](application-corpus.manifest.json) is exploratory compile/assembly coverage; it is
not a new working baseline or a negative suite that treats missing support as correct behavior.

1. **Capture the application failure.** Record the exact test IDs, application revision, explicit
   backend and optimization choices, loaded compiler/provider identities, dependency/bridge setup,
   input shape, diagnostics or bounded timeout, and a selected comparison run. Preserve the generated
   shader and source dependencies under ignored `build/`. If only runtime output fails, retain the
   input/binding/output oracle as well; compilation alone cannot reproduce that issue.
2. **Make a portable source reproducer.** Start from the failing generated wrapper. Inline or vendor
   only the required licensed source helpers, preserving origin paths and revision. Remove host
   package and absolute-path dependencies. Reduce incrementally against the original diagnostic or
   failure behavior. Record essential shapes such as overload selection, AD context, descriptor layout
   and decorated calls. Keep failed reductions locally; a different error is not faithful reduction.
   Add a passing control when it tests a concrete hypothesis. A shared diagnostic groups candidates;
   it does not prove a shared root cause. Distinguish a matching timeout from a proven identical stall.
3. **Admit the input and observations.** Store reusable shaders under `tests/cuda/applications/` and
   register source hashes, entry points, exact application mappings, preserved shape, desired semantics
   and current per-mode outcomes in `application-corpus.manifest.json`. Keep known failures opt-in,
   without default-suite `//TEST` directives or expected-rejection assertions. Reuse
   `run-complex-corpus.py` for fresh NVRTC O3 and NVVM O0/O3 compilation and assembly with a process
   timeout. Missing tools, compile failures, assembly failures, timeouts and unrun cells stay distinct
   in the assessment. Raw logs and captures stay under ignored `build/`; commands are in
   [RESULTS](RESULTS.md#application-reproducer-corpus).
4. **Debug locally, then validate the application.** Once separately authorized, fix the responsible
   producer or lowering contract using the local input. Require passing local modes and controls,
   relevant runtime oracles, and a rerun of every mapped application variant with verified routing.
   A representative rank-1 or helper-call source does not establish rank-3 or differentiated coverage.
   Update observations and unresolved/resolved history in place; do not erase the original failure.
5. **Promote exact tested configurations.** After runtime correctness is established, add the proper
   authored regression directives or existing focused runtime contract and propose admission through
   the existing tier inventory. Reference the same source and its application provenance rather than
   copying it into a second corpus. Compilation and PTX assembly alone never admit a runtime pass.
   Neither replay nor changed observations automatically promote, demote or close an application issue.

The application manifest owns selection/provenance and compact local observations for this input
class; the existing runner consumes it directly. Application suite manifests retain full suite
outcomes. Replace current observations after a reviewed replay, preserving failure transitions;
Git retains superseded versions. Source fixtures remain as regression inputs after repair. This
avoids retaining generated dumps, completed narratives or a parallel execution/reporting framework.
For a runtime-only issue, use an existing suitable runtime runner and record its contract explicitly
before claiming a faithful reproducer; do not force it into this compile-only manifest.

## Optional application validation with slang-rhi

The sibling `../slang-rhi` checkout is an additional application test source. Rebuild it against
this local Slang compiler before use; preserve its existing build and use a separate CUDA/OptiX
configuration. [RESULTS](RESULTS.md#optional-slang-rhi-cuda-suite) owns the commands. The suite is
**on demand**, not part of every iteration or the automatic working-corpus cadence. Qualify setup
with a small selection before deliberately running the broader suite.

Use explicit NVVM and selected NVRTC comparison runs. The RHI test selector covers its shared
CUDA test-device and availability paths; custom devices/sessions and internal direct NVRTC kernels
retain their own compiler choices. Record those limits, actual executed tests, skips and failures.
Require CUDA availability so a missing device/compiler cannot produce a misleading pass. Group
new failures by compiler representation/operation, RHI binding/harness or external runtime cause;
use application relevance to choose bounded next slices. Do not weaken RHI assertions or silently
add its results to the accepted Slang corpus. Full suite runs remain separately requested milestones. The maintainer has now explicitly requested
one full CUDA suite run across all feature areas. Keep its current registration outcomes and failing
families in `rhi-cuda-status.json`, including exact compiler ownership, selected comparison outcomes,
skip reasons and unresolved/resolved histories. Raw attempts remain under ignored build paths;
replace the manifest in place after subsequent runs, using Git for superseded snapshots. Reconcile
named results with assertion failures: the custom reporter can print SKIPPED after a failed CHECK.
Do not treat interrupted or unrun registrations as passes or automatic expected failures.

## Fast validation by default

Choose the smallest checks that establish the batch's actual contracts:

- For a name migration through an unchanged qualified call path, run focused compiler units and
  compile representative scalar/aggregate/Half/Float32/Float64 inputs as applicable. Inspect NVVM
  LLVM/PTX output and compare relevant operations/call paths with NVRTC PTX. Whole PTX files need
  not be identical. This is a code-generation smoke check, not proof of numerical equivalence.
- Keep direct negative tests for removed IDs/tags/text and signature/no-mutation boundaries when
  affected. Reuse existing generic coverage instead of multiplying every check per operation.
- Run a small existing runtime selection when composition, casts, special-value behavior or the
  emitted operation changes. Require focused execution for synchronization, memory, ABI or control
  semantics that compile/PTX inspection cannot establish. Expand only in response to a concrete
  failure or unresolved risk.
- Reuse existing numerical fixtures and policies. Do not generate exhaustive independent oracles,
  baseline buffers, correction proofs or old-module campaigns for straightforward migrations that
  preserve the selected operation. New numerical algorithms need their own bounded correctness
  evidence; a name migration does not create a new algorithm.
- Build incrementally with eight jobs. Reuse unchanged binaries and successful test results tied
  to their actual identity. Do not rerun focused tests just because a broader suite includes them.
  Formatting/documentation/evidence updates do not require a compiler rebuild.

No full native, frozen/discovery, surface, toolkit or material campaign is required for each batch
or stopping point. Run broader checks at an integration milestone or when a shared change/failure
makes the affected scope uncertain. Name-list additions and operation-specific legacy-route removal
alone do not trigger them. Before publication, follow the repository's required CI/review checks.

Consolidate independent review around the batch diff, chosen checks and actual results. Routine
captures and record updates do not require separate approvals. Preserve failures and investigate
regressions; do not turn wrong output, crashes, missing execution or skips into passes. A PTX smoke
check must be recorded as such, never as fresh GPU or universal accuracy evidence.

## Module compatibility

Keep the already-tested log migration at semantic module version43, provider ABI46 and container
format2. **Do not bump the semantic module version for each further intrinsic migration.** The
prototype does not need a historical-module compatibility campaign for each source change; rebuild
stale modules against the current core, including earlier modules carrying the same version43.
This does not promise compatibility for every prototype binary with that version. Preserve numeric
holes, capability numbering and serialized layouts. Retired explicit operations may diagnose through
normal validation. Revisit versioning only for an actual serialized-format/decoding change or a separately
identified compatibility requirement. Do not rewrite old modules or silently accept invalid IR.

## Acceptance and current evidence

Record the actual commands, outcomes, tested source/binary identity and evidence scope concisely.
For targeted batches, retain the last full baseline unchanged and update current feature/status
records with focused evidence. Historical GPU/numerical/performance results keep their original
identities. A source/PTX smoke batch does not reset or claim a full checkpoint.

When a full checkpoint is deliberately selected, retain exact input/outcome comparisons and existing
failure histories. Use the frozen census and existing discovery/surface/material contracts; matching
totals alone are insufficient. Requalify changed environment components with the smallest relevant
checks before relying on them. The final log-family checkpoint already qualifies the upgraded host.

## Measurement

Use optimized, qualified builds, fixed options/inputs, warmups and repeated samples without competing
work. Separate fresh-process wall time, nested Slang phases, downstream assembly, shared-session
lifetime and GPU execution. Warm filesystem/toolkit/PCH caches are compatible with fresh processes;
record cache evidence and math options. Never sum inclusive nested timers or marginal medians.
Failed compilation latency is not successful compile time. PTX/cubin sizes, registers, stack/spills
and SASS are scoped observations, not GPU speed. Material runtime needs explicit binding/input/output
contracts. Preserve and restore accepted layouts around temporary profiling; no unpaired speed claims.

## Keep current information, not a second project history

Each retained artifact has one responsibility:

| Artifact                  | Update rule                                                                           |
| ------------------------- | ------------------------------------------------------------------------------------- |
| Architecture              | Current pipeline, ownership, invariants and rationale for surviving exceptions.       |
| Feature matrix            | Supported combinations, boundaries, evidence strength and permanent test links.       |
| STATUS                    | Authority, current identities/cadence, unresolved issues and next action; keep short. |
| RESULTS                   | Reusable commands and measurement/acceptance protocols.                               |
| Current accepted evidence | Exact outcomes, inventories, provenance and unresolved/resolved histories.            |
| Tests and input manifests | Reproducible contracts and selected inputs; frozen inventories remain immutable.      |

Update these artifacts in place. Plans, five-part report/PR-description drafts, per-attempt logs,
source snapshots, repeated samples, generated presentations and exhaustive audit indexes stay
uncommitted under ignored working paths or `build/`. Put an actual PR's five-part narrative in its
PR description. Use a concise commit body when closing local-only work. Do not append slice summaries
to the architecture or matrix, or create new permanent numbered plans/reports/evidence files.

Replace the current full baseline only after accepted full validation. Preserve all current cells,
input/runtime identities and unresolved/resolved failure history; Git retains previous accepted
snapshots. Keep focused evidence outside that baseline in one current feature-keyed record while it
supports a current claim or open issue. Replace superseded entries with reviewed evidence, retaining
failure transitions; remove redundant supporting files in the same change. Do not embed full old
baselines recursively. Unchanged evidence keeps its tested identity and never becomes fresh.

At closeout, ask of every added durable artifact: which current contract does it own, why is an
existing artifact insufficient, and when is it replaced? New durable categories need a concrete
consumer and a documented retention rule. Existing slice-named inventories are retained only where
live tools depend on their exact identity. Superseded material is recoverable through Git, not copied
into a checked-in archive. `.gitignore` guards against accidental numbered snapshot accumulation.

Format changed files, inspect the final diff, verify live links/inputs and exact evidence preservation,
and obtain independent review. Update current status and stop/resume authority, then make the
reviewed local commit. Do not commit completed working plans or report drafts. No Slack notifications.

## Optional application validation with SlangPy

The sibling `../slangpy` checkout is an on-demand application checkpoint, rebuilt against this
Slang compiler and sibling RHI. It creates independent Slang sessions: selecting NVVM only on the
RHI test device is insufficient. Use the opt-in local `SGL_ENABLE_NVVM_TESTING` build and a fixed
`SLANGPY_TEST_CUDA_COMPILER=nvvm` or `nvrtc` for each process. The session producer records the
explicit route, SM80 capability and actual device OptiX version in typed compiler options and the
existing session digest. Normal SlangPy builds keep their defaults.

[RESULTS](RESULTS.md#optional-slangpy-cuda-suite) owns setup and invocation;
[slangpy-cuda-status.json](slangpy-cuda-status.json) owns current exact outcomes, comparisons and
failure histories. Preserve the existing extension/build, use a staged matching Python package,
verify loaded compiler/provider identity, and run the route gates before broad qualification.
Use one GPU worker. CUDA selection skips functions without a `device_type` parameter; missing
optional dependencies can also skip whole modules during collection. Report both boundaries,
expected failures, crashes and unrun tests explicitly. Keep required LFS assets hydrated. The maintainer pulled the LFS resources during this checkpoint;
preserve that state.

Group first-failure diagnostics into whole feature families, without assuming one repair will
make every grouped test pass. Start with fixed numeric aggregate entry ABI and correct byval
aggregate forwarding, then resource/helper transport, normalized surface conversions and intrinsic
TensorView operations. Keep resource provenance, type roles and representation caches explicit.
Application failures do not automatically alter the compiler working corpus. Full application
runs remain deliberate checkpoints; use affected tests and smoke between them.
