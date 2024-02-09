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

precision highp float;
precision highp int;

//TEST_INPUT:ubuffer(data=[9 0 0 0 0], stride=4):name=inputBuffer
buffer MyBlockName
{
    uint data[];
} inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2 
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

#include "./glsl-test-utils.slang"

void computeMain()
{
    float v = 1.0f;
    outputBuffer.data[0] = testFunctionInArithmetic2Param(subgroupClusteredAdd, v, 1, 1);
    outputBuffer.data[1] = testFunctionInArithmetic2Param(subgroupClusteredMul, v, 1, 1);
    outputBuffer.data[2] = testFunctionInArithmetic2Param(subgroupClusteredMin, v, 1, 1);
    outputBuffer.data[3] = testFunctionInArithmetic2Param(subgroupClusteredMax, v, 1, 1);
    outputBuffer.data[4] = testFunctionInInt2Param(subgroupClusteredAnd, v, 1, 1);
    outputBuffer.data[5] = testFunctionInInt2Param(subgroupClusteredOr, v, 1, 1);
    outputBuffer.data[6] = testFunctionInInt2Param(subgroupClusteredXor, v, 1, 1);



    //CHECK_GLSL: void main(
    //CHECK_SPV: OpEntryPoint
    //CHECK_HLSL: void computeMain(
    //CHECK_CUDA: void computeMain(
    //CHECK_CPP: void _computeMain(
    //BUF: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
}
