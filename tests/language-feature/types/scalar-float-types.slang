// scalar-float-types.slang

// Test that half, float, and double types can be declared and used in arithmetic.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // half type
    half h1 = half(1.5);
    half h2 = half(2.5);
    float halfResult = float(h1 + h2);

    // CHECK: 4.0
    outputBuffer[0] = halfResult;

    // float type
    float f1 = 10.0;
    float f2 = 3.0;
    float floatResult = f1 + f2;

    // CHECK: 13.0
    outputBuffer[1] = floatResult;

    // float multiplication
    float f3 = 2.5;
    float f4 = 4.0;

    // CHECK: 10.0
    outputBuffer[2] = f3 * f4;

    // double type
    double d1 = 100.0;
    double d2 = 0.25;
    float doubleResult = float(d1 * d2);

    // CHECK: 25.0
    outputBuffer[3] = doubleResult;

    // mixed arithmetic: half promoted to float
    half h3 = half(3.0);
    float f5 = 7.0;
    float mixedResult = float(h3) + f5;

    // CHECK: 10.0
    outputBuffer[4] = mixedResult;

    // float division
    float f6 = 20.0;
    float f7 = 4.0;

    // CHECK: 5.0
    outputBuffer[5] = f6 / f7;
}
