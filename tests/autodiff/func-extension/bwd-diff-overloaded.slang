//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature

// Test: __func_extension bwd_diff disambiguating an overloaded function.

// Note: NOT [Differentiable] -- derivatives come solely from __func_extension.
float myFunc(float x) { return x * x; }
float myFunc(float x, float y) { return x * y; }

// Backward derivative for the single-arg overload.
__func_extension bwd_diff(myFunc)(inout DifferentialPair<float> dpx, float dOut) -> void
{
    dpx = diffPair(dpx.p, dOut * 2.0 * dpx.p);
}

// Backward derivative for the two-arg overload.
__func_extension bwd_diff(myFunc)(
    inout DifferentialPair<float> dpx,
    inout DifferentialPair<float> dpy,
    float dOut) -> void
{
    let x = dpx.p;
    let y = dpy.p;
    dpx = diffPair(x, dOut * y);
    dpy = diffPair(y, dOut * x);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Single arg: d/dx myFunc(3.0) = 6.0.
    var dpx = diffPair(3.0, 0.0);
    bwd_diff(myFunc)(dpx, 1.0);
    outputBuffer[0] = dpx.p;
    outputBuffer[1] = dpx.d;

    // Two args: d/dx myFunc(3.0, 4.0) = 4.0, d/dy = 3.0.
    var dpxy = diffPair(3.0, 0.0);
    var dpy = diffPair(4.0, 0.0);
    bwd_diff(myFunc)(dpxy, dpy, 1.0);
    outputBuffer[2] = dpxy.d;
    outputBuffer[3] = dpy.d;

    // CHECK: 3.0
    // CHECK: 6.0
    // CHECK: 4.0
    // CHECK: 3.0
}
