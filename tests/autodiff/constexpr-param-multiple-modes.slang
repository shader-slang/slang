//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for issue #11029 (multiple specializations):
// Two different constexpr values produce distinct correct gradients.

[BackwardDerivative(scale_bwd)]
float scale(float x, constexpr int mode)
{
    return x * float(mode);
}

void scale_bwd(inout DifferentialPair<float> x, constexpr int mode, float dOut)
{
    x = DifferentialPair<float>(x.p, dOut * float(mode));
}

[Differentiable]
float run<let Mode : int>(float x)
{
    return scale(x, Mode);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var x2 = diffPair(1.0f);
    __bwd_diff(run<2>)(x2, 1.0f);
    outputBuffer[0] = x2.d; // gradient of 2*x at x=1 is 2
    //CHECK: 2

    var x3 = diffPair(1.0f);
    __bwd_diff(run<3>)(x3, 1.0f);
    outputBuffer[1] = x3.d; // gradient of 3*x at x=1 is 3
    //CHECK: 3
}
