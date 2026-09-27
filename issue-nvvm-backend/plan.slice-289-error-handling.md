# Qualify existing error-handling compute contracts

This bounded ExecPlan follows `.agent/PLANS.md` and the committed NVVM plan exception. The user’s
loop authorization remains active; skip Slack, no push/system changes. Root owns scope/execution/
acceptance; a bounded author prepares raw contracts/driver, and separate review checks them.
No tracked source, compiler, provider, corpus or runner edits are intended.

## Purpose and Observable Result

Qualify six existing error-handling compute tests at NVRTC O3/NVVM O0/O3 with independent complete
GPU-output expectations. Inventory269 found nine compute files in this directory and none selected
in the main corpus. Prior271 covered nested defer/throw interaction, but not these distinct cases.
A passing compiler diagnostic or weak partial CHECK is insufficient evidence of correct execution.

## Progress

- [x] 2026-09-27: Promotion288 committed0e3bf8ca5; WORKFLOW/STATUS reread. Accepted285 binaries remain
      installed; two persistent288 sources passed six direct native cells with no compiler change.
- [x] Assign reused author/reviewer after fresh spawn hit thread limit; baseline identities verified.
- [x] Verify selected-source corpus absence before freeze.
- [x] Freeze exact six native ordinals/bodies/input flags/oracles and18mode commands before execution.
- [x] Execute18bounded cells serially; independently check every output word and classify failures.
- [x] Verify baseline preservation, independent review and compact report/record/plan/navigation/commit.

## Surprises and Discoveries

Pre-selection read found `catch-all.slang` ends with `// CHECK-NEXT 7` (missing colon). Preserve the
original bytes and separately require its fourth actual word7; native FileCheck alone is incomplete.
Untyped output is hexadecimal, so CHECK11/12 in basic means decimal17/18, not decimal11/12.

## Decision Log

- 2026-09-27, root: Fresh289 worker spawn hit agent thread limit. Reuse dynamic_regressions288 as
  author and pch_repro275 as separate reviewer; neither authors the other’s evidence. Disclose reuse.

- 2026-09-27, root: After successful nested dynamic qualification/promotion, return to a demonstrably
  sparse language region. Use six existing compute contracts rather than inventing new semantics.
  Select basic,catch-all,generics,non-trivial-error-type,synthesized-witness,throws-with-params.
- 2026-09-27, root: Preserve all shaders/harness inputs and original CHECKs. Supplement full outputs
  independently. No compiler change unless a separate bounded correction is motivated by real failure.

## Outcomes and Retrospective

All18native/GPU cells and54independently decoded hex words pass with no timeout/retry/unrun.
All285 identities remain exact. Independent final review accepts all evidence without findings; formatting and local commit close the slice. Main580cases/576sources/1740cells with1703correct/37unresolved/20histories remains inherited285.
Discovery stays128; these focused observations do not alter the selected corpus or checkpoint cadence.

## Context and Current Pipeline

Slang typed errors and `try`/`catch` lower through ordinary control-flow and result transport. The
selected sources include two distinct error types with ordered catches, a catch-all fallback, a
throw/catch/rethrow inside a generic, an aggregate error with code/parameter fields, and a synthesized
mutating witness that forwards a nonmutating generic throwing method. The parameterized success path
covers preservation of throwing function type attributes through cleanup. Generic witnesses here
must not be relabeled runtime existential dispatch without separate live-path proof.

## Scope and Non-Goals

Unchanged files under `tests/language-feature/error-handling/`:
`basic.slang`, `catch-all.slang`, `generics.slang`, `non-trivial-error-type.slang`,
`synthesized-witness.slang`, `throws-with-params.slang`. Prefer their authored CPU compute directives:
ordinals2 for the first five,0 for throws-with-params; validate through maintained enumerator.
No optional pointer fabrication, compile-only interface exception, new source harness, numerical
accuracy, runtime performance or broad error-feature completeness claim. No production modifications.

## Architecture and Invariants

Maintain original source bodies and TEST_INPUT/CHECK directives, shader-object flags and untyped
buffer output. Select the exact authored CPU contract, use maintained census/discovery CUDA adaptation
and one isolated mirror per mode. No independently reimplemented command translator. Record original
source/sidecar hashes and exact body equivalence; do not erase missing CHECK coverage by editing sources.
Independent anticipated decimal buffers: basic[2,0,17,18,6,1],catch-all[1,16,48,7],generics[5,1,0],
non-trivial-error-type[2,19,0],synthesized-witness[4],throws-with-params[7]. Author/reviewer must derive
these from source before freezing; parse observed untyped words as hex, preserving raw bytes.

## Interfaces and Dependencies

Installed accepted285 compiler62469125/provideraf1661deABI42, version301-g8fbf0f84e, source8fbf0f84e
plus patch12f503e9. CurrentHEAD is not binary identity. Ubuntu/L4SM89/driver580.126.09,CUDASM80,
CUDA12.9.2/NVRTC12.9.86/LLVM14. No build required; future production rebuild still refreshes version
and restored286 source objects. Max4CPU workers; compiler/GPU serial,180s/cell,1800s/batch,no retries.
Raw root `build/nvvm-error-handling289`; owned_process/gate may be reused from288 without mutation.

## Milestones

1. Verify live37runtime/11qualifiedsource/2config/576maininput/22pins/100layout against285, plus no
   production diff. Check all six selected sources absent from main paths and earlier focused271/278.
2. Author freezes selected ordinal/command/flags/source/sidecars, full independent oracle, coverage
   limits (success-only paths remain success-only),18mode cells and exact serial owned runner. Root
   and independent reviewer approve before shader execution. Raw mirrors keep original bodies.
3. Execute requested18cells serially under180s process-group bounds and1800s outer gate. Preserve
   every result, native counts, return code, diagnostic, shape, raw classifier and actual full buffer.
   A timeout stops batch with explicit unrun suffix; failure never becomes pass due runner exit0.
4. Trace any new failure to a concrete source/IR/producer-consumer boundary, bounded to existing
   artifacts and source inspection unless an amended experiment is frozen/reviewed. No production fix
   here. If all pass, state the exact observed branches and fields; do not overclaim untested throws.
5. Verify accepted identities remain exact. Full285 and material/native suites remain inherited;
   no broad rerun needed without compiler/ABI/runner/config change. Keep all existing37 gaps intact.
6. Independent evidence review, compact five-part report and structured result/provenance record,
   completed plan/navigation, formatting/diffcheck/local commit. Continue authorized loop.

## Validation and Acceptance

Eighteen unique `(source,mode)` dispositions, full independent output, one executed/passed/zeroignored
for a positive cell, exact original bodies and accepted runtime. A wrong buffer with a passing weak
CHECK is still failure. Qualification describes success/throw/catch paths actually represented by
these inputs; no test-file percentage becomes semantic coverage. No builds, units or full checkpoint.

## Failure and Recovery

Preserve attempted cells/initial failures and explicit missing suffix; do not retry or repair a source
silently. Raw driver fixes require versioned freeze and evidence. No installed-layout mutation expected;
if it changes, stop and restore verified285 before unrelated work. User stop overrides further runs.

## Artifacts and Hand-Off

Raw fixtures/logs/outputs/proofs/reviews stay ignored under build. Durable report.slice-289-error-handling.md,
research-evidence.slice-289.json, completed plan and navigation retain outcomes. Immediate next action: final independent review, compact formatting and local commit.
Next bounded slice repairs the malformed catch-all CHECK and proves deliberate-corruption rejection;
runtime-loaded error path qualification remains a later opportunity.
