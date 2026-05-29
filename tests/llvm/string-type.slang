// This test just needs to run without crashing.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -llvm -compute -Xslang -O0 -Xslang -g3
//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

public void crasher<each T>(expand each T info)
{
}

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = 0;
    // This gets passed as a `String`, which will get implicitly stack allocated
    // with `-O0 -g3` so that the debugger can see the parameter values of
    // `crasher`. If the size of `String` is not calculated correctly, this will
    // mess up the stack!
    crasher("hello, world");
    // CHECK: 8
    outputBuffer[0] = sizeof(String);
}
