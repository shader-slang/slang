// Comprehensive test of compute shader system values.
// [numthreads(2,2,1)] with 4 threads. Each thread writes 4 values:
//   [groupIndex*4+0] = dispatchThreadId encoded as x*100+y*10+z
//   [groupIndex*4+1] = groupThreadId encoded as x*100+y*10+z
//   [groupIndex*4+2] = groupId.x + groupId.y + groupId.z
//   [groupIndex*4+3] = groupIndex

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 2, 1)]
void computeMain(
    uint3 dispatchId : SV_DispatchThreadID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    uint groupIndex : SV_GroupIndex)
{
    int base = int(groupIndex) * 4;
    outputBuffer[base + 0] = int(dispatchId.x) * 100 + int(dispatchId.y) * 10 + int(dispatchId.z);
    outputBuffer[base + 1] = int(groupThreadId.x) * 100 + int(groupThreadId.y) * 10 + int(groupThreadId.z);
    outputBuffer[base + 2] = int(groupId.x) + int(groupId.y) + int(groupId.z);
    outputBuffer[base + 3] = int(groupIndex);
}

// groupIndex=0: tid=(0,0,0), groupId=(0,0,0)
// CHECK: 0
// CHECK: 0
// CHECK: 0
// CHECK: 0
// groupIndex=1: tid=(1,0,0), groupId=(0,0,0)
// CHECK: 100
// CHECK: 100
// CHECK: 0
// CHECK: 1
// groupIndex=2: tid=(0,1,0), groupId=(0,0,0)
// CHECK: 10
// CHECK: 10
// CHECK: 0
// CHECK: 2
// groupIndex=3: tid=(1,1,0), groupId=(0,0,0)
// CHECK: 110
// CHECK: 110
// CHECK: 0
// CHECK: 3
