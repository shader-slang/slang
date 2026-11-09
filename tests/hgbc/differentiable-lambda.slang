// A lambda created in a differentiable function should expose its captured state to autodiff.
// Automatic differentiable-lambda conformance is deferred pending a different implementation.
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[Differentiable]
float apply<TFunc : IDifferentiableFuncWithDiffThis<float, float>>(
    TFunc function,
    float value)
{
    return function(value);
}

int applyOrdinary<TFunc : IFunc<int, float>>(TFunc function, float value)
{
    return function(value);
}

[Differentiable]
int evaluateWithNonDifferentiableResult(float value)
{
    // The parameter initially makes this lambda a differentiable candidate, but its result does
    // not satisfy the special interface. It must retain the ordinary IFunc conformance.
    return applyOrdinary((float innerValue) => int(innerValue), value);
}

float applyOrdinaryInt<TFunc : IFunc<float, int>>(TFunc function, int value)
{
    return function(value);
}

[Differentiable]
float evaluateWithNonDifferentiableInput(int value)
{
    // This lambda is not a differentiable candidate because its parameter is not differentiable.
    return applyOrdinaryInt((int innerValue) => float(innerValue), value);
}

[Differentiable]
float evaluate(
    float scale,
    float bias,
    no_diff float offset,
    int branch,
    float value)
{
    return apply(
        (float innerValue) =>
        {
            let adjustedBias = branch == 0 ? bias + offset : bias + offset;
            return scale * innerValue + adjustedBias;
        },
        value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void evaluateBackward(RWStructuredBuffer<uint> checks)
{
    var scale = diffPair(3.0);
    var bias = diffPair(2.0);
    var value = diffPair(4.0);

    bwd_diff(evaluate)(scale, bias, 5.0, 0, value, 1.0);

    checks[0] = scale.d == 4.0 ? 4 : 0;
    checks[1] = bias.d == 1.0 ? 1 : 0;
    checks[2] = value.d == 3.0 ? 3 : 0;
}

[host]
public void main()
{
    RWStructuredBuffer<uint> checks = RWStructuredBuffer<uint>::alloc(3);
    dispatch(evaluateBackward, uint3(1, 1, 1))(checks);

    print(asyncReadBack(checks, 0).wait());
    print(asyncReadBack(checks, 1).wait());
    print(asyncReadBack(checks, 2).wait());
}

// CHECK: OpStringLit {{[0-9]+}} "evaluateBackward"
// CHECK: OpSlangEntryPoint {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}}
// CHECK: OpMakeDispatcher Compute
