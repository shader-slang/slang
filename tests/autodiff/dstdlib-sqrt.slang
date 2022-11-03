//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef DifferentialPair<float2> dpfloat2;
typedef DifferentialPair<float3> dpfloat3;

[ForwardDifferentiable]
float diffSqrt(float x)
{
    return dstd.sqrt(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpx = dpfloat(2.0, 1.0);
        dpfloat res = __fwd_diff(diffSqrt)(dpx);
        outputBuffer[0] = res.p();               // Expect: 1.414214
        outputBuffer[1] = res.d();               // Expect: 0.353553
    }

    {
        dpfloat dpx = dpfloat(100.0, -3.0);
        dpfloat res = __fwd_diff(diffSqrt)(dpx);
        outputBuffer[2] = res.p();               // Expect: 10.000000
        outputBuffer[3] = res.d();               // Expect: -0.150000
    }
}
