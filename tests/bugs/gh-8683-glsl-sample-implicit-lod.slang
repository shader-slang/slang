//TEST:SIMPLE_EX(filecheck=VK_VTX): -lang glsl tests/bugs/gh-8683-glsl-sample-implicit-lod.slang -target spirv -stage vertex -entry vertexMain
//TEST:SIMPLE_EX(filecheck=VK_FRAG): -lang glsl tests/bugs/gh-8683-glsl-sample-implicit-lod.slang -target spirv -stage fragment -entry fragmentMain
//TEST:SIMPLE_EX(filecheck=GLSL_VTX): -lang glsl tests/bugs/gh-8683-glsl-sample-implicit-lod.slang -target glsl -stage vertex -entry vertexMain
//TEST:SIMPLE_EX(filecheck=GLSL_FRAG): -lang glsl tests/bugs/gh-8683-glsl-sample-implicit-lod.slang -target glsl -stage fragment -entry fragmentMain
//TEST:SIMPLE_EX(filecheck=HLSL_VTX): -lang glsl tests/bugs/gh-8683-glsl-sample-implicit-lod.slang -target hlsl -stage vertex -entry vertexMain
//TEST:SIMPLE_EX(filecheck=HLSL_FRAG): -lang glsl tests/bugs/gh-8683-glsl-sample-implicit-lod.slang -target hlsl -stage fragment -entry fragmentMain

// Regression test for https://github.com/shader-slang/slang/issues/8683
//
// GLSL specifies that texture sampling with implicit LOD is always
// allowed. In fragment stage, LOD from rasterizer is used. In other
// stages, LOD=0 is used. In SPIR-V (as well as HLSL, WGSL), LOD=0
// must be explicitly used.

#version 450

layout(set=0,binding=0) uniform sampler2D uBumpTexture;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    int result;
} outputBuffer;


[shader("vertex")]
void vertexMain() {

// VK_VTX-NOT: OpImageSampleImplicitLod
// VK_VTX: OpImageSampleExplicitLod

// GLSL_VTX: texture(

// HLSL_VTX-NOT: Sample(
// HLSL_VTX: SampleLevel(

    vec3 val = texture(uBumpTexture, vec2(0,0)).rgb;

    gl_Position = vec4(val, 0);
}


[shader("fragment")]
void fragmentMain() {

// VK_FRAG-NOT: OpImageSampleExplicitLod
// VK_FRAG: OpImageSampleImplicitLod

// GLSL_FRAG: texture(

// HLSL_FRAG-NOT: SampleLevel(
// HLSL_FRAG: Sample(

    vec3 val = texture(uBumpTexture, vec2(0,0)).rgb;

    outputBuffer.result = val.x;
}
