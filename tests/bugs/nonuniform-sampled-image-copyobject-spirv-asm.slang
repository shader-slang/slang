//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly -O0

// Ensure the non-uniform post-pass propagates through OpCopyObject before
// decorating the sampled-image value actually consumed by sampling.

// CHECK: OpCapability SampledImageArrayNonUniformIndexing
// CHECK-DAG: OpDecorate %[[NU_IDX:[A-Za-z0-9_]+]] NonUniform
// CHECK-DAG: %sampledImage = OpSampledImage
// CHECK-DAG: %sampledImageCopy = OpCopyObject
// CHECK-DAG: OpDecorate %sampledImageCopy NonUniform
// CHECK: OpImageSampleExplicitLod {{.*}} %sampledImageCopy

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
        %sampledImageCopy : %sampledImageType = OpCopyObject %sampledImage;
        %sampled : __sampledType(float) = OpImageSampleExplicitLod %sampledImageCopy $loc Lod $lod;
        __truncate $$float result __sampledType(float) %sampled;
    };
    outputTex[uint2(0, 0)] = value.xxxx;
}
