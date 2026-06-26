// Regression test for shader-slang/slang#11784.
// bwd_diff/fwd_diff through `Conditional<T, hasValue>.get()` used to abort with an
// internal error (E99999): the forward autodiff transcriber wired the `Optional`
// intrinsic family but had no case for `GetConditionalValue` / `MakeConditionalValue`,
// so the inlined getter/constructor fell through to the `InternalCompilerError` default.
//
// Two scenarios:
//  * fnGet  exercises `GetConditionalValue` on a primal-only `Conditional` parameter.
//  * fnMake exercises `MakeConditionalValue` by constructing a `Conditional` from a
//    differentiable value inside the differentiated function, then reading it back; the
//    gradient must still propagate into that value (the GetConditionalValue/
//    MakeConditionalValue pair folds to the wrapped value, so no tangent is lost).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

[Differentiable]
float3 fnGet(Conditional<float3, true> normal, float3 v)
{
    if (let n = normal.get())
        return n * v;
    return v;
}

[Differentiable]
float3 fnMake(float3 x, float3 v)
{
    Conditional<float3, true> c = x; // MakeConditionalValue(x)
    if (let n = c.get())             // GetConditionalValue(c)
        return n * v;
    return v;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Scenario 1: primal-only Conditional parameter. result = n.get() * v = (2,3,4)*(1,1,1);
    // d(result)/d(v) = (2,3,4), so with seed (1,1,1) the gradient into v1.d is (2,3,4).
    var v1 = diffPair(float3(1, 1, 1), float3(0, 0, 0));
    Conditional<float3, true> n = float3(2, 3, 4);
    bwd_diff(fnGet)(n, v1, float3(1, 1, 1));

    // Scenario 2: construct a Conditional from differentiable x, then read it.
    // result = x * v = (5,6,7)*(1,1,1); d/dx = v = (1,1,1), d/dv = x = (5,6,7).
    var x = diffPair(float3(5, 6, 7), float3(0, 0, 0));
    var v2 = diffPair(float3(1, 1, 1), float3(0, 0, 0));
    bwd_diff(fnMake)(x, v2, float3(1, 1, 1));

    // CHECK: 2.0
    // CHECK: 3.0
    // CHECK: 4.0
    outputBuffer[0] = v1.d.x;
    outputBuffer[1] = v1.d.y;
    outputBuffer[2] = v1.d.z;

    // CHECK: 1.0
    // CHECK: 1.0
    // CHECK: 1.0
    outputBuffer[3] = x.d.x;
    outputBuffer[4] = x.d.y;
    outputBuffer[5] = x.d.z;

    // CHECK: 5.0
    // CHECK: 6.0
    // CHECK: 7.0
    outputBuffer[6] = v2.d.x;
    outputBuffer[7] = v2.d.y;
    outputBuffer[8] = v2.d.z;
}
