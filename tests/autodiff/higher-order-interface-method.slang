// Higher-order derivatives of functions that call interface methods through a generic, for
// instance and static requirements, user-written derivatives of a user-written forward
// derivative, and conformances that must stay non-differentiable through an interface.
//
// `Cubic.eval(t) = t^3`: `eval'(t) = 3t^2`, `eval''(t) = 6t`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)

interface ICurve
{
    [Differentiable]
    float eval(float t);
}

interface IStaticCurve
{
    [Differentiable]
    static float eval(float t);
}

interface ISurface
{
    [Differentiable]
    float eval(float t, no_diff float s, float u);
}

interface IOutCurve
{
    [Differentiable]
    void eval(float t, out float y, inout float z);
}

interface IMaybe
{
    [MaybeDifferentiable]
    float compute(float x);
}

interface IMaybeMutate
{
    [MaybeDifferentiable]
    void mutate(inout float x);
}

interface IMaybeProduce
{
    [MaybeDifferentiable]
    void produce(float x, out float y);
}

interface IMaybeCount
{
    [MaybeDifferentiable]
    int count(float x);
}

interface IMaybeGeneric
{
    [MaybeDifferentiable]
    float compute<T : IFloat>(T x, float y);
}

struct Cubic : ICurve
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t) { return t * t * t; }
}

struct StaticCubic : IStaticCurve
{
    [Differentiable]
    static float eval(float t) { return t * t * t; }
}

// `eval(t, s, u) = s t^3 + u^2`.
struct Surface : ISurface
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t, no_diff float s, float u) { return s * t * t * t + u * u; }
}

// `t^4`, whose third derivative is `24 t`.
struct Quartic : ICurve
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t) { return t * t * t * t; }
}

// `y = t^3` and `z = z t^2`.
struct OutCubic : IOutCurve
{
    [Differentiable]
    [NoDiffThis]
    void eval(float t, out float y, inout float z)
    {
        y = t * t * t;
        z = z * t * t;
    }
}

struct NonDiff : IMaybe
{
    [NoDiffThis]
    float compute(float x) { return x * x; }
}

struct NonDiffMutate : IMaybeMutate
{
    [NoDiffThis]
    void mutate(inout float x) { x = 5.0; }
}

struct NonDiffProduce : IMaybeProduce
{
    [NoDiffThis]
    void produce(float x, out float y) { y = x * x; }
}

struct NonDiffCount : IMaybeCount
{
    [NoDiffThis]
    int count(float x) { return int(x); }
}

struct NonDiffGeneric : IMaybeGeneric
{
    float compute<T : IFloat>(T x, float y) { return y * y; }
}

// The user-written forward derivative has its own user-written backward derivative, which
// scales by 100 so that it is told apart from differentiating `evalFwd`'s body.
struct CustomCubic : ICurve
{
    [ForwardDerivative(evalFwd)]
    [BackwardDerivative(evalBwd)]
    [NoDiffThis]
    float eval(float t) { return t * t * t; }

    void evalBwd(inout DifferentialPair<float> t, float d) { t = diffPair(t.p, 3.0 * t.p * t.p * d); }

    [BackwardDerivative(evalFwdBwd)]
    DifferentialPair<float> evalFwd(DifferentialPair<float> t)
    {
        return diffPair(t.p * t.p * t.p, 3.0 * t.p * t.p * t.d);
    }

    void evalFwdBwd(inout DifferentialPair<DifferentialPair<float>> t, DifferentialPair<float> d)
    {
        t = diffPair(t.p, diffPair(100.0 * d.d, 0.0));
    }
}

[Differentiable]
float evalCurve<C : ICurve>(no_diff C c, float t) { return c.eval(t); }

[Differentiable]
float evalStatic<C : IStaticCurve>(float t) { return C.eval(t); }

[Differentiable]
float evalSurface<C : ISurface>(no_diff C c, float t, no_diff float s, float u)
{
    return c.eval(t, s, u);
}

[Differentiable]
float slopeGeneric<C : ICurve>(no_diff C c, float t)
{
    return __fwd_diff(evalCurve<C>)(c, diffPair(t, 1.0)).d;
}

[Differentiable]
float slopeStatic<C : IStaticCurve>(float t) { return __fwd_diff(evalStatic<C>)(diffPair(t, 1.0)).d; }

// `d/dt (s t^3 + u^2) = 3 s t^2` with `s = 2.5`, so its derivative is `15 t`.
[Differentiable]
float slopeSurface<C : ISurface>(no_diff C c, float t)
{
    return __fwd_diff(evalSurface<C>)(c, diffPair(t, 1.0), 2.5, diffPair(1.0, 0.0)).d;
}

// The inner `fwd_diff` computes `eval'(detach(x))`; the outer `bwd_diff` differentiates
// `x * eval'(detach(x))`. The detached argument leaves only the multiplication's derivative,
// `eval'(x) = 3x^2`, which is 27 at x = 3.
[Differentiable]
float detachedGeneric<C : ICurve>(no_diff C c, float x)
{
    return x * __fwd_diff(evalCurve<C>)(c, diffPair(detach(x), 1.0)).d;
}

// Only `+ x` is differentiable, so the derivative is 1.
[Differentiable]
float useMaybe<T : IMaybe>(no_diff T t, float x) { return t.compute(x) + x; }

// `y` is overwritten with a constant, so the derivative of `y x` is `y = 5`.
[Differentiable]
float useMaybeMutate<T : IMaybeMutate>(no_diff T t, float x)
{
    var y = x;
    t.mutate(y);
    return y * x;
}

// Only the final multiplication carries a tangent, so its derivative is y = x^2.
[Differentiable]
float useMaybeProduce<T : IMaybeProduce>(no_diff T t, float x)
{
    float y;
    t.produce(x, y);
    return y * x;
}

// The result is not differentiable, so only `+ x` is.
[Differentiable]
float useMaybeCount<T : IMaybeCount>(no_diff T t, float x) { return float(t.count(x)) + x; }

[Differentiable]
float useMaybeGeneric<T : IMaybeGeneric>(no_diff T t, float y) { return t.compute<float>(1.0, y) + y; }

// `y + z = 2 t^3` with `z` starting at `t`, so its second derivative is `12 t`.
[Differentiable]
float evalOut<C : IOutCurve>(no_diff C c, float t)
{
    float y;
    float z = t;
    c.eval(t, y, z);
    return y + z;
}

[Differentiable]
float slopeOut<C : IOutCurve>(no_diff C c, float t)
{
    return __fwd_diff(evalOut<C>)(c, diffPair(t, 1.0)).d;
}

[Differentiable]
float curvatureGeneric<C : ICurve>(no_diff C c, float t)
{
    return __fwd_diff(__fwd_diff(evalCurve<C>))(c, diffPair(diffPair(t, 1.0), diffPair(1.0, 0.0))).d.d;
}

float secondGeneric<C : ICurve>(C c, float t)
{
    return __fwd_diff(__fwd_diff(evalCurve<C>))(c, diffPair(diffPair(t, 1.0), diffPair(1.0, 0.0))).d.d;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Cubic c;

    var a = diffPair(3.0);
    __bwd_diff(slopeGeneric<Cubic>)(c, a, 1.0);
    // CHECK: {{^}}18.000000{{$}}
    outputBuffer[0] = a.d;

    // CHECK-NEXT: {{^}}30.000000{{$}}
    outputBuffer[1] = __fwd_diff(slopeGeneric<Cubic>)(c, diffPair(5.0, 1.0)).d;

    var d = diffPair(3.0);
    __bwd_diff(slopeStatic<StaticCubic>)(d, 1.0);
    // CHECK-NEXT: {{^}}18.000000{{$}}
    outputBuffer[2] = d.d;

    Surface surface;
    var e = diffPair(2.0);
    __bwd_diff(slopeSurface<Surface>)(surface, e, 1.0);
    // CHECK-NEXT: {{^}}30.000000{{$}}
    outputBuffer[3] = e.d;

    var f = diffPair(3.0);
    __bwd_diff(detachedGeneric<Cubic>)(c, f, 1.0);
    // CHECK-NEXT: {{^}}27.000000{{$}}
    outputBuffer[4] = f.d;

    NonDiff n;
    // CHECK-NEXT: {{^}}1.000000{{$}}
    outputBuffer[5] = __fwd_diff(useMaybe<NonDiff>)(n, diffPair(3.0, 1.0)).d;
    NonDiffGeneric ng;
    // CHECK-NEXT: {{^}}1.000000{{$}}
    outputBuffer[6] = __fwd_diff(useMaybeGeneric<NonDiffGeneric>)(ng, diffPair(2.0, 1.0)).d;
    var g = diffPair(3.0);
    __bwd_diff(useMaybe<NonDiff>)(n, g, 1.0);
    // CHECK-NEXT: {{^}}1.000000{{$}}
    outputBuffer[7] = g.d;

    NonDiffMutate nm;
    // CHECK-NEXT: {{^}}5.000000{{$}}
    outputBuffer[8] = __fwd_diff(useMaybeMutate<NonDiffMutate>)(nm, diffPair(3.0, 1.0)).d;
    var h = diffPair(3.0);
    __bwd_diff(useMaybeMutate<NonDiffMutate>)(nm, h, 1.0);
    // CHECK-NEXT: {{^}}5.000000{{$}}
    outputBuffer[9] = h.d;

    NonDiffCount nc;
    // CHECK-NEXT: {{^}}1.000000{{$}}
    outputBuffer[10] = __fwd_diff(useMaybeCount<NonDiffCount>)(nc, diffPair(3.0, 1.0)).d;
    var k = diffPair(3.0);
    __bwd_diff(useMaybeCount<NonDiffCount>)(nc, k, 1.0);
    // CHECK-NEXT: {{^}}1.000000{{$}}
    outputBuffer[11] = k.d;

    CustomCubic cc;
    var u = diffPair(3.0);
    __bwd_diff(slopeGeneric<CustomCubic>)(cc, u, 1.0);
    // CHECK-NEXT: {{^}}100.000000{{$}}
    outputBuffer[12] = u.d;

    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[13] = secondGeneric(c, 2.0);

    OutCubic oc;
    // CHECK-NEXT: {{^}}24.000000{{$}}
    outputBuffer[14] =
        __fwd_diff(__fwd_diff(evalOut<OutCubic>))(oc, diffPair(diffPair(2.0, 1.0), diffPair(1.0, 0.0))).d.d;
    var w = diffPair(2.0);
    __bwd_diff(slopeOut<OutCubic>)(oc, w, 1.0);
    // CHECK-NEXT: {{^}}24.000000{{$}}
    outputBuffer[15] = w.d;

    Quartic q;
    // CHECK-NEXT: {{^}}48.000000{{$}}
    outputBuffer[16] = __fwd_diff(curvatureGeneric<Quartic>)(q, diffPair(2.0, 1.0)).d;

    NonDiffProduce np;
    // CHECK-NEXT: {{^}}9.000000{{$}}
    outputBuffer[17] = __fwd_diff(useMaybeProduce<NonDiffProduce>)(np, diffPair(3.0, 1.0)).d;
    var produced = diffPair(3.0);
    __bwd_diff(useMaybeProduce<NonDiffProduce>)(np, produced, 1.0);
    // CHECK-NEXT: {{^}}9.000000{{$}}
    outputBuffer[18] = produced.d;

    var generic = diffPair(2.0);
    __bwd_diff(useMaybeGeneric<NonDiffGeneric>)(ng, generic, 1.0);
    // CHECK-NEXT: {{^}}1.000000{{$}}
    outputBuffer[19] = generic.d;
}
