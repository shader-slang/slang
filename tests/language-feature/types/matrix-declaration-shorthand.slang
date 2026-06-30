// matrix-declaration-shorthand.slang

// Test matrix declaration with generic (matrix<float,2,3>) and shorthand (float2x3) syntax.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Generic syntax: matrix<float, 2, 3> is a 2-row, 3-column matrix
    matrix<float, 2, 3> a = matrix<float, 2, 3>(
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0
    );

    // Shorthand syntax: float2x3 is also a 2-row, 3-column matrix
    float2x3 b = float2x3(
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0
    );

    // Both should be interchangeable — read same elements
    // CHECK: 1.0
    outputBuffer[0] = a[0][0];

    // CHECK: 3.0
    outputBuffer[1] = a[0][2];

    // CHECK: 6.0
    outputBuffer[2] = a[1][2];

    // Assign generic to shorthand
    float2x3 c = a;

    // CHECK: 4.0
    outputBuffer[3] = c[1][0];

    // Assign shorthand to generic
    matrix<float, 2, 3> d = b;

    // CHECK: 2.0
    outputBuffer[4] = d[0][1];

    // CHECK: 5.0
    outputBuffer[5] = d[1][1];
}
