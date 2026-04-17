// Test emission of core wave intrinsics across HLSL and GLSL targets.
// From the docs: "Threads in a wave can synchronize and share data efficiently
// using wave-tangled functions such as ballots, reductions, shuffling..."
//
// Verifies WaveIsFirstLane, WaveGetLaneIndex, WaveGetLaneCount,
// WaveActiveBallot, and WaveReadLaneFirst emit correct target-specific code.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain

RWStructuredBuffer<uint> output;

// CHECK_HLSL: [numthreads(32, 1, 1)]
// CHECK_GLSL: layout(local_size_x = 32

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // WaveIsFirstLane
    // CHECK_HLSL: WaveIsFirstLane
    // CHECK_GLSL: subgroupElect
    bool isFirst = WaveIsFirstLane();

    // WaveGetLaneIndex
    // CHECK_HLSL: WaveGetLaneIndex
    // CHECK_GLSL: gl_SubgroupInvocationID
    uint laneIdx = WaveGetLaneIndex();

    // WaveGetLaneCount
    // CHECK_HLSL: WaveGetLaneCount
    // CHECK_GLSL: gl_SubgroupSize
    uint laneCount = WaveGetLaneCount();

    // WaveActiveBallot
    // CHECK_HLSL: WaveActiveBallot
    // CHECK_GLSL: subgroupBallot
    uint4 ballot = WaveActiveBallot(tid.x < 16);

    // WaveReadLaneFirst
    // CHECK_HLSL: WaveReadLaneFirst
    // CHECK_GLSL: subgroupBroadcastFirst
    uint firstValue = WaveReadLaneFirst(tid.x);

    output[tid.x] = uint(isFirst) + laneIdx + laneCount + ballot.x + firstValue;
}
