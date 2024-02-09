// not testing glsl or spirv due to crashing issue from some odd code emitting which looks right but fails
//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv

// not testing hlsl due to missing impl
// TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl
// not testing cuda due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda 
// not testing cpp due to missing impl
// TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

// not testing glsl or spirv due to crashing issue from some odd code emitting which looks right but fails
// TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
// TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

precision highp float;
precision highp int;

// #define TEST_when_equallity_and_uint_cast_on_templates_work

//TEST_INPUT:ubuffer(data=[9 0 0 0 0], stride=4):name=inputBuffer
buffer MyBlockName
{
    uint data[];
} inputBuffer;

// TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

void computeMain()
{
    
    // note: all implementations cause a silent shader crash 
    // note: since shader crashes tests are not checking many types... since we can't verify they even run 

    float test = 1.0f;
    int test2 = 1;
    outputBuffer.data[0] = uint(subgroupAdd(test)) == 32;
    outputBuffer.data[1] = subgroupMul(test) == 1;
    outputBuffer.data[2] = subgroupMin(test) == test;
    outputBuffer.data[3] = subgroupMax(test) == test;

    outputBuffer.data[4] = subgroupAnd(test2) == 0;
    outputBuffer.data[5] = subgroupOr(test2) == 63;
    outputBuffer.data[6] = subgroupXor(test2) == 0;

    // compare with goal of true with invocation id 2, not 0
    outputBuffer.data[7] = subgroupExclusiveAdd(test) == 1;
    outputBuffer.data[8] = subgroupExclusiveMul(test) == 1;
    outputBuffer.data[9] = subgroupExclusiveMin(test) == 1;
    outputBuffer.data[10] = subgroupExclusiveMax(test) == 1;
    outputBuffer.data[11] = subgroupExclusiveAnd(test2) == 0;
    outputBuffer.data[12] = subgroupExclusiveOr(test2) == 3;
    outputBuffer.data[13] = subgroupExclusiveXor(test2) == 0;

    outputBuffer.data[14] = subgroupInclusiveAdd(test) == 1;
    outputBuffer.data[15] = subgroupInclusiveMul(test) == 1;
    outputBuffer.data[16] = subgroupInclusiveMin(test) == 1;
    outputBuffer.data[17] = subgroupInclusiveMax(test) == 1;

    outputBuffer.data[18] = subgroupInclusiveAnd(test2) == 1;
    outputBuffer.data[19] = subgroupInclusiveOr(test2) == 1;
    outputBuffer.data[20] = subgroupInclusiveXor(test2) == 1;


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
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1
    //BUF-NEXT: 1

}
