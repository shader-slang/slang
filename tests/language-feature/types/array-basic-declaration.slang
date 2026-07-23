// array-basic-declaration.slang

// Test that arrays can be declared with explicit size, elements accessed by index, and assigned.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int arr[4] = {10, 20, 30, 40};

    // CHECK: 10
    outputBuffer[0] = arr[0];

    // CHECK: 30
    outputBuffer[1] = arr[2];

    // Assign to element
    arr[3] = 99;

    // CHECK: 99
    outputBuffer[2] = arr[3];

    // Assign entire array from another
    int arr2[4] = {5, 6, 7, 8};
    arr = arr2;

    // CHECK: 6
    outputBuffer[3] = arr[1];
}
