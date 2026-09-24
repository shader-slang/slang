# Make the NVVM slice loop restartable

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires completed NVVM plans and reports
committed with each bounded slice. Raw logs and generated binaries remain under ignored `build/`.

## Purpose and Observable Result

A new session can read `WORKFLOW.md` and `STATUS.md`, identify the accepted evidence and pending
work, and begin one bounded NVVM slice without reconstructing chat history. The maintainer asked
for three ordered checkpoints: commit the complex corpus, reconcile slice 201 acceptance, then
commit the loop instructions. This task prepares the loop; it does not start another feature.

## Progress

- [x] 2026-09-24: Commit the complex corpus and assessment as `63c118b8d`.
- [x] 2026-09-24: Start slice 201 acceptance on the replacement L4/CUDA 12.9 host.
- [x] 2026-09-24: Commit slice 201 reconciliation as `187d87148`, with all 183 wave cells,
      zero regressions, explicit inherited evidence, and the preserved interrupted attempt.
- [x] 2026-09-24: Write WORKFLOW/STATUS, five-part report, and AGENTS/design entry points.
- [x] 2026-09-24: Verify runner CLI options, all relative links, accepted identity/mode counts,
      evidence hashes, formatting and whitespace; review the three separate commit scopes.

## Surprises and Discoveries

The old slice 201 plan names a failed A6000 and CUDA 13.4. This checkout now has a working L4 and
CUDA 12.9.2. The old raw slice-200 results are not present here; durable slice-195 per-identity
rows and the slice-200 summary remain available. Handoff records must distinguish these sources.
Git has no local author configuration; use the existing branch author identity via per-command
`git -c`, without changing global configuration.

## Decision Log

- 2026-09-24: Follow the maintainer's requested commit order. Reconcile the existing slice rather
  than opening another feature slice. Do not automatically resume the feature loop in this session.
- 2026-09-24: Use one stable workflow and one small current status document pointing to authoritative
  manifests. Keep scope, commands, and evidence in each bounded ExecPlan.
- 2026-09-24: Preserve frozen identities; expand runnable coverage through the existing discovery
  manifest where its contract permits. Complex compile success never substitutes for GPU correctness.

## Outcomes and Retrospective

The requested ordered checkpoints are complete; the workflow records accompany the final
documentation commit.
Slice 201 is accepted; all 165 prior-correct wave cells survive and both direct prefix-count
cells now pass. WORKFLOW/STATUS prepare a new session to establish the full current-host baseline
and select slice 202. No next feature was started. See the completed report for policy decisions.

## Context and Current Pipeline

The frozen runner selects 452 identities; the rolling discovery manifest selects 82 additional
sources. Both execute NVRTC O3 and direct NVVM O0/O3. The separate complex runner compiles and
assembles both material entries in those modes. The first complex blocker is the canonical
`CastUInt64ToDescriptorHandle` operation. The accepted next slice has not been selected.

## Scope and Non-Goals

Own workflow and handoff documentation, completed slice 201 acceptance records, and links from
repository/design instructions. No compiler feature, provider ABI, benchmark policy shortcut,
shader workaround, or change to historical corpus denominators is included.

## Architecture and Invariants

Each result belongs to an exact workload, mode, source revision, environment, and evidence origin.
Missing/skipped/interrupted results are never passes. A slice cannot accept a new correctness
regression by changing expected output, deleting a row, or silently replacing its baseline.
A session boundary is a checkpoint; only the defined conditions block an authorized loop.

## Interfaces and Dependencies

Reuse the existing census, discovery, complex, toolkit, and runtime tools. Native Linux commands
use `build/Debug/bin` and the configured isolated LLVM14 provider. The local environment helper
under `build/nvvm-setup` is a convenience, not a prerequisite distributed by Git; document an
explicit environment fallback and link the build/provider references.

## Milestones

1. Commit complex assessment independently.
2. Complete the existing wave acceptance and record exact-identity comparisons and focused gates.
3. Document selection, runnable feature coverage, implementation, acceptance, commit, repeat/stop,
   and restart procedures. Keep current candidates distinct from authorization to start a slice.
4. Format, review, and commit the final workflow with its own completed report.

## Validation and Acceptance

Run the 61-identity wave subset in all three modes and focused GPU/unit gates for slice 201.
For documentation, check every referenced file, verify CLI options against actual runner help,
check formatting and `git diff --check`, and ensure the status identifies a concrete next action.
Do not rerun unchanged compiler suites solely for documentation edits.

## Failure and Recovery

Record infrastructure failures separately and stop GPU work on device loss. Preserve interrupted
attempts without overwriting historical acceptance. If a new compiler regression appears, diagnose
it before claiming slice completion; keep the workflow capable of resuming that exact state.

## Artifacts and Hand-Off

Commit `WORKFLOW.md`, `STATUS.md`, links, this plan and its five-part report after the separate
slice 201 reconciliation commit. Keep local logs under `build/nvvm-slice201-reconcile`. End with
commit IDs and the short instruction the user can give a new session to start the loop.
