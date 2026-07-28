//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry test -stage compute

// A generic `min`/`max` on a matrix must fail as a clean language error, not a compiler abort.
// Regression test for shader-slang/slang#11075.

T genMax<T : IFloat>(T a, T b)
{
    return max(a, b);
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void test()
{
    float3x3 a = float3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    float3x3 b = float3x3(9, 8, 7, 6, 5, 4, 3, 2, 1);
    float3x3 r = genMax(a, b);
    outputBuffer[0] = r[0][0];
    //CHECK: E55215
    //CHECK: does not support operand type
}
