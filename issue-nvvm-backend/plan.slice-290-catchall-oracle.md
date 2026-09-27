# Enforce the final catch-all test output

This bounded ExecPlan follows `.agent/PLANS.md` and the committed NVVM-plan exception. The authorized
loop remains active; skip Slack, no push/system changes. One bounded author owns the single CHECK fix
and raw experiment proposal, root executes/accepts, independent reviewer is read-only.

## Purpose and Observable Result

Repair `tests/language-feature/error-handling/catch-all.slang` from `// CHECK-NEXT 7` to
`// CHECK-NEXT: 7`. The fourth output must be7; a deliberately corrupted final value9 should pass the
old weak native oracle but fail the corrected oracle. Correct outputs must continue passing.

## Progress

- [x] 2026-09-27: Research289 committedc481182c8; WORKFLOW/STATUS reread and285 baseline verified.
      All18original cells passed54independently checked words; the malformed CHECK stayed unchanged.
- [x] Author/reviewer freeze exact one-colon change, controlled counterexample and six commands.
- [x] Execute old/fixed corruption pair, three CUDA positives and one original CPU native positive.
- [x] Independent complete-output/hash review, baseline preservation and compact reviewed local commit.

## Surprises and Discoveries

Missing colon makes FileCheck ignore the final line. This is an actual oracle defect found in289,
not a shader/compiler wrong-output failure. The original fourth GPU value was correct in all modes.

## Decision Log

- 2026-09-27, root: Fix the test at its CHECK producer; keep shader semantics/inputs unchanged. Prove
  checker sensitivity with identical corrupted shader bodies under old and fixed CHECK syntax.
- 2026-09-27, root: No new compiler build, unit, broad native suite or1740checkpoint is needed. The
  selected source is outside the main corpus; all main hashes and known37gaps must remain intact.

## Outcomes and Retrospective

All six contracts match: old corrupted native false pass, corrected specific CHECK rejection, and
four normal positive passes. All 24 output words match independent expectations; no timeout/retry.
Live accepted identities remain exact. Independent review accepts all evidence without findings; final formatting and local commit close the slice. Compiler/provider identity and full285 checkpoint remain inherited. Test-only change does not
increment compiler implementation cadence. Runtime-loaded exception qualification remains follow-up.

## Context and Current Pipeline

Original `handlerFunc(3,3)` writes `3+3+1=7` to fourth output, after first three values1/16/48. Its
comment lacks the required colon, so native FileCheck does not check7. The controlled raw variant
changes only that call to `handlerFunc(3,4)`, yielding9; earlier outputs and all other CHECKs stay.
No compiler representation is involved: the responsible consumer is the existing test-output checker.

## Scope and Non-Goals

Exactly one tracked colon addition in catch-all.slang plus compact records/navigation. No shader,
compiler, provider, runner, corpus or capability changes. Raw corruption is never promoted. No new
runtime exception or performance claim; both old/fixed corruption shaders may optimize to constants.

## Architecture and Invariants

Normal original/fixed executable tokens and all inputs must be identical; only final CHECK colon
changes. Raw old/fixed corruption bodies must be identical, differing only by the same CHECK colon;
each differs from normal shader only in the final argument3→4. Expected full normal decimal output
[1,16,48,7], corrupt output[1,16,48,9]. Preserve untyped hexadecimal rendering and shader-object flags.
Native old-corrupt pass is evidence of oracle weakness, never correctness of the altered shader
relative to the intended7. Fixed-corrupt failure is expected rejection, never regression.

## Interfaces and Dependencies

Accepted285 installedcompiler62469125/provideraf1661deABI42, version301-g8fbf0f84e; qualifiedsource
8fbf0f84e+patch12f503e9. Native Ubuntu/L4SM89/driver580.126.09,SM80,CUDA12.9.2/NVRTC12.9.86,LLVM14.
Root rawdir `build/nvvm-catchall-oracle290`. Reuse owned_process/gate unchanged288;180s/cell,1800s
outer,no retries,serialwork,max4CPU. Native CPU test uses installed tools, no production build.

## Milestones

1. Snapshot original source/hash289; apply exactly one colon in tracked source. Author prepares raw
   old/fixed corrupted controls and fixed normal CUDA mirrors with maintained helpers. Select existing
   CPU ordinal2 for adaptation; also run actual tracked CPU ordinal2 directly. No source body edits.
2. Stage and run repository formatting on exact paths; verify one-colon diff and freeze all source,
   control, mirror, runner, commands and independent outputs. Root/reviewer approve before execution.
3. Run six cells serially: old-corrupt NVVMO0 (nativepass, full9), fixed-corrupt NVVMO0 (nativefail,
   full9, CHECKexpects7), fixednormalNVRTCO3/NVVMO0/O3 (nativepass,full7), fixednormalnativeCPUordinal2
   (nativepass,full7). Nativepositive actual path is `.slang.2.actual.txt`; reject stale output before
   running. Preserve rawclassification/return/counts/diagnostic/shape and all words. Source/inputmap
   equivalence permits inheriting broader289 qualification; do not relabel it fresh.
4. Failure/timeout preserves requested dispositions and explicit unrun suffix. Check the intended
   control failure is actual FileCheck mismatch on final9, not preflight or compilerfailure. No retry.
5. Verify37runtime/11qualifiedsource/2config/576maininput/22pins/100layout exact285; prove only expected
   test colon changed. Independent review, five-part compactreport/evidence/completedplan/navigation,
   formatting/diffcheck/localcommit; continueloop toward bounded runtime-loaded error qualification.

## Validation and Acceptance

Four positive native/completebuffer passes, old-corrupt native falsepass and fixed-corrupt expected
rejection with identical complete corrupt buffers. Allnative positives execute1/pass1/ignore0; control
rejection executes1/pass0/ignore0. Counts/exit alone do not replace raw output plus checker diagnostics.
No ignored/missing/timeout becomes pass. Preserve289 original source and result identity permanently.

## Failure and Recovery

If formatting/adapter changes more than intended, preserve first proposal and correct before execution.
All attempts remain recorded. No installedlayout mutation expected; restore285 if needed before other
work. Unexpected compiler failure is separate from this oracle repair. Userstop overrides furtherwork.

## Artifacts and Hand-Off

Ignored raw roots own snapshots/control sources/logs/buffers/reviews. Durable one-colon fix, report.slice-
290-catchall-oracle.md, test-evidence.slice-290.json, plan and navigation are committed. Immediate next:
complete independent evidence review, format compact closeout and commit. Next qualifies bounded
runtime-loaded error inputs; no unsupported compiler change follows from this test repair.
