// Verifies that hosts can reserve descriptor sets that are present in
// the runtime pipeline layout but not visible in the compiled shader
// IR. This covers engines that always reserve set 0 for a bindless
// table, while some entry points do not statically reference that
// table after preprocessing and optimization.

//TEST:SIMPLE(filecheck=CHECK_SET0):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-reserved-space 0
//TEST:SIMPLE(filecheck=CHECK_SET0_SET1):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-reserved-space 0 -trace-coverage-reserved-space 1
//TEST:SIMPLE(filecheck=CHECK_DUP):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-reserved-space 0 -trace-coverage-reserved-space 0

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        int x = 1 + 2;
    }
}

//CHECK_SET0-DAG: OpDecorate %__slang_coverage Binding 0
//CHECK_SET0-DAG: OpDecorate %__slang_coverage DescriptorSet 1

//CHECK_SET0_SET1-DAG: OpDecorate %__slang_coverage Binding 0
//CHECK_SET0_SET1-DAG: OpDecorate %__slang_coverage DescriptorSet 2

//CHECK_DUP-DAG: OpDecorate %__slang_coverage Binding 0
//CHECK_DUP-DAG: OpDecorate %__slang_coverage DescriptorSet 1
