// The per-row matrix-FRem legalization (legalizeMatrixFRemForGLSL) relies on
// the type checker rejecting matrix-vector mixes for component-wise
// arithmetic: its release assert that a matrix `FRem`'s operands are matrix
// or scalar can only hold while the front end rejects vector operands
// outright, with a diagnostic rather than a crash.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

RWStructuredBuffer<float> outputBuffer;

void f(float2x2 m, float2 v)
{
    //CHECK: no overload for '%' applicable to arguments of type (matrix
    float2x2 r = m % v;
    outputBuffer[0] = r[0][0];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    f(float2x2(1, 2, 3, 4), float2(1, 2));
}
