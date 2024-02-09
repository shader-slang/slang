//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv

// not testing hlsl due to missing impl
// TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl
// not testing cuda due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda 
// not testing cpp due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

//#define TEST_when_doubles_dont_crash

#version 430

precision highp float;
precision highp int;

//TEST_INPUT:ubuffer(data=[9 0 0 0 0], stride=4):name=inputBuffer
buffer MyBlockName
{
    uint data[];
} inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2 
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

void computeMain()
{
    outputBuffer.data[0] = subgroupShuffleUp(1, 1) == 1 &&
#ifdef TEST_when_doubles_dont_crash
                           subgroupShuffleUp((double)1.0, 1) == 1.0f && // crashes, spir-v & glsl
#endif
                           subgroupShuffleUp((float)1.0f, 1) == 1.0f &&
                           subgroupShuffleUp((uint)1, 1) == 1 &&
                           subgroupShuffleUp((int)1, 1) == 1 &&
                           subgroupShuffleUp(vec2(1), 1) == vec2(1) &&
                           subgroupShuffleUp(ivec4(1), 1) == ivec4(1) &&
                           subgroupShuffleUp((bool)1, 1) == true;

    outputBuffer.data[1] = subgroupShuffleDown(1, 1) == 1 &&
#ifdef TEST_when_doubles_dont_crash
                           subgroupShuffleDown((double)1, 1) == 1.0f && // crashes, spir-v & glsl
#endif
                           subgroupShuffleDown((float)1, 1) == 1.0f &&
                           subgroupShuffleDown((uint)1, 1) == 1 &&
                           subgroupShuffleDown((int)1, 1) == 1 &&
                           subgroupShuffleDown(vec2(1), 1) == vec2(1) &&
                           subgroupShuffleDown(ivec4(1), 1) == ivec4(1) &&
                           subgroupShuffleDown((bool)1, 1) == true;

    //CHECK_GLSL: void main(
    //CHECK_SPV: OpEntryPoint
    //CHECK_HLSL: void computeMain(
    //CHECK_CUDA: void computeMain(
    //CHECK_CPP: void _computeMain(
    //BUF: 1
    //BUF-NEXT: 1
}
