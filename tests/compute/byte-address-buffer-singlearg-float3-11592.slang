//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression for https://github.com/shader-slang/slang/issues/11545.
//
// The implicit single-argument `LoadAligned<T>`/`StoreAligned` forms forward `T`'s natural
// *alignment* — the largest power of two dividing `T`'s natural stride — not the stride itself.
// A `float3`'s 12-byte stride is not a power of two, so forwarding the stride would make a
// location of 16 fail the "location is a multiple of the alignment" contract (16 % 12 != 0) and
// be spuriously rejected with E41303. The natural alignment is 4, which divides 16, so the call
// is valid and must compile and round-trip the stored value.
//
// The round-tripped components are written to a typed `RWStructuredBuffer<float>` so the output
// renders as decimal floats (a raw `RWByteAddressBuffer` would render as hex bit patterns).

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):name=scratch
RWByteAddressBuffer scratch;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    scratch.StoreAligned<float3>(16, float3(2.0f, 3.0f, 4.0f));
    float3 v = scratch.LoadAligned<float3>(16);

    // CHECK: 2.0
    outputBuffer[0] = v.x;
    // CHECK: 3.0
    outputBuffer[1] = v.y;
    // CHECK: 4.0
    outputBuffer[2] = v.z;
}
