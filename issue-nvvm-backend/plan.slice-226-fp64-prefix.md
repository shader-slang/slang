# Establish FP64 masked prefix min/max semantics

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception. Base is
`d3d8a77b85757ca0d6576196dbffde25237ff4df`. Fresh-context delegation was retried for226 and again hit
the agent-thread limit; local parent execution follows WORKFLOW's fallback.

## Purpose and Observable Result

Determine the canonical CUDA source behavior for scalar and compound Float64 inclusive/exclusive
masked min/max prefixes before extending direct NVVM admission. Slice225 removes the Boolean context
restriction; four original frozen cells now reject canonical double exclusive-prefix GenericAsm.
A bounded research result must identify identities, traversal/operand order and bitwise behavior for
NaNs, signed zero, adjacent finite values and subnormals, with independently computed expectations.
No production patch or registered corpus addition belongs to this slice.

## Progress

- [x] Select on accepted full225; verify clean checkout and exact base.
- [x] Retry fresh-context worker; record unavailable delegation and local fallback.
- [x] Inspect source operation policy, producer/consumer shape and existing raw-bit probe harness.
- [x] Run smoke gate, compile/assemble and execute independent dynamic source probes.
- [x] Record direct rejection, precise semantics, hashes and bounded next implementation handoff.
- [x] Complete report/evidence/STATUS; parent review and authorized local commit.

## Surprises and Discoveries

Pending. Source `_wavePrefixScalar` maintains transmitted inclusive value separately from returned
accumulator. Low contiguous power-of-two masks use ascending shuffle-up offsets; other masks scan
original source lanes in ascending order. Verify operation initial values and compound behavior.

## Decision Log

2026-09-25: Choose FP64 prefix research because the four original frozen cells now expose this exact
boundary and existing min/max reductions already required source-order semantics. Context resource
support and ordinary FP64 vector-by-value shuffle can wait; they do not own this failure. Material
runtime remains unavailable without its application contract; no speculative runtime claim.

## Outcomes and Retrospective

196/196 source executions match the integer oracle:57,624 active binary64 results plus117,992
inactive sentinels. Eight direct probes reject canonical double prefix GenericAsm before PTX.
A universal ascending-scan countermodel differs8,658 words across32 cases, establishing that a later
implementation must retain the source tree/order and separate transmitted/returned state.
Smoke4 passes; all26 source/12 artifact/551 runtime input hashes match full225. Registered1665/1614
correct/51failure/sixresolved outcomes inherit225; cadence0 unchanged. Matrix prefix capability
rejection is retained separately. No production change. Completed plan/report/evidence are committed
under the NVVM exception after parent review.

## Context, Architecture and Interfaces

Follow hlsl.meta.slang intrinsic mapping through the CUDA prelude operation policy and
`_wavePrefixScalar`/`_wavePrefixMultiple`, then the direct emitter's existing wave reduction recipe.
Canonical signature is `double(double, vector<uint,4>)`; masks are runtime data, not source constants.
Use an integer binary64 comparator to preserve NaN payloads and signed-zero ordering without host
floating-point arithmetic. Keep source-backend behavior and mathematical expectations explicit.

Native Ubuntu L4 SM89 target80, CUDA12.9.2/NVRTC12.9.86, LLVM14, ABI36, matching RelWithDebInfo tools.
Source `build/nvvm-loop/slice-203-env.sh`. No build is needed for research-only sources. Compiler hash
14e80c03ff1571a2248f935cc795a9465ec963f9d3ac5cf4a7f80ba076a48ae1, provider unchanged from225.
Sequential suites, at most four workers; bound each compiler/GPU suite. No push or system changes.

## Milestones and Validation

1. Retain a concrete source example and exact policy/consumer trace. Reuse prior raw-bit CUDA driver
   apparatus and data families, but derive prefix expectations independently for each lane.
2. Cover inclusive/exclusive min/max, scalar and representative vectors/matrices, full/low-power-of-two,
   irregular/high-only/alternating/singleton masks. Dynamic input words include payloads in both halves,
   finite/infinite values, signed zeros, quiet/signaling NaNs and precision boundaries.
3. Smoke4 before GPU suite; source compile/assembly and expected-word comparisons, inactive sentinel
   preservation. Direct O0/O3 preflight probes must retain exact diagnostics and absence of PTX.
4. Preserve all source/artifact/runtime-input hashes from225 and all registered outcomes historically.
   Full225 remains1665 cells/1614 correct/51 failures/6 resolved histories, cadence0. Research does
   not rerun unchanged full corpora. No declaration of direct support from CUDA-only executions.

## Failure and Recovery

Keep mismatches as research findings, not adjusted expectations. Separate true source semantics from
harness bugs with concrete minimal traces. Stop on GPU loss; do not retry indefinitely or change
systems. Stop at the next independent unsupported operation once its diagnostic is recorded.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-226-prefix`; durable `semantic-evidence.slice-226.json`, five-part report,
completed plan and STATUS. State exact executed/expected counts and unchanged-hash obligations.
Parent owns acceptance and local commit. No independent worker review is claimed.

2026-09-25 source gate: matrix prefix overloads require glsl_spirv at hlsl.meta.slang:19082 and
reject CUDA with E36100/E36107 before PTX. Retain exact initial source/order/logs under
`initial-matrix-capability`; this separate capability boundary is deferred without production edits.
Narrow the executable research to scalar, double2 and double4, preserving input families and
independent per-component expectations. No matrix prefix support is claimed. Smoke4 passes.

2026-09-25: research complete. Source oracle passes all196 dynamic cases, eight direct preflight
probes remain rejected, and initial matrix capability failure is retained. Next implementation must
use source-order FP64 prefix seeds/state, not merely numeric identities; preserve existing recipes
and reassess originals without expanding into another independent blocker.
