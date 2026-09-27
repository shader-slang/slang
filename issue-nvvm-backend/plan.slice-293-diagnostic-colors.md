# Classify colored diagnostics consistently

This bounded ExecPlan follows `.agent/PLANS.md` and the NVVM committed-plan exception. The authorized
loop remains active; skip Slack, no push/system changes. Root owns scope, GPU/suite execution,
acceptance and commits; a fresh author owns the maintained runner fix and focused CPU contracts.

## Purpose and Observable Result

The preserved290 corrupt-output test fails FileCheck, but the census calls its colored diagnostic
unclassified. Removing only ANSI SGR styling lets the existing mismatch regex recognize it. Classify
that same failure consistently with its plain-text form, preserving compiler/preflight/provider
precedence and original raw logs. This is a reporting correction, not new shader correctness.

## Progress

- [x] 2026-09-27: Promotion292 committed c0d8b1b9e; WORKFLOW/STATUS read; concrete lead preserved.
- [x] Fresh author reproduces colored failure, audits normalization boundary and adds focused contracts.
- [x] Independent source/method review; root verifies identities and frozen checkpoint commands.
- [x] Run all eight gates serially: full1740 exact outcomes, runtime4/material6/toolkit18 pass;
      units1110/semantics1248 identities exact; contracts46pass/1inheritedskip. Material artifacts equal285.
- [x] Compare every outcome and identity; preserve transitions/failure histories; independent review.
- [x] Complete compact record/report/plan/navigation and formatting; independent review accepts all
      obligations without findings. This accepted local commit closes the slice; continue loop.

## Surprises and Discoveries

Read-only lead: build/nvvm-runtime-errors291/classifier-followup-lead.json. Original290 log SHA
cc460620e8630ca80184994920915fd92f49a877e34423246dd6762f77dd55ef contains ESC[1m, ESC[0m and
ESC[0;1;31m around the FileCheck diagnostic. This is valid terminal styling, not malformed shader IR.

## Decision Log

- 2026-09-27, root: Repair presentation handling at the diagnostic parser boundary. Do not broaden
  semantic mismatch detection, hide compiler failures or rewrite archived logs/outcomes.
- A shared corpus-runner change triggers the full checkpoint even though compiler bytes remain285.
  No build is necessary; current HEAD must not replace the accepted compiler source identity.
- Prefer existing normalization helpers if present. Root searches found none in relevant Python
  runner/tools files; author must confirm reuse and choose the smallest coherent boundary.

## Outcomes and Retrospective

Focused census/discovery suites pass eight tests each after retained failing red runs. Actual290
raw log/counts remain unchanged while its classification becomes runtime-mismatch. Read-only replay
of all1740 archived285 logs predicts zero main-outcome changes; fresh full checkpoint and all subsequent gates now pass.
All 100layout/37runtime/11source/2config/576input/22pin identities remain exact285.
Independent review accepts all obligations without findings. The closing local commit preserves
accepted293 as the fresh full baseline, with unchanged compiler285 bytes:580 cases/576 inputs/1740
cells,1703 correct/37 unresolved,20 resolved histories, lastfull293/targeted233/cadence0.
Native292 evidence remains inherited.

## Context and Current Pipeline

run-compute-census.py::_run_one captures slang-test output, preserves its log, calls _classify_result
and extracts execution_counts. The classifier gives explicit compiler diagnostics precedence over
render-test output wrappers and FileCheck mismatches. run-compute-discovery.py wraps classification
for unavailable-entry-point diagnostics. Inspect both boundaries to avoid inconsistent handling.
The concrete FileCheck output is valid colored text; the parser owns ignoring its presentation.

## Scope and Non-Goals

Change maintained census diagnostic normalization and focused contracts. Touch discovery only if its
wrapper needs the same normalization to preserve existing diagnostic semantics; justify with a test.
No compiler/provider/build, shader/oracle, inventory, timing or material runtime changes. Do not
implement a general terminal emulator or introduce dependencies. Preserve non-SGR diagnostic text.

## Architecture and Invariants

Recognize actual SGR color/style sequences without erasing printable diagnostics. Preserve original
log bytes and failing return codes. Existing error-phase precedence, canonical shapes and strict
executed/passed/ignored counts remain authoritative. Colored and plain semantic equivalents should
classify identically; arbitrary text, skips, compiler errors and malformed summaries cannot pass.
Every helper or special case needs a concrete failing contract and an input-shape audit.

## Interfaces and Dependencies

Raw root build/nvvm-diagnostic-colors293. Baseline issue-nvvm-backend/runtime-validation.slice-285.json.
Installed100 layout/37runtime/11qualifiedsource/2config/576inputs/22pins remain285; no build. Compiler
62469125/provideraf1661de ABI42/version301-g8fbf0f84e. Native Ubuntu/L4SM89, targetSM80, CUDA12.9.2.
Max4CPU workers; unit servers2; serialized gates bounded1800s, no retries. Reuse owned process/gate
from292 unchanged. Next production build still refreshes version metadata and restored286 sources.

## Milestones

1. Preserve original runner bytes and actual290 failure. Add focused CPU tests for that colored
   CHECK-NEXT diagnostic, plain equivalent, compiler/preflight/provider precedence and negative
   controls. Run red before fix and green afterward; retain both. Audit helper ownership/reuse.
2. Independent reviewer approves exact source diff and proposed fixed command inventory before
   root launches expensive gates. Verify baseline runtime/source/input/pin/layout maps before/after.
3. Root runs `python3 issue-nvvm-backend/nvvm-results.py checkpoint --baseline
issue-nvvm-backend/runtime-validation.slice-285.json --slangc build/RelWithDebInfo/bin/slangc
--build-label RelWithDebInfo --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so
--cuda-root /usr/local/cuda-12.9 --jobs 4 --output build/nvvm-diagnostic-colors293/full/checkpoint`.
   It must retain frozen1356/discovery384/material6 and runtime4; no unfiltered census selection.
4. Run RESULTS commands for unit1110 and semantic1248 identities, toolkit18 and all four runner
   contract suites. Preserve skips; compare each native ID/status, not just totals. Material
   PTX/cubin/resource observations must remain qualified separately from runtime performance.
5. Compare all1740(id,mode) cells and five fields against285. Retain original comparison and
   failure history. Explain any intentional classification-only transition without calling it a
   correctness gain. Missing/ignored/timeout/regressed cells block acceptance and next feature work.
6. Independent final review; one runtime-validation.slice-293.json with exact compact outcomes,
   five-part report, completed plan and navigation; format/diffcheck/local commit.

## Validation and Acceptance

The real colored290 failure must become runtime-mismatch while its original unclassified record
remains immutable. Compiler/preflight/provider failures retain precedence. Plain output semantics
and strict success/skip behavior remain. All focused contracts and required checkpoint gates pass
or preserve known exact unresolved outcomes with independent review. No unexpected source/input or
installed identity delta. Full acceptance resets cadence only after exact comparison and review.

## Failure and Recovery

Keep failed red controls, all attempted cells and original baseline records. Do not overwrite output
directories or retry failing shader cells into passes. Unexpected corpus regressions stop feature
work and get bounded diagnosis. Revert a flawed parser approach instead of broadening match rules.
No installed mutation is expected; restore285 if one occurs. User stop takes precedence.

## Artifacts and Hand-Off

Raw colored/plain logs, red/green contracts, source snapshots, freeze, suites and comparison stay
ignored under build. Durable record retains all cell obligations/failure histories and original
compiler identity. Next after acceptance: select a material-driven bounded investigation from286's
overload-screening lead; no speculative optimization. Read-only preparation confirmed267 already
closed the earlier aggregate-memory reproducer, so do not repeat that investigation.
