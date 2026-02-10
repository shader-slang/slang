// Diagnostic test: verify that GLSL layout(local_size_x) actually dispatches
// the correct number of threads on Metal.
//
// Each thread writes its own ID + 100 to its buffer slot. If only 1 thread
// runs (dispatch bug), only slot 0 will be written and slots 1-3 will be 0.
//
// See https://github.com/shader-slang/slang/issues/9953

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -allow-glsl

#version 430

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 4) in;

void computeMain()
{
    uint tid = gl_LocalInvocationIndex;
    outputBuffer.data[tid] = tid + 100;

    // BUF: 64
    // BUF-NEXT: 65
    // BUF-NEXT: 66
    // BUF-NEXT: 67
}
