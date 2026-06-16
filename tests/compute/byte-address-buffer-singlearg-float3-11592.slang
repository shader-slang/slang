//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression for https://github.com/shader-slang/slang/issues/11545.
//
// The implicit single-argument `LoadAligned<T>`/`StoreAligned` forms forward `T`'s natural
// *alignment* — the largest power of two dividing `T`'s natural stride — not the stride itself.
// A `float3`'s 12-byte stride is not a power of two, so forwarding the stride would make a
// location of 16 fail the "location is a multiple of the alignment" contract (16 % 12 != 0) and
// be spuriously rejected with E41303. The natural alignment is 4, which divides 16, so the call
// is valid and must compile and round-trip the stored value.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer.StoreAligned<float3>(16, float3(2.0f, 3.0f, 4.0f));
    float3 v = outputBuffer.LoadAligned<float3>(16);

    // CHECK: 2.0
    // CHECK: 3.0
    // CHECK: 4.0
    outputBuffer.Store<float>(0, v.x);
    outputBuffer.Store<float>(4, v.y);
    outputBuffer.Store<float>(8, v.z);
}
