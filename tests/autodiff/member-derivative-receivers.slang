// `__fwd_diff(obj.method)` and `__bwd_diff(obj.method)` for each kind of receiver: concrete,
// generic, existential, overloaded, differentiable and `[NoDiffThis]` `this`, `[mutating]`, nested
// derivatives, lambdas, a `DifferentialPair` receiver, and user-written member derivatives.
//
// `Cubic.eval(t) = t^3`, so `eval'(t) = 3t^2`.
// `Scaled.eval(t) = k t^2` with a differentiable `this`; the receiver gets a zero tangent.
// `Weighted.eval(t) = k t^2` too, but `[NoDiffThis]`, so the receiver is passed as-is.
// `Accumulator.add(x) = x^2` and adds `x` to `total`.
// `CustomFwd.eval` has the user-written forward derivative `100 t`, to tell it apart.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)

interface ICurve
{
    [Differentiable]
    float eval(float t);
}

interface IScaledCurve : IDifferentiable
{
    [Differentiable]
    float eval(float t);
}

interface IAccumulator
{
    [Differentiable]
    [mutating]
    float add(float x);
}

struct Cubic : ICurve
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t) { return t * t * t; }

    [Differentiable]
    [NoDiffThis]
    float eval(float t, float u) { return t * u; }

    float slopeAt(float t) { return __fwd_diff(this.eval)(diffPair(t, 1.0)).d; }

    float slopeInLambda(float t)
    {
        let slope = (float u) => __fwd_diff(this.eval)(diffPair(u, 1.0)).d;
        return slope(t);
    }
}

struct Scaled : IScaledCurve
{
    float k;

    [Differentiable]
    float eval(float t) { return k * t * t; }

    float gradInLambda(float t)
    {
        let grad = (float u) =>
        {
            var p = diffPair(u);
            __bwd_diff(this.eval)(p, 1.0);
            return p.d;
        };
        return grad(t);
    }
}

struct Weighted : IDifferentiable
{
    float k;

    [Differentiable]
    [NoDiffThis]
    float eval(float t) { return k * t * t; }
}

struct Accumulator : IAccumulator
{
    float total;

    [Differentiable]
    [mutating]
    [NoDiffThis]
    float add(float x)
    {
        total += detach(x);
        return x * x;
    }
}

struct CustomFwd
{
    [ForwardDerivative(evalFwd)]
    [NoDiffThis]
    float eval(float t) { return t * t * t; }

    DifferentialPair<float> evalFwd(DifferentialPair<float> t)
    {
        return diffPair(t.p * t.p * t.p, 100.0 * t.p * t.d);
    }
}

// A receiver whose own type is a `DifferentialPair`.
extension DifferentialPair<float>
{
    [Differentiable]
    float scaleSq(float t) { return this.p * t * t; }
}

// `[BackwardDerivativeOf(T.method)]` naming an overloaded method.
struct OverloadedCustom
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t) { return t * t * t; }

    [Differentiable]
    [NoDiffThis]
    float eval(float t, float u) { return t * u; }

    [BackwardDerivativeOf(OverloadedCustom.eval)]
    void evalBwd(inout DifferentialPair<float> t, float d) { t = diffPair(t.p, 100.0 * t.p * d); }
}

[Differentiable]
float callOverloadedCustom(no_diff OverloadedCustom c, float t) { return c.eval(t); }

ICurve pickCurve() { return Cubic(); }

float tangentConcrete(Cubic c, float t) { return __fwd_diff(c.eval)(diffPair(t, 1.0)).d; }

float gradConcrete(Cubic c, float t)
{
    var p = diffPair(t);
    __bwd_diff(c.eval)(p, 1.0);
    return p.d;
}

float tangentGeneric<C : ICurve>(C c, float t) { return __fwd_diff(c.eval)(diffPair(t, 1.0)).d; }

float gradGeneric<C : ICurve>(C c, float t)
{
    var p = diffPair(t);
    __bwd_diff(c.eval)(p, 1.0);
    return p.d;
}

float tangentExistential(ICurve c, float t) { return __fwd_diff(c.eval)(diffPair(t, 1.0)).d; }

float gradExistentialLocal(float t)
{
    ICurve c = pickCurve();
    var p = diffPair(t);
    __bwd_diff(c.eval)(p, 1.0);
    return p.d;
}

float tangentScaledGeneric<C : IScaledCurve>(C c, float t)
{
    return __fwd_diff(c.eval)(diffPair(t, 1.0)).d;
}

float gradScaledGeneric<C : IScaledCurve>(C c, float t)
{
    var p = diffPair(t);
    __bwd_diff(c.eval)(p, 1.0);
    return p.d;
}

float addGeneric<A : IAccumulator>(inout A a, float x)
{
    return __fwd_diff(a.add)(diffPair(x, 1.0)).d;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Cubic c;

    // CHECK: 12.0
    outputBuffer[0] = tangentConcrete(c, 2.0);
    // CHECK: 27.0
    outputBuffer[1] = gradConcrete(c, 3.0);
    // CHECK: 12.0
    outputBuffer[2] = tangentGeneric(c, 2.0);
    // CHECK: 27.0
    outputBuffer[3] = gradGeneric(c, 3.0);
    // CHECK: 12.0
    outputBuffer[4] = tangentExistential(c, 2.0);
    // CHECK: 12.0
    outputBuffer[5] = gradExistentialLocal(2.0);

    // Overloaded `eval`: `d(t u) = u dt + t du` at (3, 4) along (1, 1).
    // CHECK: 7.0
    outputBuffer[6] = __fwd_diff(c.eval)(diffPair(3.0, 1.0), diffPair(4.0, 1.0)).d;

    Scaled s;
    s.k = 5.0;
    // CHECK: 20.0
    outputBuffer[7] = __fwd_diff(s.eval)(diffPair(2.0, 1.0)).d;
    var sp = diffPair(1.0);
    __bwd_diff(s.eval)(sp, 1.0);
    // CHECK: 10.0
    outputBuffer[8] = sp.d;
    // CHECK: 20.0
    outputBuffer[9] = tangentScaledGeneric(s, 2.0);
    // CHECK: 20.0
    outputBuffer[10] = gradScaledGeneric(s, 2.0);

    // `[mutating]` receivers are written back.
    Accumulator a;
    a.total = 1.0;
    // CHECK: 6.0
    outputBuffer[11] = __fwd_diff(a.add)(diffPair(3.0, 1.0)).d;
    // CHECK: 4.0
    outputBuffer[12] = a.total;
    Accumulator b;
    b.total = 1.0;
    // CHECK: 6.0
    outputBuffer[13] = addGeneric(b, 3.0);
    // CHECK: 4.0
    outputBuffer[14] = b.total;

    // Second derivative, `6t`.
    let nested = __fwd_diff(__fwd_diff(c.eval))(diffPair(diffPair(3.0, 1.0), diffPair(1.0, 0.0)));
    // CHECK: 18.0
    outputBuffer[15] = nested.d.d;

    CustomFwd cf;
    // CHECK: 200.0
    outputBuffer[16] = __fwd_diff(cf.eval)(diffPair(2.0, 1.0)).d;
    // CHECK: 200.0
    outputBuffer[17] = __fwd_diff(CustomFwd.eval)(cf, diffPair(2.0, 1.0)).d;

    // CHECK: 12.0
    outputBuffer[18] = c.slopeAt(2.0);
    // CHECK: 27.0
    outputBuffer[19] = c.slopeInLambda(3.0);

    Weighted w;
    w.k = 5.0;
    // CHECK: 20.0
    outputBuffer[20] = __fwd_diff(w.eval)(diffPair(2.0, 1.0)).d;
    var wp = diffPair(2.0);
    __bwd_diff(w.eval)(wp, 1.0);
    // CHECK: 20.0
    outputBuffer[21] = wp.d;
    // CHECK: 20.0
    outputBuffer[22] = __fwd_diff(Weighted.eval)(w, diffPair(2.0, 1.0)).d;

    // Backward derivatives in lambdas, through a captured receiver, a lambda parameter, and
    // `this`.
    let gradCaptured = (float t) =>
    {
        var p = diffPair(t);
        __bwd_diff(s.eval)(p, 1.0);
        return p.d;
    };
    // CHECK: 20.0
    outputBuffer[23] = gradCaptured(2.0);
    let gradOf = (Scaled r, float t) =>
    {
        var p = diffPair(t);
        __bwd_diff(r.eval)(p, 1.0);
        return p.d;
    };
    // CHECK: 30.0
    outputBuffer[24] = gradOf(s, 3.0);
    // CHECK: 20.0
    outputBuffer[25] = s.gradInLambda(2.0);

    // `d/dt (p t^2) = 2 p t` at `p = 3`, `t = 2`.
    DifferentialPair<float> dp = diffPair(3.0, 0.0);
    // CHECK: 12.0
    outputBuffer[26] = __fwd_diff(dp.scaleSq)(diffPair(2.0, 1.0)).d;

    OverloadedCustom oc;
    var ocp = diffPair(3.0);
    __bwd_diff(callOverloadedCustom)(oc, ocp, 2.0);
    // CHECK: 600.0
    outputBuffer[27] = ocp.d;

    // A receiver whose type carries a modifier.
    unorm CustomFwd unormCustom = cf;
    // CHECK: 200.0
    outputBuffer[28] = __fwd_diff(unormCustom.eval)(diffPair(2.0, 1.0)).d;

    // Backward derivatives of a `[mutating]` method, through an instance and the static form.
    Accumulator ba;
    ba.total = 1.0;
    var bp = diffPair(3.0);
    __bwd_diff(ba.add)(bp, 1.0);
    // CHECK: 6.0
    outputBuffer[29] = bp.d;
    var sp2 = diffPair(4.0);
    __bwd_diff(Accumulator.add)(ba, sp2, 1.0);
    // CHECK: 8.0
    outputBuffer[30] = sp2.d;
}
