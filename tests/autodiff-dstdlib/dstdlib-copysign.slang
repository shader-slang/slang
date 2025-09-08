//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-wgpu -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef DifferentialPair<float2> dpfloat2;

[BackwardDifferentiable]
float diffCopysign(float x, float y)
{
    return copysign(x, y);
}

[BackwardDifferentiable]
float2 diffCopysign(float2 x, float2 y)
{
    return copysign(x, y);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    // Test 1: forward diff copysign(3.0, -1.0) with dx=2.0, dy=1.0
    {
        dpfloat dpx = dpfloat(3.0, 2.0);
        dpfloat dpy = dpfloat(-1.0, 1.0);
        dpfloat res = __fwd_diff(diffCopysign)(dpx, dpy);
        outputBuffer[0] = res.p;               
        outputBuffer[1] = res.d;
    }

    // Test 2: forward diff copysign(-2.0, 4.0) with dx=1.5, dy=-0.5
    {
        dpfloat dpx = dpfloat(-2.0, 1.5);
        dpfloat dpy = dpfloat(4.0, -0.5);
        dpfloat res = __fwd_diff(diffCopysign)(dpx, dpy);
        outputBuffer[2] = res.p;               
        outputBuffer[3] = res.d;
    }

    // Test 3: forward diff copysign(0.0, -1.0) with dx=3.0, dy=2.0
    {
        dpfloat dpx = dpfloat(0.0, 3.0);
        dpfloat dpy = dpfloat(-1.0, 2.0);
        dpfloat res = __fwd_diff(diffCopysign)(dpx, dpy);
        outputBuffer[4] = res.p;               
        outputBuffer[5] = res.d;
    }

    // Test 4: vector forward diff
    {
        dpfloat2 dpx = dpfloat2(float2(5.0, -3.0), float2(1.0, 2.0));
        dpfloat2 dpy = dpfloat2(float2(-2.0, 4.0), float2(0.5, -1.0));
        dpfloat2 res = __fwd_diff(diffCopysign)(dpx, dpy);
        outputBuffer[6] = res.p[0];            
        outputBuffer[7] = res.d[0];            
        outputBuffer[8] = res.p[1];            
        outputBuffer[9] = res.d[1];
    }

    // Test 5: backward diff copysign(4.0, -2.0)
    {
        dpfloat dpx = dpfloat(4.0, 0.0);
        dpfloat dpy = dpfloat(-2.0, 0.0);
        __bwd_diff(diffCopysign)(dpx, dpy, 1.0);
        outputBuffer[10] = dpx.d;              
        outputBuffer[11] = dpy.d;
    }

    // Test 6: backward diff copysign(-3.0, 5.0)  
    {
        dpfloat dpx = dpfloat(-3.0, 0.0);
        dpfloat dpy = dpfloat(5.0, 0.0);
        __bwd_diff(diffCopysign)(dpx, dpy, 2.0);
        outputBuffer[12] = dpx.d;              
        outputBuffer[13] = dpy.d;
    }

    // Test 7: copysign with y=0 - derivative should be 0
    {
        dpfloat dpx = dpfloat(3.0, 2.0);
        dpfloat dpy = dpfloat(0.0, 1.0);
        dpfloat res = __fwd_diff(diffCopysign)(dpx, dpy);
        outputBuffer[14] = res.p;               
        outputBuffer[15] = res.d;
    }

    // Test 8: copysign with x=0 - derivative should be 0
    {
        dpfloat dpx = dpfloat(0.0, 2.0);
        dpfloat dpy = dpfloat(-1.0, 1.0);
        dpfloat res = __fwd_diff(diffCopysign)(dpx, dpy);
        outputBuffer[16] = res.p;               
        outputBuffer[17] = res.d;
    }

    // Test 9: vector backward diff
    {
        dpfloat2 dpx = dpfloat2(float2(2.0, -1.0), float2(0.0, 0.0));
        dpfloat2 dpy = dpfloat2(float2(-3.0, 4.0), float2(0.0, 0.0));
        __bwd_diff(diffCopysign)(dpx, dpy, float2(1.0, 3.0));
        outputBuffer[18] = dpx.d[0];           
        outputBuffer[19] = dpx.d[1];           
    }
}