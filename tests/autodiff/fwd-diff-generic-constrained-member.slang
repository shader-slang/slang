// Regression test for shader-slang/slang#11004: `__fwd_diff` applied to
// a non-static member method on a generic-constrained interface-typed
// receiver must thread the receiver as the implicit `this` argument
// and produce the analytic derivative.
//
// `Helix.eval(t) = 2*t*t   ⇒   eval'(t) = 4*t`, so:
//   tangent1<Helix>(h, 2.0)         = 8.0
//   tangent1<Helix>(h, 3.0)         = 12.0
//
// `Helix.eval2(t, u) = 3*t*t + 5*u*u`, so the directional derivative at
// `(t, u)` along `(t.d, u.d) = (1, 1)` is `6*t + 10*u`:
//   tangent2<Helix>(h, 2.0, 3.0)    = 12 + 30 = 42.0
//   tangent2<Helix>(h, 1.0, 1.0)    = 6 + 10 = 16.0
//
// `ScalarCurve.eval(t) = k*t*t`, so `eval'(t) = 2*k*t` (the receiver
// participates in the derivative): with `k = 3.0`,
//   tangentDiffThis<ScalarCurve>(s, 2.0) = 12.0
//   tangentDiffThis<ScalarCurve>(s, 1.0) =  6.0

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)

interface ICurve : IDifferentiable
{
    [Differentiable]
    float eval(float t);

    [Differentiable]
    float eval2(float t, float u);
}

struct Helix : ICurve, IDifferentiable
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t)
    {
        return 2.0 * t * t;
    }

    [Differentiable]
    [NoDiffThis]
    float eval2(float t, float u)
    {
        return 3.0 * t * t + 5.0 * u * u;
    }
}

// Differentiable receiver: the receiver participates in the derivative.
// Exercises the implicit-`this` slot without `[NoDiffThis]` so the slot
// is differentiable rather than `no_diff`-wrapped.
struct ScalarCurve : ICurve, IDifferentiable
{
    float k;

    [Differentiable]
    float eval(float t)
    {
        return k * t * t;
    }

    [Differentiable]
    float eval2(float t, float u)
    {
        return k * (t + u);
    }
}

[Differentiable]
float tangent1<C : ICurve>(C curve, float t)
{
    let pair = __fwd_diff(curve.eval)(diffPair(t, 1.0));
    return pair.d;
}

// Multi-arg variant: exercises the per-arg `getParamInfo(i + 1)` loop
// in `visitInvokeExprImpl` with trip count > 1, so an off-by-one in the
// implicit-`this` slot adjustment would mis-route directional
// derivative components rather than slip past untouched.
[Differentiable]
float tangent2<C : ICurve>(C curve, float t, float u)
{
    let pair = __fwd_diff(curve.eval2)(diffPair(t, 1.0), diffPair(u, 1.0));
    return pair.d;
}

[Differentiable]
float tangentDiffThis<C : ICurve>(C curve, float t)
{
    let pair = __fwd_diff(curve.eval)(diffPair(t, 1.0));
    return pair.d;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Helix h;

    // CHECK: 8.0
    outputBuffer[0] = tangent1<Helix>(h, 2.0);

    // CHECK: 12.0
    outputBuffer[1] = tangent1<Helix>(h, 3.0);

    // CHECK: 42.0
    outputBuffer[2] = tangent2<Helix>(h, 2.0, 3.0);

    // CHECK: 16.0
    outputBuffer[3] = tangent2<Helix>(h, 1.0, 1.0);

    ScalarCurve s;
    s.k = 3.0;

    // CHECK: 12.0
    outputBuffer[4] = tangentDiffThis<ScalarCurve>(s, 2.0);

    // CHECK: 6.0
    outputBuffer[5] = tangentDiffThis<ScalarCurve>(s, 1.0);
}
