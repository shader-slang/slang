//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[2], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float sumFromNegativeRuntimeStart(float x, no_diff int radius)
{
    float result = 0.0;
    [MaxIters(17)]
    for (int i = -radius; i <= radius; ++i)
    {
        result += x * float(i + radius + 1);
    }
    return result;
}

[Differentiable]
float sumFromRuntimeParameter(float x, no_diff int radius)
{
    float result = 0.0;
    [MaxIters(17)]
    for (int i = radius; i >= 0; --i)
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

    var negativeStartPair = diffPair(2.0f, 0.0f);
    bwd_diff(sumFromNegativeRuntimeStart)(negativeStartPair, radius, 1.0f);
    outputBuffer[0] = negativeStartPair.d; // CHECK: 15.0

    var parameterStartPair = diffPair(2.0f, 0.0f);
    bwd_diff(sumFromRuntimeParameter)(parameterStartPair, radius, 1.0f);
    outputBuffer[1] = parameterStartPair.d; // CHECK: 3.0
}
