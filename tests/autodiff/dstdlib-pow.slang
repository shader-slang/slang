//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef DifferentialPair<float2> dpfloat2;
typedef DifferentialPair<float3> dpfloat3;

[ForwardDifferentiable]
float diffPow(float x, float y)
{
    return dstd.pow(x, y);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpx = dpfloat(5.0, 1.0);
        dpfloat dpy = dpfloat(3.0, 0.0);
        dpfloat res = __fwd_diff(diffPow)(dpx, dpy);
        outputBuffer[0] = res.p();               // Expect: 125.000000
        outputBuffer[1] = res.d();               // Expect: 75.000000
    }

    {
        dpfloat dpx = dpfloat(10.0, 0.0);
        dpfloat dpy = dpfloat(2.5, 1.0);
        dpfloat res = __fwd_diff(diffPow)(dpx, dpy);
        outputBuffer[2] = res.p();               // Expect: 316.227722
        outputBuffer[3] = res.d();               // Expect: 728.141235
    }

    {
        dpfloat dpx = dpfloat(3.0, -2.0);
        dpfloat dpy = dpfloat(4.0, 1.0);
        dpfloat res = __fwd_diff(diffPow)(dpx, dpy);
        outputBuffer[4] = res.p();               // Expect: 81.000000
        outputBuffer[5] = res.d();               // Expect: -127.012398
    }
}
