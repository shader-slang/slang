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
/*HLSL:
^^^^^^ 'exportedWrite' cannot have the 'groupshared' parameter 'scratch' when targeting HLSL, because calls to it cannot be inlined
^^^^^^ this prevents calls to the function from being inlined
                                                     ^^^^^^^ 'groupshared' parameter declared here
*/
{
    scratch[tid] = tid + 1;
}

[noinline]
void noInlineWrite(uint tid, groupshared uint scratch[N])
{
    scratch[tid] = tid + 2;
}

// `[noRefInline]` prevents calls to a function with reference parameters from being inlined, so
// the diagnostic points at the attribute.
//
[noRefInline]
/*HLSL:
 ^^^^^^^^^^^ error E30708
 ^^^^^^^^^^^ this prevents calls to the function from being inlined
*/
void noRefInlineWrite(uint tid, groupshared uint scratch[N])
//HLSL:                                          ^^^^^^^ 'groupshared' parameter declared here
{
    scratch[tid] = tid + 3;
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    exportedWrite(tid.x, groupSharedData);
    noInlineWrite(tid.x, groupSharedData);
    noRefInlineWrite(tid.x, groupSharedData);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = groupSharedData[(tid.x + 1) % N];
}
