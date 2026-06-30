// Verifies that explicit coverage binding applies when function or
// branch coverage is enabled without line coverage.

//TEST:SIMPLE(filecheck=FUNC):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-function-coverage -trace-coverage-binding 7 3
//TEST:SIMPLE(filecheck=BRANCH):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-branch-coverage -trace-coverage-binding 7 3

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        int x = 1 + 2;
    }
}

//FUNC-DAG: OpDecorate %__slang_coverage Binding 7
//FUNC-DAG: OpDecorate %__slang_coverage DescriptorSet 3

//BRANCH-DAG: OpDecorate %__slang_coverage Binding 7
//BRANCH-DAG: OpDecorate %__slang_coverage DescriptorSet 3
