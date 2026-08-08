// enum-explicit-underlying-type.slang

// Test that an enum can declare an explicit underlying type (uint16_t).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum Channel : uint16_t
{
    Red,
    Green,
    Blue,
    Alpha
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 0
    outputBuffer[0] = int(Channel.Red);

    // CHECK: 1
    outputBuffer[1] = int(Channel.Green);

    // CHECK: 2
    outputBuffer[2] = int(Channel.Blue);

    // CHECK: 3
    outputBuffer[3] = int(Channel.Alpha);
}
