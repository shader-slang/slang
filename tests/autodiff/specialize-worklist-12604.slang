//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Regression guard for shader-slang/slang#12604: memoizing the specialization work list's
// forward-use-closure walk must be output-neutral. Mixes generic specialization, autodiff,
// and dynamic dispatch — the workloads that drive that walk — and asserts the result values.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;

// IScale is not IDifferentiable, so a value of an IScale type is treated as a
// non-differentiable parameter when the function below is forward-differentiated —
// the same shape as tests/autodiff/no-diff-interface-subscript.slang.
interface IScale
{
    float factor();
}

struct Doubler : IScale
{
    float factor() { return 2.0; }
}

struct Tripler : IScale
{
    float factor() { return 3.0; }
}

// Generic over any IScale, forcing per-concrete-type specialization, and forward
// differentiable in `x`. `factor()` is not differentiable (IScale is not IDifferentiable),
// so it is wrapped in `no_diff`: f(x) = factor * x * x, so df/dx = 2 * factor * x.
[ForwardDifferentiable]
float scaledSquare<T : IScale>(T s, float x)
{
    return (no_diff s.factor()) * x * x;
}

// True dynamic dispatch: the concrete type of the returned IScale is not known
// statically at the call site, so `.factor()` dispatches through the witness table.
IScale makeScale(int which)
{
    if (which == 0)
        return Doubler();
    return Tripler();
}

float dynamicScaledLinear(int which, float x)
{
    IScale s = makeScale(which);
    return s.factor() * x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    // Generic + autodiff, specialized on Doubler: f(3) = 2*9 = 18, f'(3) = 2*2*3 = 12.
    let d = __fwd_diff(scaledSquare)(Doubler(), dpfloat(3.0, 1.0));
    outputBuffer[0] = d.p; // CHECK: 18.0
    outputBuffer[1] = d.d; // CHECK: 12.0

    // Generic + autodiff, specialized on Tripler: f(2) = 3*4 = 12, f'(2) = 2*3*2 = 12.
    let t = __fwd_diff(scaledSquare)(Tripler(), dpfloat(2.0, 1.0));
    outputBuffer[2] = t.p; // CHECK: 12.0
    outputBuffer[3] = t.d; // CHECK: 12.0

    // Dynamic dispatch: the concrete IScale is chosen at runtime, so factor() dispatches
    // through the witness table. Result is factor * x.
    outputBuffer[4] = dynamicScaledLinear(0, 5.0); // CHECK: 10.0
    outputBuffer[5] = dynamicScaledLinear(1, 5.0); // CHECK: 15.0
}
