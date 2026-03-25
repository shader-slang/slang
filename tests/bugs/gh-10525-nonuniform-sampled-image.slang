//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Test that NonUniform decorations propagate to OpSampledImage results
// and that the SampledImageArrayNonUniformIndexing capability is emitted.

// CHECK-DAG: OpCapability SampledImageArrayNonUniformIndexing

// The index, access chain, loaded texture, and sampled image must all be
// decorated NonUniform per Vulkan VUID-RuntimeSpirv-None-10148.
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %textures %[[IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: OpDecorate %[[IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[AC]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform
// CHECK-DAG: %sampledImage = OpSampledImage %{{[A-Za-z0-9_]+}} %[[LOAD]]
// CHECK-DAG: OpDecorate %sampledImage NonUniform

Texture2D<float> textures[8];
SamplerState samp;
RWTexture2D<float4> outputTex;

cbuffer Params { uint idx; };

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    float4 v = textures[NonUniformResourceIndex(i)].SampleLevel(samp, float2(0.0, 0.0), 0.0);
    outputTex[uint2(0, 0)] = v;
}
