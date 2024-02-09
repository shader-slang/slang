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

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2 
{
    uint data[];
} outputBuffer;

layout(local_size_x = 4) in;

void computeMain()
{

    if (gl_GlobalInvocationID.x == 1) {
        outputBuffer.data[0] = subgroupQuadBroadcast(1, 1) == 1 &&
                               subgroupQuadBroadcast(float(1), 1) == 1.0 &&
                               subgroupQuadBroadcast(ivec3(1), 1) == ivec3(1) &&
                               subgroupQuadBroadcast(vec2(1), 1) == vec2(1);
    }

    //not testing more types currently because that would already require a more elaborate test
    outputBuffer.data[1] = subgroupQuadSwapHorizontal(gl_GlobalInvocationID.x) != gl_GlobalInvocationID.x;
    outputBuffer.data[2] = subgroupQuadSwapVertical(2) == 2;
    outputBuffer.data[3] = subgroupQuadSwapDiagonal(3) == 3;

    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 1
}
