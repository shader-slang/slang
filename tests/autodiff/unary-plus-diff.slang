// Unary `+` is the identity, so differentiating through it must give the same result as
// differentiating the operand directly, and must mirror unary `-` up to a sign. `+` (the operator
// under test) is exercised on both a user-defined `IFloat` type and builtin scalars. The `-`
// mirror uses a builtin scalar so the test does not also depend on differentiating unary `-`
// through a user-defined `IFloat` type (`IFloat.neg()`); that path currently produces a wrong
// result on aarch64, tracked separately in shader-slang/slang#12871.

//TEST:INTERPRET(filecheck=CHECK):

struct MyFloat : IFloat, IDifferentiable
{
    float v;

    [Differentiable] __init(float value) { v = value; }
    [Differentiable] __init(MyFloat other) { v = other.v; }
    [Differentiable] float toFloat() { return v; }
    [Differentiable] MyFloat add(MyFloat o) { return MyFloat(v + o.v); }
    [Differentiable] MyFloat sub(MyFloat o) { return MyFloat(v - o.v); }
    [Differentiable] MyFloat mul(MyFloat o) { return MyFloat(v * o.v); }
    [Differentiable] MyFloat div(MyFloat o) { return MyFloat(v / o.v); }
    [Differentiable] MyFloat mod(MyFloat o) { return MyFloat(v % o.v); }
    [Differentiable] MyFloat neg() { return MyFloat(-v); }
    [Differentiable] MyFloat scale<T : __BuiltinFloatingPointType>(T s) { return MyFloat(v * s.toFloat()); }
    bool equals(MyFloat o) { return v == o.v; }
    bool lessThan(MyFloat o) { return v < o.v; }
    bool lessThanOrEquals(MyFloat o) { return v <= o.v; }
}

// f(x) = +(x*x), so f(3) = 9 and f'(3) = 6.
[Differentiable]
MyFloat squarePlus(MyFloat x) { return +(x * x); }

[Differentiable]
float squarePlusF(float x) { return +(x * x); }

[Differentiable]
float squareNegF(float x) { return -(x * x); }

[Differentiable]
float cubePlusF(float x) { return +(x * x * x); }

// The second derivative of +(x*x*x) is 6x, which is 12 at x = 2.
[Differentiable]
float firstDerivF(float x) { return fwd_diff(cubePlusF)(diffPair(x, 1.0)).d; }

void main()
{
    let fwdUser = fwd_diff(squarePlus)(diffPair(MyFloat(3.0), MyFloat(1.0)));
    let fwdNeg = fwd_diff(squareNegF)(diffPair(3.0, 1.0));
    let fwdBuiltin = fwd_diff(squarePlusF)(diffPair(3.0, 1.0));
    //CHECK: 9 6 -9 -6 9 6
    printf(
        "%d %d %d %d %d %d\n",
        (int)fwdUser.p.v,
        (int)fwdUser.d.v,
        (int)fwdNeg.p,
        (int)fwdNeg.d,
        (int)fwdBuiltin.p,
        (int)fwdBuiltin.d);

    var revUser = diffPair(MyFloat(3.0), MyFloat(0.0));
    bwd_diff(squarePlus)(revUser, MyFloat(1.0));
    var revNeg = diffPair(3.0, 0.0);
    bwd_diff(squareNegF)(revNeg, 1.0);
    var revBuiltin = diffPair(3.0, 0.0);
    bwd_diff(squarePlusF)(revBuiltin, 1.0);
    //CHECK: 6 -6 6
    printf("%d %d %d\n", (int)revUser.d.v, (int)revNeg.d, (int)revBuiltin.d);

    var second = diffPair(2.0, 0.0);
    bwd_diff(firstDerivF)(second, 1.0);
    //CHECK: 12 12
    printf("%d %d\n", (int)fwd_diff(firstDerivF)(diffPair(2.0, 1.0)).d, (int)second.d);
}
