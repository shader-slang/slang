// `[noinline]` alone does not keep a function boundary: the callee below is still inlined into the
// entry point, so the group-shared write lands directly on `groupSharedData` and the program is valid
// HLSL. Only a boundary that actually survives is rejected (shader-slang/slang#10641).

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
