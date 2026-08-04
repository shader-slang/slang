// HLSL cannot pass thread-group-shared memory across a function boundary (DXC error 0043), so a
// function that declares a bare `groupshared` parameter and forbids inlining with `[noinline]` is
// rejected when targeting HLSL (shader-slang/slang#10641).

//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL,non-exhaustive):-target hlsl -entry computeMain -stage compute

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

// HLSL: 30708
