# Admit typed CUDA quad vote helpers

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer requires completed slice plans and
reports in each accepted commit. The parent owns acceptance and commit; this worker never commits.

## Purpose and Observable Result

Make `QuadAny(bool)` and `QuadAll(bool)` GPU-correct through direct NVVM O0/O3 for defined CUDA
source quads. The frozen quad-control fixture and one dynamic registered fixture must pass all
three modes. Preserve standalone requirement-marker rejection and unrelated intrinsic boundaries.

## Progress

- [x] 2026-09-25: Read workflow, status, accepted 234 plan/report/semantic evidence, 233 ledger,
      build skill and native environment. Clean base `2cf7d42e0c259c05bc0fd7ab38e9490d05f0155e`.
- [x] 2026-09-25: Trace canonical GenericAsm lookup, typed compound recipes and function preflight.
- [x] 2026-09-25: Final readable fixture NVRTC pass/direct two E52017 failures; eight negative
      sources retain sixteen direct rejections on accepted compiler.
- [x] 2026-09-25: Implement strict whole-helper resolver and shared seven-step recipe; no global
      marker cases. Extend existing negative unit matrix with eight sources.
- [x] 2026-09-25: Corrected build, fresh smoke 4, focused 3, exact 3,072 research launches/196,608
      output words, negative checks 16, aliases 6 and source-marker negative checks 4 all pass.
- [x] 2026-09-25: Units 479 plus existing skip, toolkit 18 and runner contracts 6 pass.
- [x] 2026-09-25: Full frozen 1,356/discovery 324 cells and six material compile/assembly cells
      complete; exact preservation confirms only two original quad transitions and three additions.
- [x] 2026-09-25: Independent worker audit verifies all inputs, complete histories and raw buffers.
- [x] 2026-09-25: Explicit-path formatting complete; unrelated historical formatting exactly
      reversed. Final evidence passed independent parent acceptance; write ownership returned.

## Surprises and Discoveries

`findTargetIntrinsicDefinition` identifies the target implementation but intentionally does not
validate preceding body instructions. Existing value helper admission requires an otherwise empty
body and therefore cannot admit the two canonical quad requirement markers. A separate bounded
whole-helper recipe can validate that body without loosening unrelated value helpers.

The first build passed smoke 4 but focused direct still rejected: the recipe used unsigned lane
index in its shuffle descriptor, whereas the existing catalog requires signed i32 (the provider
uses signless i32 and computed lane indices are 0..31). Corrected only that descriptor to match
the existing masked-wave recipe convention. First attempt identities/logs retained separately.

An evidence-script edit while bash was running shifted its input offset after units and stopped
the shell with exit 127 before toolkit. No test had begun or failed in the remaining gates. Retained the
interruption record; `resume-gates.sh` runs only aliases and outstanding gates with identical final
source/artifact identity, preserving every completed gate and avoiding redundant replay.

## Decision Log

- 2026-09-25, worker: Own the entire validated helper at function preflight and function emission.
  Reuse canonical target lookup, exact bool(bool) signature and existing typed provider operations.
  Never add global marker switch no-ops. No frontend, provider, ABI, library or runner changes.
- 2026-09-25, worker: Full checkpoint is required by cadence (third implementation since 229).
  Reassess all six material compile/assembly cells; runtime bindings/input/oracle remain absent.

## Outcomes and Retrospective

Implementation and final-source validation are complete: 1,680 fresh cells, 1,635 correct, 45
retained failures and twelve resolved histories. All 1,630 prior correct cells remain correct;
only the two original quad cells resolve, with three fixture additions kept separate. Exact
research replay passes 3,072 launches/196,608 output words. Standalone/body/signature negatives
remain exact. No next family is admitted. Independent parent acceptance is complete; the latest full
checkpoint is 235 and implementation cadence is zero.

## Context and Current Pipeline

Consider `output[lane] = uint(QuadAny(input[lane] != 0));`. `hlsl.meta.slang` emits
RequireMaximallyReconverges and RequireQuadDerivatives before the CUDA GenericAsm terminator
`_slang_quadAny` (or `_slang_quadAll`). Source `CLikeSourceEmitter` consumes the whole target
intrinsic via `findTargetIntrinsicDefinition`. Before this slice, direct `_validateNVVMFunction` scanned markers first and rejected E52017.
Marker-free aliases independently rejected exact helper identity/signature.
The semantic body is canonical; fixing its producer or rebuilding syntax would be wrong.

## Scope and Non-Goals

Only two CUDA helper spellings and scalar bool(bool). Four full-mask indexed uint32 shuffles read
`(lane & ~3) | k`, k=0..3, clamp 31; OR/AND and Boolean conversion reproduce the source prelude.
No active-mask synthesis, active-only voting, global maximal reconvergence, new operation families,
provider ABI or resource changes. Source quads must be complete; every non-exited named lane must
execute matching shuffle sequences. Partial source quads and unmatched sequences have no oracle.
Target SM80 supports divergent shuffle rendezvous; no SM6x claim.

## Architecture and Invariants

Canonical target lookup is the identity source. A strict helper validator owns only a single block,
one Boolean parameter/result and GenericAsm, optionally its canonical pair of zero-operand target
requirements. Any other instruction, mismatched type/helper or incomplete requirement pair remains
on ordinary diagnostic paths. Preflight and emission use the same recipe descriptor. The typed
operation closure is complete before provider creation. No second AST/IR representation exists.

## Interfaces and Dependencies

Native Ubuntu/L4 SM89, driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, ABI 36, SM80.
Source inspected `build/nvvm-loop/slice-203-env.sh`; matching RelWithDebInfo bin/lib. Build via
`CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4 --target
slangc slang-test render-test test-server`. Maximum four CPU workers, two unit servers, sequential
GPU suites, timeout 30m each. No driver/system changes, reboot, push or worker commit.

## Milestones

1. Add final readable `tests/cuda/nvvm-quad-votes.slang`; capture accepted-binary NVRTC pass and
   direct two E52017 failures with unchanged source/TEST_INPUT hashes. Record negative boundaries.
2. Add emitter recipe at whole-helper boundary; format explicit C++ path and build.
3. Fresh smoke 4, focused 3, exact accepted 234 research input/oracle replay (3,072 launches,
   196,608 expected words across six family/mode groups), negative boundaries, units 479,
   toolkit 18, contracts 6 and material 6 compile/assembly cells.
4. Register one discovery identity. Full frozen 452/1,356 using `--workload-ids-from
issue-nvvm-backend/census.slice-195.tsv`; discovery 108/324, both runners with four jobs.
5. Compare all five outcome fields against full 229 plus 231/233 overlays; retain full histories.

## Validation and Acceptance

All 1,680 registered cells fresh, no missing/duplicates/extras/inherited. Expected old transitions:
quad-control direct O0/O3 preflight to GPU-correct, plus three separate fixture additions. Expected
1,635 correct/45 unresolved/12 resolved histories, subject to actual output evidence. Preserve
all 555 old input hashes and 30 old source paths, add fixture 556/source 31, capture 12 artifacts per
gate. Historical denominators 427/72 unchanged. Only parent acceptance resets full to 235/cadence 0.
Keep accepted 234 raw sources/input/oracle/output unchanged; read accepted binary expectations
rather than regenerate them. Preserve all failed probe constructions distinctly.

## Failure and Recovery

Stop GPU dispatch on device loss; timeouts are incomplete. Retain attempts under unique names.
No baseline resets. New regressions block acceptance: isolate or revert exact patch, rerun affected
gates and complete checkpoint. Stop at next independent blocker, preserving outstanding work.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-235-before` and `slice-235-after`; durable runtime-validation 235,
censuses 235, discovery addition 235, this plan, five-part report, design and STATUS. Full helper/
guard inventory and six-question input-shape audit in report. Parent independently audits/commits.
