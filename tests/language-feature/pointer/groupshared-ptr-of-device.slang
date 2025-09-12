//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model+sm_6_0+spvGroupNonUniformBallot
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -capability vk_mem_model+sm_6_0+spvGroupNonUniformBallot

// Tests if we pass-through and handle pointers via groupshared-memory correctly.
// Ensure SPIRV emits coherent operations here
// SPIRV: OpEntryPoint
// SPIRV-NOT: error

// CHECK: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 2
// CHECK-NEXT: 0
// CHECK-NEXT: 3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
uniform int* outputBuffer;

groupshared int* sharedPtr[3];

[numthreads(3, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    sharedPtr[group_thread_id.x] = outputBuffer + group_thread_id.x;
    sharedPtr[group_thread_id.x] = sharedPtr[group_thread_id.x]+group_thread_id.x;
    GroupMemoryBarrierWithGroupSync();

    *sharedPtr[group_thread_id.x] = group_thread_id.x+1;
}