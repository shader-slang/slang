# NVVM development workflow

Start with [STATUS](STATUS.md), the [architecture](../docs/design/nvvm-backend.md), and the
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md). Use [RESULTS](RESULTS.md) for
commands and [AGENTS](../AGENTS.md) for compiler methodology. [HISTORY](HISTORY.md) explains archive
recovery; current design must be understandable without reading completed slice narratives.

## Authority and ownership

The maintainer authorized the NVVM target/intrinsic migration sequence on 2026-09-29:
introduce explicit target selection and direct LLVM/NVVM primitives, migrate tagged operations,
replace compound CUDA-text recognizers in bounded families, then pursue the recorded wave
optimization. Preserve existing comma-separated `__intrinsic_asm` arguments; remove only the
NVVM semantic-tag extension after its users migrate. Continue through reviewed local commits
until these slices finish or a human decision is needed. The maintainer subsequently authorized
a prototype module-version break: reject earlier capability layouts before deserialization and
require old modules to be recompiled; stable historical capability decoding is outside this slice.
The maintainer's stop-after-exp request was satisfied by the accepted local exp commit. On
2026-09-30 the maintainer explicitly resumed the remaining migration and approved the streamlined
workflow below. Start with exp2, then handle log/log2/log10 as a bounded family. Keep remaining
math, compound and wave work separate where their semantics or consumers differ. Do not undertake
build-system, caching or parallelism investigations as part of this continuation.
STATUS records the current evidence and next leads. Earlier independent review
keeps a transforming local-storage pass deferred because its bounded rewrite cannot retire the
existing conversion responsibility. Accept and locally commit each bounded task before proceeding;
regressions or decisions requiring maintainer input stop continuation. Skip Slack notifications.
No push, publication, system installation, driver change or reboot is implied. Finite maintenance,
results-refresh and documentation requests alone do not resume the general development loop.

For authorized implementation work, use a fresh bounded worker and independent review when available.
The lead owns scope, acceptance and commits; one writer owns the checkout at a time. Read-only review
can overlap. If delegation is unavailable, record that and perform separate local audits without
claiming independent review. Serialize builds, GPU suites and measurements; use at most four CPU
workers, two unit-test servers and 30-minute long gates. Preserve failed and interrupted attempts.

## Select and implement

Inspect revision, working changes, submodule pins and actual compiler/provider/module/cache bytes.
Use the platform-specific slang-build skill. Record toolkit, device/driver, architecture, configuration
and exact inputs. Reuse accepted evidence only with its original source and binary identity.

Correctness regressions take priority. Select a bounded result from application relevance, feature
composition, severity and uncertainty; avoid selecting work merely to increase passing test counts.
Aim for a material-driven slice per three accepted implementations, explaining correctness or
infrastructure exceptions. A moved unsupported diagnostic alone does not justify a compiler change.
Write an uncommitted bounded ExecPlan when AGENTS requires it; include exact acceptance obligations.

Trace workload -> producer IR -> consumer -> runnable fixture. Preserve application inputs and
independent oracles; NVRTC differential comparison is supplemental. Reproduce failures before
production changes. Apply the helper inventory and input-shape audit from AGENTS. Keep frozen v1
immutable; discovery has 50–128 unique sources excluding frozen overlap. Extend a harness explicitly
for new test contracts instead of relabeling failures. Queue unrelated blockers separately.

## Reuse validation and consolidate review

Use the existing approved validation machinery with a small slice manifest containing operations,
fixtures, expected outcomes, neighboring cases and the required gates. Reuse schemas and helpers;
do not clone a new orchestration stack or add a general framework for each migration. Changes to
runner semantics require focused runner tests and a full checkpoint; manifest-only additions do not.

Use two main independent review gates:

1. Review the numerical contract, implementation and concrete validation scope before scored
   post-change execution. Freeze numerical policy and fixtures before baseline GPU observations;
   the same reviewer may review these prerequisites as they become ready. Keep this review cohesive
   rather than requiring separate approvals for each capture, helper or record-generation step.
2. Review final results, preserved identities and histories, documentation and the exact staged tree
   before the local commit. Prepare documentation during validation and fill in actual results at
   closeout. Routine capture, formatting and evidence generation run under the reviewed runner.

For a prototype semantic retirement, capture immutable old-module controls with the existing
compiler, then implement the migration and module-version bump together. Build and validate the
final compiler once. Direct tests must reject retired numeric operations and legacy text before
output mutation, independently of the version gate; a fresh tagged-source diagnostic alone is
insufficient. Also retain old-module metadata/rejection, fresh-module loading, source fallback and
isolated static version tests. These replace the intermediate same-version retirement campaign;
old-module rejection alone must not conceal an unretired backend route. Do not rewrite old module
metadata to make it pass the new reader. Keep the separate static build for now.

Group small related operations into one bounded, reviewed family when they share the established
calling convention and validation structure. Preserve operation-specific numerical contracts and
negative tests. A family accepted in one commit is one implementation for checkpoint cadence; do
not enlarge a batch merely to evade a checkpoint or combine unrelated unresolved semantics.

## Acceptance and checkpoint cadence

For compiler changes, require focused positive/boundary/negative coverage and real GPU outputs at
NVRTC O3/NVVM O0/O3; relevant units and runtime smoke; toolkit/assembly for affected ABI/emission;
explicit neighboring frozen/discovery selections; and all registered material compile/assembly cells.
Explain why the chosen coverage bounds the change. Require one result per requested `(id, mode)` and
compare classification, return code, execution counts, diagnostic and canonical shape. Wrong output,
crashes, timeouts, skips and missing execution never become passes. Preserve failure histories.

Run full frozen/discovery/physical-surface/all-material checkpoints after three accepted
implementations and before a fourth, at publication, and at a requested stopping point after any
implementation since the last full checkpoint. An immediate stop request takes precedence: stop
promptly and record any outstanding validation without claiming it passed.

Also run a full checkpoint when behavior changes in shared lowering/type legalization, signature
validation, ABI, provider/library loading or resolution, corpus-runner semantics, host/toolchain or
configuration, or when the affected scope is uncertain. Adding a function name through an unchanged,
qualified named-library path, removing its operation-specific legacy routes, or advancing the
prototype semantic-version rejection boundary does not by itself trigger a full checkpoint. Require
focused proof of those boundaries and document why shared behavior is unchanged. A larger provider
or library change still triggers the full gate; merely touching such a file does not determine risk.

Full acceptance includes the native, toolkit and runner gates in RESULTS. Targeted acceptance keeps
the last full baseline and its tested identity unchanged, records fresh source/runtime identity and
focused results separately, and compares inherited tests against their exact accepted outcomes.
Use `census.slice-195.tsv` for frozen selection; do not substitute unfiltered discovery. STATUS records
last full, last targeted and implementations since full. Documentation-only changes do not create
fresh GPU evidence or reset this cadence.

`compare` rejects missing, duplicate, changed and false-passing cells. Retain review-required results;
resolve regressions or review intentional transitions before updating the accepted baseline. A manual
status edit or matching total is insufficient. Compare input hashes and exact native identities,
including skips. Surface comparison preserves complete three-mode physical outcomes, exact source/oracle
contracts and known failures; it never converts an expected failure into a passing shader result.
New surface cells or changed obligations require explicit reviewed baseline adoption. A regression
blocks subsequent feature work. Respect user stopping instructions.

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
