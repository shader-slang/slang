//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[-3], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float signed16RuntimeStart(float x, no_diff int start)
{
    float result = 0.0f;
    [MaxIters(8)]
    for (int16_t i = int16_t(start); i < int16_t(start + 4); ++i)
    {
        result += x * float(int(i) - start + 1);
    }
    return result;
}

[Differentiable]
float signed64RuntimeStart(float x, no_diff int start)
{
    float result = 0.0f;
    [MaxIters(8)]
    for (int64_t i = int64_t(start); i < int64_t(start + 4); ++i)
    {
        result += x * float(int(i) - start + 1);
    }
    return result;
}

[Differentiable]
float unsigned16RuntimeStart(float x, no_diff int start)
{
    float result = 0.0f;
    [MaxIters(8)]
    for (uint16_t i = uint16_t(start); i < uint16_t(start + 4); ++i)
    {
        result += x * float(int(i) - start + 1);
    }
    return result;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int start = inputBuffer[0];

    var signed16Pair = diffPair(2.0f, 0.0f);
    bwd_diff(signed16RuntimeStart)(signed16Pair, start, 1.0f);
    outputBuffer[0] = signed16Pair.d; // CHECK: 10.0

    var signed64Pair = diffPair(2.0f, 0.0f);
    bwd_diff(signed64RuntimeStart)(signed64Pair, start, 1.0f);
    outputBuffer[1] = signed64Pair.d; // CHECK: 10.0

    var unsigned16Pair = diffPair(2.0f, 0.0f);
    bwd_diff(unsigned16RuntimeStart)(unsigned16Pair, -start, 1.0f);
    outputBuffer[2] = unsigned16Pair.d; // CHECK: 10.0
}
