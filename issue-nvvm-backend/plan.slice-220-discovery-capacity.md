# Expand explicit discovery capacity without changing selection

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception. Fresh-context
workers remain unavailable at the app's agent-thread limit; parent local execution uses WORKFLOW's
fallback. Generated manifests, binaries and raw logs stay ignored under `build/`.

## Purpose and Observable Result

Accept explicit discovery manifests of 50 through 128 sources while preserving every existing
selection, oracle, overlap and duplicate rule. The checked-in manifest remains exactly 100 entries;
this slice creates room for the validated FP64 admission fixture without silently expanding workload
selection. A complete runtime checkpoint must preserve all 1,656 current cells exactly.

## Progress

- [x] Inspect loader count boundary, filtering order and existing contract tests.
- [x] Select bounded infrastructure change on clean base `c66d533b5b030220abe6ca2411049b68b3365e26`.
- [x] Capture old selection and contract-test behavior, add explicit count-boundary regressions.
- [x] Raise only maximum capacity, keep duplicate test independent of current manifest length.
- [x] Run Python contract gates, GPU smoke and full frozen/discovery/complex checkpoint.
- [x] Review exact outcomes/provenance, complete report/STATUS/evidence and local commit.

## Surprises and Discoveries

The existing duplicate-source test appends an entry to the live manifest. At its current size of
100, that creates 101 entries and reaches the count error before duplicate detection. Keep the
fixture at a valid size by replacing its final row with the first; this tests duplication independently
of capacity. Record the before-test failure rather than confusing it with a compiler regression.

## Decision Log

2026-09-25: Choose capacity 128, preserving minimum 50 and a finite explicit bound. No new manifest
entries, selection heuristics, command options or automatic source discovery. This runner-contract
change triggers a full checkpoint and is also the third implementation since full 214.

## Outcomes and Retrospective

Accepted locally on 2026-09-25: explicit capacity is 50 through 128; the real manifest remains
100 entries. All 1,656 runtime cells freshly preserve their accepted outcomes (1,603 correct,
53 retained failures); six material compile/assembly cells pass. No new backend support is claimed.
Full checkpoint 220 resets implementation cadence to zero.

## Context and Current Pipeline

`run-compute-discovery.py::_load_discovery_workloads` checks manifest size before validating unique
source identity, frozen-v1 disjointness, source existence, selected active compare-compute directive
and required semantic tags. CUDA normalization preserves source oracle arguments. Filtering occurs
later in `main`; it must not bypass manifest validation. The sole production behavior change is the
maximum count. Shared constants keep validation and diagnostics consistent.

## Scope and Non-Goals

Runner count constants/check, count-boundary tests, stable duplicate fixture and workflow/design
notes plus plan/report/evidence/STATUS. No compiler/provider/ABI/library/source fixture/oracle or
manifest change. No builds, FP64 admission, timing optimization, material execution or system changes.

## Architecture and Invariants

Every supplied row still becomes one unique explicitly selected source contract or causes rejection.
Both capacity bounds apply before filtering. Positive tests exercise real synthetic source files and
loader outputs at 50, 100, 101 and 128; 49 and 129 reject. Existing native/non-native CUDA normalization,
frozen overlap and duplicate checks remain intact. Before/after full real selection metadata must
be byte-equivalent after deterministic serialization.

## Interfaces and Dependencies

Native Ubuntu, L4 SM89 target SM80, driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, ABI 36.
Source `build/nvvm-loop/slice-203-env.sh`; follow local slang-build skill. Unchanged RelWithDebInfo
compiler `a13354a47acefc8684cbfb5ae52b84bcf0331a83e5310e9675f53a0e030107d7` and provider
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`. Four corpus workers maximum,
sequential suites. No compiler rebuild is necessary when source/artifact identity matches 218.

## Milestones and Validation

1. Save deterministic loader output for all 100 real entries. Run existing contracts and retain any
   old apparatus failure. Add positive/negative boundary cases; verify 101/128 fail before the change.
2. Raise maximum to 128 via one shared constant pair and diagnostic, then run all discovery contracts
   and relevant existing runner/reporter/protocol checks. Verify real selection/oracle metadata and
   manifest bytes unchanged. Format explicit changed paths and audit any unrelated formatting.
3. Run GPU smoke 4/4 before large suites. Execute full frozen 452 identities x3=1,356 cells and full
   discovery 100x3=300 cells with four workers, then all six material compile/assembly cells. Reuse
   unchanged compiler unit 478+skip and toolkit 18 evidence; fresh runner contracts cover the change.
4. Compare every requested key and all five stable fields against full 214 plus accepted 216/218
   additions/fixes. Expect exactly 1,603 correct and 53 unchanged failures, no additions/removals,
   no inherited runtime rows. Preserve all first-known histories and four resolved records.

## Preservation and Acceptance

A full checkpoint is mandatory because the selection-validation contract changes. Acceptance resets
implementation cadence to zero and names 220 the latest full checkpoint. Compiler/provider units and
toolkit may inherit 218 only with exact source/artifact hashes. Runner exits are not pass decisions:
known infrastructure/output/preflight records remain failures. Source shader contracts, IDs, filtering
and oracle arguments must remain exact. Research 215/217 resolutions remain separate; research 219
only validates the FP64 CUDA source, not direct support. Material remains compile/assemble only.

## Failure and Recovery

Keep raw failures and reporting apparatus corrections separate. New regressions block acceptance;
fix the responsible runner change or revert it without resetting the baseline. GPU loss stops new
GPU work without driver changes/reboot. Sandbox bwrap requires approved escalated commands. No push.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-220-{before,after}`. Durable plan/report, full runtime manifest/census,
runner/tests, design/workflow note and STATUS. Parent performs local acceptance under the recorded
agent-thread limitation, then resumes bounded FP64 min/max admission with a dedicated fixture.

2026-09-25: Original contracts expose the valid-size duplicate-fixture issue. Final new boundary
tests fail before production change at 101/128 and diagnostic range assertions; all six pass after.
All 100 real selected records and manifest bytes remain identical. Final source/artifact identity
is frozen before the sequential full checkpoint; no compiler rebuild is needed.

2026-09-25 apparatus correction: the initial census invocation omitted `--workload-ids-from`
and selected 525 discoverable contracts. Stopped only its owned gate script/process group; retained
partial output under `excluded-unfrozen-partial`. No partial result counts toward acceptance.
Corrected command explicitly selects `census.slice-195.tsv` (452 immutable identities), reruns GPU
smoke after termination and then the complete checkpoint. Successful contract gates remain valid
on identical source and are not repeated. Original command/provenance snapshots are preserved.

2026-09-25 checkpoint progress: all 1,356 corrected frozen cells independently match full 214
plus accepted 218 outcomes across all five stable fields (1,333 correct, five infrastructure,
18 preflight). The previous 1,035 inherited frozen cells are now freshly validated. Discovery and
material gates subsequently completed and are included in final acceptance below.

2026-09-25 final acceptance: discovery 300/300 cells preserve all five fields, with 270 correct and
30 retained failures. Material compile/assembly 6/6, runtime smoke 4/4, discovery contracts 6/6,
routing/reporter 32/32 and protocol 15/15 pass. Units 478 plus one skip and toolkit 18 explicitly
inherit unchanged 218 evidence. All 1,656 runtime cells are fresh; 53 first-known failure records
and four resolved histories are retained. Parent verified 117 evidence references, 22 tested sources,
12 artifacts and 548 runtime inputs. No baseline reset or hidden partial-run evidence. Proceed to
local commit, then bounded FP64 min/max admission with its separate discovery fixture.
