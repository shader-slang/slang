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

// Verify the result variable and load-store pair are not optimized away.
// CHECK: %result = OpVariable %_ptr_Function_v3float Function
// CHECK: OpStore %result
// CHECK: OpLoad %v3float %result

float4 main(uint condition : A) : SV_Position
{
    float3 result;
    
    if (condition != 0)
    {
        result = g_constants.value;
    }
    
    return float4(result, 1.0);
}
