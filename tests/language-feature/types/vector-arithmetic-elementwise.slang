// vector-arithmetic-elementwise.slang

// Test that arithmetic operators (+, -, *, /) apply element-wise to vectors.
// Note: * is element-wise multiplication, NOT matrix multiply.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float2 a = float2(10.0, 20.0);
    float2 b = float2(2.0, 4.0);

    // Element-wise addition
    float2 sum = a + b;

    // CHECK: 12.0
    outputBuffer[0] = sum.x;

    // CHECK: 24.0
    outputBuffer[1] = sum.y;

    // Element-wise subtraction
    float2 diff = a - b;

    // CHECK: 8.0
    outputBuffer[2] = diff.x;

    // CHECK: 16.0
    outputBuffer[3] = diff.y;

    // Element-wise multiplication (NOT matrix multiply)
    float2 prod = a * b;

    // CHECK: 20.0
    outputBuffer[4] = prod.x;

    // CHECK: 80.0
    outputBuffer[5] = prod.y;

    // Element-wise division
    float2 quot = a / b;

    // CHECK: 5.0
    outputBuffer[6] = quot.x;

    // CHECK: 5.0
    outputBuffer[7] = quot.y;
}
