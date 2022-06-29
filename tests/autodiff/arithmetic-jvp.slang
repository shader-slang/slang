//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

__differentiate_jvp float f(float x)
{
    return x;
}

float g_jvp_(float x, float dx)
{
    return 2 * dx;
}

[__custom_jvp(g_jvp_)]
float g(float x)
{
    return x + x;
}

__differentiate_jvp float h(float x, float y)
{
    float m = x + y;
    float n = x - y;
    return m * n + 2 * x * y;
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
     {
        float a = 2.0;
        float b = 1.5;
        float da = 1.0;
        float db = 1.0;

        outputBuffer[0] = __jvp(f)(a, da);        // Expect: 1
        outputBuffer[1] = __jvp(f)(a, 0.0);       // Expect: 0
        outputBuffer[2] = __jvp(g)(a, da);        // Expect: 2
        outputBuffer[3] = __jvp(h)(a, b, da, db); // Expect: 8
    }
}
