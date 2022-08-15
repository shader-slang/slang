//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef __DifferentialPair<float> dpfloat;
typedef float.Differential dfloat;

__differentiate_jvp float f(float x)
{
    return x;
}

dpfloat g_jvp_(dpfloat dpx)
{
    return dpfloat(dpx.p(), 2 * dpx.d());
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

__differentiate_jvp float j(float x, float y)
{
    float m = x / y;
    return m * y;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 1.0);
        dpfloat dpb = dpfloat(1.5, 1.0);

        outputBuffer[0] = __jvp(f)(dpa).d();                    // Expect: 1
        outputBuffer[1] = __jvp(f)(dpfloat(dpa.p(), 0.0)).d();  // Expect: 0
        outputBuffer[2] = __jvp(g)(dpa).d();                    // Expect: 2
        outputBuffer[3] = __jvp(h)(dpa, dpb).d();               // Expect: 8
        outputBuffer[4] = __jvp(j)(dpa, dpb).d();               // Expect: 1
    }
}
