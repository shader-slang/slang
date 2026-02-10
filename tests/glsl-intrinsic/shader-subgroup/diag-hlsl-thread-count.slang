// Diagnostic control test: same logic as diag-glsl-thread-count.slang but
// using HLSL [numthreads] syntax. This should always pass on Metal, proving
// the Metal backend itself works correctly.
//
// If this passes but diag-glsl-thread-count fails, the bug is in how GLSL
// layout(local_size_x) is communicated to the Metal dispatch.
//
// See https://github.com/shader-slang/slang/issues/9953

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    outputBuffer[tid] = tid + 100;

    // BUF: 100
    // BUF-NEXT: 101
    // BUF-NEXT: 102
    // BUF-NEXT: 103
}
