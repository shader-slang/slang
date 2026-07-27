// A function that declares a bare `groupshared` parameter and forbids inlining is only legal on the
// targets that can keep the parameter across a call boundary (direct SPIR-V / Metal / CUDA). The
// targets below cannot, and must diagnose the conflict (shader-slang/slang#10641):
//   - HLSL: cannot pass thread-group-shared memory across a function boundary (`[noinline]` here).
//   - GLSL / SPIR-V-via-GLSL / WGSL: require the helper to be inlined away, which `[noinline]`
//     prevents. (SPIR-V-via-GLSL is distinguished from direct SPIR-V, which keeps the boundary.)

//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL,non-exhaustive):-target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GLSL,non-exhaustive):-target glsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL,non-exhaustive):-target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=VIAGLSL,non-exhaustive):-target spirv -emit-spirv-via-glsl -entry computeMain -stage compute

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
// GLSL: 30709
// WGSL: 30709
// VIAGLSL: 30709
