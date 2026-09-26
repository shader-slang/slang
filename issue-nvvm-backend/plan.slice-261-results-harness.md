# Consolidate the NVVM results harness and continuation workflow

This ExecPlan follows `.agent/PLANS.md`. The maintainer authorized the finite sequence: consolidate
reusable measurement/results tools and documentation, merge origin/master, validate the merged
baseline, and produce a refreshable Monday results package. Do not enter the general development
loop afterward. Local commits are authorized; no push. Send one completion Slack DM to skallweit
if available. Completed NVVM plans remain committed for this maintenance handoff.

## Purpose and Observable Result

A fresh session can reproduce correctness comparisons and material/code-quality results through
maintained commands, without chat history or scripts in an earlier slice's ignored build directory.
STATUS is concise; historical documents remain immutable and discoverable. This slice ends with a
validated harness and documentation commit. Master integration and presentation measurements each
have their own bounded plan and commit, so regressions can be attributed to their responsible change.

## Progress

- [x] Read workflow, current status, planning instructions, build skill and Slack skill.
- [x] Establish accepted260 base e673f646d0b493d7b87d1bb7c078c483595d170f and clean checkout.
- [x] 2026-09-26 Implement maintained five-command harness and11 acceptance-contract tests.
- [x] 2026-09-26 Replay all1713 accepted260 outcomes; reject missing/duplicate/changed inputs, false passes, inventory truncation and rejected-baseline laundering.
- [x] 2026-09-26 Condense STATUS/WORKFLOW; add HISTORY/HANDOFF/RESULTS and compact-report policy.
- [x] 2026-09-26 Complete independent CPU/tooling review and repository formatting.
- [ ] Parent accepts and commits maintenance slice.

## Surprises and Discoveries

The invoked Slack CLI skill is read-only, but an installed Slack app exposes user lookup and message
sending. Only the explicitly requested completion DM is authorized. The general loop stays stopped.

## Decision Log

- Parent: preserve old reports/raw evidence and their paths; no historical evidence migration or
  compiler file splitting. Consolidate active navigation and future reporting instead.
- Parent: require exact identities, executed counts, diagnostics and canonical shapes in comparisons.
  Timing uses warmups, alternating order, every sample, explicit compilation/assembly boundaries.
- Parent: one source writer at a time. Other agents may review or investigate read-only.

## Outcomes and Retrospective

Harness and docs implemented;11 new tests pass, discovery6 pass, complex14pass/1skip. Historical257
report replay validates existing132compile/66assembly artifacts; no fresh timing/GPU execution.
Parent final review/commit remains. Full260 remains the accepted compiler checkpoint:1713 cells,1674 correct,
39 unresolved,18 resolved histories; provider ABI42. No compiler behavior change is planned here.

## Context and Current Pipeline

Existing runners are issue-nvvm-backend/run-compute-{census,discovery}.py and run-complex-corpus.py.
Slice257's measure.py and summarize-measurements.py implement the proven repeated timing protocol;
slice260's comparison/capture/audits establish preservation and provenance. Reuse these contracts,
not their fixed slice numbers. Keep the existing frozen inventory census.slice-195.tsv authoritative.

## Scope and Non-Goals

Reusable Python tooling, meaningful contract tests, concise workflow/status/history navigation and
results regeneration instructions. No compiler optimization, support expansion, changed oracle,
workload deletion, source reorganization, driver change or publishing.

## Architecture and Invariants

One maintained entry point with explicit compare, checkpoint, material benchmark, corpus quality
and report modes. Preserve raw runs in unique ignored output directories; checked-in compact results
name exact revisions, binaries, inputs, tools, device, settings and limitations. Missing artifacts,
duplicate cells, unexplained changes and incomplete commands cannot become passes. Historical raw
logs may be unavailable; durable compact outcomes remain sufficient for corpus comparison.

## Interfaces and Dependencies

Native Linux; RelWithDebInfo; CUDA12.9.2; LLVM14 provider; L4; initial SM80 target. Existing binaries
under build/RelWithDebInfo. At most4 total CPU workers,2 unit servers; sequential GPU suites and
benchmarks with no competing build/profiling. Read the installed slang-build skill before building.

## Milestones

1. Extract reusable harness, test comparisons on accepted260 and adversarial result fixtures.
2. Document reproducible commands and replace STATUS history dump with current handoff plus index.
3. Parent reviews and commits. Then create plan262 for master integration, followed by plan263 for
   the results package. Those steps are finite task milestones, not renewed general-loop authority.

## Validation and Acceptance

Runner contract tests cover missing/duplicate cells, changed outcomes, warmup/sample inventory,
failed processes, and provenance. Existing discovery/complex runner tests remain passing. Replay
accepted260 outcomes with zero differences; inspect all new helpers. No new compiler test suite is
necessary until master integration, which requires a rebuilt full checkpoint.

## Failure and Recovery

Retain failed attempts; never overwrite closed evidence. If merging master introduces regressions,
fix or explicitly resolve them before freezing the results baseline; do not conceal losses. A blocked
external dependency is documented with exact next action. Preserve user changes and existing history.

## Artifacts and Hand-Off

Maintained harness under extras or issue-nvvm-backend, reusable results guide, archive index, updated
AGENTS/WORKFLOW/STATUS, compact maintenance report and this completed plan. Raw maintenance evidence
under build/nvvm-maintenance. STATUS must end the whole sequence with development loop stopped and
commands for an explicitly authorized future development or results-refresh session.

## Final maintenance evidence

- `build/nvvm-maintenance/slice-261-final-replay260/{comparison,outcomes}.json`: exact historical replay.
- `build/nvvm-maintenance/slice-261-historical257-matplotlib-report`: historical report regeneration.
- Reviewer findings fixed: missing lib/cache hashes, truncated authoritative inventory, NaN durations,
  exact-one execution contract, rejected-baseline laundering, manual-gate environment.
- Parent owns accepted-full promotion after full262 gates; RESULTS defines the compact schema.
- No compiler/source fixture changes or new GPU correctness/timing claims. General loop stays stopped.
