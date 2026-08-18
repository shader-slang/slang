// Regression guard for shader-slang/slang#12605: DCE's liveness bookkeeping must preserve
// loop-carried values that reach the output. Compiling this shader invokes DCE repeatedly, so a
// marking bug that dropped the still-live accumulator would change the result or fail compilation.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

float compute(int n)
{
    float live = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        live += float(i);
    }
    return live;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = compute(5);
    // CHECK: 10.0
}
