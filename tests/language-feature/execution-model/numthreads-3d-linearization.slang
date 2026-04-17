// Corner case: 3D thread group with [numthreads(2,2,2)] — verify
// SV_GroupIndex linearization is correct for all 8 threads.
// SV_GroupIndex = x + y * dimX + z * dimX * dimY
//              = x + y * 2 + z * 4
//
// Also verify SV_GroupThreadID matches the expected 3D coordinates.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 2, 2)]
void computeMain(uint3 groupThreadId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex)
{
    // Each thread writes 3 values: its 3D coordinates packed with groupIndex
    int base = int(groupIndex) * 3;

    // Expected linearization: x + y*2 + z*4
    int expectedIndex = int(groupThreadId.x) + int(groupThreadId.y) * 2 + int(groupThreadId.z) * 4;

    outputBuffer[base + 0] = int(groupThreadId.x * 100 + groupThreadId.y * 10 + groupThreadId.z);
    outputBuffer[base + 1] = int(groupIndex);
    outputBuffer[base + 2] = (int(groupIndex) == expectedIndex) ? 1 : 0;
}

// Thread (0,0,0): groupIndex=0, xyz=000, match=1
// CHECK: 0
// CHECK: 0
// CHECK: 1
// Thread (1,0,0): groupIndex=1, xyz=100, match=1
// CHECK: 100
// CHECK: 1
// CHECK: 1
// Thread (0,1,0): groupIndex=2, xyz=010, match=1
// CHECK: 10
// CHECK: 2
// CHECK: 1
// Thread (1,1,0): groupIndex=3, xyz=110, match=1
// CHECK: 110
// CHECK: 3
// CHECK: 1
// Thread (0,0,1): groupIndex=4, xyz=001, match=1
// CHECK: 1
// CHECK: 4
// CHECK: 1
// Thread (1,0,1): groupIndex=5, xyz=101, match=1
// CHECK: 101
// CHECK: 5
// CHECK: 1
// Thread (0,1,1): groupIndex=6, xyz=011, match=1
// CHECK: 11
// CHECK: 6
// CHECK: 1
// Thread (1,1,1): groupIndex=7, xyz=111, match=1
// CHECK: 111
// CHECK: 7
// CHECK: 1
