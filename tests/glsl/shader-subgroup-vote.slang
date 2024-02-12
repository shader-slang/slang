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

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2 
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

#include "./glsl-test-utils.slang"

void computeMain()
{
    //seperate tests since testing concurrency

    // one is true, rest false, positive
    outputBuffer.data[0] = 1;
    bool t1 = inputBuffer.data[0] == gl_GlobalInvocationID.x;
    if (subgroupAny(t1)) {
        subgroupBarrier();
        outputBuffer.data[0] = 2;
    }

    // all false, negative
    outputBuffer.data[1] = 1;
    t1 = false;
    if (!subgroupAny(t1)) {
        subgroupBarrier();
        outputBuffer.data[1] = 2;
    }

    // all true, positive
    outputBuffer.data[2] = 1;
    t1 = true;
    if (subgroupAll(t1)) {
        subgroupBarrier();
        outputBuffer.data[2] = 2;
    }

    // all false, negative
    outputBuffer.data[3] = 1;
    t1 = false;
    if (!subgroupAll(t1)) {
        subgroupBarrier();
        outputBuffer.data[3] = 2;
    }

    // all false, positive
    outputBuffer.data[4] = 1;

    if (testFunctionIn1ParamBool(subgroupAllEqual, true, 0)) {
        subgroupBarrier();
        outputBuffer.data[4] = 2;
    }

    // one is true, rest false, negative
    outputBuffer.data[5] = 1;
    t1 = inputBuffer.data[0] == gl_GlobalInvocationID.x;
    if (testFunctionIn1ParamBool(subgroupAllEqual, false, t1)) {
        subgroupBarrier();
        outputBuffer.data[5] = 2;
    }

    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 2
    // BUF-NEXT: 2
    // BUF-NEXT: 2
    // BUF-NEXT: 2
    // BUF-NEXT: 2
    // BUF-NEXT: 2
}
