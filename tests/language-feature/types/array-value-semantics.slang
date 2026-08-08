// array-value-semantics.slang

// Test that arrays have value semantics: assignment copies values,
// and modifying the copy does not affect the original.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int original[3] = {10, 20, 30};

    // Copy the array
    int copy[3] = original;

    // Modify the copy
    copy[0] = 99;
    copy[1] = 88;

    // Original is unchanged
    // CHECK: 10
    outputBuffer[0] = original[0];

    // CHECK: 20
    outputBuffer[1] = original[1];

    // Copy has new values
    // CHECK: 99
    outputBuffer[2] = copy[0];

    // CHECK: 88
    outputBuffer[3] = copy[1];
}
