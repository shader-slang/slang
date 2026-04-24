// Regression test for the deferred-stripDebugInfo code path.
//
// When `-trace-coverage` is on but `-g` is NOT set, the coverage pass
// still needs source locations to resolve counter placeholders to
// (file, line). The normal `stripDebugInfo` pass that runs under
// `-g0` is deliberately deferred until AFTER the coverage pass has
// consumed the locations. This test verifies the compile succeeds
// under -g0 and the resulting C++ still contains counter increments
// (and not placeholder IR ops).

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage -g0

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = 0;
    for (uint i = 0; i < 4; ++i)
    {
        if ((i & 1u) == 0u)
            accum += i;
    }
    outputBuffer[0] = accum;
}

// Counter buffer is emitted even without debug info.
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage

// Counter increment materializes as the CPU prelude atomic helper —
// this is stronger than just checking a counter-slot reference,
// because it verifies the full AtomicAdd lowering path runs.
//CHECK: _slang_atomic_add_u32

// No placeholder IR ops should leak into the emitted C++ — they must
// all have been rewritten by the coverage pass.
//CHECK-NOT: IncrementCoverageCounter
