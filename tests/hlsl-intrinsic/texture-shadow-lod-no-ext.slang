//TEST:SIMPLE(filecheck=CHECK): -target glsl -profile glsl_460 -entry fragMain -stage fragment

// Test that shadow LOD sampling on sampler types natively supported by GLSL 1.50
// does not require GL_EXT_texture_shadow_lod (issue #9074).

// CHECK-NOT: GL_EXT_texture_shadow_lod

Sampler2DShadow sampler2d;
Sampler1DShadow sampler1d;
Sampler1DArrayShadow sampler1dArray;

[shader("fragment")]
float4 fragMain() : SV_Target
{
    // These should use baseline GLSL textureLod, no extension required.
    float r0 = sampler2d.SampleCmpLevel(float2(0, 0), 0.5, 0.0);
    float r1 = sampler1d.SampleCmpLevel(0.0, 0.5, 0.0);
    float r2 = sampler1dArray.SampleCmpLevel(float2(0, 0), 0.5, 0.0);

    // textureLodOffset variants should also not require the extension.
    float r3 = sampler2d.SampleCmpLevel(float2(0, 0), 0.5, 0.0, int2(1, 0));
    float r4 = sampler1d.SampleCmpLevel(0.0, 0.5, 0.0, 1);
    float r5 = sampler1dArray.SampleCmpLevel(float2(0, 0), 0.5, 0.0, 1);

    return float4(r0 + r1 + r2, r3 + r4 + r5, 0.0, 1.0);
}
