//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression test for https://github.com/shader-slang/slang/issues/9849
// Ensure NonUniform decoration is applied to the index used in OpAccessChain.

// CHECK-DAG: OpDecorate %[[NU_IDX:[A-Za-z0-9_]+]] NonUniform
// CHECK: OpAccessChain %{{.*}} %{{.*}} %[[NU_IDX]]

Texture2D<float> textures[8];
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
