//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef DifferentialPair<float2> dpfloat2;

[BackwardDifferentiable]
float diffMax(float x, float y)
{
    return max(x, y);
}

[BackwardDifferentiable]
float2 diffMax(float2 x, float2 y)
{
    return max(x, y);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpx = dpfloat(2.0, 1.0);
        dpfloat dpy = dpfloat(5.0, -2.0);
        dpfloat res = __fwd_diff(diffMax)(dpx, dpy);
        outputBuffer[0] = res.p;        // Expect: 5.000000
        outputBuffer[1] = res.d;        // Expect: -2.000000
    }

    {
        dpfloat2 dpx = dpfloat2(float2(-3.0, 4.0), float2(-1.0, -1.0));
        dpfloat2 dpy = dpfloat2(float2(1.0, 2.0), float2(2.0, 2.0));
        dpfloat2 res = __fwd_diff(diffMax)(dpx, dpy);
        outputBuffer[2] = res.p[0];     // Expect: 1.000000
        outputBuffer[3] = res.d[0];     // Expect: 2.000000
        outputBuffer[4] = res.p[1];     // Expect: 4.000000
        outputBuffer[5] = res.d[1];     // Expect: -1.000000
    }

    {
        dpfloat2 dpx = dpfloat2(float2(2.0, 3.0), float2(0.0, 0.0));
        dpfloat2 dpy = dpfloat2(float2(5.0, 1.0), float2(0.0, 0.0));
        __bwd_diff(diffMax)(dpx, dpy, float2(1.0, 2.0));
        outputBuffer[6] = dpx.d[0]; // Expect: 0.000000
        outputBuffer[7] = dpx.d[1]; // Expect: 2.000000
        outputBuffer[8] = dpy.d[0]; // Expect: 1.000000
        outputBuffer[9] = dpy.d[1]; // Expect: 0.000000
    }
}
