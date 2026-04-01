//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// Verify that generic constructor type parameters are inferred
// from the argument types at the call site without explicit specialization.

struct Accumulator
{
    int result;

    __init<T : IInteger>(T a, T b)
    {
        result = a.toInt() * b.toInt();
    }
}

struct Pair
{
    int sum;

    __init<T : IInteger, U : IInteger>(T first, U second)
    {
        sum = first.toInt() + second.toInt();
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // T inferred as int8_t from both arguments
    Accumulator acc = Accumulator(int8_t(3), int8_t(5));
    // CHECK: 15
    outputBuffer[0] = acc.result;

    // T inferred as int16_t
    Accumulator acc2 = Accumulator(int16_t(4), int16_t(6));
    // CHECK: 24
    outputBuffer[1] = acc2.result;

    // T inferred as int8_t, U inferred as int16_t (multi-parameter inference)
    Pair p = Pair(int8_t(10), int16_t(20));
    // CHECK: 30
    outputBuffer[2] = p.sum;

    // T inferred as int64_t, U inferred as int16_t (wide integer inference)
    Pair p2 = Pair(int64_t(100), int16_t(7));
    // CHECK: 107
    outputBuffer[3] = p2.sum;
}
