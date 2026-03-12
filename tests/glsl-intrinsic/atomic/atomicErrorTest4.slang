//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):  -allow-glsl -stage compute -entry computeMain -target glsl -DTARGET_GLSL
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly -DTARGET_SPIRV
#version 430

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):name=one
layout(binding = 1, offset) uniform atomic_uint one;
//CHECK:            ^^^^^^ unrecognized GLSL layout qualifier
//CHECK:            ^^^^^^ GLSL layout qualifier is unrecognized or requires assignment

void computeMain()
{

}
