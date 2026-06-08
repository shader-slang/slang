// Verifies that function and branch coverage markers lower to native
// CUDA atomics on the synthesized coverage buffer.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = tid.x;
    if ((tid.x & 1u) != 0u)
        accum += 1u;
    else
        accum += 2u;
    outputBuffer[0] = accum;
}

// CUDA binds global resources through the generated GlobalParams
// symbol, so function/branch coverage must place the hidden buffer
// there just like line coverage.
//CHECK: struct GlobalParams
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage

// One function-entry marker and two branch-arm markers, each wave-aggregated
// (issue #11509): `WaveActiveCountBits` (→ `__popc(__ballot_sync(...))` on CUDA)
// counts the active lanes and `WaveIsFirstLane` elects one lane to run a single
// guarded `atomicAdd(slotPtr, laneCount)`. Bind each atomic to both its slot
// pointer and the lane-count result so the addend is provably the active-lane
// count (not a literal 1) on the correct coverage slot.
//CHECK: __popc(__ballot_sync
// function-entry marker (slot 0)
//CHECK: [[SLOT0:_S[0-9]+]] = {{.*}}_slang_coverage{{.*}}int(0)
//CHECK: [[CNT0:_S[0-9]+]] = WaveActiveCountBits
//CHECK: atomicAdd([[SLOT0]], [[CNT0]])
// branch-arm marker (slot 1)
//CHECK: [[SLOT1:_S[0-9]+]] = {{.*}}_slang_coverage{{.*}}int(1)
//CHECK: [[CNT1:_S[0-9]+]] = WaveActiveCountBits
//CHECK: atomicAdd([[SLOT1]], [[CNT1]])
// branch-arm marker (slot 2)
//CHECK: [[SLOT2:_S[0-9]+]] = {{.*}}_slang_coverage{{.*}}int(2)
//CHECK: [[CNT2:_S[0-9]+]] = WaveActiveCountBits
//CHECK: atomicAdd([[SLOT2]], [[CNT2]])
// No fourth marker slot is synthesized.
//CHECK-NOT: _slang_coverage{{.*}}int(3)
