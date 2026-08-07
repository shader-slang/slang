// HLSL cannot pass thread-group-shared memory across a function boundary (DXC error 0043), but
// `[noinline]` alone does not keep one: the callee below is still inlined into the entry point, so
// the group-shared write lands directly on `groupSharedData` and the program is valid HLSL. The
// check runs after inlining (`checkUnsupportedInst`) precisely so a case like this is not rejected
// for a boundary that never survives (shader-slang/slang#10641).

//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];

[noinline]
void writeSlot(uint tid, groupshared uint scratch[N])
{
    scratch[tid] = tid + 1;
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    writeSlot(tid.x, groupSharedData);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = groupSharedData[(tid.x + 1) % N];
}
