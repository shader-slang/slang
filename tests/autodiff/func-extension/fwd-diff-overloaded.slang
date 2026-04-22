//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension fwd_diff disambiguating an overloaded function

// Note: NOT [Differentiable] — derivatives come solely from __func_extension.
float myFunc(float x) { return x * x; }
float myFunc(float x, float y) { return x * y; }

// Forward derivative for the single-arg overload
__func_extension fwd_diff(myFunc)(DifferentialPair<float> dpx) -> DifferentialPair<float>
{
    return diffPair(myFunc(dpx.p), 2.0f * dpx.p * dpx.d);
}

// Forward derivative for the two-arg overload
__func_extension fwd_diff(myFunc)(DifferentialPair<float> dpx, DifferentialPair<float> dpy) -> DifferentialPair<float>
{
    return diffPair(myFunc(dpx.p, dpy.p), dpx.d * dpy.p + dpx.p * dpy.d);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Single arg: myFunc(3.0) = 9.0, d/dx = 6.0
    DifferentialPair<float> r1 = fwd_diff(myFunc)(diffPair(3.0, 1.0));
    outputBuffer[0] = r1.p;
    outputBuffer[1] = r1.d;

    // Two args: myFunc(3.0, 4.0) = 12.0, d = 1.0*4.0 + 3.0*1.0 = 7.0
    DifferentialPair<float> r2 = fwd_diff(myFunc)(diffPair(3.0, 1.0), diffPair(4.0, 1.0));
    outputBuffer[2] = r2.p;
    outputBuffer[3] = r2.d;

    // CHECK: 9.0
    // CHECK: 6.0
    // CHECK: 12.0
    // CHECK: 7.0
}
