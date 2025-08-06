//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model+sm_6_0+spvGroupNonUniformBallot
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -capability vk_mem_model+sm_6_0+spvGroupNonUniformBallot

// Tests if we pass-through and handle groupshared address space pointers correctly.
// Ensure SPIRV emits coherent operations here
// SPIRV: OpEntryPoint
// SPIRV-NOT: error

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 0

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

groupshared int[32] shared;

void funcCall(uint3 group_thread_id: SV_GroupThreadID, Ptr<int> sharedPtr)
{
    outputBuffer[group_thread_id.x] = sharedPtr[group_thread_id.x];
}

[numthreads(3, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    if (WaveIsFirstLane())
    {
        shared[4] = 0;
    }
    // we take a pointer to a groupshared array, offset of 1 element.
    // This is to ensure that the pointer is working correctly.
    Ptr<int> sharedPtr = &shared[1];

    // shared = {0, 1, 2, 0, 0, ...}
    // (*sharedPtr) = {1, 2, 0, 0, ...}
    shared[group_thread_id.x] = group_thread_id.x;
    GroupMemoryBarrierWithGroupSync();
    funcCall(group_thread_id, sharedPtr);
}