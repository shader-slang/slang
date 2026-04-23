// Test that vector<bool> is rejected as a varying parameter when targeting SPIR-V
// on interface-block stages (issue #9452).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage vertex

[shader("vertex")]
float4 main(vector<bool, 4> b : A) : SV_Position
//CHECK: vector<bool> is not a valid SPIR-V varying type for this stage
{
    return float4(0);
}
