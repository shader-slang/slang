//TEST:SIMPLE(filecheck=CHECK): -target glsl -profile glsl_460 -entry fragMain -stage fragment

// SampleCmpBias / SampleCmpGrad shadow sampling on the GLSL-1.50 baseline sampler types
// (sampler1DShadow, sampler1DArrayShadow, sampler2DShadow, samplerCubeShadow) must NOT require
// GL_EXT_texture_shadow_lod: GLSL's texture(P, bias) / textureGrad() for these forms are core.
// Only sampler2DArrayShadow and samplerCubeArrayShadow bias forms need the extension, and this
// shader deliberately omits those so the negative can be asserted. Companion of
// texture-shadow-lod-no-ext.slang, which covers the LOD forms.

// CHECK-NOT: GL_EXT_texture_shadow_lod

Sampler1DShadow sampler1d;
Sampler1DArrayShadow sampler1dArray;
Sampler2DShadow sampler2d;
SamplerCubeShadow samplerCube;

[shader("fragment")]
float4 fragMain(float4 pos : SV_Position) : SV_Target
{
    float depth = 0.5;
    float bias = 0.25;
    float shadow = 0;

    // Bias — baseline forms, core GLSL, no extension.
    shadow += sampler1d.SampleCmpBias(0.1, depth, bias);
    shadow += sampler1dArray.SampleCmpBias(float2(0.1, 0), depth, bias);
    shadow += sampler2d.SampleCmpBias(float2(0.1, 0.2), depth, bias);
    shadow += samplerCube.SampleCmpBias(float3(0.1, 0.2, 0.3), depth, bias);

    // Gradients — always core, never the extension.
    shadow += sampler1d.SampleCmpGrad(0.1, depth, 0.001, 0.001);
    shadow += sampler2d.SampleCmpGrad(float2(0.1, 0.2), depth, float2(0.001, 0), float2(0, 0.001));
    shadow += samplerCube.SampleCmpGrad(float3(0.1, 0.2, 0.3), depth, float3(0.001, 0, 0), float3(0, 0.001, 0));

    return float4(shadow, shadow, shadow, 1.0);
}
