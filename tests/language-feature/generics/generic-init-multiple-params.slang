//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

// Verify that a generic constructor with multiple type parameters
// correctly resolves and computes a result from both arguments.

struct Container
{
    int val;

    __init<T : IInteger, U : IInteger>(T a, U b)
    {
        val = a.toInt() + b.toInt();
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Container c = Container(int8_t(10), int16_t(20));
    // CHECK: 30
    outputBuffer[0] = c.val;

    Container c2 = Container(int64_t(100), int16_t(7));
    // CHECK: 107
    outputBuffer[1] = c2.val;
}
