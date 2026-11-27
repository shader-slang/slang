// Cube textures take a minimum LOD clamp in the HLSL Sample* methods instead of a texel offset:
// `TextureCube.SampleGrad(sampler, location, gradX, gradY, clamp)` must compile and lower to
// `OpImageSampleExplicitLod` with the `Grad|MinLod` operands and no `ConstOffset`
// (shader-slang/slang#14671).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage fragment -entry fragmentMain -profile sm_6_6 -lang hlsl

TextureCube<float4> testCube;
SamplerState testSampler;

float4 fragmentMain(
    float3 direction : TEXCOORD0,
    float3 gradientX : TEXCOORD1,
    float3 gradientY : TEXCOORD2,
    float minimumLod : TEXCOORD3) : SV_Target
{
    return testCube.SampleGrad(testSampler, direction, gradientX, gradientY, minimumLod);
}

// CHECK: OpImageSampleExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|MinLod
// CHECK-NOT: ConstOffset
