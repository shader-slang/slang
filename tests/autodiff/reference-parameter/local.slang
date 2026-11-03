// A reference parameter of a differentiable function is treated as `no_diff`, so differentiating
// the function propagates derivatives only through its other parameters. Before, backward
// differentiation aborted with "ref parameter not allowed in backward diff function". A function
// may forward its reference parameter to a differentiable callee: autodiff passes the reference
// through, so the callee's writes reach the caller's storage, and the backward derivative saves
// the values read before a write for use in the derivative.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -Xslang -experimental-feature -Xslang -warnings-disable -Xslang 38038
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj -Xslang -experimental-feature -Xslang -warnings-disable -Xslang 38038

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float scaleRef(__ref float a, float x)
{
    return a * x;
}

[Differentiable]
float scaleReadOnly(__ref_readonly float a, float x)
{
    return a * x;
}

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
float forwardRefOnce(no_diff __ref float a, float x)
{
    return scaleRef(a, x);
}

[Differentiable]
float forwardRefTwice(no_diff __ref float a, float x)
{
    return forwardRefOnce(a, x);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // d(a * x)/dx = a = 3, in each mode, and the reference is read but not written.
    float a = 3.0;
    var x = diffPair(2.0);
    bwd_diff(scaleRef)(a, x, 1.0);
    outputBuffer[0] = x.d;
    var y = diffPair(5.0);
    bwd_diff(scaleReadOnly)(a, y, 1.0);
    outputBuffer[1] = y.d;
    outputBuffer[2] = fwd_diff(scaleRef)(a, diffPair(2.0, 1.0)).d;
    outputBuffer[3] = a;

    // f = a * x * x with `a` read before the callee overwrites it: df/dx = 2 * a * x = 12.
    float b = 2.0;
    var z = diffPair(3.0);
    bwd_diff(forwardRef)(b, z, 1.0);
    outputBuffer[4] = z.d;
    outputBuffer[5] = b;

    // f = a * x through two forwarding levels: df/dx = a = 5, in each mode.
    float c = 5.0;
    var w = diffPair(3.0);
    bwd_diff(forwardRefTwice)(c, w, 1.0);
    outputBuffer[6] = w.d;
    outputBuffer[7] = fwd_diff(forwardRefTwice)(c, diffPair(3.0, 1.0)).d;
}

// CHECK: 3.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 12.0
// CHECK-NEXT: 100.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 5.0
