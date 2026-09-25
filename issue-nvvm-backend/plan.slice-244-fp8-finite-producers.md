# Repair finite FP8 conversion producers

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer exception requires this completed plan
and report to be committed by the parent; raw evidence remains ignored under build/.

## Purpose and Observable Result

`asuint(float(FloatE4M3(256.0f)))` must produce the Float32 encoding of 256 rather than448.
Minimum E4M3/E5M2 subnormals must round-trip as2^-9 and 2^-16, including negative signs.

## Progress

- [x] 2026-09-25: Read workflow, STATUS, accepted 243 contract/report, build skill and 242 harness.
- [x] 2026-09-25: Frozen52-word fixture compiles but fails all 3 modes on accepted binaries.
      Independent3548-case host grid has 220 before mismatches, 0 after. Final direct preflight IR
      contains only ordinary Float/UInt output; no new backend admission.
- [x] 2026-09-25: Implemented finite/subnormal correction and 3 independent math units; early
      diff and before evidence reviewed by parent; independent3548-case and 52-word oracles agree.
      Parent approved final build with no code changes. Source formatted before build.
- [x] 2026-09-25: Final formatted build passes758 steps. Smoke4, fixture 3, helper 3548,
      trace3, dynamicNVRTC2/24290words, units 511+one oldskip, toolkit 18, contracts 6, full frozen 1356/
      discovery 339 and material 6 pass their prescribed contracts.
- [x] 2026-09-25: Completed five-part report, durable contract, exact five-field comparison,
      census/addition, compact evidence and immutable index. Independently accepted by parent; checkout released for the local commit.

## Surprises and Discoveries

The host sandbox fails with bwrap RTM_NEWADDR; routine commands require approved escalation.
Initial formatting lacked tools on PATH; existing build/nvvm-setup tools resolved this without install.
A duplicated keyword in the evidence generator was found by py_compile and corrected before gates.
The full build changes seven artifacts, while the provider binary stays exact. All 43 source paths,
12 artifacts and 561 inputs match final identity at every gate and after. Historical indices retain
117/132/88/129/180 entries and all 19 immutable primary-source snapshots.
Research243's CUDA SATFINITE oracle is not the narrowing policy of these shared helpers.

## Decision Log

2026-09-25 worker: rank finite/subnormal correctness first, narrow FP8 backend admission second,
independent texture/dynamic-object boundaries later. Demonstrated wrong values outrank admission.
Material runtime was reconsidered: bindings, textures/LUTs, inputs and output oracle remain absent.
This correctness slice overrides complex-runtime cadence. No next-slice investigation is authorized.
Preserve E4 absolute-input >448 NaN guard and E5 rounded overflow/infinity/NaN behavior.

## Outcomes and Retrospective

Finite/subnormal producers are corrected with unchanged overflow policy. All 1692 old cells match
exactly; 3 new cells pass, yielding1695fresh/1654correct/41unresolved and 16 unchanged resolved
histories. No backend FP8 admission or new corpus blocker. Full checkpoint 244 is independently accepted; cadence resets to zero. RawLLVM243 is historical; two final actual source NVRTC controls independently
check 24290 complete words. The remaining overflow-policy difference is explicitly deferred.

## Context and Current Pipeline

`SCCPContext::evalCast` calls `IRBuilder::getFloatValue`, which narrows and widens through
`source/core/slang-math.h`. SPIR-V emission/reflection also consume these helpers. Valid canonical
finite Float32 literals reach the producer: no syntax rebuilding or alternative semantic shapes are
needed. The prior producer mishandled subnormals and clamped all E4 exponent 15 values to 448;
NVVM must not compensate. Existing FP8 runtime values remain unsupported in direct lowering.

## Scope and Non-Goals

Only FloatToFloatE4M3, FloatToFloatE5M2, FloatE4M3ToFloat, FloatE5M2ToFloat and meaningful tests.
No backend admission, descriptor/provider ABI, runtime casts, storage/vectors/dynamic dispatch or
external ABI. Preserve overflow/NaN/sign policy outside finite corrections. No worker commit/push.

## Architecture and Invariants

Each valid finite byte widens exactly. In-range Float32 narrows to nearest with ties to even,
including zero/subnormal and subnormal/normal boundaries. E4 exponent 15 values256..448 remain
ordinary normals. Signed zeros survive. Independent oracle enumerates dyadic representable values,
never copying production shift arithmetic. The helper/fallback inventory and producer audit passed parent review.

## Interfaces and Dependencies

Existing inline helper signatures stay unchanged. Native Ubuntu RelWithDebInfo preset,
CUDA 12.9 SM80/L4, ABI40; inspect/source build/nvvm-loop/slice-203-env.sh. Four CPUs maximum,
sequential GPU and each command bounded30m. Units use two servers.

## Milestones

1. Create immutable before evidence and focused fully folded source fixture. Confirm direct IR has
   only ordinary32-bit outputs. Escalate to parent if this shape cannot be achieved without admission.
2. Fix producer and add all 256-byte widening/roundtrip plus every finite value/midpoint/neighbor
   rounding tests, both signs, and overflow-policy checks. Send diff/proof for parent early review.
3. Format all source before expensive build, then run matching releaseWithDebugInfo target build.
4. Run final gates, classify every old cell exactly and finish report/contract/evidence.

## Validation and Acceptance

Before: accepted binaries SHA compiler79f46ef0dba116bdfdce8a03b40f7d3bb2c7b481529b452b483d650cf0958e43,
provider c0522674424c86dbc9444b2abc202c97146a6b41e3a9179d95d34ec9fe1b0773. Current source
base41f070c689cb46e91b3939ca038654b91ef96944 differs from tested242base d6c26eb4cf5960ac07feff8158d15163cc2757fc;
research 243 did not alter production. Capture actual helper grid and source failures before edits.
Mandatory full checkpoint because shared producers: smoke 4 first; fixture 3 if eligible; NVVM/routing/
reporter/doubleSourceLiteralsRoundTrip and math units; toolkit 18; runner contracts 6; frozen 452/1356
with explicit census.slice-195.tsv; discovery 112/336 old plus exactly one distinct eligible fixture;
material 6 compile/assembly. Replay actual Slang NVRTC dynamic research grids with unchanged oracles.
Raw LLVM controls remain inherited with their original artifact identities. Provider source/ABI
are unchanged; a shared-header rebuild may still change its binary hash. Never label controls fresh.
Before every gate and afterward capture expanded source hashes, 12 artifacts, 560 unchanged old inputs
plus fixture. Compare complete classification/return_code/execution_counts/diagnostic/canonical_shape
for every old cell: 1692total/1651correct/41unresolved/16resolved histories. Expected addition gives
1695/1654/41/16, governed by evidence. No baseline reset. Preserve accepted indices 238/240/241/242/243,
including243 immutable primary-source snapshots; live math source hash intentionally changes.

## Failure and Recovery

Stop GPU on device loss, never modify driver/system or reboot. Preserve failed attempts, fix or
revert genuine new regressions and repeat affected gates. Unsupported backend values in the fixture
require parent scope review, never emitter workaround. Overflow-policy questions stay separate.

## Artifacts and Hand-Off

Raw roots build/nvvm-loop/slice-244-before and slice-244-after. Complete plan/report, FP8 design,
compact runtime-validation.slice-244.json, census files and discovery addition if qualified.
STATUS names accepted full 244. Parent owns the authorized local commit.

Independent parent acceptance checks 3548 helper cases, all 156 fixture GPU words and 24290 dynamic
replay words with a separate rational oracle. Metadata review verifies 716 compact / 1275 total
evidence references before its seven own references, all 58 indexed artifacts, exact old outcomes
and failure histories, and all 19 historical primary-source snapshots. See parent-acceptance-audit.json
and the six companion audit artifacts referenced by the compact validation manifest.
