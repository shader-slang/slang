// Regression test for issue #11085.
//
// A `[Differentiable]` function that is a pure identity forwarding wrapper around another
// differentiable function (`outer(x) { return inner(x); }`) must produce the expected gradient
// in both cases below: when `inner` supplies a custom `[BackwardDerivative]`, and when `inner`'s
// backward is synthesized. This test guards the *gradient* (correctness). The intended #11085
// optimization additionally makes the synthesized backward forward directly to `inner`'s backward
// instead of materializing a nested backward-context copy of the primal aggregate; asserting the
// absence of that copy is a separate codegen test to be added by whoever lands the optimization.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

// inner with a user-provided backward: inner(x) = x*x, so d/dx = 2*x.
[BackwardDerivative(innerBwd)]
[Differentiable]
float inner(float x)
{
    return x * x;
}
void innerBwd(inout DifferentialPair<float> x, float dResult)
{
    x = diffPair(x.p, 2.0f * x.p * dResult);
}

// Identity forwarding wrapper over the custom-backward `inner` — the fix target.
[Differentiable]
float outerCustom(float x)
{
    return inner(x);
}

// inner whose backward is *synthesized* (no custom derivative): sinner(x) = x*x*x, d/dx = 3*x*x.
[Differentiable]
float sinner(float x)
{
    return x * x * x;
}

// Identity forwarding wrapper over the synthesized-backward `sinner` — correctness coverage for
// a synthesized-backward callee, alongside the custom-backward case above.
[Differentiable]
float outerSynth(float x)
{
    return sinner(x);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // outerCustom at x = 3 -> 2 * 3 = 6.
    var a = diffPair(3.0f);
    bwd_diff(outerCustom)(a, 1.0f);
    outputBuffer[0] = a.d;
    // CHECK: 6.0

    // outerSynth at x = 3 -> 3 * 9 = 27.
    var b = diffPair(3.0f);
    bwd_diff(outerSynth)(b, 1.0f);
    outputBuffer[1] = b.d;
    // CHECK: 27.0
}
