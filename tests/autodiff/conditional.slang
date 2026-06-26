// Regression test for shader-slang/slang#11784.
// bwd_diff/fwd_diff through `Conditional<T, hasValue>.get()` used to abort with an
// internal error (E99999): the forward autodiff transcriber wired the `Optional`
// intrinsic family but had no case for `GetConditionalValue`, so the inlined getter
// fell through to the `InternalCompilerError` default. `normal` is a primal-only
// `Conditional` argument, so the transcriber must produce the primal getter and propagate
// the gradient through the differentiable result type `float3` for `v`.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

[Differentiable]
float3 fn(Conditional<float3, true> normal, float3 v)
{
    if (let n = normal.get())
        return n * v;
    return v;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var v = diffPair(float3(1, 1, 1), float3(0, 0, 0));
    Conditional<float3, true> n = float3(2, 3, 4);

    // result = n.get() * v = (2,3,4) * (1,1,1) componentwise; d(result)/d(v) = (2,3,4).
    // With seed (1,1,1), the gradient accumulated into v.d is (2,3,4).
    bwd_diff(fn)(n, v, float3(1, 1, 1));

    // CHECK: 2.0
    // CHECK: 3.0
    // CHECK: 4.0
    outputBuffer[0] = v.d.x;
    outputBuffer[1] = v.d.y;
    outputBuffer[2] = v.d.z;
}
