// array-unsized-parameter.slang

// Test passing a sized array to an unsized array parameter.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int sumArray(int b[])
{
    int total = 0;
    for (int i = 0; i < b.getCount(); i++)
        total += b[i];
    return total;
}

int firstElement(int b[])
{
    return b[0];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int arr3[3] = {1, 2, 3};
    int arr4[4] = {10, 20, 30, 40};

    // Sum of 3-element array
    // CHECK: 6
    outputBuffer[0] = sumArray(arr3);

    // Sum of 4-element array
    // CHECK: 100
    outputBuffer[1] = sumArray(arr4);

    // CHECK: 1
    outputBuffer[2] = firstElement(arr3);

    // CHECK: 10
    outputBuffer[3] = firstElement(arr4);
}
