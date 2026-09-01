//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float weightedIdentity(float3 x)
{
    float3 numerator = 0.0;
    float denominator = 0.0;
    [MaxIters(1)]
    for (int i = 0; i < 1; ++i)
    {
        float weight = x.x;
        numerator += x * weight;
        denominator += weight;
    }
    float3 result = numerator / denominator;
    return result.x + result.y + result.z;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    var pair = diffPair(float3(0.3, 0.7, -0.2));
    bwd_diff(weightedIdentity)(pair, 1.0);
    outputBuffer[0] = pair.d.x;
    outputBuffer[1] = pair.d.y;
    outputBuffer[2] = pair.d.z;

    // weightedIdentity(x) is x.x + x.y + x.z, so its gradient is (1, 1, 1).
    // CHECK: 1.0
    // CHECK: 1.0
    // CHECK: 1.0
}
