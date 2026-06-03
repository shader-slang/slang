// Confirms the coverage instrumentation pass doesn't run when the user
// hasn't asked for it. Without `-trace-coverage`, neither the
// synthesized `__slang_coverage` buffer nor any atomic-add calls
// should appear in the emitted code. Regresses if the pass starts
// running unconditionally or if `requiredLoweringPassSet.coverageTracing`
// gating is removed.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 42u;
}

// The shader must still compile correctly (sanity anchor).
//CHECK: RWStructuredBuffer{{.*}}outputBuffer

// No synthesized coverage buffer.
//CHECK-NOT: _slang_coverage

// No atomic-add helper call — the coverage pass is the only source of
// these on a shader that has no atomics of its own.
//CHECK-NOT: _slang_atomic_add_u32
