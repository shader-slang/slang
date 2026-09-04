// ImmutablePtr<int> for read-only access, converted from the read-write pointer that
// __getAddress yields for a mutable structured-buffer element (shader-slang/slang#12581).
// The read-write -> immutable conversion is explicit (Slang does not do it implicitly,
// the same as for `&buf[i]`).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
// Note: -cpu only. Converting to `ImmutablePtr` and passing it to a function currently
// mis-lowers to SPIR-V (invalid OpBitcast on a logical pointer) for both this and `&buf[i]`
// — a pre-existing codegen limitation unrelated to shader-slang/slang#12581.

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
    ImmutablePtr<int> iptr = ImmutablePtr<int>(__getAddress(dataBuffer[0]));

    // CHECK: 7
    outputBuffer[0] = *iptr;

    ImmutablePtr<int> iptr1 = iptr + 1;
    // CHECK: 14
    outputBuffer[1] = *iptr1;

    // CHECK: 21
    outputBuffer[2] = iptr[2];

    ImmutablePtr<int> iptr3 = iptr + 3;
    // CHECK: 28
    outputBuffer[3] = readThrough(iptr3);
}
