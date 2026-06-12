//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression test: a non-uniform nested access arr[NonUniformResourceIndex(i)][0]
// decorates the access chain, and its inner index is the constant 0. Integer
// literals are deduplicated module-wide, so that same %int_0 is also the
// descriptor index of the *uniformly* accessed texture array texs[0].
//
// propagateNonUniformDecorations must not decorate a constant index (only the
// access-chain result). Otherwise the forward scan would see the shared %int_0
// already decorated and spuriously decorate the uniform texs[0] access chain
// NonUniform -- pulling in SampledImageArrayNonUniformIndexing that this shader
// does not need (texs is only ever indexed uniformly), a capability a device
// might not support.

// SampledImageArrayNonUniformIndexing must NOT be required. This CHECK-NOT is
// the first directive so its window is [start-of-input, OpMemoryModel], covering
// the entire OpCapability section regardless of capability emission order.
// CHECK-NOT: OpCapability SampledImageArrayNonUniformIndexing
// CHECK: OpMemoryModel

// Sanity: the genuinely non-uniform structured-buffer access IS decorated
// (slang-test appends "; NonUniform" inline), so the test is not vacuous.
// CHECK: OpAccessChain {{.*}} %arr {{.*}}NonUniform

RWStructuredBuffer<int> arr[];
Texture2D<float4> texs[];
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

cbuffer Params { uint i; };

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    // Non-uniform: decorates the inner index (the shared constant 0).
    arr[NonUniformResourceIndex(i + tid.x)][0] = 1;
    // Uniform: texs[0] reuses the same constant 0 as its descriptor index.
    outBuf[0] = texs[0].SampleLevel(samp, float2(0, 0), 0);
}
