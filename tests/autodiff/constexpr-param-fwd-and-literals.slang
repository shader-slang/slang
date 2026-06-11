//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression coverage for constexpr params in autodiff paths that are not
// covered by the integer-only backward tests.

[Differentiable]
float scale(float x, constexpr int mode, constexpr bool negate)
{
    static_assert(mode == 3, "mode must remain constexpr in primal");
    static_assert(negate, "bool constexpr param must remain constexpr in primal");
    return (negate ? -x : x) * float(mode);
}

[Differentiable]
float runFwd<let Mode : int, let Negate : bool>(float x)
{
    return scale(x, Mode, Negate);
}

[BackwardDerivative(scaleFloat_bwd)]
float scaleFloat(float x, no_diff constexpr float k)
{
    static_assert(k > 2.0f, "float constexpr param must remain constexpr in primal");
    return x * k;
}

void scaleFloat_bwd(inout DifferentialPair<float> x, no_diff constexpr float k, float dOut)
{
    static_assert(k > 2.0f, "float constexpr param must remain constexpr in backward derivative");
    x = DifferentialPair<float>(x.p, dOut * k);
}

[Differentiable]
float runFloat(float x)
{
    return scaleFloat(x, 2.5f);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var x = diffPair(2.0f, 1.0f);
    outputBuffer[0] = __fwd_diff(runFwd<3, true>)(x).d;
    //CHECK: -3

    var y = diffPair(1.0f);
    __bwd_diff(runFloat)(y, 1.0f);
    outputBuffer[1] = y.d;
    //CHECK: 2.5
}
