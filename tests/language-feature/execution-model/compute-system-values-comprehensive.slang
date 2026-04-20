// Comprehensive test for all compute shader system values.
// Verifies SV_DispatchThreadID, SV_GroupThreadID, SV_GroupID, and SV_GroupIndex
// all produce correct values for a 3D thread group configuration.
//
// From the docs: "An individual invocation is executed by an individual thread for
// every grid and thread group point combination."
//
// Thread group is (2,2,1). We dispatch one group, so:
//   SV_GroupID = (0,0,0) for all threads
//   SV_GroupThreadID = SV_DispatchThreadID (since single group)
//   SV_GroupIndex = threadId.x + threadId.y * groupDim.x

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 2, 1)]
void computeMain(
    uint3 dispatchId : SV_DispatchThreadID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    uint groupIndex : SV_GroupIndex)
{
    // Pack all system values into output for verification
    // Layout: 4 ints per thread, threads ordered by SV_GroupIndex
    int base = int(groupIndex) * 4;

    // SV_DispatchThreadID should equal SV_GroupThreadID for single-group dispatch
    outputBuffer[base + 0] = int(dispatchId.x * 10 + dispatchId.y);

    // SV_GroupThreadID
    outputBuffer[base + 1] = int(groupThreadId.x * 10 + groupThreadId.y);

    // SV_GroupID should be (0,0,0) for single-group dispatch
    outputBuffer[base + 2] = int(groupId.x + groupId.y + groupId.z);

    // SV_GroupIndex = flatten(groupThreadId, groupDim)
    outputBuffer[base + 3] = int(groupIndex);
}

// Thread (0,0): groupIndex=0, dispatchId=(0,0), groupThreadId=(0,0), groupId=(0,0,0)
// CHECK: 0
// CHECK: 0
// CHECK: 0
// CHECK: 0
// Thread (1,0): groupIndex=1, dispatchId=(1,0), groupThreadId=(1,0), groupId=(0,0,0)
// CHECK: 10
// CHECK: 10
// CHECK: 0
// CHECK: 1
// Thread (0,1): groupIndex=2, dispatchId=(0,1), groupThreadId=(0,1), groupId=(0,0,0)
// CHECK: 1
// CHECK: 1
// CHECK: 0
// CHECK: 2
// Thread (1,1): groupIndex=3, dispatchId=(1,1), groupThreadId=(1,1), groupId=(0,0,0)
// CHECK: 11
// CHECK: 11
// CHECK: 0
// CHECK: 3
