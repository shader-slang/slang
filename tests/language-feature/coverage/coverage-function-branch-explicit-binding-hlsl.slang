// Verifies that explicit coverage binding applies to function and
// branch coverage on HLSL targets, not just SPIR-V descriptor targets.

//TEST:SIMPLE(filecheck=FUNC):-target hlsl -stage compute -entry computeMain -profile cs_5_0 -trace-function-coverage -trace-coverage-binding 7 3
//TEST:SIMPLE(filecheck=BRANCH):-target hlsl -stage compute -entry computeMain -profile cs_5_0 -trace-branch-coverage -trace-coverage-binding 7 3

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = tid.x;
    if (tid.x == 0)
    {
        value += 1;
    }
}

//FUNC: RWStructuredBuffer{{.*}}_slang_coverage{{.*}}register(u7, space3)
//FUNC: InterlockedAdd

//BRANCH: RWStructuredBuffer{{.*}}_slang_coverage{{.*}}register(u7, space3)
//BRANCH: InterlockedAdd
