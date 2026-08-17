// pointer-generic-syntax.slang

// Test Ptr<T> generic syntax equivalence with T*.

// DISABLED: __getAddress on a structured-buffer element (buf[i]) is rejected as a
// function-local address on every target (spirv/cuda/cpp), not just GPU. Tracked by
// shader-slang/slang#12581. Re-enable with a -cpu directive once that is fixed.
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[5 15 25 35], stride=4):name=dataBuffer
RWStructuredBuffer<int> dataBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Declare pointer using T* syntax
    int* ptrStar = __getAddress(dataBuffer[0]);

    // Declare pointer using Ptr<T> generic syntax
    Ptr<int> ptrGeneric = __getAddress(dataBuffer[2]);

    // Both forms should work identically
    // CHECK: 5
    outputBuffer[0] = *ptrStar;

    // CHECK: 25
    outputBuffer[1] = *ptrGeneric;

    // Assign Ptr<T> to T* (they are the same type)
    int* ptrFromGeneric = ptrGeneric;
    // CHECK: 25
    outputBuffer[2] = *ptrFromGeneric;

    // Assign T* to Ptr<T>
    Ptr<int> genericFromStar = ptrStar;
    // CHECK: 5
    outputBuffer[3] = *genericFromStar;
}
