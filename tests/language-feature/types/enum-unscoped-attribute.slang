// enum-unscoped-attribute.slang

// Test that [UnscopedEnum] allows accessing enum values without the scope prefix.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[UnscopedEnum]
enum Direction
{
    North,
    South,
    East,
    West
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Access without scope prefix
    // CHECK: 0
    outputBuffer[0] = int(North);

    // CHECK: 1
    outputBuffer[1] = int(South);

    // CHECK: 2
    outputBuffer[2] = int(East);

    // CHECK: 3
    outputBuffer[3] = int(West);
}
