//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef float.Differential dfloat;

[BackwardDifferentiable]
float g(float y)
{
    return 4.0f * y;
}

[BackwardDifferentiable]
float f(float x)
{
    return 3.0f * g(2.0f * x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    dpfloat dpa = dpfloat(1.0, 0.0);

    __bwd_diff(f)(dpa, 1.0f);
    outputBuffer[0] = dpa.d; // Expect: 24.0
}
