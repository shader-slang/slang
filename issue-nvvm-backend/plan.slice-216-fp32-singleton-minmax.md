# Preserve FP32 singleton masked min/max operands

This ExecPlan follows `.agent/PLANS.md`. WORKFLOW requires committing completed plans/reports;
raw evidence stays ignored. Worker owns checkout writes until explicit return; parent accepts/commits.

## Purpose and Observable Result

A dynamic FP32 singleton `WaveMultiMin/Max` returns the original raw operand bits, including
quiet/signaling NaNs, for scalar, vector and matrix values on direct NVVM O0/O3 as on CUDA NVRTC.

## Progress

- [x] 2026-09-24: Read instructions, build skill, 208/213/214 evidence and accepted215 research.
- [x] 2026-09-24: Native Ubuntu clean base230c3e0eae73be3b2ff01e26e3d346e12fe9b86c; plan before edits.
- [x] 2026-09-24: Establish final dynamic fixture before-change behavior with unchanged accepted214 artifacts.
- [x] 2026-09-24: Separate common singleton selection from FP64-only seed and enable FP32 min/max preservation.
- [x] 2026-09-24: Format, build, run final gates and exact preservation comparison.
- [x] 2026-09-24: Complete self-review, durable report/results/STATUS; return write ownership with final handoff.

## Surprises and Discoveries

Accepted215 records qNaN7fc12345/sNaN7f812345 at singleton lane31 becoming +/-infinity in
NVVM O0/O3. CUDA preserves raw bits. This defect is separate from the registered53 failures.
Existing singleton predicate is mixed into the FP64 seed helper; separate those responsibilities.

## Decision Log

2026-09-24 worker: correctness takes priority over deferred FP64 admission and batching optimization.
Reuse the exact107 frozen selection from208/213 and execute full discovery98 plus one new identity.
This covers every wave/quad and selected double/helper/value/vector/matrix neighbor, with complete
discovery covering aggregate/control-flow/storage interactions. Production scope is one recipe;
no provider/library/ABI/general lowering change. Targeted acceptance advances full214 cadence0 to1.

## Outcomes and Retrospective

Implementation and validation complete; ready for parent acceptance. Fresh618 cells contain
580 correct/38 unchanged known failures. Three additions pass, all615 fresh old cells match every
required field,1035 frozen cells explicitly inherit214. Cumulative1653 cells:1600 correct/53 known
failures; four resolved histories retained. Exact215 replay fixes its four research executions
separately. Latest full214; targeted216 advances implementation cadence to1 upon acceptance.

## Context and Current Pipeline

Consider `float v = asfloat(inputBits[lane]); WaveMultiMin(v, uint4(1u << lane,0,0,0))`.
`hlsl.meta.slang` specializes valid canonical scalar/Multiple GenericAsm helpers. Exact spelling
and typed signature resolution builds `NVVMMaskedWaveScalarOperation`; aggregate recursion reuses
that recipe per leaf. `_emitNVVMMaskedWaveScalarValue` injects infinity and scans with numeric
min/max. Numeric min/max correctly prefers infinity over a NaN, but singleton source helpers do
no arithmetic. The recipe owns this algorithm mismatch; source and IR are canonical and valid.

## Scope and Non-Goals

Only shared singleton predicate/final selection and FP32 min/max recipe activation, one focused
GPU source/discovery identity, relevant structural assertions and evidence. No FP64 min/max admission,
no nonsingleton NaN/order investigation/change, unrelated sums/products/prefixes, provider contract,
runner/oracle changes, material runtime claims, system changes, commits or pushes by worker.

## Architecture and Invariants

Keep FP64 signed-zero seed logic exclusive to FP64 sums. Common singleton selection uses existing
typed negation, AND, equality and select recipes. All named participants call the same mask;
mask0 is outside this contract. No new canonical shape or fallback. Preflight operation closure
must include equality/select for affected FP32 recipes; malformed shapes and FP64 min/max still reject.

## Interfaces and Dependencies

ABI36 unchanged; native RelWithDebInfo compiler2775a5783a7dd1310ab9773d259bf1bdafc464a4f55a376c1fc22c472a6b1bf0
and provider ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372 before patch.
L4 SM89 driver580.126.09, targetSM80 CUDA12.9.2/NVRTC12.9.86 LLVM14.
Source build/nvvm-loop/slice-203-env.sh; native local slang-build skill; four CPU workers total,
sequential suites, units two servers. Sandbox bwrap RTM_NEWADDR requires approved escalated commands.

## Milestones

1. Add final runtime-loaded scalar/float4/float2x2 fixture with raw-word singleton oracle at all32
   positions and +/-0, finite, infinities, signed payload-distinct quiet/signaling NaNs; finite
   nonsingleton full/partial/sparse neighbors. Run unchanged binaries before patch.
2. Extract common singleton predicate; preserve FP64 sum seed separately; extend existing typed
   structural coverage only as needed. Format explicit changed paths and build all four tools.
3. Smoke then focused, units, toolkit18, targeted frozen321/full discovery297 and all6 material
   compile/assembly cells. Stop on GPU loss; no expensive dispatch before small smoke.
4. Verify exact requested keys and stable five-field outcomes against214, plus three additions.

## Validation and Acceptance

Build: `cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
Use WORKFLOW native commands with fresh `build/nvvm-loop/slice-216-{before,after}` outputs and
bounded timeout. Frozen `--workload-ids-from issue-nvvm-backend/selection.slice-208-frozen.tsv --jobs 4`;
discovery full authoritative manifest99 (loader requires50-100 before filtering), all three modes.
Expected old deltas zero; new fixture3 correct; separate research215 defect resolution. Every old
classification, return_code, full execution_counts, diagnostic, canonical_shape must match.
Explicit inheritance: unexecuted1035 frozen cells, all53 unresolved histories and four resolved
histories from full214; never describe inherited cells as freshly run. Cumulative ledger1653 cells,
1600 correct/53 known failures if all gates pass. Full214 stays latest full checkpoint, cadence1.
Full checkpoint triggers: broad/shared lowering, provider/library/ABI or runner contract change,
uncertain impact, unexplained regression. Independent aggregate unsupported boundary stops scope
expansion and requires parent decision. All6 material cells only compile/assemble, absent runtime oracle.

## Failure and Recovery

Retain pre-change evidence and accepted215 unchanged. Diagnose apparatus failures separately from
compiler failures. Do not weaken oracles. Stop new independent investigation after minimal handoff;
GPU loss stops dispatch without driver changes/reboot. Final source changes require affected reruns.

## Artifacts and Hand-Off

Durable: this plan, report.slice-216-fp32-singleton-minmax.md, runtime-validation.slice-216.json,
full inherited/fresh census TSVs, discovery manifest, STATUS. Raw logs/scripts/PTX under ignored
build/nvvm-loop/slice-216-*; parent receives <=500word handoff with exact identities and ownership.

## Before-Change Reproduction

Final formatted fixture on exact214 compiler/provider: NVRTC1/1 passes; NVVM O0/O3 both
execute and fail raw-bit oracle, with all lanes returning0. No aggregate unsupported boundary.
Source fixture SHA256 and before log retained separately; no source change before that run.

## Focused Milestone

Final GPU fixture passes all3 modes after the recipe change; unchanged FP64 edge fixture passes
all3. Initial structural fixture used a double kernel entry parameter outside existing admission;
replace that test apparatus with established lane-derived values and integer destination (as213).
No production admission expansion. The initial test macro needed braces around SLANG_CHECK.
Both apparatus errors are retained in raw logs, not counted as accepted validation.

## Final Source Freeze and Gate Progress

Source snapshot frozen at base230c3e0eae73be3b2ff01e26e3d346e12fe9b86c plus final emitter/test changes.
Compiler SHA256 fa55d1fdc41988e27e672d4a2ad92b93060293d101f4a7a076be3e68dd08298f; provider
unchanged ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372. Final unit library
SHA2565800c6d8dbea408ee536431d61653de18219b9b6ed96f1d1233304dde989b9fd.
Fresh smoke4/4, focused10/10, units478/478 with existing Windows-only skip, toolkit18/18 pass.
Frozen/discovery/complex and separate215 exact-source replay remain pending at this milestone.

## Final Outcome and Ownership

All final gates completed: smoke4, focused10, units478 plus existing Windows-only skip, toolkit18,
frozen321, discovery297, material6 compile/assembly. Frozen exit0 retains8 preflight failures under
diagnostic mode; discovery exit2 retains30known failures. No missing/duplicate/extra cells and no
old five-field delta. All546 original runtime source hashes match214; final fixture hash is unchanged
from before run. All17 tested-source and12 artifact hashes rechecked after gates.

The original215 source replay passes12/12 executions and24/24 exact raw words, with3/3PTX assembly;
the separate research defect is resolved without rewriting the registered53-failure ledger. No new
independent issue, GPU loss or system change. Durable design now separates singleton preservation
from FP64 sum seed semantics. Parent receives checkout ownership and accepts/commits; worker does
not commit or push. Next action is deferred min/max semantic research; nonsingletonFP32, FP64,
aggregate and order semantics remain unresolved, and singleton correctness alone admits noFP64work.

Parent acceptance, 2026-09-24: independently reviewed the final production, structural-test and
runtime-oracle changes. All 615 old fresh outcomes match accepted 214 across five stable fields,
and three additions pass. Verified 101 evidence references, 17 tested sources, 12 artifacts and
547 runtime input hashes. All first-known failure records and four resolved histories survive.
Accepted as targeted implementation; full checkpoint 214 remains current and cadence is one.
