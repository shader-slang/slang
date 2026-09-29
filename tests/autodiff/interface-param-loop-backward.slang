// Reverse-mode differentiation of a function that takes an interface-typed parameter and calls a
// `[Differentiable]` method on it from inside a loop. The backward pass stores a default-
// constructed differential into a temporary whose pointee type is a one-element
// `UntaggedUnionType`, which used to make type-flow specialization oscillate between that
// spelling and the collapsed element type and never reach a fixpoint, so the compiler hung.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[2.0 5.0], stride=4),name=weightBuffer
RWStructuredBuffer<float> weightBuffer;

interface ITensor<T : IDifferentiable, let D : int>
{
    [Differentiable]
    T load(int index);
}

struct Tensor<T : IDifferentiable, let D : int> : ITensor<T, D>
{
    RWStructuredBuffer<T> primal;

    [Differentiable]
    T load(int index) { return no_diff primal[index]; }
}

[Differentiable]
float weightedSum(ITensor<float, 2> weights, float x[2])
{
    float sum = 0.0f;
    for (int i = 0; i < 2; ++i)
        sum += weights.load(i) * x[i];
    return sum;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Tensor<float, 2> weights = { weightBuffer };

    float x[2] = { 1.0f, 1.0f };

    // CHECK: 7.0
    outputBuffer[0] = weightedSum(weights, x);

    DifferentialPair<float[2]> dpx = diffPair(x);
    bwd_diff(weightedSum)(weights, dpx, 1.0f);

    // CHECK-NEXT: 2.0
    outputBuffer[1] = dpx.d[0];
    // CHECK-NEXT: 5.0
    outputBuffer[2] = dpx.d[1];
}
