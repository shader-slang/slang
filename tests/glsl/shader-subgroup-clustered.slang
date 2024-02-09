//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv
//TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl
//TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda 
//TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp

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

void computeMain()
{
    // TODO: make more robust of a test than simple call verification

    outputBuffer.data[0] = subgroupClusteredAdd(int(1), 1) == 1 &&
                subgroupClusteredAdd(uint(1), 1) == 1;
    outputBuffer.data[1] = subgroupClusteredMul(float(1), 1) == 1.0 &&
                subgroupClusteredMul(double(1), 1) == 1.0;
    outputBuffer.data[2] = subgroupClusteredMin(int(1), 1) == 1 &&
                subgroupClusteredMin(uint(1), 1) == 1;
    outputBuffer.data[3] = subgroupClusteredMax(int(1), 1) == 1 &&
                subgroupClusteredMax(uint(1), 1) == 1;
    outputBuffer.data[4] = subgroupClusteredAnd(int(1), 1) == 1 &&
                subgroupClusteredAnd(uint(1), 1) == 1;
    outputBuffer.data[5] = subgroupClusteredOr(int(1), 1) == 1 &&
                subgroupClusteredOr(uint(1), 1) == 1;
    outputBuffer.data[6] = subgroupClusteredXor(int(1), 1) == 1 &&
                subgroupClusteredXor(uint(1), 1) == 1;



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
}
