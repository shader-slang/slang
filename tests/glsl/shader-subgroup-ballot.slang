//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv
//TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl
// not testing cuda due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda 
// not testing cpp due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

precision highp float;
precision highp int;

//TEST_INPUT:ubuffer(data=[9 0 0 0 0], stride=4):name=inputBuffer
buffer MyBlockName
{
    uint data[];
} inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2 
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

#include "./glsl-test-utils.slang"

void computeMain()
{
    uvec4 in = (0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    outputBuffer.data[0] = testFunctionIn2Param(subgroupBroadcast, 1, 0, 1);
    outputBuffer.data[1] = testFunctionIn1Param(subgroupBroadcastFirst, 1, 1);
    outputBuffer.data[2] = subgroupBallot(true).x == 0xFFFFFFFF; // all 32 should be running
    outputBuffer.data[3] = subgroupInverseBallot(in) == true;
    outputBuffer.data[4] = subgroupBallotBitExtract(in, 0) == true;
    outputBuffer.data[5] = subgroupBallotBitCount(in) == 32;
    outputBuffer.data[6] = subgroupBallotInclusiveBitCount(in) != 0;
    outputBuffer.data[7] = subgroupBallotExclusiveBitCount(in) != 0;
    outputBuffer.data[8] = subgroupBallotFindLSB(in) == 0;
    
    // only 32 in warp, so read up to first 32, hence 31
    outputBuffer.data[9] = subgroupBallotFindMSB(in) == 31; 

    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
}
