// Verifies that the Khronos auto-binding rule used for GLSL follows
// the same descriptor-set policy as SPIR-V: coverage is appended after
// the highest shader-visible set rather than filling descriptor-set
// holes or extending an existing set.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -stage compute -entry computeMain -profile glsl_450 -trace-coverage

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

//CHECK-DAG: layout(std430, binding = 50) buffer
//CHECK-DAG: layout(std430, binding = 0, set = 2) buffer
//CHECK-DAG: layout(std430, binding = 0, set = 3) buffer
//CHECK-DAG: atomicAdd
