// Test that vector<bool> is rejected as a varying parameter when targeting SPIR-V.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage vertex

[shader("vertex")]
float4 main(vector<bool, 4> b : A) : SV_Position
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ type cannot be used as entry-point varying for this target
{
    return float4(0);
}
