//TEST_IGNORE_FILE:(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST_IGNORE_FILE:(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef __DifferentialPair<float> dpfloat;
typedef __DifferentialPair<double> dpdouble;
typedef __DifferentialPair<float3> dpfloat3;

__generic<T:__BuiltinArithmeticType>
__differentiate_jvp T g(T x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 1.0);
        dpdouble dpb = dpdouble(1.5, 2.0);
        dpfloat3 dpf3 = dpfloat3(float3(1.0, 3.0, 5.0), float3(0.5, 1.5, 2.5));

        outputBuffer[0] = f(dpa.p());                               // Expect: 1
        outputBuffer[1] = __jvp(f)(dpfloat(2.0, 0.0)).d();          // Expect: 0
        outputBuffer[2] = (float)__jvp(f)(dpb).d();                 // Expect: 2
        outputBuffer[3] = __jvp(f)(dpf3).d().y;              // Expect: 1.5
    }
}
