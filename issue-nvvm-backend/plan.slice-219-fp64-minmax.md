# Establish the FP64 masked min/max admission contract

This bounded research ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit
exception. Fresh-context delegation remains unavailable at the app's agent-thread limit; the
parent uses WORKFLOW's local fallback. Raw probes remain ignored under `build/`.

## Purpose and Observable Result

Establish independent raw-word expected FP64 min/max results and verify the CUDA helper for scalar,
vector and matrix shapes. Confirm direct NVVM still rejects these helpers. This provides a concrete
admission contract after 218 repaired the FP32 source algorithm; it does not implement FP64 support.

## Progress

- [x] Read accepted 218 state, established FP64 bit transport and the shared typed recipe.
- [x] Select research on clean base `7f457354fbbdc2d7c2ec671ae91216f6f2f32fe7`.
- [x] Verify source/artifact/input identities, then GPU smoke.
- [x] Execute the bounded NVRTC 64-bit oracle matrix and assemble its PTX.
- [x] Record exact NVVM O0/O3 preflight diagnostics and the responsible admission boundary.
- [x] Complete source audit, report/evidence/STATUS, local acceptance and commit.

## Surprises and Discoveries

Discovery is at its declared 100-identity capacity. This research adds no registered workload.
A separate capacity change with a full checkpoint is required before another registered fixture.

## Decision Log

2026-09-25: Research before admission. Reuse the validated integer-word oracle algorithm from 217
at width 64, with independently checked constants and mask behavior. Keep fixed eight masks and
add adjacent-finite and subnormal families to the twelve existing semantic families. No compiler
build or production change. Stop if a source-helper/oracle discrepancy cannot be explained as apparatus.

## Outcomes and Retrospective

Completed: all 112 NVRTC cases match the independent 64-bit oracle. Both NVVM modes reject
the canonical scalar double min helper before output; no direct FP64 kernel executes. Source
semantics support a later bounded admission proposal after a separate discovery-capacity slice.

## Context and Current Pipeline

Canonical scalar and Multiple helpers already carry a complete Float64 type and mask signature.
The source algorithm is ordered comparison/selection with caller seed, low-bit contiguous
power-of-two XOR stages or ascending scan of original named-lane values. Accepted 218 implements
that algorithm for FP32. `_getNVVMMaskedWaveScalarIdentity` intentionally rejects FP64 min/max,
and source-algorithm activation currently selects FP32 only. Existing Float64 comparisons, selections
and two-word indexed shuffle transport are available; their composition needs dedicated admission
validation rather than assuming a bit-width flag is sufficient.

## Scope and Non-Goals

Durable plan/report/semantic evidence/STATUS only. No compiler/provider/prelude/runner/manifest
changes or builds. No FP64 prefix admission, unrelated arithmetic, vector-by-value shuffle work,
batching or material runtime/performance claims. Research results stay separate from the registered
53-failure ledger. A subsequent implementation must fail its final registered fixture before edits.

## Architecture and Invariants

Compare raw 64-bit words with NaN classification, equal signed zeros and sign-aware integer order;
select original words without host floating arithmetic. Butterfly reads previous stage states;
scan reads original inputs. Dynamic buffers store each double as two integer words, and output
comparisons include both halves. All named lanes use the same explicit mask; inactive output
sentinels and input storage remain unchanged. NVRTC agreement supplements the independent oracle.

## Interfaces and Dependencies

Native Ubuntu, RelWithDebInfo, L4 SM89 target SM80, driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86,
LLVM 14, provider ABI 36. Source `build/nvvm-loop/slice-203-env.sh` and follow local slang-build skill.
Compiler `a13354a47acefc8684cbfb5ae52b84bcf0331a83e5310e9675f53a0e030107d7`;
provider `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.

## Milestones and Validation

1. Verify all 18 source, 12 artifact and 548 runtime input hashes from 218, then smoke 4/4 via
   `extras/validate-nvvm-runtime.py --config RelWithDebInfo --architecture 80` with recorded CUDA path.
2. Generate one dynamic scalar/double2/double2x2 kernel. Eight masks: full, low16/high16,
   low15/high17, even/odd and singleton31. Fourteen families: finite, infinities, signed zeros,
   all quiet/signaling/mixed NaNs, quiet/signaling NaN at first/middle/last among finite values,
   adjacent finite words distinguishable only below FP32 precision, and signed subnormals.
   Run 112 NVRTC O3 launches, assemble PTX, retain all expected/actual words and oracle self-checks.
3. Compile the exact source at NVVM O0/O3; record rejection and canonical shape. No NVVM FP64
   GPU execution is expected. No permissive patch or alternate source spelling is allowed.
4. Review source-to-recipe boundary, identity hashes and exact counts. Stop broader investigation
   once the admission proposal or semantic blocker is concrete.

## Preservation and Acceptance

All 1,656 registered outcomes inherit 218: 1,603 correct, 53 open failures, four resolved histories.
Its 1,035 off-domain frozen cells remain inherited from full 214. Units 478 plus one skip, toolkit
18 and six material compile/assembly cells inherit unchanged artifacts. Latest full 214, cadence
two; research does not advance it. Only smoke and this FP64 semantic matrix are fresh claims.
No material bindings/oracle exists. Before further registered additions, resolve discovery capacity
in a separate infrastructure slice with full preservation, then perform bounded FP64 admission.

## Failure and Recovery

Approved escalated commands are needed for sandbox bwrap failure. Commands have bounded timeouts.
GPU loss stops dispatch without driver/system changes. Keep apparatus failures and corrections
explicit; never alter expected values to match GPU outputs. Uncertain guarantees remain qualified.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-219-semantics/`. Durable plan, five-part report, semantic evidence and
STATUS. Parent records local review under the delegation limitation and commits accepted research.

2026-09-25 local acceptance: reviewed both-word buffer indexing, integer oracle hand checks,
112 unique cases, all exact expected/actual arrays, strict rejection and source/artifact identities.
No discrepancies or new source-helper blockers appeared. Latest full 214 and cadence two remain.
