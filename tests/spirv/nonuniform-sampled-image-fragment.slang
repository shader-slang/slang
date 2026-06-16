//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -emit-spirv-directly

// Fragment-stage companion to nonuniform-sampled-image.slang. Issue #10525
// originally surfaced in a fragment shader, so cover the fragment stage
// explicitly (the rest of the suite is compute-only apart from
// input-attachment). The NonUniform propagation must reach the same operands
// regardless of stage.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability SampledImageArrayNonUniformIndexing

// The index, access chain, loaded texture, and sampled image must all be
// decorated NonUniform per Vulkan VUID-RuntimeSpirv-None-10148.
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %textures %[[IDX:[A-Za-z0-9_]+]]
// CHECK-DAG: OpDecorate %[[IDX]] NonUniform
// CHECK-DAG: OpDecorate %[[AC]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[AC]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform
// CHECK-DAG: %[[SI:[A-Za-z0-9_]+]] = OpSampledImage %{{[A-Za-z0-9_]+}} %[[LOAD]]
// CHECK-DAG: OpDecorate %[[SI]] NonUniform

Texture2D<float> textures[8];
SamplerState samp;

cbuffer Params { uint idx; };

float4 main(float2 uv : TEXCOORD0) : SV_Target
{
    uint i = idx;
    return textures[NonUniformResourceIndex(i)].Sample(samp, uv);
}
