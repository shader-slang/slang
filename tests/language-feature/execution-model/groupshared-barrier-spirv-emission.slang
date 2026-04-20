//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain

groupshared float sharedArray[64];
RWStructuredBuffer<float> output;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint idx : SV_GroupIndex)
{
    sharedArray[idx] = float(idx);
    // CHECK: Workgroup
    // CHECK: OpControlBarrier
    GroupMemoryBarrierWithGroupSync();
    uint neighbor = (idx + 1u) % 64u;
    float val = sharedArray[neighbor];
    // CHECK: OpMemoryBarrier
    GroupMemoryBarrier();
    // Access shared memory after barrier to prevent dead-code elimination
    output[idx] = val + sharedArray[idx];
}
