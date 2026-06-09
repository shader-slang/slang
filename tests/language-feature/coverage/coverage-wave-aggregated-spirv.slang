// Verifies wave-aggregated coverage counter increments on SPIR-V (#11509).
//
// Instead of every active lane performing its own atomic add of 1 to a
// (heavily contended) counter slot, the lanes active at each marker reduce
// their count with `OpGroupNonUniformIAdd ... Reduce` (== WaveActiveSum(1))
// and a single elected lane (`OpGroupNonUniformElect`) performs one
// `OpAtomicIAdd` of that count. The active-lane reduction preserves the exact
// per-execution total, so coverage numbers are unchanged.
//
// This path engages for Khronos targets when the counter element is uint
// (`-trace-coverage-counter-width 32`); the per-lane atomic is still used
// otherwise (verified by the other coverage SPIR-V tests, which default to
// uint64 and do not emit GroupNonUniform ops).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-branch-coverage -trace-coverage-counter-width 32

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if ((tid.x & 1u) == 0u)
        outputBuffer[tid.x] = 1;
    else
        outputBuffer[tid.x] = 2;
}

// Subgroup arithmetic capability must be auto-declared (it implies the
// base GroupNonUniform capability that the elect below also needs).
//CHECK-DAG: OpCapability GroupNonUniformArithmetic

// The active-lane count reduction and the first-lane election.
//CHECK-DAG: OpGroupNonUniformIAdd {{.*}} Reduce
//CHECK-DAG: OpGroupNonUniformElect

// The counter is still a uint atomic add (the opt-down width), now guarded.
//CHECK-DAG: OpAtomicIAdd
