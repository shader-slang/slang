// Verifies that the coverage auto-allocator accounts for uniform
// resources that enter through an entry-point parameter layout. These
// resources are flattened into globals after coverage instrumentation,
// so the allocator must scan `IREntryPointLayout` and not just global
// parameters, and place coverage in a coverage-owned descriptor set.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage

struct Params
{
    RWStructuredBuffer<uint> data;
};

[numthreads(1, 1, 1)]
void computeMain(ParameterBlock<Params> params, uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        params.data[0] = 1;
    }
}

//CHECK-DAG: OpDecorate %entryPointParams_params_data Binding 0
//CHECK-DAG: OpDecorate %entryPointParams_params_data DescriptorSet 0
//CHECK-DAG: OpDecorate %__slang_coverage Binding 0
//CHECK-DAG: OpDecorate %__slang_coverage DescriptorSet 1
