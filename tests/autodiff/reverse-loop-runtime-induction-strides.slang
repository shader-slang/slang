//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[3], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float positiveRuntimeStride(float x, no_diff int radius)
{
    float result = 0.0;
    [MaxIters(17)]
    for (int i = -radius; i <= radius; i += 2)
    {
        result += x * float(i + radius + 1);
    }
    return result;
}

[Differentiable]
float negativeRuntimeStride(float x, no_diff int radius)
{
    float result = 0.0;
    [MaxIters(17)]
    for (int i = radius * 2 + 1; i >= radius; i -= 2)
    {
        result += x * float(i);
    }
    return result;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int radius = inputBuffer[0];

    var positivePair = diffPair(2.0f, 0.0f);
    bwd_diff(positiveRuntimeStride)(positivePair, radius, 1.0f);
    outputBuffer[0] = positivePair.d; // CHECK: 16.0

    var negativePair = diffPair(2.0f, 0.0f);
    bwd_diff(negativeRuntimeStride)(negativePair, radius, 1.0f);
    outputBuffer[1] = negativePair.d; // CHECK: 15.0
}
