# Qualify runtime-loaded error inputs

This bounded ExecPlan follows `.agent/PLANS.md` and the committed NVVM-plan exception. The authorized
loop remains active; skip Slack, no push/system changes. Root owns scope, execution and acceptance.
One bounded author owns raw fixtures/proposals; independent review checks methods and evidence.
No compiler, tracked shader, corpus or runner changes are intended in this research slice.

## Purpose and Observable Result

Close the principal evidence limit of289: literal inputs may fold exception control flow, and its
synthesized throwing witness only succeeds. Two small raw fixtures must load inputs at runtime and
produce independently predicted outputs across success and failure, at NVRTC O3/NVVM O0/O3.
Emitted IR must show live input-to-result/error data flow before claiming runtime-path coverage.

## Progress

- [x] 2026-09-27: Oracle repair290 committede3d39eb5a; WORKFLOW/STATUS reread,285 identities verified.
- [x] Fresh author and separate reused reviewer froze v2: six GPU and two IR cells.
- [x] All initial eight cells passed execution; generic/aggregate live path verified.
- [x] Freeze and independently review four-cell witness v3 refinement after its tag-observability gap.
- [x] Execute all12versioned obligations: nine GPU passes/48words and three IR compiles.
      V3 PTX uses returned tag to select payload+256 versus payload; v2 generic/aggregate paths remain qualified.
- [x] Independent review, unchanged baseline proof, compact closeout/local commit; continue loop.

## Surprises and Discoveries

Initial v2 passes six GPU cells/36words and both IR compiles. Generic/aggregate tags are consumed
by different success/error paths. The witness LLVM has correct tag forwarding, but PTX removes caller
tag use because both success and catch return the same integer payload. This is valid optimization
and a test-observability gap, not a compiler failure. Preserve all v2 outcomes and their narrower claim.

## Decision Log

- 2026-09-27, root, after independent v2 IR audit: Amend the bounded experiment with exactly four
  additional witness-only cells (three GPU modes and one O3 IR capture). Add0x100 only in the catch
  return, independently predicting[272,4,11,272]. This makes a wrong error/success tag observable:
  error misread as success yields16; success misread as error yields260/267. Preserve v2 and do not
  rerun unchanged generic/aggregate cells. Reviewer agrees method; final v3 freeze requires approval.

- 2026-09-27, root: Use runtime buffers to revisit the existing generic rethrow, aggregate error and
  synthesized mutating-witness shapes. Do not widen to FP8 errors, pointers or existential catches.
- 2026-09-27, root: Permit noinline annotations where needed to retain a meaningful result/error call
  boundary. Allow legitimate inlining of adapters and predicated selection; require live data flow,
  not arbitrary instruction names or surviving redundant rethrow branches.

## Outcomes and Retrospective

Final v3 witness execution and live PTX tag discrimination pass. V2 generic/aggregate results remain
qualified; original witness payload-only limit stays recorded. All accepted285 identities are exact
after v3. Independent final review accepts all12versioned outcomes and live paths with no findings; final
formatting and local commit close the slice.
Total bounded inventory is12attempted obligations: original8 plus exactly4new witness-only cells. Main580cases/576sources/1740cells and1703correct/37unresolved/20histories remain inherited285;
lastfull285/targeted233/cadence0. No production change is justified before a concrete failing case.

## Context and Current Pipeline

`ErrorHandlingLoweringContext` converts `IRTryCall` into a call returning `Result<T,E>` and selects
success/error payloads. `processThrow` creates `makeResultError`; normal returns create result values.
Existing generics.slang catches/rethrows an enum error. non-trivial-error-type.slang transports an
integer code/parameter pair. synthesized-witness.slang adapts a nonmutating generic throwing method
to a mutating interface requirement, but its implementation never throws for existing inputs.
These canonical shapes motivate runtime-loaded variants without changing compiler representation.

## Scope and Non-Goals

At most two raw fixtures: one combines runtime generic rethrow and two-field aggregate errors;
one exercises the synthesized mutating witness with both success and error inputs. V3 changes only
its catch result by adding0x100; no new type, input lane or exception domain. Four input lanes
per fixture suffice. Use ordinary integer/float scalar values and an integer error struct/enum;
no arrays of records, FP8/BF16, pointer payloads, existential exception dispatch, resources other than
plain scalar input/output buffers, numerical accuracy or performance scope. No tracked source changes.

## Architecture and Invariants

All branch-selecting inputs must come from StructuredBuffer loads, not thread IDs or constants.
Keep expected values independent of objects/results being tested. For a representative first fixture,
float inputs[2.5,3.5,1,4] give generic results[5,1,2,1] under the existing threshold3/rethrow semantics;
integer inputs[1,3,4,2] give aggregate results[2,19,20,4] when error code1/paramn combine as16+n.
A witness fixture may use receiver values[-1,0,7,-3], returning value+sizeof(int) for nonnegative values
and enum error0x10 otherwise, giving[16,4,11,16] in v2. V3 catch-only offset changes the expected witness output to
[272,4,11,272]. Freeze actual ordering/buffers before execution;
author/reviewer must derive them independently. All output words start at a distinct nonexpected
sentinel, and full CHECK-NEXT sequences cover every word. No omitted write can masquerade as success.
The witness remains generic specialization, not runtime existential/type-ID dispatch.

## Interfaces and Dependencies

Raw root `build/nvvm-runtime-errors291`. Accepted285 compiler62469125/provideraf1661deABI42/version
301-g8fbf0f84e, source8fbf0f84e+patch12f503e9; current HEAD is not the compiled identity. Ubuntu/L4SM89,
driver580.126.09,SM80,CUDA12.9.2/NVRTC12.9.86,LLVM14. No build; future production build still refreshes
metadata and restored286 objects. Max4CPU workers; serial shader/compiler processes,180s/cell and
1800s/outer gate, no retries. Owned_process/gate are copied unchanged from288.

## Milestones

1. Root verifies baseline and source cleanliness. Author reads original289 shaders and lowerErrorHandling
   source, then prepares two minimal fixtures, exact typed/untyped full outputs and bounded modes.
   Protect TEST directives and explicit struct semicolons. Freeze source/runner/commands before work.
2. Independent reviewer/root audit canonical shapes, input loads, both paths, error payload meaning,
   witness adaptation and full oracle. No fake dynamic dispatch claim; state any simplification.
   Initial inventory: six GPU cells (two sources xthree modes), two NVVM O3 IR captures. Reviewed
   amendment adds exactly three witness-v3 GPU cells and one witness-v3 IR capture; no further probes.
3. Run fixed inventory serially, preserving all return/count/classification/diagnostic/shape/buffer
   fields and requested dispositions. Timeout stops batch with explicit unrun suffix. No retry or
   change of expected values to match observed output. Existing289/290 neighbors remain inherited
   under original identities since compiler/configuration and original executable bodies are unchanged.
4. Inspect final Slang IR, LLVM and PTX as available: runtime buffer input must reach actual success/
   failure selection and the result/error payload consumed by each checker. Verify the generic
   witness retains required throwing function/result semantics even if its adapter is inlined. A
   proof of unused helper definitions is insufficient. No demand for a redundant catch/rethrow to
   survive when legal optimization preserves behavior. If a source misses its intended path, record
   that limitation and freeze a separately reviewed follow-up rather than declaring coverage.
5. Any actual failure closes this research at a bounded producer/consumer repro and next correction
   plan; no implementation creep. If all pass, state the exact runtime paths and chosen inputs.
6. Verify37runtime/11qualifiedsource/2config/576maininput/22pins/100layout unchanged285. Inherit full
   checkpoint/native/material evidence; no unnecessary broad rerun. Independent review, compact
   report/record/completedplan/navigation, formatting/diffcheck and accepted local commit.

## Validation and Acceptance

Initial six GPU passes alone do not qualify tag-dependent witness handling. Final qualification
requires three further v3 witness passes and its live tag-dependent result mapping, alongside the
already verified v2 generic/aggregate path. Preserve v2 witness limitation as part of the evidence. Success-only, folded constant results, ignores, missing execution, timeout
or merely shifted diagnostics are not coverage. No full feature guarantee follows from four input lanes.

## Failure and Recovery

All raw proposals/attempts and unrun suffixes remain versioned. No production/layout mutation expected;
if it occurs, stop and restore accepted285. A fixture correction requires a new freeze and preserved
old outcome. User stop instructions take precedence over all otherwise unnecessary work.

## Artifacts and Hand-Off

Raw source/oracle/driver/logs/buffers/IR/reviews remain ignored under build. Durable report.slice-291-
runtime-errors.md, research-evidence.slice-291.json, completed plan and navigation own outcomes.
Immediate next action: final independent evidence review, compact formatting and local commit.
Next bounded slice promotes final v2 generic/aggregate and v3 witness probes as native regressions,
keeping runtime inputs, noinline boundaries and tag-distinct catch oracle.
