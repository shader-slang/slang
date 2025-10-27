//TEST:SIMPLE(filecheck=VK-CHECK-VTX): -target spirv -stage vertex -entry vertexMain -allow-glsl
//TEST:SIMPLE(filecheck=VK-CHECK-FRAG): -target spirv -stage fragment -entry fragmentMain -allow-glsl
//TEST:SIMPLE(filecheck=GLSL-CHECK-VTX): -target glsl -stage vertex -entry vertexMain -allow-glsl
//TEST:SIMPLE(filecheck=GLSL-CHECK-FRAG): -target glsl -stage fragment -entry fragmentMain -allow-glsl
//TEST:SIMPLE(filecheck=HLSL-CHECK-VTX): -target hlsl -stage vertex -entry vertexMain -allow-glsl
//TEST:SIMPLE(filecheck=HLSL-CHECK-FRAG): -target hlsl -stage fragment -entry fragmentMain -allow-glsl

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

// VK-CHECK-VTX: OpImageSampleExplicitLod
// VK-CHECK-VTX-NOT: OpImageSampleImplicitLod

// GLSL-CHECK-VTX: texture(

// HLSL-CHECK-VTX: SampleLevel(
// HLSL-CHECK-VTX-NOT: Sample(

    vec3 val = texture(uBumpTexture, vec2(0,0)).rgb;

    gl_Position = vec4(val, 0);
}


[shader("fragment")]
void fragmentMain() {

// VK-CHECK-FRAG: OpImageSampleImplicitLod
// VK-CHECK-FRAG-NOT: OpImageSampleExplicitLod

// GLSL-CHECK-FRAG: texture(

// HLSL-CHECK-FRAG-NOT: SampleLevel(
// HLSL-CHECK-FRAG: Sample(

    vec3 val = texture(uBumpTexture, vec2(0,0)).rgb;

    outputBuffer.result = val.x;
}
