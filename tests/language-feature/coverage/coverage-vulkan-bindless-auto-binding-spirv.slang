// Verifies that Vulkan auto-allocation does not place the synthesized
// coverage buffer after an unsized descriptor array in the same
// descriptor set. Such arrays are commonly backed by variable
// descriptor count layouts and must remain the highest binding in
// their set.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage

[allow("overlapping-bindings")]
[[vk::binding(0, 0)]]
RWStructuredBuffer<uint> bindlessBuffers[];

[[vk::binding(0, 1)]]
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        bindlessBuffers[0][0] = 1;
        outputBuffer[0] = 2;
    }
}

//CHECK-DAG: OpDecorate %bindlessBuffers Binding 0
//CHECK-DAG: OpDecorate %bindlessBuffers DescriptorSet 0
//CHECK-DAG: OpDecorate %outputBuffer Binding 0
//CHECK-DAG: OpDecorate %outputBuffer DescriptorSet 1
//CHECK-DAG: OpDecorate %__slang_coverage Binding 0
//CHECK-DAG: OpDecorate %__slang_coverage DescriptorSet 2
