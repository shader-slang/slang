//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -stage vertex -entry main

// Regression test for SPIR-V dominance error with conditionally assigned variables.
// The load-store redundancy-removal pass saw that `result` is assigned from an immutable
// source inside the conditional, and tried to eliminate `result` by loading directly from
// the source. But the source pointer is only computed inside the conditional, so using it
// outside creates invalid SPIR-V.

struct Constants
{
    float3 value;
};

ConstantBuffer<Constants> g_constants;

// The source pointer and load stay on the defining branch. The merged value
// uses a native phi, so the source pointer is not needed outside that branch.
// CHECK: OpBranchConditional {{%[A-Za-z0-9_]+}} [[DEFINED:%[A-Za-z0-9_]+]] {{%[A-Za-z0-9_]+}}
// CHECK: [[DEFINED]] = OpLabel
// CHECK: [[SOURCE:%[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Uniform_v3float
// CHECK: [[VALUE:%[A-Za-z0-9_]+]] = OpLoad %v3float [[SOURCE]]
// CHECK: OpBranch [[MERGE:%[A-Za-z0-9_]+]]
// CHECK: [[MERGE]] = OpLabel
// CHECK: [[RESULT:%[A-Za-z0-9_]+]] = OpPhi %v3float {{.*}}[[VALUE]] [[DEFINED]]
// CHECK: OpCompositeConstruct %v4float [[RESULT]] %float_1

float4 main(uint condition : A) : SV_Position
{
    float3 result;
    
    if (condition != 0)
    {
        result = g_constants.value;
    }
    
    return float4(result, 1.0);
}
