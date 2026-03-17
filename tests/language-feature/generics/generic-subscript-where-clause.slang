//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// Verify that a generic subscript with a where-clause constraint
// supports both get and set accessors with different integer types.

struct Grid
{
    float data[4];

    __subscript<T>(T index) -> float where T : IInteger
    {
        get { return data[index.toInt()]; }
        set { data[index.toInt()] = newValue; }
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Grid g;
    // Write through subscript using int8_t index
    g[int8_t(0)] = 1.5;
    g[int8_t(1)] = 2.5;

    // Write using int16_t index
    g[int16_t(2)] = 3.5;

    // Write using int index
    g[int(3)] = 4.5;

    // Read back using various integer types
    // CHECK: 1.5
    outputBuffer[0] = g[int8_t(0)];

    // CHECK: 2.5
    outputBuffer[1] = g[int16_t(1)];

    // CHECK: 3.5
    outputBuffer[2] = g[int(2)];

    // CHECK: 4.5
    outputBuffer[3] = g[int64_t(3)];
}
