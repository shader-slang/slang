//TEST:COMPILE: -target hlsl -profile cs_6_0 -entry computeMain -no-codegen
//TEST:COMPILE: -target spirv -profile cs_6_0 -entry computeMain -no-codegen
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)

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

[ForwardDifferentiable]
float scaleForward(float x, constexpr int mode)
{
    static_assert(mode == 3, "mode must remain constexpr");
    return x * float(mode);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var x = diffPair(3.0);
    __bwd_diff(run<2>)(x, 1.0);
    outputBuffer[0] = x.d;
    // CHECK: 2.0

    var y = __fwd_diff(scaleForward)(diffPair(4.0, 1.0), 3);
    outputBuffer[1] = y.d;
    // CHECK: 3.0
}
