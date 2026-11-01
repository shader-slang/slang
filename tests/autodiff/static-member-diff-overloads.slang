//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Mixed
{
    [Differentiable]
    static float eval(float x) { return x * x; }

    [Differentiable]
    [NoDiffThis]
    float eval(float2 x) { return x.x * x.y; }

    [Differentiable]
    static float genericEval<T>(float x) { return x * x; }

    [Differentiable]
    [NoDiffThis]
    float genericEval<T>(float2 x) { return x.x * x.y; }
}

struct Unbound
{
    [Differentiable]
    [NoDiffThis]
    float eval(float x) { return x * x; }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Mixed mixed;

    // Ordinary lookup produces an OverloadedExpr. Filtering the instance method
    // must leave the static method available through the same instance.
    // CHECK: 6.0
    outputBuffer[0] = fwd_diff(mixed.eval)(diffPair(3.0, 1.0)).d;
    var x = diffPair(3.0);
    bwd_diff(mixed.eval)(x, 1.0);
    // CHECK: 6.0
    outputBuffer[1] = x.d;

    // Explicit generic arguments produce an OverloadedExpr2. The static overload
    // must still be available after filtering the specialized instance method.
    // CHECK: 6.0
    outputBuffer[2] = fwd_diff(mixed.genericEval<float>)(diffPair(3.0, 1.0)).d;
    x = diffPair(3.0);
    bwd_diff(mixed.genericEval<float>)(x, 1.0);
    // CHECK: 6.0
    outputBuffer[3] = x.d;

    // Unbound instance methods accept the receiver as an explicit argument.
    Unbound instance;
    // CHECK: 6.0
    outputBuffer[4] = fwd_diff(Unbound.eval)(instance, diffPair(3.0, 1.0)).d;
    x = diffPair(3.0);
    bwd_diff(Unbound.eval)(instance, x, 1.0);
    // CHECK: 6.0
    outputBuffer[5] = x.d;

    // A nested derivative keeps referring to the selected static overload.
    // CHECK: 2.0
    outputBuffer[6] = fwd_diff(fwd_diff(mixed.genericEval<float>))(
        diffPair(diffPair(3.0, 1.0), diffPair(1.0, 0.0))).d.d;
}
