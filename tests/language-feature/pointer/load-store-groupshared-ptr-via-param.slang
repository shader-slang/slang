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

void funcCall(uint3 group_thread_id: SV_GroupThreadID, groupshared int[32] sharedVar)
{
    outputBuffer[group_thread_id.x] = sharedVar[group_thread_id.x+1];
}

[numthreads(3, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    if (WaveIsFirstLane())
    {
        shared[4] = 0;
    }

    // shared = {0, 1, 2, 0, 0, ...}
    // (*sharedPtr) = {1, 2, 0, 0, ...}
    shared[group_thread_id.x] = group_thread_id.x;
    GroupMemoryBarrierWithGroupSync();
    funcCall(group_thread_id, shared);
}