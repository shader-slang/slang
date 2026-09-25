# Preserve signed BF16 literal bits at the NVVM builder boundary

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow explicitly requires committing completed
plans and reports with the accepted slice. Parent owns acceptance/commit; this worker does not commit.

## Purpose and Observable Result

`pack(BFloat16(-1.25f))` through a noinline helper should execute and return unsigned bits 49056
(0xbfa0), instead of E52018 `canonical BF16 constant bits`, result -2147024809.

## Progress

- [x] 2026-09-25: Read workflow, accepted249, local build skill, producer and provider contracts.
- [x] 2026-09-25: Select bounded correction and record full acceptance domain before implementation.
- [x] 2026-09-25: Before smoke4 passes; unchanged12-word fixture passes NVRTC and fails both direct modes.
- [x] 2026-09-25: Adapt argument with bitCast<int16_t>; explicit emitter formatting and optimized build pass.
- [x] 2026-09-25: Full checkpoint, exact five-field/history comparisons, self-review and evidence complete.
- [x] 2026-09-25: Final handoff prepared for parent independent acceptance; no worker commit.

- [x] 2026-09-25: Parent independently accepted exact corpus/history preservation, output/IR/PTX proof, unit IDs, all identities,119 snapshots and2132 indexed artifacts.

## Surprises and Discoveries

Accepted249 already retains the minimal failure with accepted244 and 249 compiler libraries. Existing
BF16 scalar coverage tests dynamic narrowing and positive constants, so negative canonical literal
arguments require a distinct fixture. The provider accepts signed in-width integers; uint16 0xbfa0
is outside signed16 range even though the bit pattern itself is valid. Final O0/O3 IR preserves
all 12 canonical literals, including -0 and minimum subnormals; emitted helper arguments preserve
all expected encodings exactly. No producer-side correction is necessary.

## Decision Log

2026-09-25, worker: add one distinct discovery source, preserving every old fixture/oracle/ID.
This corpus addition triggers a full checkpoint, regardless of the narrow production change.
Keep provider ABI 41, shared rounding/overflow, BF16 type roles and all storage/vector contracts.
Use existing `bitCast<int16_t>` at the physical builder boundary; no new production helper.

## Outcomes and Retrospective

Final focused 18, units 513 with one unchanged skip, runtime 4, toolkit 18 and runner contracts6 pass.
Full frozen 1356 preserves 1347 correct and 9 unresolved; discovery 345 preserves all 342 old outcomes
and adds 3 correct. All1698 old five-field outcomes match exactly; combined 1701/1662 correct/39
unresolved. All39 unresolved and 18 resolved histories survive. Complex6 compile/assembly passes.
Independent parent acceptance passed; the slice is ready for its local commit.
Independent complete-output audit checks 3444 final words and 12 before words with zero mismatches.
Final identity has119 source/generated/test paths,12 artifacts and 563 runtime inputs. Every final
gate has the same identity; provider binary and ABI 41 are unchanged. Raw index 2132 entries covers
before/final artifacts. The fix resolves the signed API boundary without any producer change.
Next action belongs to parent: commit the accepted slice, then refresh material compile-time work with a fresh
profile. No profiling or next independent blocker investigation is included in250.

## Context and Current Pipeline

`BFloat16(-1.25f)` is checked semantic input. SCCPContext::evalCast uses
IRBuilder::getFloatValue, which canonicalizes with BFloat16ToFloat(FloatToBFloat16(value)).
_getLoweredNVVMValue recovers the existing bits using FloatToBFloat16; lowerType has chosen i16.
The provider `_getIntegerConstant` checks llvm::isIntN then calls ConstantInt::getSigned.
The unsigned short must therefore cross this API as its signed16 bit representation. The producer
is canonical; changing it or widening the provider contract would fix the wrong layer.

## Scope and Non-Goals

Only BF16 canonical literal materialization and finite boundary/signed-zero coverage. No rounding,
overflow, NaN policy, API/ABI, vector/storage/general arithmetic, FP8 expansion or refactoring.

## Architecture and Invariants

Canonical checked IRFloatLit remains the semantic source of truth. The backend preserves its bits
and adapts the argument to the existing signed-in-width API. No syntax reconstruction or fallback.

## Interfaces and Dependencies

No interface changes. Native Ubuntu, CUDA12.9.2, LLVM14, L4SM89 targetSM80, RelWithDebInfo, ABI 41.
Inspect/source `build/nvvm-loop/slice-203-env.sh`; at most 4 CPU workers, sequential GPU suites.

## Milestones

1. Capture accepted249 identity and smoke; add `tests/cuda/nvvm-bf16-signed-literals.slang` covering
   exact positive/negative finite normal/subnormal boundaries and both zeros through a noinline helper.
2. Retain before NVRTC O3 success and NVVM O0/O3 failure, then adjust only
   `source/slang/slang-emit-nvvm.cpp`; format that explicit C++ file before final tests.
3. Register source, build, run full final acceptance and produce durable250 evidence.

## Validation and Acceptance

Fresh domain: frozen 452 sources/1356 cells and discovery 115 sources/345 cells, each NVRTC O3,
NVVM O0/O3; all 6 complex cells compile/assembly only. Old1698 outcomes compared exactly on
classification, return_code, execution_counts, diagnostic, canonical_shape; all 1659 old passes
and 39 unresolved histories plus 18 resolved histories preserved. New3 cells reported separately.
Fresh smoke4, focused BF16 scalar/vector/dot and FP8 literal/transport neighbors plus new fixture,
all NVVM/routing/reporter/math and doubleSourceLiteralsRoundTrip units, toolkit 18, runner contracts6.
Every new literal output checked against independent exact encodings; NVRTC only supplements them.
Historical material timings, texture/matrix research and FP8 exhaustive supplements remain inherited
with accepted source identities. No full before rerun when accepted249 source/artifact/input hashes match.
Build: `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4
--target slangc slang-test render-test test-server`. Gate commands derive from WORKFLOW reference,
with corpus jobs4 and units 2 servers, each under `timeout --kill-after=30s 30m`.

## Failure and Recovery

Each source state has fresh roots `build/nvvm-loop/slice-250-before` and `slice-250-after`.
Preserve failed attempts. New regressions block acceptance; isolate/revert and rerun affected checks.
Stop GPU dispatch on device loss. No driver/system changes, reboot, push or material runtime claim.
Record any next independent blocker minimally and defer investigation.

## Artifacts and Hand-Off

Retain source/binary/input identities, snapshots, raw logs/output, commands, exact comparison and
artifact index. Deliver completed plan, five-part report, validation250, census/discovery 250 and
STATUS draft. Parent independently reviews final diff/evidence and owns the local commit.
