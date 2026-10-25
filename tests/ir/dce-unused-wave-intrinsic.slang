// A wave intrinsic call whose result is unused is removed as dead code (#13910), but wave
// intrinsic calls are still never moved or merged, since they are convergent.

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -profile cs_6_0 -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute

//TEST:SIMPLE(filecheck=QUAD_WGSL): -target wgsl -entry quadMain -stage compute
//TEST:SIMPLE(filecheck=QUAD_HLSL): -target hlsl -entry quadMain -stage compute -profile cs_6_0 -line-directive-mode none

RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float x = outputBuffer[tid.x];

    // Unused: removed.
    float unusedSum = WaveActiveSum(x);
    float unusedFirst = WaveReadLaneFirst(x);
    uint4 unusedBallot = WaveActiveBallot(x > 0.0);

    // Used outside and inside a branch. The two identical calls run with different sets of
    // invocations, so both are kept, and the second stays inside the branch.
    float max0 = WaveActiveMax(x);
    outputBuffer[tid.x] = max0;
    if (x > 1.0)
    {
        outputBuffer[tid.x + 4] = WaveActiveMax(x);
    }
}

// WGSL-LABEL: fn computeMain
// WGSL-NOT: subgroupAdd
// WGSL-NOT: subgroupBroadcastFirst
// WGSL-NOT: subgroupBallot
// WGSL: subgroupMax(
// WGSL-NOT: subgroupMax(
// WGSL: if
// WGSL: subgroupMax(
// WGSL-NOT: subgroupMax(
// WGSL: }

// HLSL-LABEL: void computeMain
// HLSL-NOT: WaveActiveSum
// HLSL-NOT: WaveReadLaneFirst
// HLSL-NOT: WaveActiveBallot
// HLSL: WaveActiveMax(
// HLSL-NOT: WaveActiveMax(
// HLSL: if
// HLSL: WaveActiveMax(
// HLSL-NOT: WaveActiveMax(
// HLSL: }

// GLSL-LABEL: void main
// GLSL-NOT: subgroupAdd
// GLSL-NOT: subgroupBroadcastFirst
// GLSL-NOT: subgroupBallot
// GLSL: subgroupMax(
// GLSL-NOT: subgroupMax(
// GLSL: if
// GLSL: subgroupMax(
// GLSL-NOT: subgroupMax(
// GLSL: }

// SPIRV-NOT: OpGroupNonUniformFAdd
// SPIRV-NOT: OpGroupNonUniformBroadcastFirst
// SPIRV-NOT: OpGroupNonUniformBallot
// SPIRV: OpGroupNonUniformFMax
// SPIRV-NOT: OpGroupNonUniformFMax
// SPIRV: OpBranchConditional
// SPIRV: OpGroupNonUniformFMax
// SPIRV-NOT: OpGroupNonUniformFMax

// The shape of WGSL's `QuadReadLaneAt` on #13649: every quad read runs unconditionally, and the
// lane selects one. With a constant lane, only the selected read is used, so it is the only one
// left.
[ForceInline]
float readQuadLane(float x, uint lane)
{
    float q0 = x;
    float q1 = QuadReadAcrossX(x);
    float q2 = QuadReadAcrossY(x);
    float q3 = QuadReadAcrossDiagonal(x);
    return lane == 0 ? q0 : (lane == 1 ? q1 : (lane == 2 ? q2 : q3));
}

[numthreads(4, 1, 1)]
void quadMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = readQuadLane(outputBuffer[tid.x], 2);
}

// QUAD_WGSL-LABEL: fn quadMain
// QUAD_WGSL-NOT: quadSwapX
// QUAD_WGSL-NOT: quadSwapDiagonal
// QUAD_WGSL: quadSwapY
// QUAD_WGSL-NOT: quadSwapX
// QUAD_WGSL-NOT: quadSwapDiagonal

// QUAD_HLSL-LABEL: void quadMain
// QUAD_HLSL-NOT: QuadReadAcrossX
// QUAD_HLSL-NOT: QuadReadAcrossDiagonal
// QUAD_HLSL: QuadReadAcrossY
// QUAD_HLSL-NOT: QuadReadAcrossX
// QUAD_HLSL-NOT: QuadReadAcrossDiagonal
