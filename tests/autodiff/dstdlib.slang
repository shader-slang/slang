//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;

[ForwardDifferentiable]
float f(float x)
{
    return dstd.exp(x);
}

[ForwardDifferentiable]
float g(float x)
{
    return dstd.sin(x);
}

[ForwardDifferentiable]
float h(float x)
{
    return dstd.cos(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 1.0);

        outputBuffer[0] = f(dpa.p());        // Expect: 7.389056
        outputBuffer[1] = __fwd_diff(f)(dpa).d(); // Expect: 7.389056
        outputBuffer[2] = g(dpa.p());        // Expect: 0.909297 
        outputBuffer[3] = __fwd_diff(g)(dpa).d(); // Expect: -0.416146
        outputBuffer[4] = h(dpa.p());        // Expect: -0.416146
        outputBuffer[5] = __fwd_diff(h)(dpa).d(); // Expect: -0.909297 
    }
}