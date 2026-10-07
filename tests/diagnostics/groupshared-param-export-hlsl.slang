// HLSL cannot pass thread-group-shared memory across a function boundary (DXC error 0043), so a
// `groupshared` parameter is rejected when the boundary survives: `export` keeps the function
// reachable as its own symbol, so the parameter is still there at HLSL emit, where it is diagnosed
// (shader-slang/slang#10641).
//
// The second run pins the same rejection under `-minimum-slang-optimization`: the diagnostic fires
// from the HLSL emitter, which always runs, so it is unaffected by the optimization level.

//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL):-target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL):-target hlsl -entry computeMain -stage compute -minimum-slang-optimization

static const uint N = 8;
groupshared uint groupSharedData[N];

export void writeSlot(uint tid, groupshared uint scratch[N])
//HLSL:                                          ^^^^^^^ error E30708
//HLSL:                                          ^^^^^^^ this function keeps a call boundary that could not be inlined away
{
    scratch[tid] = tid + 1;
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    writeSlot(tid.x, groupSharedData);
}
