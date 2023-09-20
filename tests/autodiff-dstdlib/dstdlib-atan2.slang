//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef DifferentialPair<float2> dpfloat2;
typedef DifferentialPair<float3> dpfloat3;

[BackwardDifferentiable]
float diffAtan2(float x, float y)
{
    return atan2(x, y);
}

// Finite difference version of atan2
DifferentialPair<float> fd_atan2(DifferentialPair<float> dpx, DifferentialPair<float> dpy, float eps)
{
    float x = dpx.p;
    float y = dpy.p;
    float dx = dpx.d;
    float dy = dpy.d;

    float res = atan2(x, y);
    float dres = (atan2(x + eps * dx, y + eps * dy) - atan2(x - eps * dx, y - eps* dy)) / (2 * eps);

    return DifferentialPair<float>(res, dres);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    float eps = 1e-4;
    {
        dpfloat dpx = dpfloat(5.0, 1.0);
        dpfloat dpy = dpfloat(3.0, 0.0);
        dpfloat res = __fwd_diff(diffAtan2)(dpx, dpy);
        dpfloat fres = fd_atan2(dpx, dpy, eps);
        outputBuffer[0] = res.d;
        outputBuffer[1] = fres.d;
    }

    {
        dpfloat dpx = dpfloat(5.0, 0.0);
        dpfloat dpy = dpfloat(3.0, 1.0);
        dpfloat res = __fwd_diff(diffAtan2)(dpx, dpy);
        dpfloat fres = fd_atan2(dpx, dpy, eps);
        outputBuffer[2] = res.d;
        outputBuffer[3] = fres.d;
    }

    // Test the other 3 quadrants
    {
        dpfloat dpx = dpfloat(-5.0, 1.0);
        dpfloat dpy = dpfloat(3.0, 0.0);
        dpfloat res = __fwd_diff(diffAtan2)(dpx, dpy);
        dpfloat fres = fd_atan2(dpx, dpy, eps);
        outputBuffer[4] = res.d;
        outputBuffer[5] = fres.d;
    }

    {
        dpfloat dpx = dpfloat(-5.0, 0.0);
        dpfloat dpy = dpfloat(3.0, 1.0);
        dpfloat res = __fwd_diff(diffAtan2)(dpx, dpy);
        dpfloat fres = fd_atan2(dpx, dpy, eps);
        outputBuffer[6] = res.d;
        outputBuffer[7] = fres.d;
    }

    {
        dpfloat dpx = dpfloat(-5.0, 1.0);
        dpfloat dpy = dpfloat(-3.0, 0.0);
        dpfloat res = __fwd_diff(diffAtan2)(dpx, dpy);
        dpfloat fres = fd_atan2(dpx, dpy, eps);
        outputBuffer[8] = res.d;
        outputBuffer[9] = fres.d;
    }

    {
        dpfloat dpx = dpfloat(-5.0, 0.0);
        dpfloat dpy = dpfloat(-3.0, 1.0);
        dpfloat res = __fwd_diff(diffAtan2)(dpx, dpy);
        dpfloat fres = fd_atan2(dpx, dpy, eps);
        outputBuffer[10] = res.d;
        outputBuffer[11] = fres.d;
    }

    {
        dpfloat dpx = dpfloat(5.0, 1.0);
        dpfloat dpy = dpfloat(-3.0, 0.0);
        dpfloat res = __fwd_diff(diffAtan2)(dpx, dpy);
        dpfloat fres = fd_atan2(dpx, dpy, eps);
        outputBuffer[12] = res.d;
        outputBuffer[13] = fres.d;
    }

    {
        dpfloat dpx = dpfloat(5.0, 0.0);
        dpfloat dpy = dpfloat(-3.0, 1.0);
        dpfloat res = __fwd_diff(diffAtan2)(dpx, dpy);
        dpfloat fres = fd_atan2(dpx, dpy, eps);
        outputBuffer[14] = res.d;
        outputBuffer[15] = fres.d;
    }

    {
        dpfloat dpx = diffPair(5.0);
        dpfloat dpy = diffPair(3.0);
        __bwd_diff(diffAtan2)(dpx, dpy, 1.0);
        outputBuffer[16] = dpx.d; // Should be equal to outputBuffer[0]
        outputBuffer[17] = dpy.d; // Should be equal to outputBuffer[2]
    }
}
