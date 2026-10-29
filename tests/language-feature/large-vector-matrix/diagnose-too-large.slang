// Vectors and matrices are limited to 16 elements, rows and columns. Non-exhaustive: the
// oversized matrix type is also reported once without a source location.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

[noinline]
float last(vector<float, 17> v)
{
    return v[16];
}

[noinline]
float corner(matrix<float, 20, 3> m)
{
    return m[19][2];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[1] = last(vector<float, 17>(outputBuffer[0])) + corner(matrix<float, 20, 3>(outputBuffer[0]));
//CHECK:                  ^ invalid vector element count
//CHECK:                  ^ vector has invalid element count '17', valid values are between '1' and '16' inclusive
//CHECK:                                                               ^ invalid vector element count
//CHECK:                                                               ^ vector has invalid element count '20', valid values are between '1' and '16' inclusive
}
