//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: both fwd_diff and bwd_diff extensions on the same function

// Note: cube is NOT [Differentiable] — derivatives come solely from __func_extension.
float cube(float x) { return x * x * x; }

__func_extension fwd_diff(cube)(DifferentialPair<float> dpx) -> DifferentialPair<float>
{
    return diffPair(cube(dpx.p), 3.0 * dpx.p * dpx.p * dpx.d);
}

__func_extension bwd_diff(cube)(inout DifferentialPair<float> dpx, float dOut) -> void
{
    dpx = diffPair(dpx.p, dOut * 3.0 * dpx.p * dpx.p);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Forward: cube(2.0) = 8.0, d/dx = 12.0
    DifferentialPair<float> fwd = fwd_diff(cube)(diffPair(2.0, 1.0));
    outputBuffer[0] = fwd.p;
    outputBuffer[1] = fwd.d;

    // Backward: d/dx(cube) at x=2.0 = 12.0
    var dpx = diffPair(2.0, 0.0);
    bwd_diff(cube)(dpx, 1.0);
    outputBuffer[2] = dpx.d;

    // CHECK: 8.0
    // CHECK: 12.0
    // CHECK: 12.0
}
