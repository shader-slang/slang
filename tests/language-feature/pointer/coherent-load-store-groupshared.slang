//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -capability vk_mem_model

// Tests if we pass-through and handle groupshared address space pointers correctly.
// Ensure SPIRV emits coherent operations here
// SPIRV: MakePointerAvailable
// SPIRV: MakePointerVisible

// CHECK: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

groupshared int[32] shared;

#define THREAD_GROUP_SIZE 3
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    Ptr<int, Access::ReadWrite, AddressSpace::GroupShared> ptr = __getAddress(shared[0]);
    Ptr<int, Access::ReadWrite, AddressSpace::GroupShared> secondPtr = ptr + group_thread_id.x;
    coherentStore(secondPtr, (int)group_thread_id.x, 4, MemoryScope::Workgroup);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[group_thread_id.x] = coherentLoad(secondPtr, 4, MemoryScope::Workgroup);
}