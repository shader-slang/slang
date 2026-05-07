//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Ensure raw spirv_asm NonUniform decorations seed the post-pass, and that
// non-uniform provenance propagates through OpImage to the consuming image op.

// CHECK: OpCapability SampledImageArrayNonUniformIndexing
// CHECK-DAG: OpDecorate %sampledImage NonUniform
// CHECK-DAG: OpDecorate %image NonUniform
// CHECK: %sampledImage = OpSampledImage
// CHECK: %image = OpImage
// CHECK: OpImageQuerySizeLod {{.*}} %image

Texture2D<float> textures[8];
SamplerState samp;
RWTexture2D<float4> outputTex;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Texture2D<float> tex = textures[0];
    int zero = 0;
    int2 size = spirv_asm
    {
        %sampledImageType = OpTypeSampledImage $$Texture2D<float>;
        %sampledImage : %sampledImageType = OpSampledImage $tex $samp;
        OpDecorate %sampledImage NonUniform;
        %image : $$Texture2D<float> = OpImage %sampledImage;
        %size : $$int2 = OpImageQuerySizeLod %image $zero;
        __truncate $$int2 result $$int2 %size;
    };
    outputTex[uint2(0, 0)] = float4(size, 0, 0);
}
