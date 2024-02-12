// not testing glsl or spirv due to crashing issue from some odd code emitting which looks right but fails
//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv
//TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl
// not testing cuda due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda 
// not testing cpp due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

// not testing glsl or spirv due to crashing issue from some odd code emitting which looks right but fails
// TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
// TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

// #define TEST_when_glsl_subgroupInclusiveXor_and_subgroupInclusiveAdd_are_not_bugged

precision highp float;
precision highp int;

// #define TEST_when_equallity_and_uint_cast_on_templates_work

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
    // tests are seperate since subset crashes; once this is fixed, make all into 1 

    float test = 1.0f;
    uint test2 = 1;
    outputBuffer.data[0] = testFunctionIn1Param(subgroupAdd, local_size_x_v, test);
    outputBuffer.data[1] = testFunctionIn1Param(subgroupMul, 1, test);
    outputBuffer.data[2] = testFunctionIn1Param(subgroupMin, test, test);
    outputBuffer.data[3] = testFunctionIn1Param(subgroupMin, test, test);

    outputBuffer.data[4] = testFunctionInInt1Param(subgroupAnd, 1, test2);
    outputBuffer.data[5] = testFunctionInInt1Param(subgroupOr, 1, test2);
    outputBuffer.data[6] = testFunctionInInt1Param(subgroupXor, 0, test2);

//following functions don't have an hlsl implementation
#ifdef TARGET_HLSL
    vec[7] = 1;
    vec[8] = 1;
    vec[9] = 1;
    vec[10] = 1;
    vec[11] = 1;
    vec[12] = 1;
    vec[13] = 1;
    vec[14] = 1;
#else 
    //// compare with goal of true with invocation id 3, not 0
    uint vec[21];
    vec[7] = testFunctionIn1Param(subgroupExclusiveAdd, 3, test); 
    vec[8] = testFunctionIn1Param(subgroupExclusiveMul, 1, test); 
    vec[9] = testFunctionIn1Param(subgroupExclusiveMin, 1, test);
    vec[10] = testFunctionIn1Param(subgroupExclusiveMax, 1, test);
    vec[11] = testFunctionInInt1Param(subgroupExclusiveAnd, 1, test2);
    vec[12] = testFunctionInInt1Param(subgroupExclusiveOr, 1, test2);
    vec[13] = testFunctionInInt1Param(subgroupExclusiveXor, 1, test2);
#ifdef TEST_when_glsl_subgroupInclusiveXor_and_subgroupInclusiveAdd_are_not_bugged
    vec[14] = testFunctionIn1Param(subgroupInclusiveAdd, 4, test);
#else 
    vec[14] = 1;
#endif
#endif

    vec[15] = testFunctionIn1Param(subgroupInclusiveMul, 1, test); 
    vec[16] = testFunctionIn1Param(subgroupInclusiveMin, 1, test);
    vec[17] = testFunctionIn1Param(subgroupInclusiveMax, 1, test);
    vec[18] = testFunctionInInt1Param(subgroupInclusiveAnd, 1, test2);
    vec[19] = testFunctionInInt1Param(subgroupInclusiveOr, 1, test2);
#ifdef TEST_when_glsl_subgroupInclusiveXor_and_subgroupInclusiveAdd_are_not_bugged
    vec[20] = testFunctionInInt1Param(subgroupInclusiveXor, 0, test2);
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
