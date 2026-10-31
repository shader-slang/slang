// Regression test for https://github.com/shader-slang/slang/issues/10071
// fmod(x,y) autodiff gradient w.r.t. y is always 0 (should be -floor(x/y))
// This test will FAIL until the bug is fixed.

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float test_fmod(float x, float y) { return fmod(x, y); }

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // fmod(5.5, 2.0) = 1.5
    // d/dy fmod(x,y) = -floor(x/y) = -floor(2.75) = -2
    DifferentialPair<float> dpx = diffPair(5.5f, 0.0f);
    DifferentialPair<float> dpy = diffPair(2.0f, 0.0f);
    
    __bwd_diff(test_fmod)(dpx, dpy, 1.0f);
    
    outputBuffer[0] = dpy.d;  // d/dy: should be -2.0
    outputBuffer[1] = dpx.d;  // d/dx: should be 1.0
    outputBuffer[2] = fmod(5.5f, 2.0f);  // primal: 1.5
}
