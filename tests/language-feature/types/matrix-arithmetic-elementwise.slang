// matrix-arithmetic-elementwise.slang

// Test element-wise operations on matrices (+ and * are element-wise).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float2x2 a = float2x2(
        1.0, 2.0,
        3.0, 4.0
    );

    float2x2 b = float2x2(
        10.0, 20.0,
        30.0, 40.0
    );

    // Element-wise addition
    float2x2 sum = a + b;

    // CHECK: 11.0
    outputBuffer[0] = sum[0][0];

    // CHECK: 22.0
    outputBuffer[1] = sum[0][1];

    // CHECK: 33.0
    outputBuffer[2] = sum[1][0];

    // CHECK: 44.0
    outputBuffer[3] = sum[1][1];

    // Element-wise multiplication (NOT matrix multiply)
    float2x2 prod = a * b;

    // CHECK: 10.0
    outputBuffer[4] = prod[0][0];

    // CHECK: 40.0
    outputBuffer[5] = prod[0][1];

    // CHECK: 90.0
    outputBuffer[6] = prod[1][0];

    // CHECK: 160.0
    outputBuffer[7] = prod[1][1];
}
