# Test conservative alignment on whole nested-array stores

This bounded ExecPlan follows `.agent/PLANS.md` and the NVVM committed-plan exception. The development
loop remains active; skip Slack, no push/system changes. Fresh author alignment_probe283 owns ignored
prototypes, reused independent reviewer pch275 audits read-only, root owns acceptance/docs/commits.

## Purpose and Observable Result

Test whether changing only the declared alignment of valid whole-array stores from4 to1 fixes the
installed NVVM padding-loss bug while actual destination roots remain aligned4. This preserves the
canonical value and call ABI. Gate282's ordinary unaligned path supplies a lead, not a guarantee:
optimization could infer the stronger actual alignment and still reproduce the bug.

## Progress

- [x] 2026-09-27: Gate282 committedee4bc4f44; general typed-helper method rejected at large-size gate.
- [x] 2026-09-27: Select conservative annotation experiment; fresh author available, one writer.
- [x] 2026-09-27: Accepted37runtime/8source/2config/576input/22pin identities exact.
      Freeze12cells; root and independent pre-execution review confirm exact4/2annotation deltas,
      unchangedactualroot/load/scalaralignment4 and unchangedsignatures.
- [x] 2026-09-27: Both originalO3 controls reproduce14/9234; all four candidateGPU cells
      pass O0/O3. PTX preserves paddedoffsets with byte stores; actual roots remain aligned4.
- [x] 2026-09-27: N17O0/O3 andN65536O0 compile; N65536O3 times out120s under4GiB.
      All12cells have explicit dispositions, no retry. Source signatures unchanged, but NVVM O3
      promotes incoming pointer arguments internally; no claim of unchanged downstream ABI.
- [x] 2026-09-27: Independent reused reviewer verifies55 references, all12 dispositions, exact
      deltas and PTX/source/identity evidence. Root strict record generation and unchanged identities
      pass. Completed report/record/navigation ready for formatting and local research commit.

## Context and Current Pipeline

Accepted279 splits direct nested structs but leaves array stores opaque. Research280's canonical
Cell={i16,{i16,i32}} array has leaves0/4/8 and stride12; optimized stores incorrectly merge0/4 into0/2.
Prototype281 pointer copies pass but need valid snapshot addresses. Typed helpers preserve arbitrary
small SSA cases in282, but introduce aggregate call-argument expansion and fail a120-second large gate.
A weaker truthful alignment guarantee introduces no new semantic representation, volatile side effect,
copy helper or ABI. It still needs direct counterfactual proof on actually aligned storage.

## Scope and Non-Goals

Freeze maximum12 cells, all direct NVVM prototypes:

1. Four GPU cells: exact280 emitted nested-array LLVM, original and only four whole-array-store
   alignment annotations changed4→1, atO0/O3. Original source/LLVM independent oracle remains exact.
2. If both candidates pass, four GPU cells: exact281 baseline pointer-copy LLVM, original and only
   its two whole-array-store alignment annotations changed4→1, atO0/O3. Keep roots/load alignment4.
3. If both families pass, four compile-only candidate cells from281 atN17/N65536, O0/O3, with only
   array capacity/count-bound changes. No largeGPU launch. Compare small/large source and PTX growth;
   no original-large PTX baseline or speed claim.

Stop this candidate on its first correctness, timeout or compiler failure. Preserve every remaining
frozen cell explicitly as not run, never a pass. No additional variant/fallback, NVRTC correction,
production compiler/provider/harness/corpus change or rebuild in this slice.

## Architecture and Invariants

Alloca roots stay aligned4; semantic leaves and scalar operations remain unchanged. Existing pointer
function signatures/noinline remain; no new helper/optnone/calling convention. Explicit align1 is a
conservative guarantee on an aligned pointer, not a claim that the address becomes misaligned. Check
actual optimized PTX offsets because LLVM may strengthen alignment using other facts. The independent
four-word oracle is0,123,0,456 at inputseed0 over65536patterns; source/destination/snapshot/self-alias
contracts remain those frozen by280/281. Padding is not a semantic oracle.

## Interfaces and Dependencies

Use ignored `build/nvvm-array-store-alignment283`, reusing281/282 direct libNVVM/CUDA replay and owned
process gates. Actual compiler remains279:9e013b2c/version295-g0043e8d17, provider a861b242/ABI42,
CUDA12.9.2, targetSM80/L4SM89. Verify37runtime/8source/2config/576input/22pin identities before/after.

## Milestones

1. Root readsSTATUS/WORKFLOW and verifies accepted identity. Author freezes12cell conditional inventory,
   exact source copies/annotation deltas, independent oracle and runner hashes; reviewer audits them.
2. Run family1 then family2 with120-second owned process bounds, max4CPU, serialized GPU/compilation.
   Overall gate≤1800seconds. Large compile-only children additionally have4GiB virtual-memory bound.
   Retain all failures and not-run dispositions under the frozen stopping rule; no retries.
3. Prove changed annotations are the only small-source differences and all actual roots remain aligned.
   Inspect canonical field stores or surviving bad vector stores and caller signatures in actual PTX.
4. On positive small results, inspect large source instruction count, parameter/local storage growth,
   compile completion and PTX size. On a negative result, close the candidate without rescuing it with
   a new helper, arbitrary size threshold or reduced optimization.
5. Verify installed identities; independent review then compact five-part report/record/plan/navigation,
   formatting and local commit. A production change, if justified, requires its own full-checkpoint slice.

## Validation and Acceptance

One explicit outcome per requested cell: compile/process return, executed/passed/ignored counts,
diagnostic, canonical shape, source/options hash and complete output. Unrun cells remain visibly
unqualified. Positive prototypes are not new language support; negative prototypes remain useful
research. Full279/targeted233/cadence0 and1740corpus outcomes remain inherited;280 optimized defects
stay open. No material GPU/performance claim.

## Failure and Recovery

No installed binaries are modified. Timeout/memory limits define this gate, not all possible compiler
behavior. Record failed original/candidate cells separately. Stop a failed candidate rather than
extending scope; a new method gets a new bounded plan. Explicit user stopping instructions take precedence.

## Decision Log

- 2026-09-27, root: Prefer a semantics-preserving annotation counterfactual before broader lowering or
  ABI work. Gate282 shows a useful alignment distinction but did not test weak alignment on actualaligned
  roots. Rejected typed-helper method remains rejected; no silent reuse of that fallback.

## Surprises and Discoveries

Pending. Fresh author available for283; independent review reuses a separate context.

## Outcomes and Retrospective

Both small annotation counterfactuals correct the observed output; general size qualification fails
atN65536O3 under120s/4GiB. No original-large baseline was run, so this is not evidence of an
annotation-specific compile regression. Next compare original-large behavior under identical bounds
before deciding production scope. Accepted279 identities remain unchanged; no implementation selected.

## Artifacts and Hand-Off

Raw root above; durable report.slice-283-array-store-alignment.md, research-evidence.slice-283.json
and this plan. Resume from frozen conditional inventory/process records before rerunning any cell.
