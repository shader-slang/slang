// Test that GLSL layout(local_size_x) is correctly reported by the reflection
// API, ensuring the Metal backend dispatches the right number of threads.
// Regression test for https://github.com/shader-slang/slang/issues/9953

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -allow-glsl

#version 430

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2 { uint data[]; } outputBuffer;

layout(local_size_x = 4) in;

void computeMain()
{
    uint tid = gl_LocalInvocationIndex;
    outputBuffer.data[tid] = tid + 1;

    // BUF: 1
    // BUF-NEXT: 2
    // BUF-NEXT: 3
    // BUF-NEXT: 4
}
