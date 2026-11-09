//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o bwd-diff-generic-member-differentiable-this.spv

// Regression test for differentiating a generic member function with differentiable `this`.

interface IUnary : IDifferentiable
{
    [Differentiable]
    float operator()(float value);
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
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Scale function = {2.0};
    trigger(function, 3.0);
}
