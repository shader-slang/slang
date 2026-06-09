// Verifies wave-aggregated coverage counter increments on SPIR-V (#11509).
//
// Instead of every active lane performing its own atomic add of 1 to a
// (heavily contended) counter slot, the lanes active at each marker reduce
// their count with `OpGroupNonUniformIAdd ... Reduce` (== WaveActiveSum(1))
// and a single elected lane (`OpGroupNonUniformElect`) performs one
// `OpAtomicIAdd` of that count. The active-lane reduction preserves the exact
// per-execution total, so coverage numbers are unchanged.
//
// This path engages for the direct SPIR-V backend (GLSL and other targets
// keep the per-lane atomic) for both counter widths: uint32 adds the count
// directly; uint64 widens the uint count with `OpUConvert` before the
// 64-bit atomic.

// uint32 counters (`-trace-coverage-counter-width 32`).
//TEST:SIMPLE(filecheck=CHECK32):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-branch-coverage -trace-coverage-counter-width 32

// uint64 counters (the default width).
//TEST:SIMPLE(filecheck=CHECK64):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-branch-coverage

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

// --- uint32 path ---
// Subgroup arithmetic capability is auto-declared (it implies the base
// GroupNonUniform capability that the elect also needs).
//CHECK32-DAG: OpCapability GroupNonUniformArithmetic
//CHECK32-DAG: OpGroupNonUniformIAdd {{.*}} Reduce
//CHECK32-DAG: OpGroupNonUniformElect
//CHECK32-DAG: OpAtomicIAdd

// --- uint64 path: same aggregation, plus a widen of the uint count to the
// 64-bit counter element before the atomic. ---
//CHECK64-DAG: OpCapability GroupNonUniformArithmetic
//CHECK64-DAG: OpCapability Int64Atomics
//CHECK64-DAG: OpGroupNonUniformIAdd {{.*}} Reduce
//CHECK64-DAG: OpGroupNonUniformElect
//CHECK64-DAG: OpUConvert %ulong
//CHECK64-DAG: OpAtomicIAdd %ulong
