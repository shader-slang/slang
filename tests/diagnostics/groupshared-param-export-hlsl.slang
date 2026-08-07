// HLSL cannot pass thread-group-shared memory across a function boundary (DXC error 0043), so a
// `groupshared` parameter is rejected on a function whose boundary survives inlining. `export` makes
// one: the function stays reachable as its own symbol, so the parameter is still there when
// `checkUnsupportedInst` runs (shader-slang/slang#10641).
//
// The diagnostic comes from after inlining rather than from the declaration, because whether a
// boundary survives is not knowable in the front end. See the companion case in
// `groupshared-param-noinline-target.slang`, where `[noinline]` alone is inlined away and the
// program is accepted.

//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL):-target hlsl -entry computeMain -stage compute

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
