// Verifies that Vulkan/SPIR-V coverage auto-allocation does not
// append to an existing descriptor set or fill descriptor-set holes,
// even if set 0 has an apparently free high binding. Hosts commonly
// build descriptor set layouts from their own reflection data, so
// coverage should use a fresh synthetic-resource set after all shader-
// visible sets instead.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage

[[vk::binding(50, 0)]]
RWStructuredBuffer<uint> set0Data;

[[vk::binding(0, 2)]]
RWStructuredBuffer<uint> set2Data;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        set0Data[0] = 1;
        set2Data[0] = 2;
    }
}

//CHECK-DAG: OpDecorate %set0Data Binding 50
//CHECK-DAG: OpDecorate %set0Data DescriptorSet 0
//CHECK-DAG: OpDecorate %set2Data Binding 0
//CHECK-DAG: OpDecorate %set2Data DescriptorSet 2
//CHECK-DAG: OpDecorate %__slang_coverage Binding 0
//CHECK-DAG: OpDecorate %__slang_coverage DescriptorSet 3
