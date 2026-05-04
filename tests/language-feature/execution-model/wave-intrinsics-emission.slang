// Test emission of core wave intrinsics across HLSL and GLSL targets.
// From the docs: "Threads in a wave can synchronize and share data efficiently
// using wave-tangled functions such as ballots, reductions, shuffling..."
//
// Verifies WaveIsFirstLane, WaveGetLaneIndex, WaveGetLaneCount,
// WaveActiveBallot, and WaveReadLaneFirst emit correct target-specific code.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly

RWStructuredBuffer<uint> output;

// CHECK_HLSL: [numthreads(32, 1, 1)]
// CHECK_GLSL: layout(local_size_x = 32
// CHECK_SPIRV: BuiltIn SubgroupLocalInvocationId
// CHECK_SPIRV: BuiltIn SubgroupSize

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // WaveIsFirstLane
    // CHECK_HLSL: WaveIsFirstLane
    // CHECK_GLSL: subgroupElect
    // CHECK_SPIRV: OpGroupNonUniformElect
    bool isFirst = WaveIsFirstLane();

    // WaveGetLaneIndex
    // CHECK_HLSL: WaveGetLaneIndex
    // CHECK_GLSL: gl_SubgroupInvocationID
    // CHECK_SPIRV: OpLoad {{.*}} %SubgroupLocalInvocationId
    uint laneIdx = WaveGetLaneIndex();

    // WaveGetLaneCount
    // CHECK_HLSL: WaveGetLaneCount
    // CHECK_GLSL: gl_SubgroupSize
    // CHECK_SPIRV: OpLoad {{.*}} %SubgroupSize
    uint laneCount = WaveGetLaneCount();

    // WaveActiveBallot
    // CHECK_HLSL: WaveActiveBallot
    // CHECK_GLSL: subgroupBallot
    // CHECK_SPIRV: OpGroupNonUniformBallot
    uint4 ballot = WaveActiveBallot(tid.x < 16);

    // WaveReadLaneFirst
    // CHECK_HLSL: WaveReadLaneFirst
    // CHECK_GLSL: subgroupBroadcastFirst
    // CHECK_SPIRV: OpGroupNonUniformBroadcastFirst
    uint firstValue = WaveReadLaneFirst(tid.x);

    output[tid.x] = uint(isFirst) + laneIdx + laneCount + ballot.x + firstValue;
}
