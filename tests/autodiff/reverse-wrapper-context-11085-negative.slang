// Negative test for issue #11085's identity-forwarding-wrapper optimization.
//
// The fix forwards a `[Differentiable]` wrapper's backward directly to its callee's ONLY when
// the wrapper is a pure identity forward (`outer(args) { return inner(args); }` with args passed
// through 1:1). A wrapper that permutes or transforms its arguments must NOT be treated as an
// identity forward — doing so would produce wrong gradients. These cases use distinguishable
// per-argument gradients so a mistaken forward is caught numerically.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

// inner2(a, b) = a - 2*b, so d/da = 1 and d/db = -2 (distinct so a swap is observable).
[Differentiable]
float inner2(float a, float b)
{
    return a - 2.0f * b;
}

// Argument-reordering wrapper: forwards (b, a), NOT the identity. Its true gradient wrt the
// wrapper's own (a, b) is d/da = -2 (a lands in inner2's `b` slot), d/db = 1 (b lands in `a`).
[Differentiable]
float outerReorder(float a, float b)
{
    return inner2(b, a);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var a = diffPair(5.0f);
    var b = diffPair(7.0f);
    bwd_diff(outerReorder)(a, b, 1.0f);
    outputBuffer[0] = a.d;
    // CHECK: -2.0
    outputBuffer[1] = b.d;
    // CHECK: 1.0
}
