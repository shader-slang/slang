//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -capability vk_mem_model -output-using-type

// Tests if we optimize redundant load's correctly

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 11 10], stride=4),name=buffer
uniform int* buffer;

[numthreads(2, 1, 1)]
void computeMain(uint3 group_thread_id: SV_GroupThreadID)
{
    Ptr<int, Access::ReadWrite, AddressSpace::Device> ptr = __getAddress(buffer[0]);
    
    // Consider the load from this store-load pattern as redundant since 
    // the load is a sub-set memory-scope of the memory-scope of the store.
    // Invocation == Invocation.
    *ptr = 8;
    outputBuffer[0] = loadCoherent<4, MemoryScope::Invocation>(ptr);
    // CHECK: 8
    // SPIRV: OpStore %ptr %int_8
    // SPIRV-NOT: OpLoad
    // SPIRV: %[[#OUTPUT_BUFFER1:]] = OpAccessChain {{.*}} %outputBuffer %{{.*}} %int_0
    // SPIRV: OpStore %[[#OUTPUT_BUFFER1]] %int_8

    // Consider the load from this store-load pattern as redundant since 
    // the load is a sub-set memory-scope of the memory-scope of the store.
    // Device > Workgroup.
    let offset1 = ptr + 1;
    storeCoherent<4, MemoryScope::Device>(offset1, 9);
    outputBuffer[1] = loadCoherent<4, MemoryScope::Workgroup>(offset1);
    // CHECK-NEXT: 9
    // SPIRV: %[[#PTR_OFFSET:]] = OpPtrAccessChain {{.*}} %ptr %int_1
    // SPIRV: OpStore %[[#PTR_OFFSET]] %int_9
    // SPIRV-NOT: OpLoad
    // SPIRV: %[[#OUTPUT_BUFFER2:]] = OpAccessChain {{.*}} %outputBuffer %{{.*}} %int_1
    // SPIRV: OpStore %[[#OUTPUT_BUFFER2]] %int_9

    // Consider the following store-load pattern as not redundant since the data stored
    // may not be the same data that will be loaded if Workgroup-scope contains
    // different data than the Subgroup-scope.
    // Subgroup < Workgroup.
    let offset2 = ptr + 2;
    storeCoherent<4, MemoryScope::Subgroup>(offset2, buffer[3]);
    if(group_thread_id.x == 1)
    {
        storeCoherent<4, MemoryScope::Invocation>(offset2, buffer[4]);
        let result = loadCoherent<4, MemoryScope::Workgroup>(offset2);
        outputBuffer[2] = (result == 11 || result == 10) ? 12 : 0;
    }
    // CHECK-NEXT: 12
    // SPIRV: OpStore {{.*}}MakePointerAvailable{{.*}} 4 %int_3
    // SPIRV: OpStore {{.*}}MakePointerAvailable{{.*}} 4 %int_4
    // SPIRV: OpLoad {{.*}}MakePointerVisible{{.*}} 4 %int_2
}