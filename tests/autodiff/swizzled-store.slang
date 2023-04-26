//TEST_IGNORE_FILE:
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[BackwardDifferentiable]
float3 f(float3 x)
{
    float3 y;
    y.x = x.x * x.x;
    y.y = x.y * x.y;
    y.z = x.z * x.z;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        DifferentialPair<float3> dpa = diffPair(float3(2.0, 0.0, 5.0), float3(1.0, 0.5, 2.0));
        DifferentialPair<float3> dpb = diffPair(float3(2.0, 1.0, 1.0), float3(1.0, 1.5, 2.0));

        outputBuffer[0] = __fwd_diff(f)(dpa).d.y;      // Expect: 1
        outputBuffer[1] = __fwd_diff(f)(dpb).d.z;     // Expect: 0
    }
}
