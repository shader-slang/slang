// Test SPIRV emission of groupshared memory and barrier operations.
// From the docs: "Related barriers include GroupMemoryBarrier()
// and GroupMemoryBarrierWithGroupSync()."
//
// Verifies that groupshared variables emit as Workgroup storage class,
// and barriers emit correct OpControlBarrier instructions.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain

groupshared float sharedArray[64];

RWStructuredBuffer<float> output;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint idx : SV_GroupIndex)
{
    // Write to groupshared
    sharedArray[idx] = float(idx);

    // Verify groupshared uses Workgroup storage class
    // CHECK: Workgroup

    // GroupMemoryBarrierWithGroupSync should emit OpControlBarrier
    // CHECK: OpControlBarrier
    GroupMemoryBarrierWithGroupSync();

    // Read neighbor's value
    uint neighbor = (idx + 1u) % 64u;
    float val = sharedArray[neighbor];

    // GroupMemoryBarrier (without sync) emits OpMemoryBarrier
    // CHECK: OpMemoryBarrier
    GroupMemoryBarrier();

    output[idx] = val;
}
