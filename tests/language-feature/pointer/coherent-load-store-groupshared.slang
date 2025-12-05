//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -capability vk_mem_model

// Tests if we pass-through and handle groupshared address space pointers correctly.
// Ensure SPIRV emits coherent operations here
// SPIRV: MakePointerAvailable|NonPrivatePointer
// SPIRV: MakePointerVisible|NonPrivatePointer

// CHECK: 2
// CHECK-NEXT: 1
// CHECK-NEXT: 0

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

groupshared int[32] shared;

#define THREAD_GROUP_SIZE 3
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    Ptr<int, Access::ReadWrite, AddressSpace::GroupShared> ptr = __getAddress(shared[0]);
    storeCoherent<4, MemoryScope::Workgroup>(ptr + group_thread_id.x, (int)group_thread_id.x);
    AllMemoryBarrierWithGroupSync();
    outputBuffer[group_thread_id.x] = loadCoherent<4, MemoryScope::Workgroup>(ptr + THREAD_GROUP_SIZE - group_thread_id.x - 1);
}