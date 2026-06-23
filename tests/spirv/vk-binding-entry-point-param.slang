//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> globalSink;

[numthreads(32, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(/* binding */ 0, /* set */ 1)]]
    StructuredBuffer<float> a,
    [[vk::binding(/* binding */ 3, /* set */ 2)]]
    RWStructuredBuffer<float> b,
)
{
    globalSink[0] = a[0];
    b[0] = a[0];
}

// CHECK-DAG: OpDecorate %globalSink Binding 0
// CHECK-DAG: OpDecorate %globalSink DescriptorSet 0
// CHECK-DAG: OpDecorate %entryPointParams_a Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_a DescriptorSet 1
// CHECK-DAG: OpDecorate %entryPointParams_b Binding 3
// CHECK-DAG: OpDecorate %entryPointParams_b DescriptorSet 2
