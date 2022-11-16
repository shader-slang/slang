//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef DifferentialPair<float2> dpfloat2;
typedef DifferentialPair<float3> dpfloat3;

[ForwardDifferentiable]
float diffDetach(float x)
{
    return detach(x);
}

[ForwardDifferentiable]
float3 diffDetach3(float3 x)
{
    return detach(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpx = dpfloat(-5.0, 1.0);
        dpfloat res = __fwd_diff(diffDetach)(dpx);
        outputBuffer[0] = res.p;               // Expect: -5.000000
        outputBuffer[1] = res.d;               // Expect: 0.000000
    }

    {
        dpfloat3 dpx = dpfloat3(float3(2.f, -3.f, -1.f), float3(-2.f, 3.f, -1.f));
        dpfloat3 res = __fwd_diff(diffDetach3)(dpx);
        outputBuffer[2] = res.p[0];            // Expect: 2.000000
        outputBuffer[3] = res.d[0];            // Expect: 0.000000
        outputBuffer[4] = res.p[1];            // Expect: -3.000000
        outputBuffer[5] = res.d[1];            // Expect: 0.000000
        outputBuffer[6] = res.p[2];           // Expect: -1.000000
        outputBuffer[7] = res.d[2];           // Expect: 0.000000
    }
}
