// Test that CoopMat is rejected as an entry-point varying parameter on
// interface-block stages (issue #9449).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage vertex

import linalg;

[shader("vertex")]
float4 main(
    linalg::CoopMat<float, MemoryScope::CrossDevice, 1, 1, linalg::CoopMatMatrixUse::MatrixA> m : A)
    : SV_Position
//CHECK: CoopMat is not a valid varying type for this stage
{
    return float4(0);
}
