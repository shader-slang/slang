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
#version 430

//#define TEST_when_doubles_dont_crash

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

    outputBuffer.data[0] = subgroupShuffle(1, 1) == 1 &&
#ifdef TEST_when_doubles_dont_crash
                           subgroupShuffle((double)1, 1) == 1.0 && // crashes, spir-v & glsl
#endif
                           subgroupShuffle((float)1, 1) == 1.0f &&
                           subgroupShuffle((int)1, 1) == 1 &&
                           subgroupShuffle((uint)1, 1) == 1 &&
                           subgroupShuffle(vec2(1), 1) == vec2(1) &&
                           subgroupShuffle(ivec4(1), 1) == ivec4(1) &&
                           subgroupShuffle((bool)1, 1) == true;

    outputBuffer.data[1] = subgroupShuffleXor(1, 1) == 1 &&
#ifdef TEST_when_doubles_dont_crash
                           subgroupShuffleXor((double)1, 1) == 1.0 && // crashes, spir-v & glsl
#endif
                           subgroupShuffleXor((float)1, 1) == 1.0f &&
                           subgroupShuffleXor((int)1, 1) == 1 &&
                           subgroupShuffleXor((uint)1, 1) == 1 &&
                           subgroupShuffleXor(vec2(1), 1) == vec2(1) &&
                           subgroupShuffleXor(ivec4(1), 1) == ivec4(1) &&
                           subgroupShuffleXor((bool)1, 1) == true;


    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1
    // BUF-NEXT: 1
}
