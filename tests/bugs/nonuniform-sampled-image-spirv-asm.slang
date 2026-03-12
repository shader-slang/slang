//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Ensure typed spirv_asm OpSampledImage decoration targets the sampled-image
// result, not the named OpTypeSampledImage helper ID.

// CHECK: OpCapability SampledImageArrayNonUniformIndexing
// CHECK: OpDecorate %[[NU_IDX:[A-Za-z0-9_]+]] NonUniform
// CHECK-NOT: OpDecorate %sampledImageType NonUniform
// CHECK: OpDecorate %sampledImage NonUniform
// CHECK: OpAccessChain %{{.*}} %{{.*}} %[[NU_IDX]]
// CHECK: %sampledImage = OpSampledImage
// CHECK: OpImageSampleExplicitLod {{.*}} %sampledImage

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
    Texture2D<float> tex = textures[NonUniformResourceIndex(i)];
    float2 loc = float2(0.0, 0.0);
    float lod = 0.0;
    float value = spirv_asm
    {
        %sampledImageType = OpTypeSampledImage $$Texture2D<float>;
        %sampledImage : %sampledImageType = OpSampledImage $tex $samp;
        %sampled : __sampledType(float) = OpImageSampleExplicitLod %sampledImage $loc Lod $lod;
        __truncate $$float result __sampledType(float) %sampled;
    };
    outputTex[uint2(0, 0)] = value.xxxx;
}
