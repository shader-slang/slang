// Second derivatives through a generic wrapper of interface requirements of different kinds: a
// method with a differentiable `this`, a `[mutating]` method, a method of a generic struct or of
// an extension, subscript accessors, initializers and a variadic generic method.
//
// Each cubic gives `d2/dt2 (k t^3) = 6 k t`, evaluated at `t = 2`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)

// `a t^3`, differentiable in both `a` and `t`.
interface IScaledCurve : IDifferentiable
{
    [Differentiable]
    float eval(float t);
}

struct ScaledCubic : IScaledCurve
{
    float a;

    [Differentiable]
    float eval(float t) { return a * t * t * t; }
}

interface IMutatingCurve
{
    [Differentiable]
    [mutating]
    float eval(float t);
}

interface IMutatingScaledCurve : IDifferentiable
{
    [Differentiable]
    [mutating]
    float eval(float t);
}

struct MutatingScaledCubic : IMutatingScaledCurve
{
    float a;

    [Differentiable]
    [mutating]
    float eval(float t)
    {
        a *= t;
        return a * t * t;
    }
}

struct CountingCubic : IMutatingCurve
{
    int count;

    [Differentiable]
    [NoDiffThis]
    [mutating]
    float eval(float t)
    {
        count++;
        return t * t * t;
    }
}

interface ICurve
{
    [Differentiable]
    float eval(float t);
}

struct Poly<let K : int> : ICurve
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t) { return float(K) * t * t * t; }
}

struct ExtCubic
{
}

extension ExtCubic : ICurve
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t) { return t * t * t; }
}

interface ISubscriptCurve
{
    __subscript(float t)->float
    {
        [Differentiable]
        get;
    }
}

struct SubscriptCubic : ISubscriptCurve
{
    __subscript(float t)->float
    {
        [Differentiable]
        [NoDiffThis]
        get { return t * t * t; }
    }
}

interface IGenericSubscriptCurve
{
    __subscript<T : IFloat>(T t)->float
    {
        [Differentiable]
        get;
    }
}

struct GenericSubscriptCubic : IGenericSubscriptCurve
{
    __subscript<T : IFloat>(T t)->float
    {
        [Differentiable]
        [NoDiffThis]
        get
        {
            float x = t.toFloat();
            return x * x * x;
        }
    }
}

// The setter stores the cube of the value written.
interface IBox : IDifferentiable
{
    __subscript(int i)->float
    {
        [Differentiable]
        get;
        [Differentiable]
        set;
    }
}

struct Box : IBox
{
    float v;

    __subscript(int i)->float
    {
        [Differentiable]
        get { return v; }
        [Differentiable]
        set { v = newValue * newValue * newValue; }
    }
}

interface IMadeCurve : IDifferentiable
{
    [Differentiable]
    __init(float t);

    [Differentiable]
    float get();
}

struct MadeCubic : IMadeCurve
{
    float value;

    [Differentiable]
    __init(float t) { value = t * t * t; }

    [Differentiable]
    float get() { return value; }
}

struct MadePoly<let K : int> : IMadeCurve
{
    float value;

    [Differentiable]
    __init(float t) { value = float(K) * t * t * t; }

    [Differentiable]
    float get() { return value; }
}

struct MadeGeneric<T : IFloat> : IMadeCurve
{
    T value;

    [Differentiable]
    __init(float t) { value = T(t * t * t); }

    [Differentiable]
    float get() { return value.toFloat(); }
}

// `a^2` plus the squares of the pack elements.
interface IVariadic
{
    [Differentiable]
    float sumSquares<each T>(float a, expand each T v)
        where T == float;
}

struct Variadic : IVariadic
{
    [Differentiable]
    [NoDiffThis]
    float sumSquares<each T>(float a, expand each T v)
        where T == float
    {
        float s = a * a;
        expand s = s + (each v) * (each v);
        return s;
    }
}

[Differentiable]
float evalScaled<C : IScaledCurve>(C c, float t) { return c.eval(t); }

// `3 a t^2`: `d/da = 3 t^2 = 12`, `d/dt = 6 a t = 24` at `a = t = 2`.
[Differentiable]
float slopeScaled<C : IScaledCurve>(C c, float t)
{
    return __fwd_diff(evalScaled<C>)(diffPair(c, C.dzero()), diffPair(t, 1.0)).d;
}

[Differentiable]
float evalMutating<C : IMutatingCurve>(no_diff C c, float t) { return c.eval(t); }

[Differentiable]
float evalMutatingScaled<C : IMutatingScaledCurve>(C c, float t) { return c.eval(t); }

[Differentiable]
float slopeMutatingScaled<C : IMutatingScaledCurve>(C c, float t)
{
    return __fwd_diff(evalMutatingScaled<C>)(diffPair(c, C.dzero()), diffPair(t, 1.0)).d;
}

[Differentiable]
float evalCurve<C : ICurve>(no_diff C c, float t) { return c.eval(t); }

[Differentiable]
float evalSubscript<C : ISubscriptCurve>(no_diff C c, float t) { return c[t]; }

[Differentiable]
float slopeSubscript<C : ISubscriptCurve>(no_diff C c, float t)
{
    return __fwd_diff(evalSubscript<C>)(c, diffPair(t, 1.0)).d;
}

[Differentiable]
float evalGenericSubscript<C : IGenericSubscriptCurve>(no_diff C c, float t) { return c[t]; }

[Differentiable]
float roundtrip<B : IBox>(B b, float t)
{
    b[0] = t;
    return b[0];
}

[Differentiable]
float slopeRoundtrip<B : IBox>(B b, float t)
{
    return __fwd_diff(roundtrip<B>)(diffPair(b, B.dzero()), diffPair(t, 1.0)).d;
}

[Differentiable]
float evalVariadic<C : IVariadic>(no_diff C c, float x) { return c.sumSquares(x); }

[Differentiable]
float make<C : IMadeCurve>(float t)
{
    C c = C(t);
    return c.get();
}

[Differentiable]
float slopeMake<C : IMadeCurve>(float t) { return __fwd_diff(make<C>)(diffPair(t, 1.0)).d; }

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let t = diffPair(diffPair(2.0, 1.0), diffPair(1.0, 0.0));

    ScaledCubic scaled;
    scaled.a = 2.0;
    var c = diffPair(scaled);
    var s = diffPair(2.0);
    __bwd_diff(slopeScaled<ScaledCubic>)(c, s, 1.0);
    // CHECK: {{^}}12.000000{{$}}
    outputBuffer[0] = c.d.a;
    // CHECK-NEXT: {{^}}24.000000{{$}}
    outputBuffer[1] = s.d;

    CountingCubic counting;
    counting.count = 0;
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[2] = __fwd_diff(__fwd_diff(evalMutating<CountingCubic>))(counting, t).d.d;

    Poly<2> poly;
    // CHECK-NEXT: {{^}}24.000000{{$}}
    outputBuffer[3] = __fwd_diff(__fwd_diff(evalCurve<Poly<2>>))(poly, t).d.d;

    ExtCubic ext;
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[4] = __fwd_diff(__fwd_diff(evalCurve<ExtCubic>))(ext, t).d.d;

    SubscriptCubic subscript;
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[5] = __fwd_diff(__fwd_diff(evalSubscript<SubscriptCubic>))(subscript, t).d.d;
    var u = diffPair(2.0);
    __bwd_diff(slopeSubscript<SubscriptCubic>)(subscript, u, 1.0);
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[6] = u.d;

    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[7] = __fwd_diff(__fwd_diff(make<MadeCubic>))(t).d.d;
    var v = diffPair(2.0);
    __bwd_diff(slopeMake<MadeCubic>)(v, 1.0);
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[8] = v.d;

    GenericSubscriptCubic genericSubscript;
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[9] =
        __fwd_diff(__fwd_diff(evalGenericSubscript<GenericSubscriptCubic>))(genericSubscript, t).d.d;

    Box box;
    box.v = 0.0;
    let z = Box.dzero();
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[10] =
        __fwd_diff(__fwd_diff(roundtrip<Box>))(diffPair(diffPair(box, z), diffPair(z, z)), t).d.d;
    var b = diffPair(box);
    var r = diffPair(2.0);
    __bwd_diff(slopeRoundtrip<Box>)(b, r, 1.0);
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[11] = r.d;

    // CHECK-NEXT: {{^}}24.000000{{$}}
    outputBuffer[12] = __fwd_diff(__fwd_diff(make<MadePoly<2>>))(t).d.d;
    var g = diffPair(2.0);
    __bwd_diff(slopeMake<MadeGeneric<float>>)(g, 1.0);
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[13] = g.d;

    Variadic variadic;
    // CHECK-NEXT: {{^}}2.000000{{$}}
    outputBuffer[14] = __fwd_diff(__fwd_diff(evalVariadic<Variadic>))(variadic, t).d.d;

    MutatingScaledCubic mutating;
    mutating.a = 2.0;
    let mz = MutatingScaledCubic.dzero();
    // CHECK-NEXT: {{^}}24.000000{{$}}
    outputBuffer[15] = __fwd_diff(__fwd_diff(evalMutatingScaled<MutatingScaledCubic>))(
        diffPair(diffPair(mutating, mz), diffPair(mz, mz)), t).d.d;
    var mc = diffPair(mutating);
    var mt = diffPair(2.0);
    __bwd_diff(slopeMutatingScaled<MutatingScaledCubic>)(mc, mt, 1.0);
    // CHECK-NEXT: {{^}}24.000000{{$}}
    outputBuffer[16] = mt.d;
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[17] = mc.d.a;
}
