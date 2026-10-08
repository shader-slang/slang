//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression test: when a non-uniformly-indexed texture is combined with a
// UNIFORM sampler (a single global, not indexed), only the texture path is
// non-uniform. The float pass must decorate the texture's access chain (and the
// combined OpSampledImage result) NonUniform, but must NOT spuriously decorate
// the uniform sampler global -- the non-uniformity only flows through the
// texture operand of the MakeCombinedTextureSampler.

// CHECK: OpCapability SampledImageArrayNonUniformIndexing

// The uniform sampler global must never be decorated NonUniform. OpName sits in
// the debug section (before annotations) and OpFunction after them, so this
// CHECK-NOT window covers every OpDecorate in the module.
// CHECK: OpName %samp
// CHECK-NOT: OpDecorate %samp NonUniform
// CHECK: OpFunction

// The non-uniformly-indexed texture's access chain is decorated (slang-test
// appends "; NonUniform" inline to decorated instructions).
// CHECK: OpAccessChain {{.*}} %textures {{.*}}NonUniform

Texture2D<float4> textures[];
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

cbuffer Params { uint idx; };

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    outBuf[0] = textures[NonUniformResourceIndex(i)].SampleLevel(samp, float2(0.5, 0.5), 0);
}
