//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// Regression test for differentiating a generic member function with differentiable `this`.

interface IUnary : IDifferentiable
{
    [Differentiable]
    float operator()(float value);

    // Keep this overload so higher-order checking expands the member lookup candidates.
    // Each candidate must retain `dFunction.p` as its base while overload resolution selects
    // the one-argument method at the derivative call.
    [Differentiable]
    float operator()(float left, float right);
}

void trigger<F : IUnary>(
    inout DifferentialPair<F> dFunction,
    inout DifferentialPair<float> dValue)
{
    bwd_diff(dFunction.p.operator())(dFunction, dValue, 1.0);
}

struct Scale : IUnary
{
    float factor;

    [Differentiable]
    float operator()(float value)
    {
        return factor * value;
    }

    [Differentiable]
    float operator()(float left, float right)
    {
        return factor * (left + right);
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Scale function = {2.0};
    var dFunction = diffPair(function);
    var dValue = diffPair(3.0);
    trigger(dFunction, dValue);

    outputBuffer[0] = dFunction.d.factor;
    outputBuffer[1] = dValue.d;
    // CHECK: 3.0
    // CHECK: 2.0
}
