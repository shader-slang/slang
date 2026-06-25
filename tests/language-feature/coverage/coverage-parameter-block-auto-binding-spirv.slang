// Verifies that the coverage auto-allocator accounts for resources
// hidden behind `ParameterBlock<T>` layouts. The `params.data`
// resource flattens to `(set=0, binding=0)` on SPIR-V, so
// `__slang_coverage` must choose a coverage-owned descriptor set.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage

struct Params
{
    RWStructuredBuffer<uint> data;
};

[[vk::binding(0, 0)]]
ParameterBlock<Params> params;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        params.data[0] = 1;
    }
}

//CHECK-DAG: OpDecorate %params_data Binding 0
//CHECK-DAG: OpDecorate %params_data DescriptorSet 0
//CHECK-DAG: OpDecorate %__slang_coverage Binding 0
//CHECK-DAG: OpDecorate %__slang_coverage DescriptorSet 1
