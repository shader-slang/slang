//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef __DifferentialPair<float> dpfloat;
typedef __DifferentialPair<float3> dpfloat3;

__differentiate_jvp float f(float a)
{
    return a * a + a;
}

__differentiate_jvp float f(float3 a)
{
    return a.x * a.y + a.z;
}

__differentiate_jvp float g(float a)
{
    // df((2.0, 4.0, 6.0), (1.0, 2.0, 3.0)) 
    // 2.0 * 2.0 + 4.0 * 1.0 + 3.0 = 11.0
    return f(float3(a, 2*a, 3*a));
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 1.0);
        dpfloat3 dpf3 = dpfloat3(float3(1.0, 3.0, 5.0), float3(0.5, 1.5, 2.5));

        outputBuffer[0] = f(dpa.p());                               // Expect: 6
        outputBuffer[1] = f(dpf3.p());                              // Expect: 8
        outputBuffer[2] = __jvp(f)(dpf3).d();                       // Expect: 5.5
        outputBuffer[3] = __jvp(f)(dpa).d();                        // Expect: 5
        outputBuffer[4] = __jvp(g)(dpa).d();                        // Expect: 11.0
    }
}
