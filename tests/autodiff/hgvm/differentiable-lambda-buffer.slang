//TEST:INTERPRET(hgvm):

void fail(String message)
{
    hgbc_asm
    {
        OpFail $message;
    };
}

void check(bool condition, String message)
{
    if (!condition)
        fail(message);
}

struct DifferentiableBuffer
{
    StructuredBuffer<float> primal;
    RWStructuredBuffer<float> differential;

    [BackwardDifferentiable]
    [NoDiffThis]
    float load(uint index)
    {
        return primal[index];
    }

    [BackwardDerivativeOf(load)]
    [NoDiffThis]
    void loadBackward(uint index, float differentialValue)
    {
        differential[index] += differentialValue;
    }
}

[Differentiable]
float apply<TFunc : IDifferentiableFuncWithDiffThis<float, float>>(TFunc function, float value)
{
    return function(value);
}

[Differentiable]
float evaluate(no_diff DifferentiableBuffer values, float value)
{
    return apply((float innerValue)~ => values.load(0) * innerValue, value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void evaluateBackward(
    StructuredBuffer<float> primal,
    RWStructuredBuffer<float> differential,
    RWStructuredBuffer<uint> checks)
{
    DifferentiableBuffer values = {primal, differential};
    var value = diffPair(2.0);
    bwd_diff(evaluate)(values, value, 1.0);

    checks[0] = evaluate(values, 2.0) == 6.0 ? 1 : 0;
    checks[1] = value.d == 3.0 ? 1 : 0;
    checks[2] = differential[0] == 2.0 ? 1 : 0;
}

[host]
void main()
{
    RuntimeArray<float> source = RuntimeArray<float>::alloc(1);
    source[0] = 3.0;

    StructuredBuffer<float> primal = StructuredBuffer<float>::alloc(1);
    copy(primal, 0, source, 0, 1);

    RWStructuredBuffer<float> differential = RWStructuredBuffer<float>::alloc(1);
    RWStructuredBuffer<uint> checks = RWStructuredBuffer<uint>::alloc(3);
    dispatch(evaluateBackward, uint3(1, 1, 1))(primal, differential, checks);

    check(asyncReadBack(checks, 0).wait() == 1, "primal result is incorrect");
    check(asyncReadBack(checks, 1).wait() == 1, "input derivative is incorrect");
    check(asyncReadBack(checks, 2).wait() == 1, "buffer derivative is incorrect");
}
