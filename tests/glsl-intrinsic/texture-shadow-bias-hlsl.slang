//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/texture-shadow-bias-hlsl.slang -target hlsl -profile sm_6_8 -entry fragMain -stage fragment

// On HLSL, the GLSL-source shadow texture(..., bias) and textureOffset(..., bias) builtins must
// lower to SampleCmpBias. Falling back to SampleCmp would still compile but silently drop the
// bias, so every call samples its own uniform and is checked by that uniform's name.

uniform sampler1DShadow plain1D;
uniform sampler2DShadow plain2D;
uniform samplerCubeShadow plainCube;
uniform sampler1DArrayShadow layered1D;
uniform sampler2DArrayShadow layered2D;
uniform samplerCubeArrayShadow layeredCube;
uniform sampler1DShadow offset1D;
uniform sampler2DShadow offset2D;
uniform sampler1DArrayShadow offsetLayered1D;
uniform sampler2DArrayShadow offsetLayered2D;

// CHECK: plain1D_texture{{.*}}.SampleCmpBias(
// CHECK: plain2D_texture{{.*}}.SampleCmpBias(
// CHECK: plainCube_texture{{.*}}.SampleCmpBias(
// CHECK: layered1D_texture{{.*}}.SampleCmpBias(
// CHECK: layered2D_texture{{.*}}.SampleCmpBias(
// CHECK: layeredCube_texture{{.*}}.SampleCmpBias(
// CHECK: offset1D_texture{{.*}}.SampleCmpBias(
// CHECK: offset2D_texture{{.*}}.SampleCmpBias(
// CHECK: offsetLayered1D_texture{{.*}}.SampleCmpBias(
// CHECK: offsetLayered2D_texture{{.*}}.SampleCmpBias(
[shader("fragment")]
float4 fragMain(float4 pos : SV_Position) : SV_Target
{
    float r = texture(plain1D, vec3(0.1, 0.0, 0.5), 0.25);
    r += texture(plain2D, vec3(0.1, 0.2, 0.5), 0.25);
    r += texture(plainCube, vec4(0.1, 0.2, 0.3, 0.5), 0.25);
    r += texture(layered1D, vec3(0.1, 0.0, 0.5), 0.25);
    r += texture(layered2D, vec4(0.1, 0.2, 0.0, 0.5), 0.25);
    r += texture(layeredCube, vec4(0.1, 0.2, 0.3, 0.0), 0.5, 0.25);
    r += textureOffset(offset1D, vec3(0.1, 0.0, 0.5), 1, 0.25);
    r += textureOffset(offset2D, vec3(0.1, 0.2, 0.5), ivec2(1, 0), 0.25);
    r += textureOffset(offsetLayered1D, vec3(0.1, 0.0, 0.5), 1, 0.25);
    r += textureOffset(offsetLayered2D, vec4(0.1, 0.2, 0.0, 0.5), ivec2(1, 0), 0.25);
    return float4(r, r, r, 1.0);
}
