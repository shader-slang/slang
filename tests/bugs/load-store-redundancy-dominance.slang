//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -stage vertex -entry main

// Regression test for SPIR-V dominance error with uninitialized variables in loop with break.
// The load-store redundancy-removal pass replaced uses of a temporary variable with a pointer
// computed inside a conditional branch. When the temporary was used after the loop exit, the
// replacement pointer's definition did not dominate that use site, resulting in invalid SPIR-V.

struct Constants
{
    uint count;
    uint targetIndex;
    float3 values[16];
};

ConstantBuffer<Constants> g_constants;

// CHECK-NOT: error
// CHECK: result code = 0

float4 main() : SV_Position
{
    // Uninitialized variable - triggers the bug.
    float3 result;

    // Loop with conditional assignment and break
    for (uint i = 0; i < g_constants.count; i++)
    {
        if (i == g_constants.targetIndex)
        {
            result = g_constants.values[i];
            break;
        }
    }

    // Use after loop - this is where the dominance violation occurred.
    return float4(result, 1.0);
}
