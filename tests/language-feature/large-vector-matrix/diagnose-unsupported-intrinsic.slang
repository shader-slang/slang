// Intrinsics that only have a native implementation report vectors and matrices larger than 4
// instead of emitting invalid code. `determinant` is limited to 4x4 matrices.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    vector<float, 8> v = vector<float, 8>(outputBuffer[0]);
    matrix<float, 6, 6> m = matrix<float, 6, 6>(outputBuffer[1]);
    outputBuffer[1] = WaveActiveSum(v)[0];
//CHECK:                           ^ intrinsic does not support vectors or matrices larger than 4
//CHECK:                           ^ 'WaveActiveSum' does not support vectors with more than 4 elements or matrices with more than 4 rows or columns on this target
    outputBuffer[2] = determinant(m);
//CHECK:                         ^ intrinsic does not support vectors or matrices larger than 4
//CHECK:                         ^ 'determinant' does not support vectors with more than 4 elements or matrices with more than 4 rows or columns on this target
}
