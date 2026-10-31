//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/texture-shadow-bias-hlsl.slang -target hlsl -profile sm_6_8 -entry fragMain -stage fragment

// In a stage with implicit derivatives (fragment here), the GLSL-source shadow texture(..., bias),
// textureOffset(..., bias), textureProj(..., bias) and textureProjOffset(..., bias) builtins lower
// to SampleCmpBias on HLSL; lowering them to SampleCmp would still compile but silently drop the
// bias. Each call samples its own uniform, and each CHECK anchors on both halves of the
// <name>_texture_N / <name>_sampler_N pair that HLSL legalization makes of that uniform. No name is
// a suffix of another, so a failing CHECK identifies the form that regressed.

uniform sampler1DShadow bias1D;
uniform sampler2DShadow bias2D;
uniform samplerCubeShadow biasCube;
uniform sampler1DArrayShadow bias1DArray;
uniform sampler2DArrayShadow bias2DArray;
uniform samplerCubeArrayShadow biasCubeArray;
uniform sampler1DShadow offs1D;
uniform sampler2DShadow offs2D;
uniform sampler1DArrayShadow offs1DArray;
uniform sampler2DArrayShadow offs2DArray;
uniform sampler1DShadow proj1D;
uniform sampler2DShadow proj2D;
uniform sampler1DShadow projOffset1D;
uniform sampler2DShadow projOffset2D;

// CHECK: bias1D_texture_{{[0-9]+}}.SampleCmpBias(bias1D_sampler
// CHECK: bias2D_texture_{{[0-9]+}}.SampleCmpBias(bias2D_sampler
// CHECK: biasCube_texture_{{[0-9]+}}.SampleCmpBias(biasCube_sampler
// CHECK: bias1DArray_texture_{{[0-9]+}}.SampleCmpBias(bias1DArray_sampler
// CHECK: bias2DArray_texture_{{[0-9]+}}.SampleCmpBias(bias2DArray_sampler
// CHECK: biasCubeArray_texture_{{[0-9]+}}.SampleCmpBias(biasCubeArray_sampler
// CHECK: offs1D_texture_{{[0-9]+}}.SampleCmpBias(offs1D_sampler
// CHECK: offs2D_texture_{{[0-9]+}}.SampleCmpBias(offs2D_sampler
// CHECK: offs1DArray_texture_{{[0-9]+}}.SampleCmpBias(offs1DArray_sampler
// CHECK: offs2DArray_texture_{{[0-9]+}}.SampleCmpBias(offs2DArray_sampler
// CHECK: proj1D_texture_{{[0-9]+}}.SampleCmpBias(proj1D_sampler
// CHECK: proj2D_texture_{{[0-9]+}}.SampleCmpBias(proj2D_sampler
// CHECK: projOffset1D_texture_{{[0-9]+}}.SampleCmpBias(projOffset1D_sampler
// CHECK: projOffset2D_texture_{{[0-9]+}}.SampleCmpBias(projOffset2D_sampler
[shader("fragment")]
float4 fragMain(float4 pos : SV_Position) : SV_Target
{
    float r = texture(bias1D, vec3(0.1, 0.0, 0.5), 0.25);
    r += texture(bias2D, vec3(0.1, 0.2, 0.5), 0.25);
    r += texture(biasCube, vec4(0.1, 0.2, 0.3, 0.5), 0.25);
    r += texture(bias1DArray, vec3(0.1, 0.0, 0.5), 0.25);
    r += texture(bias2DArray, vec4(0.1, 0.2, 0.0, 0.5), 0.25);
    r += texture(biasCubeArray, vec4(0.1, 0.2, 0.3, 0.0), 0.5, 0.25);
    r += textureOffset(offs1D, vec3(0.1, 0.0, 0.5), 1, 0.25);
    r += textureOffset(offs2D, vec3(0.1, 0.2, 0.5), ivec2(1, 0), 0.25);
    r += textureOffset(offs1DArray, vec3(0.1, 0.0, 0.5), 1, 0.25);
    r += textureOffset(offs2DArray, vec4(0.1, 0.2, 0.0, 0.5), ivec2(1, 0), 0.25);
    r += textureProj(proj1D, vec4(0.1, 0.0, 0.5, 1.0), 0.25);
    r += textureProj(proj2D, vec4(0.1, 0.2, 0.5, 1.0), 0.25);
    r += textureProjOffset(projOffset1D, vec4(0.1, 0.0, 0.5, 1.0), 1, 0.25);
    r += textureProjOffset(projOffset2D, vec4(0.1, 0.2, 0.5, 1.0), ivec2(1, 0), 0.25);
    return float4(r, r, r, 1.0);
}
