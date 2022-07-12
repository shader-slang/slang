//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

__differentiate_jvp float3 f(float3 x)
{
    return x;
}

__differentiate_jvp float3 g(float3 x, float3 y)
{
    float3 a = x + y;
    float3 b = x - y;
    return a * b + 2 * x * y;
}

__differentiate_jvp float2 h(float2 x, float2 y)
{
    float2 a = x + y;
    float2 b = x - y;
    return a * b + 2 * x * y;
}

__differentiate_jvp float4 j(float4 x, float4 y)
{
    float4 a = x + y;
    float4 b = x - y;
    return a * b + 2 * x * y;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
     {
        float3 a = float3(2.0, 2.0, 2.0);
        float3 b = float3(1.5, 1.5, 1.5);
        float3 da = float3(1.0, 1.0, 1.0);

        float2 a2 = float2(2.0, 1.0);
        float2 b2 = float2(1.5, -2.0);

        float4 a4 = float4(2.0, 1.0, 0.0, 2.0);
        float4 b4 = float4(1.5, -2.0, 1.0, 1.5);

        outputBuffer[0] = __jvp(f)(a, da).z;                                            // Expect: 1
        outputBuffer[1] = __jvp(g)(a, b, da, float3(2.0, 1.0, 0.0)).y;                  // Expect: 8
        outputBuffer[2] = __jvp(h)(a2, b2, float2(1.0, 0.0), float2(1.0, 1.0)).x;       // Expect: 8
        outputBuffer[3] = __jvp(j)(a4, b4, float4(1.0), float4(2.0)).w;                 // Expect: 9
    }
}
