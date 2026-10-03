// A callee without side effects that reads `groupshared` memory observes the store made
// before the call, so that store must not be eliminated as dead. The CPU targets
// materialize groupshared memory as a local of the entry point that callees reach
// through the kernel context.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

groupshared float shared[4];

[noinline]
float readShared(uint i)
{
    return shared[i];
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_GroupThreadID)
{
    uint t = tid.x;
    shared[t] = 1.0;
    float x = readShared(t);
    shared[t] = 2.0;
    // BUF: 3.0
    // BUF-NEXT: 3.0
    // BUF-NEXT: 3.0
    // BUF-NEXT: 3.0
    outputBuffer[t] = x + shared[t];
}
