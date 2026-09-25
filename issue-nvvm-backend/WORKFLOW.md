# NVVM development slice workflow

Start with [STATUS.md](STATUS.md), then the active ExecPlan and the evidence it names. Follow
[AGENTS.md](../AGENTS.md) and [.agent/PLANS.md](../.agent/PLANS.md). This document defines the loop;
STATUS owns the current handoff, manifests own workload selection and results, and individual
plans/reports own implementation detail. Do not use chat history as the only source of a decision.

## Authorization and session boundaries

When the maintainer asks to start or resume this loop, autonomously select, implement, validate,
and commit bounded slices, then repeat. Do not ask for routine implementation or commit approval.
This does not authorize pushing, publishing, driver replacement, or rebooting. A request to prepare
or discuss the loop does not start it. Respect later scope changes or stop requests.

At a session boundary, update STATUS and the active plan with the exact next action. A checkpoint
is not accepted completion. A fresh session resumes unfinished acceptance before selecting another
feature. Commit completed plans and five-part reports with each NVVM slice; keep raw logs, generated
mirrors, binaries, IR dumps, and timing samples in ignored `build/` directories.

## Agent ownership and context budget

Run each bounded slice in one fresh-context subagent. The main agent owns slice selection,
acceptance, checkpoint cadence, and the final local commit. Give the worker explicit branch/base
revision, scope, plan/evidence paths, validation requirements, and stopping conditions; use
`fork_turns="none"` where supported rather than inheriting the full conversation. The worker owns
implementation, the living plan, tests, self-review, and draft report/STATUS updates. Only one
worker may mutate the checkout at a time; the main agent integrates after the worker finishes.
If delegation is unavailable, record that limitation and use the same bounded handoff locally.

Return a compact summary (normally at most 500 words): observable change, files changed, tested
source/binary identity, validation table and exact deltas, inherited evidence, unresolved risks,
and artifact paths. The main agent reads the relevant diff and evidence before accepting; it need
not replay the worker transcript. Keep raw logs and IR dumps on disk, retrieve narrow excerpts,
and report meaningful transitions instead of repeated unchanged polling output. Stop investigating
the next independent blocker once its diagnostic and minimal handoff are recorded.

## 1. Establish the starting state

- Read STATUS, the active plan, latest accepted result references, and recorded unresolved failures.
- Inspect branch, HEAD, working changes, submodule pins, and build configuration. Preserve unrelated
  user work. Record the source revision and any relevant uncommitted source changes for each run.
- Follow the `slang-build` skill for the current platform; use `docs/building.md` if unavailable.
  Do not assume the previous host, toolkit, GPU, or local `build/` files exist.
- Default to a `RelWithDebInfo` host compiler (native preset `releaseWithDebugInfo`). Use matching
  compiler, provider, test tools, and libraries from that configuration; audit local helper paths
  for stale Debug selections. Shader NVRTC O3 and NVVM O0/O3 coverage remains unchanged. Use
  targeted Debug/assertion checks when investigating invariants or when the affected layer needs
  them. Measure host-build speedups on a fixed subset rather than assuming their size.
- Run the small GPU runtime gate before expensive GPU suites. Record actual device, driver,
  architecture, compiler/provider/toolkit versions and hashes, and optimization settings.
- Before the first feature on a new host/toolchain/build configuration, establish full frozen and
  discovery results and a complex-corpus checkpoint. Classify environment differences before
  using them as a baseline; the initial switch from Debug to RelWithDebInfo needs this checkpoint.
  An old pass is still a preservation obligation, even if its old raw logs are unavailable.
- Reuse recorded before-change evidence when source, binaries, toolchain, runner, inputs, and
  environment match. Do not repeat full before runs for each slice. For a new reproduction or
  affected cell without matching evidence, run only the required before-change checks. Results
  inherited across intervening changes remain historical evidence, not a fresh baseline run.

## 2. Select one demonstrable slice

Correctness regressions take priority. Otherwise compare a short list of candidates using evidence:
workloads affected, severity, breadth of reusable support, relevance to the complex corpus, estimated
scope, and uncertainty. Record the selected candidate and why alternatives can wait.

Aim for at least one complex-corpus-driven slice in every three accepted feature slices. Prefer
items that also unlock runnable workloads. Record each slice's primary motivation in STATUS's
rolling three-slice history. Correctness or infrastructure work may override the cadence; record
why and reconsider the deferred material work next time. Do not force speculative changes merely
to satisfy a quota. Compile-time and code-quality work are eligible once their acceptance metrics
can be measured reliably; support completeness need not become an endless prerequisite for all
performance work on already runnable cases.

Write the bounded ExecPlan before implementation: motivating workload and canonical IR shape,
observable result, responsible layer, scope/non-goals, acceptance commands, and failure recovery.
Name the affected regression domain, exact fresh cells, inherited evidence, and full-checkpoint
triggers in the plan before implementation. One unsupported instruction becoming another diagnostic
is a useful observation, but is not by itself correctness evidence or justification for a compiler
patch.

## 3. Connect complex blockers to executable coverage

For each complex-driven feature, search existing `tests/` coverage first. Choose a test that reaches
the same producer/IR/consumer boundary, including the relevant resource representation and ABI.
Preserve its inputs and output oracle. NVRTC differential comparison supplements, not replaces,
independently expected output. Include boundary cases appropriate to the semantics.

Use existing manifest entries by reference. Add eligible new source contracts to
[discovery-corpus.manifest.tsv](discovery-corpus.manifest.tsv), preserving old IDs and recording
additions separately from preservation counts. Discovery excludes sources already in frozen v1;
if the needed shape is in an existing frozen source, keep its focused regression beside that source
or add a distinct focused fixture. Do not defeat overlap checks or silently expand frozen v1.
An explicitly selected source outside frozen v1 may already have a native CUDA directive; the
discovery runner normalizes that target and preserves the selected inputs and output oracle.
The explicit discovery manifest must contain 50 through 128 unique sources before filtering. Keep
capacity changes separate from additions, preserve all old source identities/oracles, and validate
runner-contract changes with a full checkpoint.
If the discovery runner cannot faithfully express the required harness, add a documented runnable
extension with explicit selection and expected results before counting it as corpus coverage.

Record the complex workload -> feature/IR shape -> runnable test mapping in the slice report, and
retain stable architectural facts in the design document. Verify the focused test fails before the
change and passes afterward at NVVM O0/O3. A reduced test proves that feature, not the complete
material's runtime behavior. Keep the application shader intact.

## 4. Implement at the responsible layer

Trace valid input from producer to consumer and apply the representation audit in AGENTS.md.
Prefer existing canonical operations and helpers; fix malformed representations at their producer.
Do not add source-text interpretation, fallbacks, default values, or shape-specific exceptions only
to move a diagnostic. Run focused tests while developing and update discoveries and decisions.

Reassess both complex entries after the feature works. Record each next blocker with its exact
mode/diagnostic and a minimal trace. Queue independent blockers for later slices. If a second issue
is inseparable from the first, revise the plan explicitly and keep the slice bounded. An experiment
that disproves its approach should be discarded with the finding retained, not promoted as support.

## 5. Accept against explicit preservation obligations

### Targeted slice acceptance

For compiler feature slices, run these gates after the last relevant source change:

1. Focused positive, boundary, and appropriate negative tests, with real GPU output for the selected
   runnable feature at NVRTC O3 and NVVM O0/O3.
2. Relevant NVVM/routing/reporter unit coverage and the small runtime smoke gate; toolkit/assembly
   checks when emission, lowering, ABI, provider, or library contracts are affected.
3. Explicitly selected frozen/discovery subsets covering the changed producer/IR/consumer boundary
   and neighboring supported behavior, in all three modes. Reassess every registered complex
   entry/backend/optimization cell for compiler changes; these compile/assembly probes remain
   support checks, not material runtime proof.
4. Review exact identity/mode deltas against the latest applicable accepted evidence. Require one
   result per requested cell, no duplicates or omissions, and actual executed/passed counts. Report
   additions separately.

Targeted acceptance is the default for a bounded change with understood impact. Record why the
selected domain covers that impact, which rows are fresh, and the source/configuration of each
inherited result. Do not imply unexecuted cells passed on the new source. If impact cannot be
bounded confidently, require a full checkpoint. Documentation-only work needs no compiler suite
and does not advance the slice cadence.

### Full-corpus checkpoints

Run the complete frozen and discovery runtime corpora in all three modes and every registered
complex cell after every three accepted implementation slices, before beginning a fourth.
For frozen checkpoints, pass `--workload-ids-from issue-nvvm-backend/census.slice-195.tsv`
to `run-compute-census.py`. Its unfiltered discovery default is not the immutable frozen inventory. Also
require a full checkpoint before accepting changes to broadly shared lowering/type handling, ABI,
provider or library contracts, corpus selection/execution/reporting, or any change with uncertain
impact.
Host/toolchain/build-configuration transitions require the starting checkpoint described above.
Require a full checkpoint of the final source before pushing, publishing, merging, or releasing;
the checkpoint itself does not authorize those actions.

STATUS must separately name the latest targeted acceptance, the last full checkpoint, and the
number of implementation slices since it. A full checkpoint resets that counter only after exact
preservation review succeeds. Compare against the last full checkpoint plus subsequent accepted
additions/fixes, retaining the original preservation obligations. A regression blocks further
slices: isolate and fix or revert the responsible change, then rerun affected checks and complete
the checkpoint. Never reset the baseline to conceal a loss. If a stop request arrives before a
checkpoint is due, record the outstanding cadence without launching extra slices.

Known unsupported cells mean a corpus need not be all green. A runner exit code alone is not an
acceptance decision: census diagnostic mode allows known preflight stops, while the complex runner
returns 1 for incomplete support. Inspect structured results and exact deltas. Missing or ignored
execution, timeouts, compiler crashes, GPU failures, and wrong outputs never become passes.

For every unresolved failure, retain workload ID/mode, category, first-known revision/environment,
reproduction, diagnostic, evidence, and whether it predates the slice. Link this record from STATUS.
Use manifests/reports as the ledger rather than duplicating full results in STATUS.

Fix or revert newly introduced correctness regressions before accepting a slice. Establish that an
unrelated discovered failure predates the change before deferring it. Keep existing failures visible.
Do not lower expected output, remove failing workloads, turn them into expected-error tests, or reset
the baseline to hide losses. A compiler crash mislabeled by a runner still needs investigation.
If a regression cannot be isolated/resolved within the agreed scope, preserve an unaccepted
checkpoint and stop with the specific decision needed.

Performance evidence uses optimized compiler builds, fixed comparable inputs/options, warmups,
repeated samples, and no competing builds/benchmarks. Separate process/session overhead, Slang
phases, downstream compile/assembly cost, and GPU execution. Timings of rejection are not successful
compile times; nested phase timers must not be summed. Register/stack/spill counts and PTX sizes
are observations, not proof of faster kernels. Before material runtime claims, obtain its binding,
texture/LUT/input, and expected-output contract. Measure semantic correctness before kernel speed.

## 6. Review and commit the accepted slice

Perform the helper/fallback inventory and input-shape self-review required by AGENTS.md. Format
changed files, check the staged diff, and confirm required tests correspond to the final source.
Commit implementation, tests/manifest additions, completed plan, five-part report, durable result
summaries, and STATUS together. Update the design document when architectural facts change.

STATUS names the latest accepted slice/evidence, not its own commit hash (which cannot be known
before committing). Record already-known source/tested revisions in results; after committing,
verify the commit contains exactly the intended files and working changes are accounted for.
Do not mark a blocked checkpoint accepted or leave an accepted plan describing an obsolete blocker.

## 7. Repeat or stop

Re-rank candidates from actual new results and continue an authorized loop. Ordinary diagnostic
failures, newly exposed unsupported features, and recoverable implementation mistakes do not require
human intervention. Do not begin a next slice merely because this session was asked to prepare it.

Stop for unavailable access/resources that cannot be restored within authorization, GPU/device loss,
missing application semantics necessary for correctness, a consequential unresolved design/scope
choice, or an unresolvable regression. Stop GPU dispatches on device loss; do not retry indefinitely
or reboot/change drivers without authorization. Record exact evidence, attempts, safe independent
work completed, and the smallest question/action needed to resume. Bound test runs and investigate
stalls; an interrupted command remains incomplete.

## Native Linux command reference

Run from the repository root. STATUS records the current paths and local helper. Inspect that
helper before sourcing it: the slice-202 version selects Debug. Override its build paths using
the environment block below before tests. After build setup, the native optimized build command is:

```bash
cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server
```

Select the installed toolkit/provider explicitly; the current host uses these values. Missing tools
require setup following the build skill and
[provider README](../source/slang-llvm-nvvm/README.md), not unrecorded fallback to another toolkit.

```bash
export CUDA_PATH=/usr/local/cuda-12.9
export CUDA_HOME="$CUDA_PATH"
export LIBNVVM_HOME="$CUDA_PATH"
export SLANG_NVVM_TEST_ARCH=80
export SLANG_NVVM_BUILDER_PATH="$PWD/build/RelWithDebInfo/bin"
export PATH="$CUDA_PATH/bin:$PWD/build/RelWithDebInfo/bin:$PATH"
export LD_LIBRARY_PATH="$CUDA_PATH/nvvm/lib64:$CUDA_PATH/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
```

Choose a new evidence directory for each source state; do not overwrite accepted raw evidence.
The commands below are the full-checkpoint reference, not a per-slice checklist. Run the applicable
gates individually and record each exit code, including expected diagnostic failures.
Use bounded supervision (for example native Linux `timeout --kill-after=30s 30m` around each suite).
A timeout needs investigation and cannot satisfy acceptance.

```bash
NVVM_RUN=build/nvvm-loop/relwithdebinfo-baseline
mkdir -p "$NVVM_RUN"
python3 extras/validate-nvvm-runtime.py --config RelWithDebInfo \
    --cuda-path "$CUDA_PATH" --architecture 80 --output "$NVVM_RUN/runtime"
build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 2 -disable-retries \
    slang-unit-test-tool/nvvm slang-unit-test-tool/cudaEmissionMethod \
    slang-unit-test-tool/invalidCUDAEmissionMethod slang-unit-test-tool/testServerIgnore \
    slang-unit-test-tool/slangTestReporter
python3 extras/validate-nvvm-toolkit.py \
    --slangc build/RelWithDebInfo/bin/slangc --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so \
    --cuda-root "$CUDA_PATH" --expected-toolkit 12.9 --architectures 80 \
    --output "$NVVM_RUN/toolkit"
python3 issue-nvvm-backend/run-compute-census.py --config RelWithDebInfo \
    --bin-dir build/RelWithDebInfo/bin --provider build/RelWithDebInfo/bin --architecture 80 \
    --workload-ids-from issue-nvvm-backend/census.slice-195.tsv \
    --jobs 2 --output "$NVVM_RUN/frozen"
python3 issue-nvvm-backend/run-compute-discovery.py --config RelWithDebInfo \
    --bin-dir build/RelWithDebInfo/bin --provider build/RelWithDebInfo/bin --architecture 80 \
    --manifest issue-nvvm-backend/discovery-corpus.manifest.tsv \
    --jobs 2 --output "$NVVM_RUN/discovery"
python3 issue-nvvm-backend/run-complex-corpus.py \
    --slangc build/RelWithDebInfo/bin/slangc --build-label RelWithDebInfo \
    --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so --cuda-root "$CUDA_PATH" \
    --warmup 0 --samples 1 --output "$NVVM_RUN/complex"
```

For targeted replay, select frozen rows with `--workload-ids-from` using an explicit subset TSV
(or `--match` / `--match-regex`), and discovery rows with `--match` or a separate subset manifest.
Retain the resolved identity list in the evidence and keep the authoritative full manifests intact.
Record all three modes explicitly in the result inventory; do not infer domain coverage from a
name filter alone. Run focused new fixtures directly with the matching `slang-test`.

The one-sample complex command checks support, not performance. Frozen/discovery `results.json`
contain rows keyed by `(id, mode)`; compare those keys exactly and retain transitions and counts.
Complex `results.json` retains its workload/entry/backend cells and status. Require the requested
inventory even when a command returns success. Use `--require-all-correct` only for explicitly
supported runtime subsets; unsupported full-corpus cases remain measured gaps.

For each accepted slice, preserve a compact checked-in result set (including per-cell outcomes and
baseline/evidence references), the comparison, and toolchain provenance. Full raw logs stay local.
The next session must be able to compare results even if a previous machine's `build/` is gone.

## Execution efficiency and runner follow-ups

Use one shared concurrency budget across builds and all test suites. On the current four-CPU host,
start with at most four active CPU workers in total, reducing this if GPU or memory contention
appears. Run suites sequentially or divide that budget between them; do not give each overlapping
runner its own four-worker allowance. Performance measurements run without competing work.

Prefer compact, machine-generated acceptance summaries containing provenance, expected/observed
cell counts, missing/duplicate cells, fresh/inherited counts, exact outcome transitions, and gate
status. Keep full raw evidence on disk. Until a checker automates all of these, explicitly verify
the missing checks; a successful runner exit does not substitute for acceptance review.

Persistent test-server batching is a future runner improvement, not an existing capability of the
corpus runners. Measure startup costs before implementing it. Any batching change must preserve
per-cell identity, output oracles, timeouts, crash isolation, and coverage of fresh-session behavior.
Starting a new test-server process for each cell does not amortize startup. Validate runner changes
against a full checkpoint before relying on their results.
