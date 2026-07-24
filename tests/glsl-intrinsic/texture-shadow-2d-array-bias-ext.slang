// Each GLSL-source sampler2DArrayShadow overload that GL_EXT_texture_shadow_lod introduces is
// compiled as its own entry point, so the #extension directive is asserted per form — a shared
// check would still pass if only one of the three still requested the extension.

//TEST:SIMPLE(filecheck=CHECK_BIAS): -allow-glsl -target glsl -profile glsl_460 -entry biasMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK_BIAS_OFFSET): -allow-glsl -target glsl -profile glsl_460 -entry biasOffsetMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK_LOD_OFFSET): -allow-glsl -target glsl -profile glsl_460 -entry lodOffsetMain -stage fragment

uniform sampler2DArrayShadow s;

// CHECK_BIAS: #extension GL_EXT_texture_shadow_lod
// CHECK_BIAS: texture(
[shader("fragment")]
float4 biasMain(float4 pos : SV_Position) : SV_Target
{
    float r = texture(s, vec4(0.1, 0.2, 0.0, 0.5), 0.25);
    return float4(r, r, r, 1.0);
}

// CHECK_BIAS_OFFSET: #extension GL_EXT_texture_shadow_lod
// CHECK_BIAS_OFFSET: textureOffset(
[shader("fragment")]
float4 biasOffsetMain(float4 pos : SV_Position) : SV_Target
{
    float r = textureOffset(s, vec4(0.1, 0.2, 0.0, 0.5), ivec2(1, 0), 0.25);
    return float4(r, r, r, 1.0);
}

// CHECK_LOD_OFFSET: #extension GL_EXT_texture_shadow_lod
// CHECK_LOD_OFFSET: textureLodOffset(
[shader("fragment")]
float4 lodOffsetMain(float4 pos : SV_Position) : SV_Target
{
    float r = textureLodOffset(s, vec4(0.1, 0.2, 0.0, 0.5), 0.0, ivec2(1, 0));
    return float4(r, r, r, 1.0);
}
