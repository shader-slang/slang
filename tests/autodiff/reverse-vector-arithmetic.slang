//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float3> dpfloat3;
typedef float3.Differential dfloat3;

typedef DifferentialPair<float2> dpfloat2;
typedef float2.Differential dfloat2;

[BackwardDifferentiable]
float3 test_simple(float3 x, float3 y)
{
    return x + y;
}

[BackwardDifferentiable]
float test_swizzles(float3 x, float3 y)
{
    return x.y + y.x;
}

[BackwardDifferentiable]
float test_constructor(float3 x, float3 y)
{
    return float3(x.y + y.x, y.z, x.z);
}

[BackwardDifferentiable]
float3 test_complex_arith(float3 x, float2 y)
{
    float2 t = float2(x.x, y.x) + 2.0 * float2(x.z, y.y);
    return float3(
        2.0f * x.z + y.y * 4.0f - t.y, 
        (3.0f * t + y).z,
        t.x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    dpfloat dpx = dpfloat3(float3(2.0, 3.0, 4.0), float3(0.0, 0.0, 0.0));
    dpfloat dpy = dpfloat2(float2(1.5, 0.0), float2(0.0, 0.0));

    __bwd_diff(test_simple)(dpa, dpb, dfloat3(1.0, 2.0, 3.0));
    outputBuffer[0] = dpa.d; // Expect: 1
    outputBuffer[1] = dpb.d; // Expect: 4
}
