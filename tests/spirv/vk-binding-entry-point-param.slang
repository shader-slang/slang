//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> globalSink;

struct Params
{
    RWStructuredBuffer<float> data;
};

[numthreads(32, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(/* binding */ 0, /* set */ 1)]]
    StructuredBuffer<float> a,
    [[vk::binding(/* binding */ 3, /* set */ 2)]]
    RWStructuredBuffer<float> b,
    [[vk::binding(/* binding */ 0, /* set */ 4)]]
    ParameterBlock<Params> params,
    RWStructuredBuffer<float> c,
    [[vk::binding(/* binding */ 2, /* set */ 0)]]
    RWStructuredBuffer<float> d,
)
{
    globalSink[0] = a[0];
    params.data[0] = a[0];
    b[0] = a[0];
    c[0] = a[0];
    d[0] = a[0];
}

// CHECK-DAG: OpDecorate %globalSink Binding 0
// CHECK-DAG: OpDecorate %globalSink DescriptorSet 0
// CHECK-DAG: OpDecorate %entryPointParams_a Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_a DescriptorSet 1
// CHECK-DAG: OpDecorate %entryPointParams_b Binding 3
// CHECK-DAG: OpDecorate %entryPointParams_b DescriptorSet 2
// CHECK-DAG: OpDecorate %entryPointParams_params_data Binding 0
// CHECK-DAG: OpDecorate %entryPointParams_params_data DescriptorSet 4
// CHECK-DAG: OpDecorate %entryPointParams_c Binding 1
// CHECK-DAG: OpDecorate %entryPointParams_c DescriptorSet 0
// CHECK-DAG: OpDecorate %entryPointParams_d Binding 2
// CHECK-DAG: OpDecorate %entryPointParams_d DescriptorSet 0
