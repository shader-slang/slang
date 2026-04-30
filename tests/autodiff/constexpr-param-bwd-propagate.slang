//TEST:COMPILE: -target hlsl -profile cs_6_0 -entry computeMain -no-codegen

// Regression for constexpr parameters used by generated backward-propagation wrappers.
// The generated bwd-prop call must preserve the literal Mode argument instead of
// replacing it with void for being nondifferentiable.

[BackwardDerivative(scale_bwd)]
float scale(float x, constexpr int mode)
{
    static_assert(mode == 2, "mode must remain constexpr");
    return x * float(mode);
}

void scale_bwd(inout DifferentialPair<float> x, constexpr int mode, float dOut)
{
    static_assert(mode == 2, "mode must remain constexpr");
    updateDiff(x, dOut * float(mode));
}

[Differentiable]
float run<let Mode : int>(float x)
{
    return scale(x, Mode);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var x = diffPair(3.0);
    __bwd_diff(run<2>)(x, 1.0);
    outputBuffer[dispatchThreadID.x] = x.d;
}
