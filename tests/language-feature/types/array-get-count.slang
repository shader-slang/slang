// array-get-count.slang

// Test that getCount() returns the array length.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int getLength(int arr[])
{
    return arr.getCount();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int a[3] = {1, 2, 3};
    int b[5] = {10, 20, 30, 40, 50};
    int c[1] = {99};

    // CHECK: 3
    outputBuffer[0] = a.getCount();

    // CHECK: 5
    outputBuffer[1] = b.getCount();

    // getCount() through unsized parameter
    // CHECK: 3
    outputBuffer[2] = getLength(a);

    // CHECK: 1
    outputBuffer[3] = getLength(c);
}
