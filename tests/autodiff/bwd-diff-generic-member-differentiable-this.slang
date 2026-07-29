//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o bwd-diff-generic-member-differentiable-this.spv

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

void trigger<F : IUnary>(F function, float value)
{
    var dFunction = diffPair(function);
    var dValue = diffPair(value);

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
    trigger(function, 3.0);
}
