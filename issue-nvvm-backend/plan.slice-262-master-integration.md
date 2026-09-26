# Integrate master before freezing the NVVM results baseline

This bounded ExecPlan follows `.agent/PLANS.md`. It is the second step of the maintainer-authorized
finite sequence: harness consolidation, master integration, results package, then stop. It does not
authorize the general development loop, pushing, publishing or system changes.

## Purpose and Observable Result

The nvvm-backend branch includes origin/master at 6eb89786ca882d71049c8568638e247f60864b6f, retains
qualified NVVM support and a rebuilt full correctness checkpoint, and provides a named revision for
presentation measurements. Every changed result must be explained against accepted260.

## Progress

- [x] Fetch and identify upstream. Merge base30d3e0f3e8c1fae72826988aa8576db344683834; upstream207 commits.
- [x] Independent integration audit identifies12 text conflict files and14 changed runtime fixture paths; actual merge adds one modify/delete conflict.
- [x] Complete harness consolidation, committed as7395e6114 before the merge.
- [x] Resolve13 conflicts and migrate removed version-query API; merge committed5294b5ae6.
- [x] Update all22 submodules to exact pins; official archive/API recovery verifies3 commits/trees.
- [x] Rebuild matching compiler/provider/test tools; configure passed, initial build reached688/1089 then found one
      remaining coexistence-parent helper call. Its retained-set argument is fixed; retry passed.
- [x] First smoke passed. Initial checkpoint stopped because cached version string named old setup.
- [x] Refresh cached CMake metadata and rebuild:2026.18.3-275-g49593da72.
- [x] Full checkpoint: all1674 correct outcomes retained, frozen1356 exact, six intentional upstream
      rejection transitions in existing unresolved texture cells. All6 material support cells pass.
- [x] Full units:1086 pass13 skip; independent identity audit preserves1062 common statuses and records37 additions/two API replacements.
- [x] Semantics1170 pass78 skip; focused29; toolkit18; AST705 tags/497025 pairs; all runner contracts pass.
- [x] Fix compiler-error classification after3426-log replay and30-case contract check (c3455e606).
- [x] Fresh full checkpoint after runner change completed; compiler bytes remain49593da72.
- [x] One NVRTC PCH deletion incident closed for serial correctness by3 declared rounds/9 passing cells; concurrent reliability remains open.
- [x] Run full compiler/semantic/runtime/toolkit/contracts/frozen/discovery/material gates.
- [x] Lead and independent review accept the explicit composite checkpoint in runtime-validation.slice-262.json.

## Surprises and Discoveries

Upstream adds automatic NVRTC PCH support and precise-mode --fmad=false. Its output hashes and
performance cannot be assumed identical to premerge evidence. Upstream introduces public option
IDs158/159 and stable IR IDs902/903 that collide with branch additions. No changed registered fixture
alters its TEST_INPUT data or expected buffer output in the read-only upstream audit; resolved merge
files must be checked again. Upstream expands compiler unit inventory and changes submodule pins. The actual merge additionally
finds deletion of the old compiler-version CLI fixture. Upstream replaces its public API with a
compiler-path query; the NVVM adapter and two branch unit calls need matching migration. Initial
new dependency clones failed with HTTP503. Official GitHub archives/API reconstructed exact commit
and tree objects for Catch2/json/replxx; strict fsck passed. These3 checkouts have shallow history.
All22 submodules are clean and at their pinned commits; no gitlink substitutions were made.

## Decision Log

- Preserve upstream published option and instruction IDs, assign unused IDs to NVVM-only additions,
  rebuild all generated/module caches and cover routing/serialization.
- Remove the unused historical E52014 NVVM placeholder (no callers since slice6) to preserve the
  upstream interpreter diagnostic. Preserve active NVVM E52015–18 IDs and fail-closed diagnostics.
- Retain upstream's complete immutable-load classification, including constant-parameter-group
  exclusion and cast/offset peeling. Remove the redundant local classification helper; keep tests.
- Preserve upstream texture float3 shaping and NVVM texture operation ownership together.
- Preserve both package component additions and union test coverage without duplicate directives.
- Frozen census.slice-195.tsv and discovery registered IDs remain preservation obligations.
  Changed fixture hashes, new tests, diagnostic changes and actual outcomes are reviewed separately.

## Outcomes and Retrospective

Merge/rebuild and all gates complete. Accepted262 uses the full checkpoint plus explicit serial
infrastructure closure; it must retain the parallel NVRTC PCH incident. Effective outcomes preserve
1674 correct,39 unresolved and18 histories. Six upstream rejection transitions and14 input hashes
are reviewed separately. Measurements follow explicit acceptance; the general loop stays stopped.

## Context and Current Pipeline

Merge-risk evidence and proposed resolutions live under build/nvvm-maintenance/integration-review.
The maintained nvvm-results.py compares compact baselines without old raw logs and retains new
provenance. All existing preserved tests remain selected despite upstream's new inventory.

## Scope and Non-Goals

Merge integration, necessary conflict/compatibility fixes, updated dependency pins and validation.
No unrelated feature, compiler optimization, broad file reorganization, driver change or push.

## Architecture and Invariants

Keep semantic identity and stable upstream numbering authoritative. No alternate representation or
fallback to conceal malformed merged IR. Preserve active provider ABI42 unless a proven interface
change requires a deliberate compatible update. Prior failures retain history, and intended fixes
must have evidence. New runtime failures block acceptance until fixed or proved preexisting upstream
and explicitly classified. Never replace an output oracle merely to make the merge pass.

## Interfaces and Dependencies

Native Linux, CMake Ninja Multi-Config releaseWithDebugInfo, LLVM14 provider integration enabled,
CUDA12.9.2, L4 hardware, SM80 initial target. Read the local slang-build skill. Update submodules at
recorded merge pins. At most4 CPU build/corpus workers,2 unit servers; suites sequential and bounded.

## Milestones

1. Commit maintenance; merge the exact fetched upstream SHA and verify all conflict resolutions.
2. Update dependency checkouts; optimized rebuild with compiler/provider and all test tools.
3. Runtime smoke, units, semantic regressions, toolkit, runner contracts and full corpora/material.
4. Review corpus five-field changes, old/new unit identity maps, source/input/submodule deltas,
   active diagnostics and rebuilt binary hashes. Freeze the accepted revision for the results plan.

## Validation and Acceptance

Use maintained harness commands plus full units and generics/overload/operator-overload/diagnostics/
serialization suites. Preserve original registered inputs/oracles and enumerate additions. Require
exact requested inventories, matching tested artifacts, no crashes/timeouts/missing tests relabeled
as passes. The old checkpoint has1713 cells,1674 correct,39 unresolved and18 resolved histories.
Compare new results explicitly and retain all original failure records, including intended resolves.

## Failure and Recovery

Keep failed builds/checkpoints and incomplete attempts in distinct evidence directories. Resolve
producer/consumer root causes in the merge boundary. A rollback uses explicit recorded source
identities; no destructive reset over user changes. Stop on GPU loss without driver/system changes.

## Artifacts and Hand-Off

Completed integration plan/report, accepted compact checkpoint and STATUS identify the exact merge
SHA, source/binary identities, full validation and any reviewed deltas. Raw logs remain ignored.
Next authorized step is the bounded refreshable presentation results package; the general loop
remains stopped, and must still be stopped after the package and completion Slack notification.

## Explicit infrastructure closure

The final frozen run has one extra NVRTC failure in
`language-feature/dynamic-dispatch/layout-optional-field.slang#cuda-1`: NVRTC cannot delete
`default_program.pch` (ENOENT), before shader execution. The earlier full run passed this cell.
Upstream automatic PCH enablement with shared cwd/default program names is consistent with a cache
collision, but serial success cannot prove concurrent reliability fixed. Before running supplements,
predeclare three separate serial rounds of this fixture in all three modes (nine cells), with normal
cache options and identical source/binaries. Every round must match accepted260's five fields.
Keep the full failed run/comparison immutable; only this one NVRTC cell may take its accepted outcome
from explicitly linked supplemental evidence. Preserve the original failure as a validation incident,
separate from39 semantic/support gaps. Describe acceptance as full checkpoint plus serial closure;
any supplemental failure blocks it. Independent integration review agrees this bounds the observed
infrastructure failure without concealing it or claiming the PCH problem fixed.
