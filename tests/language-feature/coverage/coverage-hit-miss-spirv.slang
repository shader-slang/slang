// Verifies hit/miss coverage mode (`-trace-coverage-hit-miss`, #11509).
//
// In hit/miss mode each coverage counter is written with a plain non-atomic
// store of 1 (CoverageCounterMode::Boolean) instead of an atomic add, so it
// records whether an entry executed (0 / non-zero) rather than an exact count.
// This removes all atomic contention. Off by default — the default count mode
// still uses atomic adds (asserted by the COUNT run below).

// Default (count) mode: coverage uses atomic adds.
//TEST:SIMPLE(filecheck=COUNT):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-branch-coverage

// Hit/miss mode: no atomics at all.
//TEST:SIMPLE(filecheck=HITMISS):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-branch-coverage -trace-coverage-hit-miss

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

// Count mode increments counters atomically.
//COUNT: OpAtomicIAdd

// Hit/miss mode emits a plain store of 1 (not an atomic add).
// The positive OpStore check ensures the instrumentation actually ran and
// emitted something — preventing the NOT check from passing vacuously if
// the instrumentation were silently dropped.
//HITMISS: OpStore
//HITMISS-NOT: OpAtomicIAdd
