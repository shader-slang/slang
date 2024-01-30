//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
#version 430
precision highp float;
precision highp int;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint foo;
    uint data[];
} outputBuffer;

layout(local_size_x = 1) in;
void computeMain()
{
    outputBuffer.foo = 1;
    outputBuffer.data[0] = 2;
    outputBuffer.data[1] = 3;
    outputBuffer.data[2] = 4;
    // BUF: 1
    // BUF: 2
    // BUF: 3
    // BUF: 4
}
