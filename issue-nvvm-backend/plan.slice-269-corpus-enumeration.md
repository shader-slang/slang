# Repair corpus enumeration and broaden language coverage

This bounded ExecPlan follows `.agent/PLANS.md`. It is the second and final slice authorized by
"fix the float3 issue followed by steps1 and2". 268 was accepted and committed as5f9364247. Complete
one reviewed local commit, record the user-requested notification skip, then stop. No general-loop resume,
autodiff pilot, compiler-feature extension or push.

## Purpose and Observable Result

The census currently misses valid test directives written `// TEST` or with extra slashes. Make
census and discovery selection agree on harness-valid directives and their source indices, while
preserving every frozen ID, selected contract and oracle. Qualify five audit-identified CUDA
backfills and four core-language cases in all three modes, increasing discovery119→128 and
combined571→580 cases (1713→1740 mode cells), without changing frozen v1.

## Progress

- [x] Accepted268 is5f9364247, clean closeout, compiler07881e5f and providerfbef1a9e unchanged.
      The268 notification was auto-review rejected; the user then explicitly requested skipping
      both268/269 Slack notifications. Record that override in closeouts; it does not block269.
- [x] Shared native-index enumeration and regression contracts implemented. All452+119 old
      selected lines/arguments/source hashes/oracle paths+hashes preserved;83frozen and3discovery
      ordinal changes reviewed, frozen manifest unchanged. Independent candidate review passed.
- [x] All five backfills and four prescribed breadth cases pass27/27 after supplying existing
      numerics module prerequisite; four added modules, zero preexisting installed artifact changes.
- [x] Final parser/contract/focused evidence independently approved; full checkpoint and all side gates
      passed on2026-09-26. All old1,713 cells preserved;27 additions separately reviewed. Original
      additions-only review-required comparison retained.
- [x] 2026-09-26: Independent final review approved by corpus_review269; lead accepted record.
      Report, formatting and exact byte checks complete. This completed plan accompanies the
      closing local commit; both notifications skipped and the general loop remains stopped.

## Surprises and Discoveries

The previous audit is under `build/nvvm-coverage-audit`. It counted514 policy-eligible CUDA files,
509 already selected; historical exclusions are not support declarations. The two268 regression
fixtures add focused coverage and stay separate from the nine selected discovery additions.
The actual harness counts disabled TEST and DIAGNOSTIC_TEST entries in its subtest indices;
old Python runners count only strict active TEST. Preliminary read-only audit finds83 frozen
ordinal changes, with no expected-sidecar path/hash changes, and three discovery ordinal changes.
`pre269-selected-contracts.json` preserves all452+119 old selections before any parser edits.
These were preliminary findings; the repaired-parser comparison confirms all contracts/oracles
unchanged with83/3 index-only changes.

The first candidate run passes eight cases in all modes but the fixed numerics backfill fails all
three with E20001 importing the absent standard numerics module. The existing CMake
`slang-numerics-modules` target was not built in this layout. Preserve that attempt, inspect its dry
run and supply this existing prerequisite without changing compiler bytes or fixture source. This
is packaging qualification for the fixed backfill, not permission for an autodiff pilot.

## Decision Log

- The user explicitly requested skipping the268 and269 Slack notifications after automatic review
  rejected the268 send. This overrides standing notification policy for this finite sequence.

- Execute this corpus-only slice after the independent268 compiler repair; keep compiler bytes fixed.
- One fresh writer owns Python/manifests and all execution; lead owns acceptance/docs/commit.
  Read-only reviewer may overlap; builds/GPU gates are serialized, max4CPU and2unit servers.
- Preserve frozen v1 byte-for-byte. Migrate discovery ordinals only to preserve the same selected
  directive, argument/oracle contract. Every changed ordinal is reviewed rather than silently reused.
- Add new cases only after all three modes produce the unchanged independent oracle. Record any
  independent blocker without unrelated compiler edits; use the next audited breadth candidate.
- Lead authorizes building/staging existing numerics module artifacts after a dry run proves no
  compiler rebuild; capture old/new layout and module hashes. Any compiler rebuild needs separate
  lead review. The required smoke/full gate qualifies the added artifacts.
- Corpus-runner changes require a full checkpoint. Preserve the original review-required comparison
  for additions; review them separately with `compare --allow-additions` before explicit acceptance.

## Outcomes and Retrospective

All nine cases pass all three modes. Full outcomes are 1,701 correct and 39 unchanged gaps across
1,740 cells; 18 resolved histories remain intact. Units and semantics preserve exact maps; runtime4,
toolkit18, material6 and runner contracts pass. The original 567 source hashes are unchanged and nine
are added. Compiler/provider bytes are unchanged; only four missing numerics modules were supplied.
The parser now selects native authored ordinals and preserves every prior source/oracle contract.
Final record review approved acceptance on2026-09-26. The closing local commit completes this finite
sequence; both notifications are skipped at
user request and the general loop stays stopped.

## Context, Scope and Invariants

Read AGENTS, WORKFLOW, STATUS and RESULTS. Existing runners are
`run-compute-census.py`, `run-compute-discovery.py`; inventory is
`census.slice-195.tsv` (immutable) and `discovery-corpus.manifest.tsv` (119sources before expansion).
Share one directive interpretation instead of duplicate regular expressions. The actual reference
is `_gatherTestsForFile` in `tools/slang-test/slang-test-main.cpp`, including leading comment spaces,
extra slashes, disabled entries, diagnostics and whole-file ignore. Do not confuse authored tests
with synthesized host/API variants. Preserve non-target arguments, source text and filecheck oracles.
No tests may become a pass through missing execution, skipping, source simplification or retries.

## Milestones and Execution

1. Save initial identities and old selection contracts; run meaningful parser regression cases that
   fail before the fix. Compare old/new source, directive, categories/arguments, resolved expected
   files/hashes and all452 frozen IDs. Explain any ordinal migration.
2. Backfill `cuda/nvvm-aggregate-param-snapshot.slang`,
   `cuda/nvvm-aggregate-param-resource-snapshot.slang`, `cuda/wave-lane-index-multidim.slang`,
   `hlsl-intrinsic/wave-prefix-count-bits-cuda.slang`, `numerics/differentiable-from-scalar.slang`.
3. Qualify four breadth candidates: interfaces/conjunction-assoc-type,
   generics/assoc-type-default-init (retain dynamic-dispatch scaffolding), enums/enum-array-indexing,
   bitfield/default-init-mixed (default-layout contract). These paths are under language-feature.
   If an independent blocker appears, preserve evidence and qualify the next audited candidate,
   starting with switch-fallthrough/fallthrough-loop-interaction. Retain source and existing oracle.
4. Run maintained checkpoint against validation268 with frozen1356 and expanded discovery384,
   runtime4/material6, exact units/semantics, toolkit18 and runner contracts. Bound each long gate
   to30minutes, no competing workloads. Review old1713 cells exactly and27 additions separately.
5. Build accepted269 from complete outcomes; attach the nine new source hashes explicitly, since
   current checkpoint provenance derives runtime input keys from the previous baseline. Preserve
   old567 hashes,39 unresolved and18 resolved histories plus inherited reliability incidents.
   Record compiler identity as inherited268 and runner/manifest identity as fresh269.
6. Update compact report, STATUS/HANDOFF/HISTORY and durable manifest rationale. Review final bytes,
   commit completed plan/report/evidence and runner changes, record the user-requested notification skip, then stop.

## Validation and Acceptance

Contract tests must cover whitespace/extra slashes, disabled and whole-file-ignored exclusions,
indexing across non-compute/diagnostic/disabled directives, and correct indexed expected-file copying.
Preserve manifest capacity50–128, unique sources/no frozen overlap, target adaptation and all existing
classification/result contracts. Old cells must match id/mode and all five outcome fields. New cells
must all execute and pass NVRTC O3/NVVM O0/O3. Do not claim language coverage percentages from paths;
state concrete demonstrated features and distinguish selected cases, source files and mode cells.

## Failure, Recovery and Artifacts

Keep every failed/review-required attempt under a unique ignored `build/nvvm-corpus269` path.
Never promote an unreviewed checkpoint or erase failures. Stop on an introduced regression until
resolved. Independent feature blockers remain reported evidence, not authorization for feature work.
Retain one structured accepted ledger, compact five-part report, completed plan and manifest deltas.
Raw logs/source snapshots stay ignored. Closing commit and notification skip go in the ignored closeout.
