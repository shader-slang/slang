//TEST:SIMPLE(filecheck=CHECK): -allow-glsl -target glsl -profile glsl_460 -entry fragMain -stage fragment

// The GLSL-source texture(samplerCubeArrayShadow, vec4 P, float compare, float bias) builtin is a
// GL_EXT_texture_shadow_lod form, so lowering it must emit the extension directive (a regression that
// dropped the branch-local __requireCapability would emit the builtin with no #extension — invalid
// GLSL). Positively assert the pragma; companion of the negative sample-cmp-bias-grad-no-ext.slang.

// CHECK: #extension GL_EXT_texture_shadow_lod

uniform samplerCubeArrayShadow uniform_samplerCubeArrayShadow;

[shader("fragment")]
float4 fragMain(float4 pos : SV_Position) : SV_Target
{
    float r = texture(uniform_samplerCubeArrayShadow, vec4(0.1, 0.2, 0.3, 0.0), 0.5, 0.25);
    return float4(r, r, r, 1.0);
}
