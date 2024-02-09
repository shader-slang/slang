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

// #define TEST_when_glsl_subgroupInclusiveXor_and_subgroupInclusiveAdd_are_not_bugged

precision highp float;
precision highp int;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

#define local_size_x_v 32
layout(local_size_x = local_size_x_v) in;

#include "./glsl-test-utils.slang"

void computeMain()
{
    
    float test = 1.0f;
    uint test2 = 1;
    outputBuffer.data[0] = testFunctionInArithmetic1Param(subgroupAdd, test, local_size_x_v);
    outputBuffer.data[1] = testFunctionInArithmetic1Param(subgroupMul, test, 1);
    outputBuffer.data[2] = testFunctionInArithmetic1Param(subgroupMin, test, test);
    outputBuffer.data[3] = testFunctionInArithmetic1Param(subgroupMin, test, test);

    outputBuffer.data[4] = testFunctionInInt1Param(subgroupAnd, test2, 1);
    outputBuffer.data[5] = testFunctionInInt1Param(subgroupOr, test2, 1);
    outputBuffer.data[6] = testFunctionInInt1Param(subgroupXor, test2, 0);

    //// compare with goal of true with invocation id 3, not 0
    uint vec[21];
    vec[7] = testFunctionInArithmetic1Param(subgroupExclusiveAdd, test, 3); 
    vec[8] = testFunctionInArithmetic1Param(subgroupExclusiveMul, test, 1); 
    vec[9] = testFunctionInArithmetic1Param(subgroupExclusiveMin, test, 1);
    vec[10] = testFunctionInArithmetic1Param(subgroupExclusiveMax, test, 1);
    vec[11] = testFunctionInInt1Param(subgroupExclusiveAnd, test2, 1);
    vec[12] = testFunctionInInt1Param(subgroupExclusiveOr, test2, 1);
    vec[13] = testFunctionInInt1Param(subgroupExclusiveXor, test2, 1);
#ifdef TEST_when_glsl_subgroupInclusiveXor_and_subgroupInclusiveAdd_are_not_bugged
    vec[14] = testFunctionInArithmetic1Param(subgroupInclusiveAdd, test, 4);
#else 
    vec[14] = 1;
#endif
    vec[15] = testFunctionInArithmetic1Param(subgroupInclusiveMul, test, 1); 
    vec[16] = testFunctionInArithmetic1Param(subgroupInclusiveMin, test, 1);
    vec[17] = testFunctionInArithmetic1Param(subgroupInclusiveMax, test, 1);
    vec[18] = testFunctionInInt1Param(subgroupInclusiveAnd, test2, 1);
    vec[19] = testFunctionInInt1Param(subgroupInclusiveOr, test2, 1);
#ifdef TEST_when_glsl_subgroupInclusiveXor_and_subgroupInclusiveAdd_are_not_bugged
    vec[20] = testFunctionInInt1Param(subgroupInclusiveXor, test2, 0);
#else
    vec[20] = 1;
#endif

    if (gl_LocalInvocationID.x == 3) {
        outputBuffer.data[7] = vec[7];
        outputBuffer.data[8] = vec[8];
        outputBuffer.data[9] = vec[9];
        outputBuffer.data[10] = vec[10];

        outputBuffer.data[11] = vec[11];
        outputBuffer.data[12] = vec[12];
        outputBuffer.data[13] = vec[13];

        outputBuffer.data[14] = vec[14];
        outputBuffer.data[15] = vec[15];
        outputBuffer.data[16] = vec[16];
        outputBuffer.data[17] = vec[17];

        outputBuffer.data[18] = vec[18];
        outputBuffer.data[19] = vec[19];
        outputBuffer.data[20] = vec[20];
    }

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
    // BUF-NEXT: 1
    // BUF-NEXT: 1
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
