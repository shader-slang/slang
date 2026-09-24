# Slice 213: FP64 implicit aggregate indexed shuffles

## Motivation

Consider a full-warp kernel that moves all four components of a double matrix:

```slang
RWStructuredBuffer<uint64_t> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    double value = bit_cast<double>(
        (uint64_t(0x40000000u + lane) << 32) | uint64_t(0x12345678u + lane));
    double2x2 matrixValue = double2x2(value, value, value, value);
    double2x2 shuffled = WaveReadLaneAt(matrixValue, int(31 - lane));
    outputBuffer[lane] = bit_cast<uint64_t>(shuffled[0][0]);
}
```

CUDA's canonical matrix helper is valid, but direct NVVM rejected its Float64 leaf at a
conservative admission guard. Slice 208 deliberately retained that guard because its old implicit
mask composition named bypassing lanes. Slice 209 corrected the composition using hardware reads
and ABI 36 while leaving new FP64 admission unvalidated. The final focused fixture passes NVRTC
and fails both direct modes at this exact guard on accepted 210 binaries.

## Proposed solution

Remove only the Float64 implicit-helper rejection. Reuse complete signature validation, homogeneous
aggregate recursion, slice 209 raw-read/ballot composition and existing exact 64-bit scalar shuffles.
No new semantic representation, arithmetic, fallback, provider operation or ABI is needed.

## Change summary

- `source/slang/slang-emit-nvvm.cpp`: remove seven-line deferred-admission guard.
- `tests/cuda/nvvm-fp64-implicit-shuffle.slang`: one independently expected bit-pattern fixture,
  with full-warp reverse-lane movement and self reads under partial, sparse and singleton branches.
- NVVM emitter/support units: a separate FP64 matrix session checks typed scalar leaves and mask
  provenance; remove the superseded negative case and honor declared operand width in the fake.
- Discovery manifest: add one source identity, preserving all old source contracts and frozen v1.
- Design note, completed plan, result manifest/outcome tables and STATUS: record exact scope/evidence.

## Concepts and vocabulary

An implicit aggregate shuffle obtains its mask inside the canonical CUDA helper. The hardware
snapshot is scheduling-dependent; a source branch does not promise that every branch lane appears
in that snapshot. A lowered matrix is a fixed array of vectors, returned by a Void helper through
an OutParam. A typed scalar leaf shuffle transports one component using its declared binary width.

## Process report

The matrix overload in `hlsl.meta.slang` produces
`GenericAsm("_waveShuffleMultiple(_getActiveMask(), $0, $1)")`. Matrix and return lowering preserve
its checked element type as `Void(Array<vector<double,2>,2>, int, OutParam<Array<vector<double,2>,2>>)`,
exactly the before-change diagnostic. This is an intentionally canonical shape, not a producer
accident. `_resolveNVVMAggregateWaveOperation` validates the finite helper spelling, signed lane,
homogeneous leaves and exact out-pointer type. The only rejected property was Float64 with an
implicit mask. The producer requires no change.

`_emitNVVMAggregateWaveOperation` reads `WAVE_ACTIVE_MASK`, then computes
`WAVE_MASK_BALLOT(snapshot,true)` to implement the CUDA prelude `_getActiveMask()` contract.
`_emitNVVMAggregateWaveShuffleValue` recursively extracts each component and emits the selected
`WAVE_READ_LANE_AT` descriptor. Provider `_emitWaveReadLaneAt` bitcasts double to i64, shuffles
each i32 half with identical mask/lane/clamp, combines the halves and bitcasts back. It performs
no floating arithmetic, so neither signed zeros nor signaling/quiet NaN payloads may change.
The semantic descriptor and lowered aggregate type remain the source of truth throughout.

The final GPU oracle constructs independently specified integer words for a finite value with
lane-varying high and low halves, both zero signs, a quiet NaN and a signaling NaN. It compares
all returned components as uint64 bits and requires one marker for every lane. Reverse-lane
movement occurs before divergence along straight-line full-warp code. Partial low16, sparse
lane-mod4 and singleton lane31 branches read each caller's own lane, which belongs to the helper's
snapshot regardless of scheduling. No earlier sampled mask is assumed equal to the later helper
snapshot. PTX retention findings and exact per-mode counts are in the result manifest.

The new structural unit also found a test-double defect. `_fakeNVVMBuilderEmitIntrinsic` consumed
a valid Float64 operand descriptor but passed a hardcoded 32 to
`_isFakeNVVMBuilderFloatingPointValue`, returning invalid-argument for the shuffle. It now uses
`operation.operandTypes[i].bitWidth`; the existing validator already supports that width. The
parent approved this test-only correction. Existing 32-bit unit coverage remains unchanged.
An initial combined int/double matrix fixture also exceeded the fake's single array-element-type
slot; a separate fresh fake session avoids changing that unrelated infrastructure. A provisional
Ptr<double> CUDAKernel entry was replaced by its established Ptr<int> output contract; no new
entry ABI is claimed. Those rejected probes remain raw investigation evidence.

Helper/fallback inventory: no new production helper or fallback; remove one conservative special
case. The new test helper `checkBits` owns the independent raw-bit oracle and survives. The fake
validator correction survives because it consumes the existing typed descriptor rather than
inventing a special Float64 exception. No arbitrary graph walk, syntax reconstruction, alternate
value representation or structural equivalence is introduced. The unchanged final GPU fixture's
original preflight failures provide the minimal revert proof; no provider artifacts were mixed.
Malformed mask/signature, floating bitwise and FP64 min/max rejection coverage stays intact.

Targeted acceptance is appropriate because only this explicit type-domain guard changes in
production. It replays 107 frozen wave/quad/double/helper neighbors and all 98 discovery identities,
all three modes. The remaining 1035 frozen cells inherit full 210 explicitly. Full checkpoint 210
remains current; implementation cadence is 1 after parent acceptance. The complex
corpus's six cells are compile/assembly probes; missing material bindings, textures/LUT/inputs and
output oracle prevent material runtime claims. Production batching remains separate research.

Validation completion and exact identity/counts are recorded in `runtime-validation.slice-213.json`.
Parent owns acceptance and local commit; worker performs no commit or push.

### Final validation and preservation

| Gate            | Result                                                                         |
| --------------- | ------------------------------------------------------------------------------ |
| Focused         | 6/6: three GPU modes, FP64 mask chain, existing aggregate and strict negatives |
| Runtime smoke   | 4/4                                                                            |
| Units           | 477/477; one existing Windows-only skip                                        |
| Toolkit         | 18/18 compile/assembly                                                         |
| Frozen targeted | 321 fresh: 313 correct, eight unchanged preflight stops                        |
| Discovery full  | 294 fresh: 264 correct, 30 unchanged failures                                  |
| Complex         | 6/6 compile/assembly; no material runtime claim                                |
| Focused PTX     | Three additional successful assemblies and exact operation/control-flow audits |

There are 615 fresh cells and 1035 explicitly inherited frozen cells. All 612 fresh old cells match
five exact stable fields; 574 previously correct cells are freshly preserved and three additions
pass. Cumulative 1650 cells contain 1597 correct, 53 open failure histories and four resolved
histories. No old delta, missing cell or duplicate exists. The frozen diagnostic subset returns 0
for preflight-only gaps; discovery returns 2 for retained infrastructure/output gaps.

Actual PTX counts (raw reads / ballots / i32 shuffles) are 4/4/32 for NVRTC O3, 1/1/8 for NVVM O0
and 4/4/32 for NVVM O3. O0 calls its shared helper four times. Both optimized modes retain all
four inline sites and the low16, sparse lane-mod4 and singleton lane31 branch predicates. Every
ballot's mask register comes from a hardware read, its predicate is true, and each ballot feeds
eight shuffles. No divergent self-shuffle site is folded away. These are static PTX observations
plus independently checked GPU output, not an assertion about arbitrary sparse cross-lane cohorts.

Compiler SHA256: `2775a5783a7dd1310ab9773d259bf1bdafc464a4f55a376c1fc22c472a6b1bf0`.
Provider SHA256: `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372` (ABI 36).
Base `8075158ca3631ba72cd11df810ffc29d2628e3f7` plus final source hashes identifies the tested tree.
All recorded hashes match after gates. Raw logs, rejected test-development probes, PTX and binaries
remain under `build/nvvm-loop/slice-213-{before,after}`. No GPU loss or system changes occurred.

Parent independently reviewed the production/test diffs, exact preservation, source/artifact hashes
and PTX mask/transport evidence and accepted this targeted slice.
