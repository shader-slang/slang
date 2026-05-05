//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for issue #11029:
// constexpr parameters in functions with user-provided backward derivatives
// must survive the autodiff backward-pass synthesis unchanged.
// Before the fix, they were silently replaced with void, causing static_assert
// inside the backward derivative to fail at compile time.

[BackwardDerivative(scale_bwd)]
float scale(float x, constexpr int mode)
{
    static_assert(mode == 2, "mode must remain constexpr through backward pass");
    return x * float(mode);
}

void scale_bwd(inout DifferentialPair<float> x, constexpr int mode, float dOut)
{
    static_assert(mode == 2, "mode must remain constexpr in backward derivative");
    x = DifferentialPair<float>(x.p, dOut * float(mode));
}

[Differentiable]
float run<let Mode : int>(float x)
{
    return scale(x, Mode);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var x = diffPair(3.0f);
    __bwd_diff(run<2>)(x, 1.0f);
    outputBuffer[dispatchThreadID.x] = x.d;
    // d(2*x)/dx = 2; times dOut=1.0 -> gradient is 2.0
    //CHECK: 2
}
