// Test groupshared memory with GroupMemoryBarrierWithGroupSync.
// Threads write to groupshared array, synchronize, then read neighbor's value.
// This verifies the thread group execution model from the docs:
// "All threads within a thread group execute on the same set of execution resources.
//  This allows a thread group to share local memory allocated with the groupshared attribute."

// Functional test requires GPU (barriers not available on CPU target)
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
// Emission test verifiable without GPU
//TEST:SIMPLE(filecheck=EMIT): -target hlsl -stage compute -entry computeMain

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

groupshared int sharedData[4];

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint idx : SV_GroupIndex)
{
    // Each thread writes its own index
    sharedData[idx] = int(idx) * 10;

    // Barrier ensures all writes are visible
    GroupMemoryBarrierWithGroupSync();

    // Each thread reads the *next* thread's value (wrapping)
    int neighborIdx = (int(idx) + 1) % 4;
    outputBuffer[idx] = sharedData[neighborIdx];
}

// EMIT: groupshared
// EMIT: GroupMemoryBarrierWithGroupSync

// Thread 0 reads sharedData[1] = 10
// Thread 1 reads sharedData[2] = 20
// Thread 2 reads sharedData[3] = 30
// Thread 3 reads sharedData[0] = 0
// CHECK: 10
// CHECK: 20
// CHECK: 30
// CHECK: 0
