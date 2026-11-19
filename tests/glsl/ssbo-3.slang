//TEST:COMPARE_COMPUTE(targets=vk+glsl, filecheck-buffer=BUF):-entry computeMain -source-language glsl
#version 430
precision highp float;
precision highp int;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint foo;
    uint bar;
    uint data[];
} outputBuffer;

layout(local_size_x = 1) in;
void computeMain()
{
    outputBuffer.foo = 1;
    outputBuffer.bar = 2;
    outputBuffer.data[0] = 3;
    outputBuffer.data[1] = 4;
    // BUF: 1
    // BUF-NEXT: 2
    // BUF-NEXT: 3
    // BUF-NEXT: 4
}
