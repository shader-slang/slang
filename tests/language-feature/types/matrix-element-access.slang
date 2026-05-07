// matrix-element-access.slang

// Test matrix element access via m[row] (returns row vector) and m[row][col].

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float3x2 m = float3x2(
        1.0, 2.0,
        3.0, 4.0,
        5.0, 6.0
    );

    // Access element via m[row][col]
    // CHECK: 1.0
    outputBuffer[0] = m[0][0];

    // CHECK: 2.0
    outputBuffer[1] = m[0][1];

    // CHECK: 3.0
    outputBuffer[2] = m[1][0];

    // CHECK: 6.0
    outputBuffer[3] = m[2][1];

    // Access row vector via m[row]
    float2 row0 = m[0];
    float2 row1 = m[1];
    float2 row2 = m[2];

    // Row 0 sum
    // CHECK: 3.0
    outputBuffer[4] = row0.x + row0.y;

    // Row 1 sum
    // CHECK: 7.0
    outputBuffer[5] = row1.x + row1.y;

    // Row 2 sum
    // CHECK: 11.0
    outputBuffer[6] = row2.x + row2.y;

    // Total sum of all elements
    // CHECK: 21.0
    outputBuffer[7] = m[0][0] + m[0][1] + m[1][0] + m[1][1] + m[2][0] + m[2][1];
}
