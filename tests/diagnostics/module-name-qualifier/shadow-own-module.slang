// A module whose declared name is the same as a core-module function can call
// that function, as the same file without its `module` line can
// (shader-slang/slang#14063).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

module saturate;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 1.0
    outputBuffer[0] = saturate(2.5f);
    // CHECK: 0.0
    outputBuffer[1] = saturate(-2.5f);
}
