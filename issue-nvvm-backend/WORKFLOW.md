# NVVM development workflow

Read [STATUS](STATUS.md), the active bounded plan, and [AGENTS](../AGENTS.md). For commands use
[RESULTS](RESULTS.md); for a new session use [HANDOFF](HANDOFF.md). Historical plans and reports are
indexed by [HISTORY](HISTORY.md). Manifests own inventories; compact results own outcomes and failure
histories. Do not reconstruct decisions from chat or repeat the historical ledger here.

## Authority and ownership

The general development loop is **stopped**. The currently authorized finite maintenance sequence is
harness consolidation, master integration, correctness baseline, results package, then stop. Results
refreshes do not authorize feature development. A later explicit start/resume request can authorize
bounded implementation slices and local commits; preparation alone cannot. No push, publication,
driver change or reboot is implied. Respect newer maintainer scope and stopping instructions.

Use one bounded fresh-context worker per slice when delegation is available. The lead owns scope,
acceptance and commits; one writer owns the checkout at a time. Read-only review can overlap; builds,
GPU suites and performance measurements cannot compete. If delegation is unavailable, record it and
use separate local audits without claiming independent review. Share at most four CPU workers on
this host; units use two servers. Bound long gates to 30 minutes and retain interrupted attempts.

## Establish and select

Inspect revision, working changes, submodule pins and actual loaded compiler/provider/cache bytes.
Use the platform-specific `slang-build` skill and matching RelWithDebInfo tools. Record toolkit,
device/driver, architecture, configuration and exact inputs. On host/toolchain/configuration changes,
run the smoke gate before a full frozen/discovery/material checkpoint. Reuse matching accepted
before-evidence; inherited results must keep their original source identity and never become fresh.

Correctness regressions take priority. Otherwise select one demonstrable slice from evidence of
breadth, severity, material relevance, scope and uncertainty. Aim for one material-driven slice per
three accepted implementations; explain justified correctness/infrastructure exceptions. A diagnostic
moving to another unsupported instruction alone does not justify a compiler change. Write a bounded
ExecPlan first, including exact acceptance domain, fresh/inherited cells and checkpoint triggers.

Trace complex workload -> producer/IR/consumer -> existing runnable fixture. Preserve application
shaders, inputs and independent oracles; NVRTC differential comparison is supplemental. Add boundary
coverage and reproduce failures before changing production. Keep frozen v1 immutable; discovery has
50–128 unique sources and excludes frozen overlap. Use documented runner extensions for unsupported
harness contracts instead of relabeling tests. Queue independent next blockers rather than extending
the slice indefinitely. Apply AGENTS' helper inventory and input-shape audit at the responsible layer.

## Accept without losing obligations

For a bounded compiler slice, require focused positive/boundary/negative tests and real GPU output
at NVRTC O3/NVVM O0/O3; relevant units and runtime smoke; toolkit/assembly for affected ABI/emission;
explicit neighboring frozen/discovery subsets; every registered material compile/assembly cell.
Explain why targeted coverage bounds the change. Review exact `(id, mode)` and five fields:
classification, return code, executed/passed/ignored counts, diagnostic and canonical shape.
Require one result per requested cell. Show additions separately; retain unresolved and resolved
failure history. Wrong output, crash, timeout, ignored or missing execution never becomes a pass.

Run full frozen/discovery/all-material checkpoints after three implementations, before a fourth;
also after shared lowering/type, ABI/provider/library or corpus-runner changes, uncertain impact,
host/toolchain/configuration transitions, and before publishing/releasing. The frozen inventory is
`census.slice-195.tsv`, never the census runner's unfiltered discovery default. STATUS separately
names last full, last targeted and implementations since full. Only accepted exact preservation resets
cadence. A stop instruction takes precedence over launching an otherwise unnecessary checkpoint.

The maintained `checkpoint` command wraps runtime smoke and all three corpora. Full acceptance also
requires units, semantic suites, toolkit and runner contracts listed in RESULTS. Exit codes alone are
insufficient: census can exit 2 for known gaps. `compare` refuses missing, duplicate, changed or false
passing cells. It emits `review-required` for differences; changed input hashes also require review.
Preserve that comparison. Resolve regressions, or prove an intentional upstream transition and update
its failure history in a separately reviewed `accepted-full` record. Never use failed/review-required
outcomes as a new baseline to erase the loss. Old raw logs are optional for comparison; checked-in
per-cell compact outcomes are not. A regression blocks subsequent feature work.

## Measure and report

Use optimized builds, fixed comparable inputs/options, warmups and repeated samples, with no competing
work. Separate fresh-process wall time, nested Slang phases, downstream assembly, bounded shared-session
lifetime and GPU execution. Fresh processes can share warmed filesystem/toolkit/PCH caches; retain
cache diagnostics and effective math/toolkit options. Do not sum nested phase timers. Failed compile
latency is not successful compile time. PTX/cubin bytes, registers, stack/spills, SASS and executable
sections are observations with explicit entry/module scope, not kernel speed. Material runtime needs
binding, texture/LUT/input and expected-output contracts before any runtime/performance claim.

Future accepted slices retain a compact five-part report (normally 1–2 pages), completed bounded plan,
one structured outcome/comparison/provenance record and relevant manifest/design deltas. Keep STATUS
short: current accepted state, known gaps, current action, authority, and links. Raw logs, repeated
samples, binaries, source snapshots and exhaustive indexes stay under ignored `build/`. Do not create
hundreds of checked-in raw-artifact references or copy unchanged historical prose into each slice.
Preserve exact per-cell outcomes and failure histories even when that data exceeds the prose budget.

Format changed files, inspect the final diff and confirm validation corresponds to final bytes before
local commit. Commit completed NVVM plans/reports under the explicit maintainer exception in AGENTS.
At boundaries update STATUS and the plan with remaining acceptance, exact next command and stop state.
