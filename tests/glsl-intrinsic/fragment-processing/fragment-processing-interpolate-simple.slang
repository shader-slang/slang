//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage fragment -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv -emit-spirv-directly -stage fragment -entry main -allow-glsl
#version 450
// CHECK_SPV-DAG: OpEntryPoint
// CHECK_GLSL-DAG: void main(

buffer MyBlockName
{
    vec4 fdata[2];
    ivec4 udata[2];
    uvec4 idata[2];
} keepAliveBuffer;

layout(location = 0) out ivec4 outColorActual;

layout (location = 0) in float inDataV1;
layout (location = 1) in vec2 inDataV2;
layout (location = 2) in vec3 inDataV3;
layout (location = 3) in vec4 inDataV4;
bool testFragmentProcessingInterpolateFunctions()
{
    return true
        && interpolateAtCentroid(inDataV1) != -1.0f
        && interpolateAtSample(inDataV1, 0) != -1.0f
        && interpolateAtOffset(inDataV1, vec2(0.0f)) != -1.0f
        ;
}

void main() {
    
    outColorActual = ivec4(true
        && testFragmentProcessingInterpolateFunctions()
        );
}