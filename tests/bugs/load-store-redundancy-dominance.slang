//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -stage vertex -entry main

// Regression test for SPIR-V dominance error with conditionally assigned variables.
// The load-store redundancy-removal pass replaced uses of a temporary variable with a pointer
// computed inside a conditional branch. When the temporary was used after the branch, the
// replacement pointer's definition did not dominate that use site, resulting in invalid SPIR-V.

struct Constants
{
    float3 value;
};

ConstantBuffer<Constants> g_constants;

// Verify the temporary variable and load-store pair are not optimized away.
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
