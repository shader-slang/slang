// Companion to `fwd-diff-generic-constrained-member.slang`: `__bwd_diff`
// applied to a non-static member
// method on a generic-constrained interface-typed receiver must thread
// the receiver as the implicit `this` argument and back-propagate the
// gradient correctly.
//
// `Helix.eval(t) = 2*t*t  ⇒  ∂eval/∂t = 4*t`, so for
// `__bwd_diff(curve.eval)(pair, dResult)` with `pair = (t, 0)`:
//   pair.d after call = 4*t * dResult
//
// `Helix.eval2(t, u) = 3*t*t + 5*u*u` takes several arguments after the
// receiver: ∂/∂t = 6*t, ∂/∂u = 10*u.
//
// `ScalarCurve.eval(t) = k*t*t` (no `[NoDiffThis]`) has a differentiable
// `this`, as in the fwd peer; its gradient is not returned to the caller:
//   ∂eval/∂t = 2*k*t, so with k=3 and dResult=1.0:
//     gradDiffThis<ScalarCurve>(s, 2.0) = 12.0
//     gradDiffThis<ScalarCurve>(s, 1.0) =  6.0

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4)

interface ICurve : IDifferentiable
{
    [Differentiable]
    float eval(float t);

    [Differentiable]
    float eval2(float t, float u);

    // `inout` parameter: an `inout DifferentialPair<T>` argument after the
    // receiver, not just the by-value shape of the other variants.
    [Differentiable]
    void accumulate(inout float acc, float t);
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

    [Differentiable]
    [NoDiffThis]
    void accumulate(inout float acc, float t)
    {
        acc = acc + 7.0 * t * t;
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

    [Differentiable]
    void accumulate(inout float acc, float t)
    {
        acc = acc + k * t * t;
    }
}

float gradGeneric<C : ICurve>(C curve, float t, float dResult)
{
    var pair = diffPair(t, 0.0);
    __bwd_diff(curve.eval)(pair, dResult);
    return pair.d;
}

// Multi-arg variant, as in the fwd peer. Returns ∂/∂t.
float gradMulti<C : ICurve>(C curve, float t, float u, float dResult)
{
    var pt = diffPair(t, 0.0);
    var pu = diffPair(u, 0.0);
    __bwd_diff(curve.eval2)(pt, pu, dResult);
    return pt.d;
}

float gradDiffThis<C : ICurve>(C curve, float t, float dResult)
{
    var pair = diffPair(t, 0.0);
    __bwd_diff(curve.eval)(pair, dResult);
    return pair.d;
}

// Multi-arg + differentiable-`this` intersection: same idea as the fwd
// peer's `tangent2DiffThis`, mirrored to exercise the bwd lookup
// through both axes simultaneously.
float gradMulti2DiffThis<C : ICurve>(C curve, float t, float u, float dResult)
{
    var pt = diffPair(t, 0.0);
    var pu = diffPair(u, 0.0);
    __bwd_diff(curve.eval2)(pt, pu, dResult);
    return pt.d;
}

// `inout DifferentialPair<T>` parameter shape after the receiver.
// `accumulate(inout acc, t)` adds `7*t*t` (Helix) or `k*t*t`
// (ScalarCurve). The bwd-mode signature wraps both `acc` and `t` as
// `inout DifferentialPair<float>`. With `dResult = 1.0` flowing
// backward through `acc`, the gradient w.r.t. `t` is the `acc`-mode
// derivative `2 * <coeff> * t`.
float gradAccumulate<C : ICurve>(C curve, float t, float dAcc)
{
    var pAcc = diffPair(0.0, dAcc);
    var pT = diffPair(t, 0.0);
    __bwd_diff(curve.accumulate)(pAcc, pT);
    return pT.d;
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

    // Multi-arg + differentiable-`this`: ∂(k*(t+u))/∂t = k = 3.0
    // CHECK: 3.0
    outputBuffer[6] = gradMulti2DiffThis<ScalarCurve>(s, 1.0, 2.0, 1.0);

    // `inout DifferentialPair<T>` shape: ∂(7*t*t)/∂t with dAcc=1
    // gives 2*7*t = 14*t. At t=2: 28.0.
    // CHECK: 28.0
    outputBuffer[7] = gradAccumulate<Helix>(h, 2.0, 1.0);

    // ScalarCurve.accumulate adds k*t*t: ∂/∂t with dAcc=1 is 2*k*t.
    // At t=2, k=3: 12.0.
    // CHECK: 12.0
    outputBuffer[8] = gradAccumulate<ScalarCurve>(s, 2.0, 1.0);
}
