//TEST:SIMPLE(filecheck=CHECK): -target glsl -profile glsl_460 -entry fragMain -stage fragment

// SampleCmpBias / SampleCmpGrad shadow sampling on the GLSL-1.50 baseline forms must NOT require
// GL_EXT_texture_shadow_lod. Two families of baseline forms are covered here:
//  - bias on the baseline shapes (sampler1DShadow, sampler1DArrayShadow, sampler2DShadow,
//    samplerCubeShadow): GLSL's texture(P, bias) for these is core; only sampler2DArrayShadow and
//    samplerCubeArrayShadow bias need the extension, and this shader deliberately omits those.
//  - ALL gradient forms, including sampler2DArrayShadow: textureGrad()/textureGradOffset() on shadow
//    samplers are entirely core GLSL — the extension adds no gradient variants. sampler2DArrayShadow
//    grad is included precisely because it is the shape most at risk of being mis-routed to an
//    extension wrapper.
// Both the combined-sampler and the separate Texture*+SamplerComparisonState paths are exercised,
// since hlsl.meta.slang has an independent wrapper set for each. Companion of
// texture-shadow-lod-no-ext.slang, which covers the LOD forms.

// Anchor on `#version`, which GLSL always emits first — before any `#extension` line. This makes
// the test non-vacuous (it only passes on real, non-empty GLSL output) while keeping the CHECK-NOT
// region covering the preamble, where `#extension GL_EXT_texture_shadow_lod` would appear if a
// baseline form ever leaked it. (Anchoring on a body `texture(` call would leave the preamble
// unscanned — and `texture` even matches inside the `GL_EXT_texture_shadow_lod` token itself.)
// CHECK: #version
// CHECK-NOT: GL_EXT_texture_shadow_lod

// Combined shadow samplers.
Sampler1DShadow sampler1d;
Sampler1DArrayShadow sampler1dArray;
Sampler2DShadow sampler2d;
Sampler2DArrayShadow sampler2dArray;
SamplerCubeShadow samplerCube;

// Separate Texture* + SamplerComparisonState path.
Texture1D<float> tex1d;
Texture2D<float> tex2d;
SamplerComparisonState cmp;

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

    // Gradients — always core, never the extension, including sampler2DArrayShadow.
    shadow += sampler1d.SampleCmpGrad(0.1, depth, 0.001, 0.001);
    shadow += sampler2d.SampleCmpGrad(float2(0.1, 0.2), depth, float2(0.001, 0), float2(0, 0.001));
    shadow += samplerCube.SampleCmpGrad(float3(0.1, 0.2, 0.3), depth, float3(0.001, 0, 0), float3(0, 0.001, 0));
    shadow += sampler2dArray.SampleCmpGrad(float3(0.1, 0.2, 0), depth, float2(0.001, 0), float2(0, 0.001));
    shadow += sampler2dArray.SampleCmpGrad(float3(0.1, 0.2, 0), depth, float2(0.001, 0), float2(0, 0.001), int2(1, 0));

    // Separate-sampler path — baseline bias and gradient forms, also core.
    shadow += tex1d.SampleCmpBias(cmp, 0.1, depth, bias);
    shadow += tex2d.SampleCmpBias(cmp, float2(0.1, 0.2), depth, bias);
    shadow += tex2d.SampleCmpGrad(cmp, float2(0.1, 0.2), depth, float2(0.001, 0), float2(0, 0.001));

    return float4(shadow, shadow, shadow, 1.0);
}
