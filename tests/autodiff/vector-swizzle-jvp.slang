//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

__differentiate_jvp float2 f(float3 x)
{
    return x.zy;
}

__differentiate_jvp float2 g(float3 x, float4 y)
{
    //float3 a = x + y.zyx;
    //float2 b = x.zx - y.yw;
    //return a.yz * b + 2 * x.xy * y.yz;
    return x.yz;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        float3 a = float3(2.0, 2.0, 2.0);
        float3 da = float3(1.0, 0.5, 1.0);

        outputBuffer[0] = __jvp(f)(a, da).x;  // Expect: 1
        outputBuffer[1] = __jvp(f)(a, da).y;  // Expect: 0.5
        
        float3 x = float3(0.5, 2.0, 0.5);
        float4 y = float4(-1.5, 1.0, 4.0, 2.0);
        float3 dx = float3(1.0, 0.0, -1.0);
        float4 dy = float4(0.0, 0.5, -0.25, 1.0);
        
        outputBuffer[2] = __jvp(g)(y, dy).x;
        //outputBuffer[2] = __jvp(g)(x, y, dx, dy).x;  // Expect: 1
        //outputBuffer[3] = (g(x + dx * 1e-3, y + dy * 1e-3).x - g(x - dx * 1e-3, y - dy * 1e-3).x) / (2e-3);
        //outputBuffer[3] = __jvp(g)(x, y, dx, dy).y;  // Expect: 0.5
    }
}
