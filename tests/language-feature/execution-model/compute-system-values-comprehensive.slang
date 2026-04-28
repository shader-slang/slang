// Test compute shader system values within a single dispatch group.
// Uses [numthreads(2,2,2)] — 8 threads spanning full 3D groupThreadId coordinates.
// Dispatch is single-group, so SV_GroupID is always (0,0,0) and
// SV_DispatchThreadID == SV_GroupThreadID. SV_GroupIndex follows the
// linearization x + y*Nx + z*Nx*Ny = x + y*2 + z*4.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 2, 2)]
void computeMain(
    uint3 dispatchId : SV_DispatchThreadID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    uint groupIndex : SV_GroupIndex)
{
    int base = int(groupIndex) * 4;

    // Pack 3D coordinates into a single scalar as x*100 + y*10 + z.
    outputBuffer[base + 0] = int(dispatchId.x * 100 + dispatchId.y * 10 + dispatchId.z);
    outputBuffer[base + 1] = int(groupThreadId.x * 100 + groupThreadId.y * 10 + groupThreadId.z);
    outputBuffer[base + 2] = int(groupId.x + groupId.y + groupId.z);
    outputBuffer[base + 3] = int(groupIndex);
}

// Thread (0,0,0): groupIndex=0
// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// Thread (1,0,0): groupIndex=1
// CHECK-NEXT: 100
// CHECK-NEXT: 100
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// Thread (0,1,0): groupIndex=2
// CHECK-NEXT: 10
// CHECK-NEXT: 10
// CHECK-NEXT: 0
// CHECK-NEXT: 2
// Thread (1,1,0): groupIndex=3
// CHECK-NEXT: 110
// CHECK-NEXT: 110
// CHECK-NEXT: 0
// CHECK-NEXT: 3
// Thread (0,0,1): groupIndex=4
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 4
// Thread (1,0,1): groupIndex=5
// CHECK-NEXT: 101
// CHECK-NEXT: 101
// CHECK-NEXT: 0
// CHECK-NEXT: 5
// Thread (0,1,1): groupIndex=6
// CHECK-NEXT: 11
// CHECK-NEXT: 11
// CHECK-NEXT: 0
// CHECK-NEXT: 6
// Thread (1,1,1): groupIndex=7
// CHECK-NEXT: 111
// CHECK-NEXT: 111
// CHECK-NEXT: 0
// CHECK-NEXT: 7
