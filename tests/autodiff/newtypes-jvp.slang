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

dfloat g_jvp_(dpfloat dpx)
{
    return 2 * dpx.d(); // Or dpx.getDifferential();
}

[__custom_jvp(g_jvp_)]
float g(float x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 1.0);
        dpfloat dpb = dpfloat(1.5, 0.0);

        outputBuffer[0] = __jvp(f)(dpa); // Expect: 1
        outputBuffer[1] = __jvp(f)(dpfloat(2.0, 0.0));     // Expect: 0
        outputBuffer[2] = __jvp(g)(dpa);        // Expect: 2
        //outputBuffer[3] = __jvp(h)(dpa, dpb); // Expect: 8
        //outputBuffer[4] = __jvp(j)(dpa, dpb); // Expect: 1
    }
}
