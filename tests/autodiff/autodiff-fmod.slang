//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef float.Differential dfloat;

[Differentiable]
float test_fmod(float x, float y) { return fmod(x, y); }

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // fmod(5.5, 2.0) = 1.5
    // d/dx fmod(x,y) = 1
    // d/dy fmod(x,y) = -floor(x/y) = -floor(5.5/2.0) = -floor(2.75) = -2
    {
        dpfloat dpx = dpfloat(5.5, 0.0);
        dpfloat dpy = dpfloat(2.0, 0.0);
        __bwd_diff(test_fmod)(dpx, dpy, 1.0);
        outputBuffer[0] = dpx.d; // d/dx: expect 1.0
        outputBuffer[1] = dpy.d; // d/dy: expect -2.0
    }

    // Forward mode test
    // fmod(5.5, 2.0) with dx=1, dy=0 -> derivative = 1
    // fmod(5.5, 2.0) with dx=0, dy=1 -> derivative = -floor(5.5/2.0) = -2
    {
        dpfloat dpx = dpfloat(5.5, 1.0);
        dpfloat dpy = dpfloat(2.0, 0.0);
        dpfloat result = __fwd_diff(test_fmod)(dpx, dpy);
        outputBuffer[2] = result.d; // expect 1.0
    }
    {
        dpfloat dpx = dpfloat(5.5, 0.0);
        dpfloat dpy = dpfloat(2.0, 1.0);
        dpfloat result = __fwd_diff(test_fmod)(dpx, dpy);
        outputBuffer[3] = result.d; // expect -2.0
    }

    // Test with different values: fmod(7.0, 3.0) = 1.0
    // d/dy = -floor(7/3) = -floor(2.333) = -2
    {
        dpfloat dpx = dpfloat(7.0, 0.0);
        dpfloat dpy = dpfloat(3.0, 0.0);
        __bwd_diff(test_fmod)(dpx, dpy, 1.0);
        outputBuffer[4] = dpy.d; // d/dy: expect -2.0
    }

    // Primal check
    outputBuffer[5] = fmod(5.5, 2.0); // expect 1.5
}
