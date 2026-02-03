// IGNORE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
// IGNORE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
// IGNORE_TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float3> dpfloat3;
typedef float3.Differential dfloat3;

typedef DifferentialPair<float2> dpfloat2;
typedef float2.Differential dfloat2;

[BackwardDifferentiable]
T simple<T : IFloat>(T x, T y)
{
    return x + y;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        dpfloat3 dpx = dpfloat3(float3(2.0, 3.0, 4.0), float3(0.0, 0.0, 0.0));
        dpfloat3 dpy = dpfloat3(float3(1.5, 2.5, 3.5), float3(0.0, 0.0, 0.0));
        /*FooImpl fooImpl = FooImpl(2.0);
        FooImpl.Differential fooImplDiff = FooImpl(0.0);
        DifferentialPair<FooImpl> dpImpl = DifferentialPair<FooImpl>(fooImpl, FooImpl(0.0));
        DifferentialPair<float> dpY = DifferentialPair<float>(1.5f, 1.0f);*/

        //bwd_diff(simple)(dpImpl, dpY, fooImplDiff);
        bwd_diff(simple)(dpx, dpy, dfloat3(1.0, 1.0, 1.0));

        //outputBuffer[0] = dpImpl.d.value; // Expect: 2
        //outputBuffer[1] = dpY.d; // Expect: 1
        outputBuffer[0] = dpx.d.x; // Expect: 1
        outputBuffer[1] = dpy.d.y; // Expect: 1
        outputBuffer[2] = dpx.d.z; // Expect: 1
    }
}
