//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Ensure non-uniform sampled-image values carry the NonUniform decoration to
// the OpSampledImage result and request the corresponding capability.

// CHECK: OpCapability SampledImageArrayNonUniformIndexing
// CHECK-DAG: OpDecorate %[[NU_IDX:[A-Za-z0-9_]+]] NonUniform
// CHECK-DAG: OpDecorate %sampledImage NonUniform
// CHECK: OpAccessChain %{{.*}} %{{.*}} %[[NU_IDX]]
// CHECK: %sampledImage = OpSampledImage
// CHECK: OpImageSampleExplicitLod {{.*}} %sampledImage

Texture2D<float4> textures[8];
SamplerState samp;
RWTexture2D<float4> outputTex;

cbuffer Params
{
    uint idx;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    float4 v = textures[NonUniformResourceIndex(i)].SampleLevel(samp, float2(0.0, 0.0), 0.0);
    outputTex[uint2(0, 0)] = v;
}
