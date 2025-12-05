//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
// BUF: 1.000000
// BUF: 1.000000

struct TestStruct : IDifferentiable
{
    static const no_diff float foo = 1.0f;

    int x;

    void assignOut()
    {
        outputBuffer[0] = foo;
    }
}
no_diff static const float foo = 1.0f;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint thread_idx: SV_DispatchThreadID)
{
    if (thread_idx == 0) {
        TestStruct t;
        t.x = 10;
        t.assignOut();
    }
    outputBuffer[1] = foo;
}
