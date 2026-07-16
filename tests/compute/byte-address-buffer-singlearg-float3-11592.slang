//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression for https://github.com/shader-slang/slang/issues/11545.
//
// The implicit single-argument `LoadAligned<T>`/`StoreAligned` forms forward `T`'s *base*
// (std430 whole-type) alignment, which for a `float3` is 16 — the 4-component base alignment,
// not the 12-byte natural stride. A location of 16 satisfies that promise (16 % 16 == 0), so the
// call is valid, emits no below-base warning, and round-trips the stored value through a single
// wide access.
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
