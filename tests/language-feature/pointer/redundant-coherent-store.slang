//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -capability vk_mem_model

// Tests if we optimize redundant store's correctly

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;
//TEST_INPUT:ubuffer(data=[0 0], stride=4),name=buffer
uniform int* buffer;

[numthreads(128, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    Ptr<int, Access::ReadWrite, AddressSpace::Device> ptr = __getAddress(buffer[0]);
    if (group_thread_id.x == 0)
    {
        // This store will not optimize out, Device > Invocation.
        // SPIRV: OpStore %ptr %int_1
        storeCoherent<4, MemoryScope::Device>(ptr, 1);
        // SPIRV-NEXT: OpStore %ptr %int_2
        storeCoherent<4, MemoryScope::Invocation>(ptr, 2);

        // Both of these stores will optimize out, Subgroup > Invocation.
        // SPIRV-NOT: OpStore {{.*}} %int_3
        *(ptr + 1) = 3;
        // SPIRV-NOT: OpStore {{.*}} %int_4
        storeCoherent<4, MemoryScope::Invocation>(ptr + 1, 4);
        // SPIRV: OpStore {{.*}} %int_5
        storeCoherent<4, MemoryScope::Workgroup>(ptr + 1, 5);
    }
    AllMemoryBarrierWithGroupSync();
    if (group_thread_id.x == 127)
    {
        // CHECK: 1
        outputBuffer[0] = (*ptr == 1 || *ptr == 2) ? 1 : 0;

        // CHECK-NEXT: 5
        outputBuffer[1] = loadCoherent<4, MemoryScope::Workgroup>(ptr+1);
    }
}