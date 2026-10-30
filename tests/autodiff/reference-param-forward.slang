// A differentiable function may forward its reference parameter to a differentiable callee.
// Autodiff passes the reference argument by copy-in/copy-out, as it does for `inout`, so the
// callee's writes reach the caller's storage and values read before a write are the ones used
// for the derivative. Shared memory cannot be passed this way: the errors are covered by
// tests/diagnostics/groupshared-autodiff/warns.slang.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float readThenOverwrite(no_diff __ref float a, float x)
{
    float r = a * x;
    a = 100.0;
    return r;
}

[Differentiable]
float forwardRef(no_diff __ref float a, float x)
{
    return readThenOverwrite(a, x) * x;
}

[Differentiable]
float scale(no_diff __ref float a, float x)
{
    return a * x;
}

[Differentiable]
float forwardRefTwice(no_diff __ref float a, float x)
{
    return forwardRefOnce(a, x);
}

[Differentiable]
float forwardRefOnce(no_diff __ref float a, float x)
{
    return scale(a, x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // f = a * x * x with `a` read before the callee overwrites it: df/dx = 2 * a * x = 12.
    float a = 2.0;
    var x = diffPair(3.0, 0.0);
    bwd_diff(forwardRef)(a, x, 1.0);
    outputBuffer[0] = x.d;
    outputBuffer[1] = a;

    // f = a * x through two forwarding levels: df/dx = a = 5.
    float b = 5.0;
    var y = diffPair(3.0, 0.0);
    bwd_diff(forwardRefTwice)(b, y, 1.0);
    outputBuffer[2] = y.d;

    // The forward derivative forwards the reference the same way: d/dx (a * x) = a = 5.
    outputBuffer[3] = fwd_diff(forwardRefTwice)(b, diffPair(3.0, 1.0)).d;
}

// CHECK: 12.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 5.0
