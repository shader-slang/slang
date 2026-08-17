// pointer-immutable.slang

// Test ImmutablePtr<T> for read-only pointer access.

// DISABLED: __getAddress on a structured-buffer element (buf[i]) is rejected as a
// function-local address on every target (spirv/cuda/cpp), not just GPU. Tracked by
// shader-slang/slang#12581. Re-enable with a -cpu directive once that is fixed.
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[7 14 21 28], stride=4):name=dataBuffer
RWStructuredBuffer<int> dataBuffer;

int readThrough(ImmutablePtr<int> p)
{
    return *p;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Create an ImmutablePtr from buffer data
    ImmutablePtr<int> iptr = __getAddress(dataBuffer[0]);

    // Read through ImmutablePtr
    // CHECK: 7
    outputBuffer[0] = *iptr;

    // ImmutablePtr supports pointer arithmetic
    ImmutablePtr<int> iptr1 = iptr + 1;
    // CHECK: 14
    outputBuffer[1] = *iptr1;

    // ImmutablePtr supports subscript
    // CHECK: 21
    outputBuffer[2] = iptr[2];

    // Pass ImmutablePtr to function
    ImmutablePtr<int> iptr3 = iptr + 3;
    // CHECK: 28
    outputBuffer[3] = readThrough(iptr3);
}
