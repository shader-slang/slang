//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl -DTARGET_HLSL

// not testing cuda due to missing impl
//DISABLE_TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda -DTARGET_CUDA 
// not testing cpp due to missing impl
//DISABLE_TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp -DTARGET_CPP

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2 
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

shared uint shareMem;

void computeMain()
{
    // TODO: no test for image memory was done -- subgroupMemoryBarrierImage();
    // tests are seperate since concurrency testing

    shareMem = 100;
    subgroupMemoryBarrierShared();
    outputBuffer.data[0] = 1;
    subgroupBarrier();
    outputBuffer.data[0] = 2;
    subgroupBarrier();

    outputBuffer.data[1] = 1;
    subgroupMemoryBarrier();
    outputBuffer.data[1] = 2;
    subgroupBarrier();

    outputBuffer.data[2] = 1;
    subgroupMemoryBarrierBuffer();
    outputBuffer.data[2] = 2;
    subgroupBarrier();

    shareMem = 2;
    subgroupMemoryBarrierShared();
    outputBuffer.data[3] = shareMem;
    subgroupBarrier();

    if (subgroupElect()) {
        outputBuffer.data[4] = gl_GlobalInvocationID.x + 2;
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
}
