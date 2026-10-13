//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[2], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float nestedRuntimeStarts(float x, no_diff int radius)
{
    float result = 0.0;
    [MaxIters(17)]
    for (int i = -radius; i <= radius; ++i)
    {
        [MaxIters(17)]
        for (int j = i; j <= radius; ++j)
        {
            result += x * float(j - i + 1);
        }
    }
    return result;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int radius = inputBuffer[0];
    var pair = diffPair(2.0f, 0.0f);
    bwd_diff(nestedRuntimeStarts)(pair, radius, 1.0f);
    outputBuffer[0] = pair.d; // CHECK: 35.0
}
