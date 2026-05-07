// enum-basic-scoped.slang

// Test that enums are scoped by default and values must be accessed via EnumType.Value.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum Color
{
    Red,
    Green,
    Blue
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 0
    outputBuffer[0] = int(Color.Red);

    // CHECK: 1
    outputBuffer[1] = int(Color.Green);

    // CHECK: 2
    outputBuffer[2] = int(Color.Blue);

    Color c = Color.Green;
    // CHECK: 1
    outputBuffer[3] = int(c);
}
