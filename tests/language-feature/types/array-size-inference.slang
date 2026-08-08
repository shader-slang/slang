// array-size-inference.slang

// Test that array size can be inferred from the initializer list.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Size inferred as 3
    int a[] = {100, 200, 300};

    // CHECK: 100
    outputBuffer[0] = a[0];

    // CHECK: 200
    outputBuffer[1] = a[1];

    // CHECK: 300
    outputBuffer[2] = a[2];

    // Size inferred as 4
    int b[] = {1, 2, 3, 4};

    // CHECK: 10
    outputBuffer[3] = b[0] + b[1] + b[2] + b[3];
}
