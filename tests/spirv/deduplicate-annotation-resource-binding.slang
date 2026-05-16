//TEST:SIMPLE(filecheck=CHECK):-emit-spirv-directly -target spirv-asm -entry computeMain -stage compute

// Check that the Binding and DescriptorSet decorations emitted from
// [[vk::binding]] and repeated by spirv_asm appear only once for outputBuffer.

// CHECK: OpName %[[BUFFER:[A-Za-z0-9_]+]] "outputBuffer"
// CHECK: OpDecorate %[[BUFFER]] Binding 1
// CHECK-NOT: OpDecorate %[[BUFFER]] Binding 1
// CHECK: OpDecorate %[[BUFFER]] DescriptorSet 0
// CHECK-NOT: OpDecorate %[[BUFFER]] Binding 1
// CHECK-NOT: OpDecorate %[[BUFFER]] DescriptorSet 0
// CHECK: ; Types, variables and constants

[[vk::binding(1, 0)]]
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int value = spirv_asm
    {
        OpDecorate $outputBuffer Binding 1;
        OpDecorate $outputBuffer Binding 1;
        OpDecorate $outputBuffer DescriptorSet 0;
        OpDecorate $outputBuffer DescriptorSet 0;
        result : $$int = OpConstant 0;
    };

    outputBuffer[0] = value;
}
