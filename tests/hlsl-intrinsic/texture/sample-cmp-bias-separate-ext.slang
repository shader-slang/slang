// Each separate Texture* + SamplerComparisonState SampleCmpBias form that needs
// GL_EXT_texture_shadow_lod is compiled as its own entry point, so the #extension directive is
// asserted per form. sample-cmp.slang compiles all of them in one shader, where a single emitted
// directive would satisfy its check even if only one of these wrappers still requested the
// extension. Companion of tests/glsl-intrinsic/texture-shadow-2d-array-bias-ext.slang, which does
// the same for the combined-sampler path.

//TEST:SIMPLE(filecheck=CHECK_BIAS): -target glsl -profile glsl_460 -entry biasMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK_BIAS_OFFSET): -target glsl -profile glsl_460 -entry biasOffsetMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK_CUBE_ARRAY_BIAS): -target glsl -profile glsl_460 -entry cubeArrayBiasMain -stage fragment

Texture2DArray<float> shadowMap2DArray;
TextureCubeArray<float> shadowMapCubeArray;
SamplerComparisonState shadowSampler;

// CHECK_BIAS: #extension GL_EXT_texture_shadow_lod
// CHECK_BIAS: texture(sampler2DArrayShadow(
[shader("fragment")]
float4 biasMain(float4 pos : SV_Position) : SV_Target
{
    float r = shadowMap2DArray.SampleCmpBias(shadowSampler, float3(0.1, 0.2, 0.0), 0.5, 0.25);
    return float4(r, r, r, 1.0);
}

// CHECK_BIAS_OFFSET: #extension GL_EXT_texture_shadow_lod
// CHECK_BIAS_OFFSET: textureOffset(sampler2DArrayShadow(
[shader("fragment")]
float4 biasOffsetMain(float4 pos : SV_Position) : SV_Target
{
    float r = shadowMap2DArray.SampleCmpBias(shadowSampler, float3(0.1, 0.2, 0.0), 0.5, 0.25, int2(1, 0));
    return float4(r, r, r, 1.0);
}

// CHECK_CUBE_ARRAY_BIAS: #extension GL_EXT_texture_shadow_lod
// CHECK_CUBE_ARRAY_BIAS: texture(samplerCubeArrayShadow(
[shader("fragment")]
float4 cubeArrayBiasMain(float4 pos : SV_Position) : SV_Target
{
    float r = shadowMapCubeArray.SampleCmpBias(shadowSampler, float4(0.1, 0.2, 0.3, 0.0), 0.5, 0.25);
    return float4(r, r, r, 1.0);
}
