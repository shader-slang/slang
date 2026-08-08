// enum-explicit-values.slang

// Test explicit value assignment with auto-increment for unassigned cases.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum Color
{
    Red = 5,
    Green,    // auto-increments to 6
    Blue,     // auto-increments to 7
    Alpha = 20
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 5
    outputBuffer[0] = int(Color.Red);

    // CHECK: 6
    outputBuffer[1] = int(Color.Green);

    // CHECK: 7
    outputBuffer[2] = int(Color.Blue);

    // CHECK: 20
    outputBuffer[3] = int(Color.Alpha);
}
