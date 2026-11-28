//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -entry main -stage compute
// CHECK: OpEntryPoint GLCompute

// Regression test for https://github.com/shader-slang/slang/issues/10309
// A global variable whose type is a type_param causes ICE 99999 on SPIR-V:
// "Unexpected context type for parameter info retrieval"
// Using type_param only in function signatures works correctly.

interface IMaterial
{
    float3 shade(float3 normal);
}

type_param T : IMaterial;

T material;

RWStructuredBuffer<float3> output;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = material.shade(float3(0, 1, 0));
}
