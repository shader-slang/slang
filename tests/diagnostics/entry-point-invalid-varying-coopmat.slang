// Test that CoopMat is rejected as an entry-point varying parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage vertex

import linalg;

[shader("vertex")]
float4 main(
    linalg::CoopMat<float, MemoryScope::CrossDevice, 1, 1, linalg::CoopMatMatrixUse::MatrixA> m : A)
    : SV_Position
//CHECK: error 38050
{
    return float4(0);
}
