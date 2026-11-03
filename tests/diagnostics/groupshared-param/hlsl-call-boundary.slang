// HLSL does not allow an `export` function to have a `groupshared` parameter (HLSL proposal 0043,
// `groupshared` Arguments). An `export` function keeps its definition, so its parameter is still
// there when the HLSL IR check runs, which reports it (shader-slang/slang#10641). `[noinline]`
// alone does not keep a definition: the callee is inlined, the write lands directly on the
// `groupshared` global, and there is nothing to report. The check runs at the end of IR
// optimization at every optimization level, so the second run repeats the first with
// `-minimum-slang-optimization`.

//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL):-target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL):-target hlsl -entry computeMain -stage compute -minimum-slang-optimization

RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];

export void exportedWrite(uint tid, groupshared uint scratch[N])
//HLSL:                                              ^^^^^^^ error E30708
//HLSL:                                              ^^^^^^^ this function is 'export', so its definition is kept, and HLSL cannot pass thread-group-shared memory across a call; remove 'export', or have the function access the 'groupshared' variable directly instead of taking it as a parameter
{
    scratch[tid] = tid + 1;
}

[noinline]
void noInlineWrite(uint tid, groupshared uint scratch[N])
{
    scratch[tid] = tid + 2;
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    exportedWrite(tid.x, groupSharedData);
    noInlineWrite(tid.x, groupSharedData);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = groupSharedData[(tid.x + 1) % N];
}
