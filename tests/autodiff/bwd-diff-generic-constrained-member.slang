// Companion to `fwd-diff-generic-constrained-member.slang` for
// shader-slang/slang#11004: `__bwd_diff` applied to a non-static member
// method on a generic-constrained interface-typed receiver must thread
// the receiver as the implicit `this` argument and back-propagate the
// gradient correctly.
//
// `Helix.eval(t) = 2*t*t  ⇒  ∂eval/∂t = 4*t`, so for
// `__bwd_diff(curve.eval)(pair, dResult)` with `pair = (t, 0)`:
//   pair.d after call = 4*t * dResult
//
// `Helix.eval2(t, u) = 3*t*t + 5*u*u` exercises the per-arg loop with
// trip count > 1: ∂/∂t = 6*t, ∂/∂u = 10*u.
//
// `ScalarCurve.eval(t) = k*t*t` (no `[NoDiffThis]`) exercises the
// differentiable-receiver lowering symmetrically with the fwd peer:
//   ∂eval/∂t = 2*k*t, so with k=3 and dResult=1.0:
//     gradDiffThis<ScalarCurve>(s, 2.0) = 12.0
//     gradDiffThis<ScalarCurve>(s, 1.0) =  6.0

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
float gradGeneric<C : ICurve>(C curve, float t, float dResult)
{
    var pair = diffPair(t, 0.0);
    __bwd_diff(curve.eval)(pair, dResult);
    return pair.d;
}

// Multi-arg variant: same per-arg `getParamInfo(i + 1)` loop as the fwd
// peer, exercised on the bwd lookup. Returns ∂/∂t.
[Differentiable]
float gradMulti<C : ICurve>(C curve, float t, float u, float dResult)
{
    var pt = diffPair(t, 0.0);
    var pu = diffPair(u, 0.0);
    __bwd_diff(curve.eval2)(pt, pu, dResult);
    return pt.d;
}

[Differentiable]
float gradDiffThis<C : ICurve>(C curve, float t, float dResult)
{
    var pair = diffPair(t, 0.0);
    __bwd_diff(curve.eval)(pair, dResult);
    return pair.d;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Helix h;

    // CHECK: 8.0
    outputBuffer[0] = gradGeneric<Helix>(h, 2.0, 1.0);

    // CHECK: 12.0
    outputBuffer[1] = gradGeneric<Helix>(h, 3.0, 1.0);

    // ∂eval2/∂t at (t=2, u=3) = 6*2 = 12.0
    // CHECK: 12.0
    outputBuffer[2] = gradMulti<Helix>(h, 2.0, 3.0, 1.0);

    // ∂eval2/∂t at (t=1, u=1) = 6*1 = 6.0
    // CHECK: 6.0
    outputBuffer[3] = gradMulti<Helix>(h, 1.0, 1.0, 1.0);

    ScalarCurve s;
    s.k = 3.0;

    // ∂(k*t*t)/∂t at t=2, k=3, dResult=1 = 2*3*2 = 12.0
    // CHECK: 12.0
    outputBuffer[4] = gradDiffThis<ScalarCurve>(s, 2.0, 1.0);

    // ∂(k*t*t)/∂t at t=1, k=3, dResult=1 = 2*3*1 = 6.0
    // CHECK: 6.0
    outputBuffer[5] = gradDiffThis<ScalarCurve>(s, 1.0, 1.0);
}
