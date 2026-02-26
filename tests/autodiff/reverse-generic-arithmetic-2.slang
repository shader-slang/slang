//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

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

        bwd_diff(simple)(dpx, dpy, dfloat3(1.0, 1.0, 1.0));

        outputBuffer[0] = dpx.d.x; // CHECK: 1.0
        outputBuffer[1] = dpy.d.y; // CHECK-NEXT: 1.0
        outputBuffer[2] = dpx.d.z; // CHECK-NEXT: 1.0
    }
}
