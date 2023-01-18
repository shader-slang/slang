//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef DifferentialPair<float2> dpfloat2;
typedef DifferentialPair<float3> dpfloat3;

[BackwardDifferentiable]
float diffExp(float x)
{
    return exp(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpx = dpfloat(2.0, 1.0);
        __bwd_diff(diffExp)(dpx, 1.0);
        outputBuffer[0] = dpx.p; // Expect: 2
        outputBuffer[1] = dpx.d; // Expect: 7.389056
    }

    {
        dpfloat dpx = dpfloat(0.0, 1.0);
        __bwd_diff(diffExp)(dpx, 1.0);
        outputBuffer[2] = dpx.p; // Expect: 0.000000
        outputBuffer[3] = dpx.d; // Expect: 1.000000
    }
}
