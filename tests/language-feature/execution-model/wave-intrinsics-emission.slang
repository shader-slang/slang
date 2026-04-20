// Test that wave intrinsics emit correctly to HLSL and GLSL targets.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain

RWStructuredBuffer<uint> outputBuffer;

// CHECK_HLSL: WaveIsFirstLane
// CHECK_HLSL: WaveGetLaneIndex
// CHECK_HLSL: WaveGetLaneCount
// CHECK_HLSL: WaveReadLaneFirst

// CHECK_GLSL: subgroupElect
// CHECK_GLSL: gl_SubgroupInvocationID
// CHECK_GLSL: gl_SubgroupSize
// CHECK_GLSL: subgroupBroadcastFirst

[shader("compute")]
[numthreads(64,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    bool isFirst = WaveIsFirstLane();
    uint laneIdx = WaveGetLaneIndex();
    uint laneCount = WaveGetLaneCount();
    float value = float(idx);
    float firstVal = WaveReadLaneFirst(value);
    outputBuffer[idx] = isFirst ? laneCount : laneIdx + uint(firstVal);
}
